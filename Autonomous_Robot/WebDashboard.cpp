#include "WebDashboard.h"

// ============================================================
//                          Constructor
// ============================================================
WebDashboard::WebDashboard()
    : _server(80),
      _speed(0),
      _mode("UNKNOWN"),
      _upperDistance(-1),
      _lowerDistance(-1),
      _status("NORMAL"),
      _eventCount(0) {}
// ============================================================
//                    Update dashboard values
// ============================================================
void WebDashboard::updateData(uint8_t speed,
                              const String& mode,
                              int upperDistance,
                              int lowerDistance,
                              const String& status)
{
    _speed = speed;
    _mode = mode;
    _upperDistance = upperDistance;
    _lowerDistance = lowerDistance;
    _status = status;
}
// ============================================================
//                        Add event log entry
// ============================================================
void WebDashboard::addEvent(const String& event)
{
    // Rotate the buffer to keep the most recent events available.
    if (_eventCount >= 10)
    {
        for (uint8_t i = 0; i < 9; ++i)
        {
            _events[i] = _events[i + 1];
        }
        _eventCount = 9;
    }
    _events[_eventCount] = event;
    ++_eventCount;
}
// ============================================================
//                        Start web server
// ============================================================
void WebDashboard::begin()
{
    // ========================================================
    // Home page
    // ========================================================
    _server.on(
        "/",
        HTTP_GET,
        [this](AsyncWebServerRequest *request)
        {
            const char PROGMEM html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport"
      content="width=device-width, initial-scale=1">
<title>ESP32 Robot Dashboard</title>
<style>
/* ==========================================================
                                 GLOBAL
    ========================================================== */
* {
    box-sizing: border-box;
}
body {
    margin: 0;
    font-family:
        Arial,
        Helvetica,
        sans-serif;
    background:
        linear-gradient(
            135deg,
            #0f172a,
            #111827
        );
    color: white;
    min-height: 100vh;
}
/* ==========================================================
                                 HEADER
    ========================================================== */
.header {
    text-align: center;
    padding: 25px 15px 15px;
}
.header-title {
    font-size: 28px;
    font-weight: bold;
}
.header-subtitle {
    color: #94a3b8;
    font-size: 14px;
    margin-top: 6px;
}
/* ==========================================================
                             MAIN CONTAINER
    ========================================================== */
.container {
    width: 92%;
    max-width: 650px;
    margin: auto;
    padding-bottom: 30px;
}
/* ==========================================================
                                 GRID
    ========================================================== */
.top-grid {
    display: grid;
    grid-template-columns:
        1fr 1fr;
    gap: 14px;
}
/* ==========================================================
                                 CARD
    ========================================================== */
.card {
    background:
        rgba(30, 41, 59, 0.92);
    border:
        1px solid
        rgba(148, 163, 184, 0.15);
    border-radius: 18px;
    padding: 20px;
    margin: 14px 0;
    box-shadow:
        0 8px 25px
        rgba(0, 0, 0, 0.25);
}
/* ==========================================================
                                LABEL
    ========================================================== */
.label {
    color: #94a3b8;
    font-size: 14px;
    text-transform: uppercase;
    letter-spacing: 1px;
}
/* ==========================================================
                                VALUE
    ========================================================== */
.value {
    font-size: 30px;
    font-weight: bold;
    margin-top: 8px;
}
/* ==========================================================
                                 MODE
    ========================================================== */
.mode {
    font-size: 21px;
    font-weight: bold;
    margin-top: 10px;
}
/* ==========================================================
                          DISTANCE GRID
    ========================================================== */
.distance-grid {
    display: grid;
    grid-template-columns:
        1fr 1fr;
    gap: 15px;
    margin-top: 18px;
}
.distance-box {
    background:
        rgba(15, 23, 42, 0.8);
    border-radius: 14px;
    padding: 18px;
}
.distance-label {
    color: #94a3b8;
    font-size: 14px;
}
.distance-value {
    font-size: 30px;
    font-weight: bold;
    margin-top: 6px;
}
/* ==========================================================
                                STATUS
    ========================================================== */
.status {
    display: flex;
    align-items: center;
    gap: 10px;
    margin-top: 12px;
    font-size: 20px;
    font-weight: bold;
}
.status-dot {
    width: 12px;
    height: 12px;
    border-radius: 50%;
    background: #22c55e;
    box-shadow:
        0 0 10px
        rgba(34, 197, 94, 0.8);
}
/* ==========================================================
                                EVENTS
    ========================================================== */
.events {
    text-align: left;
    max-height: 260px;
    overflow-y: auto;
    margin-top: 12px;
    background:
        rgba(15, 23, 42, 0.8);
    border-radius: 12px;
}
.event {
    padding: 12px;
    border-bottom:
        1px solid
        rgba(148, 163, 184, 0.12);
    font-size: 14px;
}
.event:last-child {
    border-bottom: none;
}
.no-events {
    color: #64748b;
    padding: 15px;
    text-align: center;
}
/* ==========================================================
                             WIFI INFO
    ========================================================== */
.connection {
    text-align: center;
    color: #64748b;
    font-size: 12px;
    margin-top: 18px;
}
/* ==========================================================
                              MOBILE
    ========================================================== */
@media (max-width: 500px)
{
    .top-grid {
        grid-template-columns:
            1fr;
    }
    .distance-grid {
        grid-template-columns:
            1fr 1fr;
    }
    .header-title {
        font-size: 24px;
    }
}
</style>
</head>
<body>
<!-- ========================================================
                    HEADER
    ======================================================== -->
<div class="header">
    <div class="header-title">
        ROBOT DASHBOARD
    </div>
    <div class="header-subtitle">
        ESP32 Autonomous Robot
    </div>
</div>
<div class="container">
<!-- ========================================================
                  SPEED + MODE
    ======================================================== -->
<div class="top-grid">
    <!-- SPEED -->
    <div class="card">
        <div class="label">
            Speed
        </div>
        <div class="value">
            <span id="speed">
                0
            </span> %
        </div>
    </div>
    <!-- MODE -->
    <div class="card">
        <div class="label">
            Current Mode
        </div>
        <div class="mode"
             id="mode">
            UNKNOWN
        </div>
    </div>
</div>
<!-- ========================================================
                   ULTRASONIC
    ======================================================== -->
<div class="card">
    <div class="label">
        Ultrasonic Sensors
    </div>
    <div class="distance-grid">
        <!-- UPPER -->
        <div class="distance-box">
            <div class="distance-label">
                Upper Ultrasonic
            </div>
            <div class="distance-value">
                <span id="upperDistance">
                    --
                </span>
                cm
            </div>
        </div>
        <!-- LOWER -->
        <div class="distance-box">
            <div class="distance-label">
                Lower Ultrasonic
            </div>
            <div class="distance-value">
                <span id="lowerDistance">
                    --
                </span>
                cm
            </div>
        </div>
    </div>
</div>
<!-- ========================================================
                 SYSTEM STATUS
    ======================================================== -->
<div class="card">
    <div class="label">
        System Status
    </div>
    <div class="status">
        <div class="status-dot"
             id="statusDot">
        </div>
        <span id="status">
            NORMAL
        </span>
    </div>
</div>
<!-- ========================================================
                   EVENT LOG
    ======================================================== -->
<div class="card">
    <div class="label">
        Event Log
    </div>
    <div class="events"
         id="events">
        <div class="no-events">
            No events yet.
        </div>
    </div>
</div>
<!-- ========================================================
                  CONNECTION
    ======================================================== -->
<div class="connection">
    Connected directly to ESP32 Wi-Fi
    <br>
    Dashboard updates automatically
</div>
</div>
<script>
// ============================================================
//                       Update dashboard
// ============================================================
function updateDashboard() {
    fetch('/data')
        .then((response) => response.json())
        .then((data) => {
            // Speed.
            document.getElementById("speed").innerText = data.speed;
            // Current mode.
            document.getElementById("mode").innerText = data.mode;
            // Upper ultrasonic distance.
            if (data.upperDistance >= 0) {
                document.getElementById("upperDistance").innerText = data.upperDistance;
            } else {
                document.getElementById("upperDistance").innerText = "--";
            }
            // Lower ultrasonic distance.
            if (data.lowerDistance >= 0) {
                document.getElementById("lowerDistance").innerText = data.lowerDistance;
            } else {
                document.getElementById("lowerDistance").innerText = "--";
            }
            // System status text.
            document.getElementById("status").innerText = data.status;
            // Status indicator color.
            const dot = document.getElementById("statusDot");
            if (data.status === "EMERGENCY") {
                dot.style.background = "#ef4444";
                dot.style.boxShadow = "0 0 12px rgba(239,68,68,0.9)";
            } else if (data.status === "POWER OFF") {
                dot.style.background = "#f59e0b";
                dot.style.boxShadow = "0 0 12px rgba(245,158,11,0.9)";
            } else {
                dot.style.background = "#22c55e";
                dot.style.boxShadow = "0 0 12px rgba(34,197,94,0.8)";
            }
            // Event log.
            document.getElementById("events").innerHTML = data.events;
        })
        .catch((error) => {
            console.log("Dashboard connection error:", error);
        });
}
// ============================================================
//                    Refresh every 250 ms
// ============================================================
setInterval(updateDashboard, 250);
// ============================================================
//                       Initial refresh
// ============================================================
updateDashboard();
</script>
</body>
</html>
)rawliteral";
            request->send(
                200,
                "text/html",
                html);
        });
    // ========================================================
    // DATA API
    // ========================================================
    _server.on(
        "/data",
        HTTP_GET,
        [this](AsyncWebServerRequest* request)
        {
            String json = "{";
            // Speed.
            json += "\"speed\":";
            json += String(_speed);
            // Current mode.
            json += ",\"mode\":\"";
            json += _mode;
            json += "\"";
            // Upper sensor distance.
            json += ",\"upperDistance\":";
            json += String(_upperDistance);
            // Lower sensor distance.
            json += ",\"lowerDistance\":";
            json += String(_lowerDistance);
            // System status.
            json += ",\"status\":\"";
            json += _status;
            json += "\"";
            // Event log HTML.
            json += ",\"events\":\"";
            if (_eventCount == 0)
            {
                json += "<div class='no-events'>No events yet.</div>";
            }
            else
            {
                for (int i = _eventCount - 1; i >= 0; --i)
                {
                    json += "<div class='event'>";
                    json += _events[i];
                    json += "</div>";
                }
            }
            json += "\"}";
            request->send(200, "application/json", json);
        });
    // ========================================================
    //                      Start server
    // ========================================================
    _server.begin();
    Serial.println(F("[WEB] Local Web Server started."));
    Serial.print(F("[WEB] Dashboard: http://"));
    Serial.println(WiFi.softAPIP());
}