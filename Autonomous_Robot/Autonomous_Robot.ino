#include "SystemTasks.h"

// ============================================================
//                  Robot GPIO pin mapping
// ============================================================

// Flame sensor and safety devices.
const byte FlamePin = 23;        // ESP32 GPIO23: flame sensor output.
const byte ResetButtonPin = 22;  // ESP32 GPIO22: emergency reset button.
const byte BuzzerPin = 4;        // ESP32 GPIO4: passive buzzer.

FlameSensor flame_sensor(FlamePin);
Buzzer buzzer(BuzzerPin);
ResetButton resetButton(ResetButtonPin);

// L298N dual H-bridge motor driver.
const byte MotorEnableLeftPin = 25;   // ESP32 GPIO25: L298N ENA (left motor PWM).
const byte MotorEnableRightPin = 26;  // ESP32 GPIO26: L298N ENB (right motor PWM).
const byte MotorLeftIn1Pin = 27;      // ESP32 GPIO27: L298N IN1.
const byte MotorLeftIn2Pin = 14;      // ESP32 GPIO14: L298N IN2.
const byte MotorRightIn3Pin = 16;     // ESP32 GPIO16: L298N IN3.
const byte MotorRightIn4Pin = 17;     // ESP32 GPIO17: L298N IN4.

H_Bridge motor(
    MotorEnableLeftPin,
    MotorEnableRightPin,
    MotorLeftIn1Pin,
    MotorLeftIn2Pin,
    MotorRightIn3Pin,
    MotorRightIn4Pin,
    85  // Initial speed percentage.
);

const byte ServoPin = 13;  // ESP32 GPIO13: scanning servo signal.
RadarServo radarServo(ServoPin);

// Infrared line sensors.
const byte LeftIrPin = 33;    // ESP32 GPIO33: left infrared sensor.
const byte CenterIrPin = 32;  // ESP32 GPIO32: center infrared sensor.
const byte RightIrPin = 35;   // ESP32 GPIO35: right infrared sensor.

IRSensor left_ir(LeftIrPin);
IRSensor center_ir(CenterIrPin);
IRSensor right_ir(RightIrPin);

// Ultrasonic range sensors.
const byte UpperUltrasonicTriggerPin = 5;   // ESP32 GPIO5: upper sensor trigger.
const byte UpperUltrasonicEchoPin = 18;     // ESP32 GPIO18: upper sensor echo.
const byte LowerUltrasonicTriggerPin = 19;  // ESP32 GPIO19: lower sensor trigger.
const byte LowerUltrasonicEchoPin = 21;     // ESP32 GPIO21: lower sensor echo.

UltrasonicSensor upper_ultrasonic(
    UpperUltrasonicTriggerPin,
    UpperUltrasonicEchoPin);
UltrasonicSensor lower_ultrasonic(
    LowerUltrasonicTriggerPin,
    LowerUltrasonicEchoPin);

// ============================================================
//                       Setup and loop
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(500);

    // Initialize all hardware and sensors.
    radarServo.begin();
    buzzer.begin();
    motor.begin();
    resetButton.begin();
    upper_ultrasonic.begin();
    lower_ultrasonic.begin();
    flame_sensor.begin();
    left_ir.begin();
    center_ir.begin();
    right_ir.begin();

    // Start the FreeRTOS, dashboard, and connectivity services.
    initStorageAndRTOS();
}

void loop() {
    vTaskDelete(NULL);
}