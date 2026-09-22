#include "PowerManager.h"

PowerManager::PowerManager() : _currentMode(POWER_ACTIVE) {}

void PowerManager::setMode(PowerMode mode) {
    _currentMode = mode;
}

PowerMode PowerManager::getMode() const {
    return _currentMode;
}

bool PowerManager::isActive() const {
    return _currentMode == POWER_ACTIVE;
}

bool PowerManager::isManualStandby() const {
    return _currentMode == POWER_MANUAL_STANDBY;
}

bool PowerManager::isEmergency() const {
    return _currentMode == POWER_EMERGENCY;
}

bool PowerManager::isAppStandby() const {
    return _currentMode == POWER_APP_STANDBY;
}