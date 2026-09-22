#include "RobotModes.h"

// ============================================================
//                    Movement speed constants
// ============================================================
const uint8_t SPEED_FORWARD = 85;
const uint8_t SPEED_LINE_CORRECT = 75;
const uint8_t SPEED_AVOID_TURN = 80;
const uint8_t SPEED_BACKWARD = 60;
const uint8_t SPEED_MANUAL = 80;
const int TURN_90_DELAY_MS = 600;

// ============================================================
//                Clean ultrasonic distance helper
// ============================================================
int readCleanDistance(UltrasonicSensor& sensor) {
    const int distance = sensor.getDistance();
    if (distance <= 3 || distance > 400) {
        return 400;
    }
    return distance;
}

// ============================================================
//                         Drive mode
// ============================================================
void DriveMode::execute() {
    const bool left = left_ir.detected();
    const bool center = center_ir.detected();
    const bool right = right_ir.detected();

    Serial.println(F("----- DRIVE MODE (3x IR) -----"));
    Serial.print(F("L: "));
    Serial.print(left);
    Serial.print(F(" | C: "));
    Serial.print(center);
    Serial.print(F(" | R: "));
    Serial.println(right);

    // No line detected: free forward roaming.
    if (left == 0 && center == 0 && right == 0) {
        Serial.println(F("Decision: ALL ZERO -> Free Roaming Forward"));
        motor.setSpeed(SPEED_FORWARD);
        motor.forward();
    }
    // Left and right readings are balanced: continue forward.
    else if (right == left) {
        Serial.println(F("Decision: BALANCED (R == L) -> FORWARD"));
        motor.setSpeed(SPEED_FORWARD);
        motor.forward();
    }
    // Left sensor detected a line: turn right to correct course.
    else if (left == 1) {
        Serial.println(F("Decision: LEFT HIT -> TURN RIGHT"));
        motor.setSpeed(SPEED_LINE_CORRECT);
        motor.turnRight();
    }
    // Right sensor detected a line: turn left to correct course.
    else if (right == 1) {
        Serial.println(F("Decision: RIGHT HIT -> TURN LEFT"));
        motor.setSpeed(SPEED_LINE_CORRECT);
        motor.turnLeft();
    }

    Serial.println();
}

// ============================================================
//                        Scan mode
// ============================================================
void ScanMode::execute() {
    Serial.println(F("----- SCAN MODE -----"));

    // Center position: 90 degrees.
    radarServo.lookStraight();
    delay(250);
    _distanceCenter = readCleanDistance(upper_ultrasonic);
    delay(20); // Short pause to reduce acoustic interference between reads.
    _lowerDistance = readCleanDistance(lower_ultrasonic);

    // Right position: 45 degrees.
    radarServo.lookRight();
    delay(450);
    _distanceRight = readCleanDistance(upper_ultrasonic);

    // Left position: 135 degrees.
    radarServo.lookLeft();
    delay(550);
    _distanceLeft = readCleanDistance(upper_ultrasonic);

    // Return to center position.
    radarServo.lookStraight();
    delay(300);
}

int ScanMode::getRightDistance() const {
    return _distanceRight;
}

int ScanMode::getCenterDistance() const {
    return _distanceCenter;
}

int ScanMode::getLeftDistance() const {
    return _distanceLeft;
}

int ScanMode::getLowerDistance() const {
    return _lowerDistance;
}

// ============================================================
//                       Avoid mode
// ============================================================
void AvoidMode::execute() {
    Serial.println(F("========== AVOID MODE =========="));
    motor.stop();
    delay(150);

    // Sweep the ultrasonic sensor around the robot.
    _scanObject.execute();

    const int right = _scanObject.getRightDistance();
    const int center = _scanObject.getCenterDistance();
    const int left = _scanObject.getLeftDistance();
    const int lower = _scanObject.getLowerDistance();

    // Path is clear: continue forward.
    if (center > 30 && lower > 30) {
        Serial.println(F("[AVOID DECISION]: Clear -> Forward"));
        motor.setSpeed(SPEED_FORWARD);
        motor.forward();
    }
    // Right side is wider: turn 90 degrees right.
    else if (right > left && right >= 25) {
        Serial.println(F("[AVOID DECISION]: Turn RIGHT"));
        motor.setSpeed(SPEED_AVOID_TURN);
        motor.turnRight();
        delay(TURN_90_DELAY_MS);
        motor.stop();
    }
    // Left side is wider: turn 90 degrees left.
    else if (left > right && left >= 25) {
        Serial.println(F("[AVOID DECISION]: Turn LEFT"));
        motor.setSpeed(SPEED_AVOID_TURN);
        motor.turnLeft();
        delay(TURN_90_DELAY_MS);
        motor.stop();
    }
    // Dead-end situation: reverse then rotate to escape.
    else {
        Serial.println(F("[AVOID DECISION]: Dead End -> Escape"));
        motor.setSpeed(SPEED_BACKWARD);
        motor.backward();
        delay(750);

        motor.setSpeed(SPEED_AVOID_TURN);
        motor.turnRight();
        delay(TURN_90_DELAY_MS * 2);
        motor.stop();
    }
}

// ============================================================
//                         Manual mode
// ============================================================
ManualMode::ManualMode() : _lastCommand('S') {}

void ManualMode::setCommand(char cmd, uint8_t speed) {
    _lastCommand = cmd;
    motor.setSpeed(speed);

    switch (cmd) {
        case 'F':
        case 'f':
            motor.forward();
            break;
        case 'B':
        case 'b':
            motor.backward();
            break;
        case 'L':
        case 'l':
            motor.turnLeft();
            break;
        case 'R':
        case 'r':
            motor.turnRight();
            break;
        case 'S':
        case 's':
        default:
            motor.stop();
            break;
    }
}

void ManualMode::execute() {}

// ============================================================
//                    Behaviour factory
// ============================================================
RobotModes* BehaviourFactory::getMode(ModeType type) {
    static DriveMode driveMode;
    static ScanMode scanMode;
    static AvoidMode avoidMode;
    static ManualMode manualMode;

    switch (type) {
        case MODE_DRIVE:
            return &driveMode;
        case MODE_SCAN:
            return &scanMode;
        case MODE_AVOID:
            return &avoidMode;
        case MODE_MANUAL:
            return &manualMode;
        default:
            return nullptr;
    }
}

ManualMode* BehaviourFactory::getManualMode() {
    return static_cast<ManualMode*>(getMode(MODE_MANUAL));
}