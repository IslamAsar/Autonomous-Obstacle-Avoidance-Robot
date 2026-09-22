#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>

// Power states used by the robot controller and dashboard.
enum PowerMode {
    POWER_ACTIVE,
    POWER_MANUAL_STANDBY,
    POWER_EMERGENCY,
    POWER_APP_STANDBY
};

class PowerManager {
private:
    PowerMode _currentMode;

public:
    PowerManager();

    void setMode(PowerMode mode);
    PowerMode getMode() const;

    bool isActive() const;
    bool isManualStandby() const;
    bool isEmergency() const;
    bool isAppStandby() const;
};

#endif // POWER_MANAGER_H