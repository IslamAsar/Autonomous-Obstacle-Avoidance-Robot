#include "HardwareComponent.h"

// ============================================================
//               Infrared sensor implementation
// ============================================================
IRSensor::IRSensor(uint8_t irpin) : _irpin(irpin) {}

void IRSensor::begin() {
    pinMode(_irpin, INPUT);
}

bool IRSensor::detected() {
    return digitalRead(_irpin);
}

// ============================================================
//          Flame sensor interrupt implementation
// ============================================================
volatile bool FlameSensor::_triggered = false;

FlameSensor::FlameSensor(uint8_t fpin) : _fpin(fpin) {}

void IRAM_ATTR FlameSensor::isr() {
    _triggered = true;
}

void FlameSensor::begin() {
    pinMode(_fpin, INPUT);
    _triggered = false;
    attachInterrupt(digitalPinToInterrupt(_fpin), FlameSensor::isr, RISING);
}

bool FlameSensor::detected() {
    return digitalRead(_fpin);
}

bool FlameSensor::isTriggered() {
    return _triggered;
}

void FlameSensor::clearTrigger() {
    _triggered = false;
}

// ============================================================
//              Ultrasonic sensor implementation
// ============================================================
UltrasonicSensor::UltrasonicSensor(uint8_t trigPin, uint8_t echoPin)
    : _trigPin(trigPin), _echoPin(echoPin) {}

void UltrasonicSensor::begin() {
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
}

int UltrasonicSensor::getDistance() {
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    // Use a bounded timeout to avoid stalling on missed echoes.
    const unsigned long duration = pulseIn(_echoPin, HIGH, 25000);
    if (duration == 0) {
        return -1;
    }

    const uint16_t distance = (duration * 0.0343) / 2;
    if (distance < 2 || distance > 400) {
        return -1;
    }

    return distance;
}

// ============================================================
//             Reset button implementation
// ============================================================
ResetButton::ResetButton(uint8_t pin) : _pin(pin) {}

void ResetButton::begin() {
    pinMode(_pin, INPUT_PULLUP);
}

bool ResetButton::isPressed() {
    if (digitalRead(_pin) == LOW) {
        delay(50); // Ignore contact bounce noise.
        if (digitalRead(_pin) == LOW) {
            return true;
        }
    }

    return false;
}

// ============================================================
//              L298N motor driver implementation
// ============================================================
H_Bridge::H_Bridge(uint8_t pinENA,
                   uint8_t pinENB,
                   uint8_t pinIN1,
                   uint8_t pinIN2,
                   uint8_t pinIN3,
                   uint8_t pinIN4,
                   uint8_t currentSpeed)
    : _pinENA(pinENA),
      _pinENB(pinENB),
      _pinIN1(pinIN1),
      _pinIN2(pinIN2),
      _pinIN3(pinIN3),
      _pinIN4(pinIN4),
      _currentSpeed(currentSpeed) {}

void H_Bridge::begin() {
    pinMode(_pinENA, OUTPUT);
    pinMode(_pinENB, OUTPUT);
    pinMode(_pinIN1, OUTPUT);
    pinMode(_pinIN2, OUTPUT);
    pinMode(_pinIN3, OUTPUT);
    pinMode(_pinIN4, OUTPUT);
}

void H_Bridge::setSpeed(uint8_t wantedSpeed) {
    _currentSpeed = constrain(wantedSpeed, 0, 100);
}

void H_Bridge::forward() {
    const uint8_t pwmValue = map(_currentSpeed, 0, 100, 0, 255);
    analogWrite(_pinENA, pwmValue);
    analogWrite(_pinENB, pwmValue);

    digitalWrite(_pinIN1, HIGH);
    digitalWrite(_pinIN2, LOW);
    digitalWrite(_pinIN3, HIGH);
    digitalWrite(_pinIN4, LOW);
}

void H_Bridge::backward() {
    const uint8_t pwmValue = map(_currentSpeed, 0, 100, 0, 255);
    analogWrite(_pinENA, pwmValue);
    analogWrite(_pinENB, pwmValue);

    digitalWrite(_pinIN1, LOW);
    digitalWrite(_pinIN2, HIGH);
    digitalWrite(_pinIN3, LOW);
    digitalWrite(_pinIN4, HIGH);
}

void H_Bridge::turnRight() {
    const uint8_t pwmValue = map(_currentSpeed, 0, 100, 0, 255);
    analogWrite(_pinENA, pwmValue);
    analogWrite(_pinENB, pwmValue);

    digitalWrite(_pinIN1, HIGH);
    digitalWrite(_pinIN2, LOW);
    digitalWrite(_pinIN3, LOW);
    digitalWrite(_pinIN4, HIGH);
}

void H_Bridge::turnLeft() {
    const uint8_t pwmValue = map(_currentSpeed, 0, 100, 0, 255);
    analogWrite(_pinENA, pwmValue);
    analogWrite(_pinENB, pwmValue);

    digitalWrite(_pinIN1, LOW);
    digitalWrite(_pinIN2, HIGH);
    digitalWrite(_pinIN3, HIGH);
    digitalWrite(_pinIN4, LOW);
}

void H_Bridge::stop() {
    analogWrite(_pinENA, 0);
    analogWrite(_pinENB, 0);

    digitalWrite(_pinIN1, LOW);
    digitalWrite(_pinIN2, LOW);
    digitalWrite(_pinIN3, LOW);
    digitalWrite(_pinIN4, LOW);
}

// ============================================================
//                 Radar servo implementation
// ============================================================
RadarServo::RadarServo(uint8_t pin) : _pin(pin) {}

void RadarServo::begin() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    activate();
    lookStraight();
}

void RadarServo::activate() {
    _servo.setPeriodHertz(50);
    if (!_servo.attached()) {
        _servo.attach(_pin, 500, 2400);
    }
}

void RadarServo::deactivate() {
    _servo.detach();
}

void RadarServo::lookStraight() {
    _servo.write(90);
}

void RadarServo::lookRight() {
    _servo.write(45);
}

void RadarServo::lookLeft() {
    _servo.write(135);
}

// ============================================================
//                  Buzzer implementation
// ============================================================
Buzzer::Buzzer(uint8_t pin) : _pin(pin) {}

void Buzzer::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void Buzzer::alarm(unsigned int frequency) {
    tone(_pin, frequency);
}

void Buzzer::off() {
    noTone(_pin);
}