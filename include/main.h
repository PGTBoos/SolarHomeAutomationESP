#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include "GlobalVars.h"
#include "DisplayManager.h"
#include "EnvironmentSensor.h"
#include "HomeP1Device.h"
#include "HomeSocketDevice.h"
#include "TimeSync.h"
#include "WebInterface.h"
#include "NetworkCheck.h"

extern HomeP1Device *p1Meter;
extern HomeSocketDevice *socket1;
extern HomeSocketDevice *socket2;
extern HomeSocketDevice *socket3;
extern unsigned long lastStateChangeTime[3];
extern NetworkCheck *phoneCheck;
// Timing control structure

unsigned long lastLightSensorUpdate;
unsigned long lastPhoneCheck;

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
extern HomeP1Device *p1Meter;
extern EnvironmentSensors sensors;