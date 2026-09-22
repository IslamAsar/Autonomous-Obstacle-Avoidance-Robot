#include "SystemTasks.h"
#include "RemoteXYConfig.h"

// ============================================================
//                    Global state and RTOS objects
// ============================================================
RTC_DATA_ATTR int bootCount = 0;
Preferences preferences;
PowerManager powerManager;
WebDashboard dashboard;

RobotTelemetry sharedTelemetry;
SemaphoreHandle_t telemetryMutex;

SystemState currentSystemState = STATE_AUTO;
int obstacleThreshold = 20;

unsigned long lastManualActivity = 0;
const unsigned long MANUAL_STANDBY_TIMEOUT = 3000;
int8_t previousJoystickX = 0;
int8_t previousJoystickY = 0;
bool emergencyMessagePrinted = false;
static bool isAppStandbyActive = false;

void stopAllActuators() {
    motor.stop();
    radarServo.deactivate();
    buzzer.off();
}

void enterEmergency() {
    powerManager.setMode(POWER_EMERGENCY);
    stopAllActuators();
    buzzer.alarm(1000);

    dashboard.addEvent("EMERGENCY: Flame Detected! Motors Locked.");

    if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        sharedTelemetry.flameEmergency = true;
        sharedTelemetry.totalFlameEvents++;
        preferences.putUInt("flame_cnt", sharedTelemetry.totalFlameEvents);
        xSemaphoreGive(telemetryMutex);
    }

    if (!emergencyMessagePrinted) {
        Serial.println(F("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
        Serial.println(F("[EMERGENCY] FLAME DETECTED! MOTORS LOCKED."));
        Serial.println(F("Press Physical Button OR Mobile App Reset."));
        Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
        emergencyMessagePrinted = true;
    }
}

void handleEmergency() {
    motor.stop();

    if (flame_sensor.detected()) {
        buzzer.alarm(1000);
        return;
    }

    if (resetButton.isPressed() || RemoteXY.button_01 == 1) {
        Serial.println(F("\n[EMERGENCY] Reset Confirmed!"));
        buzzer.off();
        flame_sensor.clearTrigger();
        emergencyMessagePrinted = false;

        dashboard.addEvent("Emergency Cleared - System Restored");

        while (resetButton.isPressed()) {
            delay(10);
        }

        radarServo.activate();
        radarServo.lookStraight();

        // Re-check the app state immediately after the emergency is cleared.
        if (RemoteXY.connect_flag == 1) {
            if (RemoteXY.robot_power == 0) {
                isAppStandbyActive = true;
                powerManager.setMode(POWER_APP_STANDBY);
                Serial.println(F("[SYSTEM] Restored to APP STANDBY (Power is OFF)."));
            } else {
                isAppStandbyActive = false;
                powerManager.setMode(POWER_ACTIVE);
                Serial.println(F("[SYSTEM] Restored to ACTIVE (Power is ON)."));
            }

            // 1 = AUTO | 0 = MANUAL.
            if (RemoteXY.mode_switch == 1) {
                currentSystemState = STATE_AUTO;
                Serial.println(F("[SYSTEM] Resumed Mode: AUTO."));
            } else {
                currentSystemState = STATE_MANUAL;
                Serial.println(F("[SYSTEM] Resumed Mode: MANUAL."));
            }
        } else {
            isAppStandbyActive = false;
            powerManager.setMode(POWER_ACTIVE);
            currentSystemState = STATE_AUTO;
            Serial.println(F("[SYSTEM] Restored to ACTIVE (Physical Button)."));
        }

        lastManualActivity = millis();

        if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            sharedTelemetry.flameEmergency = false;
            xSemaphoreGive(telemetryMutex);
        }
    }
}

void enterManualStandby() {
    Serial.println(F("\n[POWER] MANUAL STANDBY: Inactive > 3s"));
    motor.stop();
    powerManager.setMode(POWER_MANUAL_STANDBY);
    dashboard.addEvent("Manual Standby (Inactive > 3s)");
}

void exitManualStandby() {
    powerManager.setMode(POWER_ACTIVE);
    lastManualActivity = millis();
}

void enterAppStandby() {
    Serial.println(F("\n[POWER] APP VIRTUAL STANDBY (Switch = OFF)"));
    motor.stop();
    buzzer.off();
    powerManager.setMode(POWER_APP_STANDBY);
    dashboard.addEvent("Robot Power OFF (App Standby)");
}

void exitAppStandby() {
    Serial.println(F("[POWER] APP STANDBY -> ACTIVE (Switch = ON)"));
    powerManager.setMode(POWER_ACTIVE);
    lastManualActivity = millis();
    dashboard.addEvent("Robot Power ON (Active)");
}

