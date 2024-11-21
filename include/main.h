#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include "DisplayManager.h"
#include "EnvironmentSensor.h"
#include "HomeP1Device.h"
#include "HomeSocketDevice.h"
#include "TimeSync.h"
#include "WebInterface.h"
#include "NetworkCheck.h"

extern NetworkCheck *phoneCheck;
// Timing control structure
struct TimingControl
{
    unsigned long lastP1Update;
    unsigned long lastSocket1Update;
    unsigned long lastSocket2Update;
    unsigned long lastSocket3Update;
    unsigned long lastDisplayUpdate;
    unsigned long lastSensorUpdate;
    unsigned long lastWiFiCheck;
    unsigned long lastEnvSensorUpdate;
    unsigned long lastLightSensorUpdate;
    unsigned long lastPhoneCheck;

    // Update intervals
    const unsigned long P1_INTERVAL = 2000;           // 2 seconds
    const unsigned long SOCKET_INTERVAL = 5000;       // 5 seconds
    const unsigned long DISPLAY_INTERVAL = 1000;      // 1 second
    const unsigned long SENSOR_INTERVAL = 2000;       // 2 seconds
    const unsigned long TIME_SYNC_INTERVAL = 3600000; // 1 hour
    const unsigned long WIFI_CHECK_INTERVAL = 30000;  // 30 seconds
    const unsigned long ENV_SENSOR_INTERVAL = 5000;   // 5 seconds
    const unsigned long LIGHT_SENSOR_INTERVAL = 5000; // 5 seconds
    const unsigned long PHONE_CHECK_INTERVAL = 60000; // 1 minute
};

// Configuration structure
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

// Function declarations
bool loadConfiguration();
void connectWiFi();
bool canChangeState(int switchIndex, bool newState);
void checkMaxOnTime();
void updateSwitch1Logic();
void updateSwitch2Logic();
void updateSwitch3Logic();
void updateDisplay();
void setup();
void reconnectWiFi();
void loop();

// External variable declarations
extern TimingControl timing;
extern Config config;
extern DisplayManager display;
extern EnvironmentSensors sensors;
extern HomeP1Device *p1Meter;
extern HomeSocketDevice *socket1;
extern HomeSocketDevice *socket2;
extern HomeSocketDevice *socket3;
extern TimeSync timeSync;
extern WebInterface webServer;
extern unsigned long lastStateChangeTime[3];
extern bool switchForceOff[3];
extern unsigned long lastTimeDisplay;
