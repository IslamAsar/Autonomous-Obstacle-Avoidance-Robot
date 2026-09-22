#ifndef ROBOT_MODES_H
#define ROBOT_MODES_H

#include "HardwareComponent.h"

// Hardware references shared with the robot behavior layer.
extern H_Bridge motor;
extern RadarServo radarServo;
extern IRSensor right_ir;
extern IRSensor center_ir;
extern IRSensor left_ir;
extern UltrasonicSensor upper_ultrasonic;
extern UltrasonicSensor lower_ultrasonic;

int readCleanDistance(UltrasonicSensor& sensor);

// Base interface for all robot behavior modes.
class RobotModes {
public:
    virtual ~RobotModes() = default;
    virtual void execute() = 0;
};

// Autonomous forward-driving behavior based on the IR sensors.
class DriveMode : public RobotModes {
public:
    void execute() override;
};

// Radar-style scan behavior used by obstacle avoidance.
class ScanMode : public RobotModes {
private:
    int _distanceRight;
    int _distanceCenter;
    int _distanceLeft;
    int _lowerDistance;

public:
    void execute() override;
    int getRightDistance() const;
    int getCenterDistance() const;
    int getLeftDistance() const;
    int getLowerDistance() const;
};

// Obstacle avoidance behavior based on the latest sensor scan.
class AvoidMode : public RobotModes {
private:
    ScanMode _scanObject;

public:
    void execute() override;
};

// Remote-controlled driving mode driven by joystick commands.
class ManualMode : public RobotModes {
private:
    char _lastCommand;

public:
    ManualMode();

    void execute() override;
    void setCommand(char cmd, uint8_t speed);
};

// Identifiers used to create or retrieve a behavior mode.
enum ModeType {
    MODE_DRIVE,
    MODE_SCAN,
    MODE_AVOID,
    MODE_MANUAL
};

class BehaviourFactory {
public:
    static RobotModes* getMode(ModeType type);
    static ManualMode* getManualMode();
};

#endif // ROBOT_MODES_H