void handleManualJoystick() {
    const int8_t x = RemoteXY.joystick_01_x;
    const int8_t y = RemoteXY.joystick_01_y;
    const int8_t sliderVal = RemoteXY.speed_slider;

    uint8_t currentSpeed = 0;
    if (sliderVal > 0) {
        currentSpeed = map(sliderVal, 0, 100, 50, 130);
    }

    if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        sharedTelemetry.speedPWM = currentSpeed;
        xSemaphoreGive(telemetryMutex);
    }

    char cmd = 'S';
    const char* dirStr = "STOP";

    if (sliderVal <= 0 || (x == 0 && y == 0)) {
        cmd = 'S';
        dirStr = "STOP";
    } else if (y > 30) {
        cmd = 'F';
        dirStr = "FORWARD";
    } else if (y < -30) {
        cmd = 'B';
        dirStr = "BACKWARD";
    } else if (x > 30) {
        cmd = 'R';
        dirStr = "RIGHT";
    } else if (x < -30) {
        cmd = 'L';
        dirStr = "LEFT";
    }

    static char lastCmd = ' ';
    static int8_t lastSlider = -1;

    if (cmd != lastCmd || sliderVal != lastSlider) {
        Serial.print(F("[MANUAL RC] Direction: "));
        Serial.print(dirStr);
        Serial.print(F(" | Speed: "));
        Serial.print(sliderVal);
        Serial.print(F("% | PWM: "));
        Serial.println(cmd == 'S' ? 0 : currentSpeed);

        lastCmd = cmd;
        lastSlider = sliderVal;
    }

    BehaviourFactory::getManualMode()->setCommand(cmd, (cmd == 'S' ? 0 : currentSpeed));
}

