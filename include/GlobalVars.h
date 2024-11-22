// GlobalVars.h
#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include "HomeP1Device.h"
#include "HomeSocketDevice.h"
#include "EnvironmentSensor.h"
#include "TimeSync.h"
#include "DisplayManager.h"
#include "NetworkCheck.h"

// External variable declarations
extern HomeP1Device *p1Meter;
extern HomeSocketDevice *socket1;
extern HomeSocketDevice *socket2;
extern HomeSocketDevice *socket3;
extern unsigned long lastStateChangeTime[3];
extern EnvironmentSensors sensors; // Make sure this matches your actual class name
extern DisplayManager display;
extern TimeSync timeSync;
extern NetworkCheck *phoneCheck;

// Config structure
struct Config
{
    String wifi_ssid;
    String wifi_password;
    String p1_ip;
    String socket_1;
    String socket_2;
    String socket_3;
    String phone_ip;
    float power_on_threshold;
    float power_off_threshold;
    unsigned long min_on_time;
    unsigned long min_off_time;
    unsigned long max_on_time;
};

extern Config config;

// Timing control structure
struct TimingControl
{
    const unsigned long ENV_SENSOR_INTERVAL = 30000;   // 30 seconds
    const unsigned long LIGHT_SENSOR_INTERVAL = 30000; // 30 seconds
    const unsigned long DISPLAY_INTERVAL = 1000;       // 1 second
    const unsigned long P1_INTERVAL = 1000;            // 1 second
    const unsigned long SOCKET_INTERVAL = 5000;        // 5 seconds
    const unsigned long WIFI_CHECK_INTERVAL = 30000;   // 30 seconds
    const unsigned long PHONE_CHECK_INTERVAL = 60000;  // 60 seconds

    unsigned long lastEnvSensorUpdate = 0;
    unsigned long lastLightSensorUpdate = 0;
    unsigned long lastDisplayUpdate = 0;
    unsigned long lastP1Update = 0;
    unsigned long lastSocket1Update = 0;
    unsigned long lastSocket2Update = 0;
    unsigned long lastSocket3Update = 0;
    unsigned long lastWiFiCheck = 0;
    unsigned long lastPhoneCheck = 0;
};

extern TimingControl timing;
#endif