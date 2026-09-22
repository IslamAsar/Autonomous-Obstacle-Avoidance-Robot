#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

// Common interface for every hardware module that requires initialization.
class HardwareComponent {
public:
    virtual ~HardwareComponent() = default;
    virtual void begin() = 0;
};

// Base class for sensor devices.
class Sensor : public HardwareComponent {
public:
    virtual ~Sensor() = default;
};

// Base class for actuator devices.
class Actuator : public HardwareComponent {
public:
    virtual ~Actuator() = default;
};

//===========================================================
//                          Sensors
//===========================================================
// Digital infrared sensor used for line detection.
class IRSensor : public Sensor {
private:
    uint8_t _irpin;

public:
    explicit IRSensor(uint8_t irpin);

    void begin() override;
    bool detected();
};

// Flame sensor with interrupt-based event detection on the ESP32.
class FlameSensor : public Sensor {
private:
    uint8_t _fpin;
    static volatile bool _triggered;

    static void IRAM_ATTR isr();

public:
    explicit FlameSensor(uint8_t fpin);

    void begin() override;
    bool detected();
    bool isTriggered();
    void clearTrigger();
};

// Ultrasonic ranging sensor with separate trigger and echo pins.
class UltrasonicSensor : public Sensor {
private:
    uint8_t _trigPin;
    uint8_t _echoPin;

public:
    explicit UltrasonicSensor(uint8_t trigPin, uint8_t echoPin);

    void begin() override;
    // Return the measured distance in centimeters, or -1 when invalid.
    int getDistance();
};

// Pull-up button used to clear an emergency state.
class ResetButton : public Sensor {
private:
    uint8_t _pin;

public:
    explicit ResetButton(uint8_t pin);

    void begin() override;
    bool isPressed();
};

//===========================================================
//                          Actuators
//===========================================================
// Controls two DC motors through an L298N dual H-bridge driver.
class H_Bridge : public Actuator {
private:
    uint8_t _pinENA;
    uint8_t _pinENB;
    uint8_t _pinIN1;
    uint8_t _pinIN2;
    uint8_t _pinIN3;
    uint8_t _pinIN4;
    uint8_t _currentSpeed;

public:
    H_Bridge(uint8_t pinENA,
             uint8_t pinENB,
             uint8_t pinIN1,
             uint8_t pinIN2,
             uint8_t pinIN3,
             uint8_t pinIN4,
             uint8_t currentSpeed);

    void begin() override;
    // Set the motor speed as a percentage from 0 to 100.
    void setSpeed(uint8_t wantedSpeed);

    void forward();
    void backward();
    void turnRight();
    void turnLeft();
    void stop();
};

// Positions the ultrasonic sensor for radar-style obstacle scanning.
class RadarServo : public Actuator {
private:
    Servo _servo;
    uint8_t _pin;

public:
    explicit RadarServo(uint8_t pin);

    void begin() override;
    void activate();
    void deactivate();
    void lookStraight();
    void lookRight();
    void lookLeft();
};

// Passive buzzer used for warnings and emergency alerts.
class Buzzer : public Actuator {
private:
    uint8_t _pin;

public:
    explicit Buzzer(uint8_t pin);

    void begin() override;
    void alarm(unsigned int frequency = 1000);
    void off();
};