// ============================================================
//        Task 1: movement and sensor processing (Core 1)
// ============================================================
void robotControlTask(void* pvParameters) {
    RobotModes* currentMode = nullptr;
    static const char* lastAutoState = nullptr;
    static bool lastObstacleState = false;

    for (;;) {
        if (flame_sensor.isTriggered() || powerManager.isEmergency()) {
            if (!powerManager.isEmergency()) {
                enterEmergency();
            } else {
                handleEmergency();
            }
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        if (powerManager.isAppStandby()) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        const int upperDist = readCleanDistance(upper_ultrasonic);
        vTaskDelay(pdMS_TO_TICKS(20));
        const int lowerDist = readCleanDistance(lower_ultrasonic);

        if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            sharedTelemetry.upperDistance = upperDist;
            sharedTelemetry.lowerDistance = lowerDist;
            sharedTelemetry.powerMode = powerManager.isEmergency() ? "EMERGENCY"
                                      : (powerManager.isAppStandby() ? "POWER OFF"
                                      : (powerManager.isManualStandby() ? "STANDBY" : "ACTIVE"));
            xSemaphoreGive(telemetryMutex);
        }

        if (currentSystemState == STATE_MANUAL) {
            if (powerManager.isActive() && (millis() - lastManualActivity >= MANUAL_STANDBY_TIMEOUT)) {
                enterManualStandby();
            }

            if (!powerManager.isManualStandby()) {
                currentMode = BehaviourFactory::getMode(MODE_MANUAL);
                if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                    sharedTelemetry.currentMode = "MANUAL";
                    xSemaphoreGive(telemetryMutex);
                }
                handleManualJoystick();
            }
        } else {
            if (powerManager.isManualStandby()) {
                exitManualStandby();
            }

            const bool obstacleDetected = (upperDist <= obstacleThreshold || lowerDist <= obstacleThreshold);
            const char* autoState = obstacleDetected ? "AVOID" : "DRIVE";

            if (autoState != lastAutoState || obstacleDetected != lastObstacleState) {
                Serial.print(F("[AUTO] State: "));
                Serial.print(autoState);
                Serial.print(F(" | Upper: "));
                Serial.print(upperDist);
                Serial.print(F(" cm | Lower: "));
                Serial.print(lowerDist);
                Serial.print(F(" cm | Threshold: "));
                Serial.println(obstacleThreshold);

                lastAutoState = autoState;
                lastObstacleState = obstacleDetected;
            }

            if (obstacleDetected) {
                currentMode = BehaviourFactory::getMode(MODE_AVOID);
                if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                    sharedTelemetry.currentMode = "AVOID";
                    xSemaphoreGive(telemetryMutex);
                }
                currentMode->execute();
            } else {
                currentMode = BehaviourFactory::getMode(MODE_DRIVE);
                if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                    sharedTelemetry.currentMode = "DRIVE";
                    xSemaphoreGive(telemetryMutex);
                }
                currentMode->execute();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

// ============================================================
//       Task 2: RemoteXY, dashboard, and survival logic (Core 0)
// ============================================================
void telemetryNetworkTask(void* pvParameters) {
    static uint8_t previousConnectFlag = 0;
    static unsigned long syncCooldownTimer = 0;
    static unsigned long lastDashboardUpdate = 0;

    for (;;) {
        RemoteXYEngine.handler();

        // Handle the mobile reconnect event and synchronize the state.
        if (RemoteXY.connect_flag == 1 && previousConnectFlag == 0) {
            Serial.println(F("\n>>> [IoT]: RemoteXY Connected! Syncing State... <<<"));
            dashboard.addEvent("RemoteXY Connected");

            RemoteXY.robot_power = (powerManager.isAppStandby() || isAppStandbyActive) ? 0 : 1;

            // 1 = AUTO | 0 = MANUAL.
            if (currentSystemState == STATE_AUTO) {
                RemoteXY.mode_switch = 1;
            } else {
                RemoteXY.mode_switch = 0;
            }

            syncCooldownTimer = millis() + 500;
        }
        previousConnectFlag = RemoteXY.connect_flag;

        // Handle RemoteXY app commands.
        if (RemoteXY.connect_flag == 1 && millis() > syncCooldownTimer) {
            // Power switch.
            if (RemoteXY.robot_power == 0 && !isAppStandbyActive) {
                isAppStandbyActive = true;
                enterAppStandby();
            } else if (RemoteXY.robot_power == 1 && isAppStandbyActive) {
                isAppStandbyActive = false;
                exitAppStandby();
            }

            // Mode switch: 1 = Auto, 0 = Manual.
            if (RemoteXY.mode_switch == 1) {
                if (currentSystemState != STATE_AUTO) {
                    currentSystemState = STATE_AUTO;
                    motor.stop();
                    if (powerManager.isManualStandby()) {
                        exitManualStandby();
                    }
                    dashboard.addEvent("Mode switched to AUTO");
                    Serial.println(F("\n>>> [REMOTEXY]: AUTO MODE <<<"));
                }
            } else {
                if (currentSystemState != STATE_MANUAL) {
                    currentSystemState = STATE_MANUAL;
                    motor.stop();
                    lastManualActivity = millis();
                    dashboard.addEvent("Mode switched to MANUAL");
                    Serial.println(F("\n>>> [REMOTEXY]: MANUAL MODE <<<"));
                }
            }

            // Update joystick activity for manual mode.
            if (currentSystemState == STATE_MANUAL) {
                const int8_t curX = RemoteXY.joystick_01_x;
                const int8_t curY = RemoteXY.joystick_01_y;

                if (curX != 0 || curY != 0 || curX != previousJoystickX || curY != previousJoystickY) {
                    lastManualActivity = millis();
                    previousJoystickX = curX;
                    previousJoystickY = curY;
                    if (powerManager.isManualStandby()) {
                        exitManualStandby();
                    }
                }
            }
        } else if (RemoteXY.connect_flag == 0 && currentSystemState == STATE_MANUAL) {
            motor.stop();
        }

        // Refresh the dashboard data every 100 ms.
        if (millis() - lastDashboardUpdate >= 100) {
            lastDashboardUpdate = millis();

            int upD = -1;
            int lowD = -1;
            String curM = "UNKNOWN";
            String curS = "NORMAL";
            uint8_t spd = 0;

            if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                upD = sharedTelemetry.upperDistance;
                lowD = sharedTelemetry.lowerDistance;
                curM = sharedTelemetry.currentMode ? sharedTelemetry.currentMode : "IDLE";
                spd = sharedTelemetry.speedPWM;

                if (sharedTelemetry.flameEmergency) {
                    curS = "EMERGENCY";
                } else if (powerManager.isAppStandby() || isAppStandbyActive) {
                    curS = "POWER OFF";
                } else if (powerManager.isManualStandby()) {
                    curS = "STANDBY";
                } else {
                    curS = "NORMAL";
                }
                xSemaphoreGive(telemetryMutex);
            }

            uint8_t speedPct = 0;
            if (currentSystemState == STATE_MANUAL) {
                speedPct = (RemoteXY.speed_slider > 0) ? RemoteXY.speed_slider : 0;
            } else if (currentSystemState == STATE_AUTO && !powerManager.isAppStandby()) {
                speedPct = 85;
            }

            dashboard.updateData(speedPct, curM, upD, lowD, curS);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ============================================================
//                    Initialize the system
// ============================================================
void initStorageAndRTOS() {
    bootCount++;

    // Start the RemoteXY interface.
    RemoteXY_Init();

    // Start the ESPAsyncWebServer dashboard.
    dashboard.begin();
    dashboard.addEvent("System Boot #" + String(bootCount) + " Started");

    preferences.begin("robot_nvs", false);
    sharedTelemetry.totalFlameEvents = preferences.getUInt("flame_cnt", 0);
    obstacleThreshold = preferences.getInt("obs_thresh", 20);

    telemetryMutex = xSemaphoreCreateMutex();
    lastManualActivity = millis();
    powerManager.setMode(POWER_ACTIVE);

    xTaskCreatePinnedToCore(robotControlTask, "RobotControlTask", 8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(telemetryNetworkTask, "TelemetryNetworkTask", 8192, NULL, 1, NULL, 0);
}