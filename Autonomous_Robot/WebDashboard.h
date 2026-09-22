#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

class WebDashboard {
private:
    AsyncWebServer _server;

    // Values displayed by the dashboard.
    uint8_t _speed;
    String _mode;
    int _upperDistance;
    int _lowerDistance;
    String _status;

    // Fixed-size event log containing the most recent activity.
    String _events[10];
    uint8_t _eventCount;

public:
    WebDashboard();

    void begin();

    void updateData(uint8_t speed,
                    const String& mode,
                    int upperDistance,
                    int lowerDistance,
                    const String& status);

    void addEvent(const String& event);
};

#endif // WEB_DASHBOARD_H