#ifndef SYSTEM_TASKS_H
#define SYSTEM_TASKS_H

#include <Preferences.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

#include "PowerManager.h"
#include "RobotModes.h"
#include "WebDashboard.h"

// Hardware references used by the RTOS tasks.
extern H_Bridge motor;
extern RadarServo radarServo;
extern const byte ResetButtonPin;
extern FlameSensor flame_sensor;
extern Buzzer buzzer;
extern ResetButton resetButton;
extern UltrasonicSensor upper_ultrasonic;
extern UltrasonicSensor lower_ultrasonic;

// Persistent system state and shared telemetry.
extern RTC_DATA_ATTR int bootCount;
extern Preferences preferences;
extern PowerManager powerManager;
extern WebDashboard dashboard;

struct RobotTelemetry {
    int upperDistance;
    int lowerDistance;
    const char* currentMode;
    const char* powerMode;
    bool flameEmergency;
    uint32_t totalFlameEvents;
    int speedPWM;
};

extern RobotTelemetry sharedTelemetry;
extern SemaphoreHandle_t telemetryMutex;

enum SystemState {
    STATE_AUTO,
    STATE_MANUAL
};

extern SystemState currentSystemState;
extern int obstacleThreshold;

// FreeRTOS task and system-control declarations.
void initStorageAndRTOS();
void robotControlTask(void* pvParameters);
void telemetryNetworkTask(void* pvParameters);

void stopAllActuators();
void enterEmergency();
void handleEmergency();
void enterManualStandby();
void exitManualStandby();
void enterAppStandby();
void exitAppStandby();

#endif // SYSTEM_TASKS_H