#ifndef REMOTEXY_CONFIG_H
#define REMOTEXY_CONFIG_H

#define REMOTEXY_MODE__WIFI_POINT
#include <WiFi.h>

#define REMOTEXY_WIFI_SSID "Robot_ESP32"
#define REMOTEXY_WIFI_PASSWORD "Robot1234"
#define REMOTEXY_SERVER_PORT 6377
#define REMOTEXY_ACCESS_PASSWORD "12345678910"

#include <RemoteXY.h>

// RemoteXY design matrix V19 with speed slider support.
#pragma pack(push, 1)
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =
  { 255,6,0,0,0,188,0,19,0,0,0,82,111,98,111,116,0,24,1,106,
  200,1,1,11,0,2,52,66,49,28,1,15,23,31,16,65,117,116,111,0,
  77,97,110,117,97,108,0,5,9,116,53,53,32,15,23,31,129,10,9,86,
  14,64,249,82,111,98,111,116,32,67,111,110,116,114,111,108,0,10,69,27,
  29,30,50,15,23,31,79,78,0,31,79,70,70,0,1,79,172,24,24,0,
  232,31,0,129,12,102,40,10,64,251,74,111,121,115,116,105,99,107,32,0,
  129,3,182,73,8,64,251,69,77,69,82,71,69,78,67,89,32,82,69,83,
  69,84,0,129,6,36,58,10,64,251,82,111,98,111,116,32,80,111,119,101,
  114,0,129,10,74,28,11,64,251,77,111,100,101,0,4,75,111,17,51,0,
  232,237,129,67,102,28,10,64,251,83,112,101,101,100,0 };

struct {
  uint8_t mode_switch;    // 0 = Manual, 1 = Auto
  int8_t joystick_01_x;   // -100 .. 100
  int8_t joystick_01_y;   // -100 .. 100
  uint8_t robot_power;    // 1 = ON, 0 = OFF
  uint8_t button_01;      // 1 = Emergency reset
  int8_t speed_slider;    // 0 .. 100

  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

#endif // REMOTEXY_CONFIG_H