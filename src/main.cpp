// main.cpp
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WiFi.h>

// libs without headers..i'm lazy
#include "DisplayManager.h"
#include "EnvironmentSensor.h"
#include "HomeP1Device.h"
#include "HomeSocketDevice.h"
#include "TimeSync.h"
#include "WebInterface.h"

// Configuration structure
struct Config
{
  String wifi_ssid;
  String wifi_password;
  String p1_ip;
  String socket_1;
  String socket_2;
  String socket_3;
  float power_on_threshold;
  float power_off_threshold;
  unsigned long min_on_time;
  unsigned long min_off_time;
  unsigned long max_on_time;
} config;

// Component instances
DisplayManager display;
EnvironmentSensors sensors;
HomeP1Device *p1Meter = nullptr;
HomeSocketDevice *socket1 = nullptr;
HomeSocketDevice *socket2 = nullptr;
HomeSocketDevice *socket3 = nullptr;
TimeSync timeSync;
WebInterface webServer;

// Timing variables
unsigned long lastStateChangeTime[3] = {0, 0, 0};
bool switchForceOff[3] = {false, false, false};

bool loadConfiguration()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount SPIFFS");
    return false;
  }

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();

  if (error)
  {
    Serial.println("Failed to parse config file");
    return false;
  }

  // Load configuration
  config.wifi_ssid = doc["wifi_ssid"].as<String>();
  config.wifi_password = doc["wifi_password"].as<String>();
  config.p1_ip = doc["p1_ip"].as<String>();
  config.socket_1 = doc["socket_1"].as<String>();
  config.socket_2 = doc["socket_2"].as<String>();
  config.socket_3 = doc["socket_3"].as<String>();
  config.power_on_threshold = doc["power_on_threshold"] | 1000.0f;
  config.power_off_threshold = doc["power_off_threshold"] | 990.0f;
  config.min_on_time = doc["min_on_time"] | 300UL;
  config.min_off_time = doc["min_off_time"] | 300UL;
  config.max_on_time = doc["max_on_time"] | 1800UL;

  return true;
}

void connectWiFi()
{
  Serial.println("Connecting to WiFi...");
  WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
  }
  else
  {
    Serial.println("\nWiFi connection failed!");
  }
}

bool canChangeState(int switchIndex, bool newState)
{
  unsigned long currentTime = millis();
  unsigned long timeSinceChange = currentTime - lastStateChangeTime[switchIndex];

  if (newState)
  { // Turning ON
    if (switchForceOff[switchIndex] && timeSinceChange < config.min_off_time)
    {
      return false;
    }
    switchForceOff[switchIndex] = false;
  }
  else
  { // Turning OFF
    if (timeSinceChange < config.min_on_time)
    {
      return false;
    }
  }

  return true;
}

void checkMaxOnTime()
{
  unsigned long currentTime = millis();

  for (int i = 0; i < 3; i++)
  {
    bool currentState = false;
    if (i == 0 && socket1)
      currentState = socket1->getCurrentState();
    if (i == 1 && socket2)
      currentState = socket2->getCurrentState();
    if (i == 2 && socket3)
      currentState = socket3->getCurrentState();

    if (currentState && (currentTime - lastStateChangeTime[i]) > config.max_on_time)
    {
      if (i == 0 && socket1)
        socket1->setState(false);
      if (i == 1 && socket2)
        socket2->setState(false);
      if (i == 2 && socket3)
        socket3->setState(false);
      switchForceOff[i] = true;
      lastStateChangeTime[i] = currentTime;
    }
  }
}

void updateSwitch1Logic()
{
  if (!socket1 || !p1Meter)
    return;

  float exportPower = p1Meter->getCurrentExport();
  bool currentState = socket1->getCurrentState();
  bool newState = currentState;

  if (exportPower > config.power_on_threshold && !currentState)
  {
    newState = true;
  }
  else if (exportPower < config.power_off_threshold && currentState)
  {
    newState = false;
  }

  if (newState != currentState && canChangeState(0, newState))
  {
    socket1->setState(newState);
    lastStateChangeTime[0] = millis();
  }
}

void updateSwitch2Logic()
{
  if (!socket2)
    return;

  int hour, minute;
  timeSync.getCurrentHourMinute(hour, minute);
  float light = sensors.getLightLevel();
  bool currentState = socket2->getCurrentState();
  bool newState = currentState;

  // After 17:45 and light < 75 lux
  if (hour >= 17 && minute >= 45 && light < 75)
  {
    newState = true;
  }
  else if (light >= 75)
  {
    newState = false;
  }

  if (newState != currentState && canChangeState(1, newState))
  {
    socket2->setState(newState);
    lastStateChangeTime[1] = millis();
  }
}

void updateSwitch3Logic()
{
  if (!socket3)
    return;

  int hour, minute;
  timeSync.getCurrentHourMinute(hour, minute);
  float light = sensors.getLightLevel();
  bool currentState = socket3->getCurrentState();
  bool newState = currentState;

  // After 17:30 and light < 50 lux
  if (hour >= 17 && minute >= 30 && light < 50)
  {
    newState = true;
  }
  else if (light >= 50)
  {
    newState = false;
  }

  if (newState != currentState && canChangeState(2, newState))
  {
    socket3->setState(newState);
    lastStateChangeTime[2] = millis();
  }
}

void updateDisplay()
{
  if (!p1Meter)
    return;

  display.updateDisplay(
      p1Meter->getCurrentImport(),
      p1Meter->getCurrentExport(),
      sensors.getTemperature(),
      sensors.getHumidity(),
      sensors.getLightLevel(),
      socket1 ? socket1->getCurrentState() : false,
      socket2 ? socket2->getCurrentState() : false,
      socket3 ? socket3->getCurrentState() : false,
      String(millis() - lastStateChangeTime[0]),
      String(millis() - lastStateChangeTime[1]),
      String(millis() - lastStateChangeTime[2]));
}

void setup()
{
  Serial.begin(115200);

  // Add these debug lines at the start
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    Serial.println("Trying to format SPIFFS...");
    if (SPIFFS.format())
    {
      Serial.println("SPIFFS formatted successfully");
      if (SPIFFS.begin(true))
      {
        Serial.println("SPIFFS mounted successfully after format");
      }
      else
      {
        Serial.println("SPIFFS mount failed even after format");
      }
    }
    else
    {
      Serial.println("SPIFFS format failed");
    }
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }

  // Load configuration
  if (!loadConfiguration())
  {
    Serial.println("Using default configuration");
  }

  // Initialize components with checks
  Wire.begin(); // Start I2C bus first

  // Try to initialize display
  if (display.begin())
  {
    Serial.println("Display initialized successfully");
  }
  else
  {
    Serial.println("Display not connected or initialization failed!");
    // Program can continue without display
  }

  // Try to initialize sensors
  if (sensors.begin())
  {
    Serial.println("Environmental sensors initialized successfully");
  }
  else
  {
    Serial.println("Environmental sensors not connected or initialization failed!");
    // Program can continue without sensors
  }

  connectWiFi();

  // Initialize network components after WiFi connection
  // Initialize network components after WiFi connection
  if (WiFi.status() == WL_CONNECTED)
  {
    // Debug prints for config values
    Serial.println("Config values:");
    Serial.println("P1 IP: " + config.p1_ip);
    Serial.println("Socket 1: " + config.socket_1);
    Serial.println("Socket 2: " + config.socket_2);
    Serial.println("Socket 3: " + config.socket_3);

    // Only initialize devices with valid IPs
    if (config.p1_ip != "" && config.p1_ip != "0" && config.p1_ip != "null")
    {
      p1Meter = new HomeP1Device(config.p1_ip.c_str());
      Serial.println("P1 Meter initialized at: " + config.p1_ip);
    }

    if (config.socket_1 != "" && config.socket_1 != "0" && config.socket_1 != "null")
    {
      socket1 = new HomeSocketDevice(config.socket_1.c_str());
      Serial.println("Socket 1 initialized at: " + config.socket_1);
    }

    if (config.socket_2 != "" && config.socket_2 != "0" && config.socket_2 != "null")
    {
      socket2 = new HomeSocketDevice(config.socket_2.c_str());
      Serial.println("Socket 2 initialized at: " + config.socket_2);
    }

    if (config.socket_3 != "" && config.socket_3 != "0" && config.socket_3 != "null")
    {
      socket3 = new HomeSocketDevice(config.socket_3.c_str());
      Serial.println("Socket 3 initialized at: " + config.socket_3);
    }

    timeSync.begin();
    webServer.begin();
  }

  // Initial state
  for (int i = 0; i < 3; i++)
  {
    lastStateChangeTime[i] = millis();
    switchForceOff[i] = false;
  }
}

void reconnectWiFi()
{
  static unsigned long lastAttempt = 0;
  const unsigned long RETRY_INTERVAL = 30000; // 30 seconds

  if (WiFi.status() != WL_CONNECTED &&
      (millis() - lastAttempt > RETRY_INTERVAL || lastAttempt == 0))
  {

    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());
    lastAttempt = millis();
  }
}

void loop()
{
  reconnectWiFi();
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    return;
  }
  // Update sensor readings
  sensors.update();

  // Reminder the arrow (->) is equivalent to:  float power = (*p1Meter).getCurrentExport(); as by pointer reference (multiple socket devices).
  if (p1Meter)
    p1Meter->update();
  if (socket1)
    socket1->update();
  if (socket2)
    socket2->update();
  if (socket3)
    socket3->update();

  // Update automation logic
  updateSwitch1Logic();
  updateSwitch2Logic();
  updateSwitch3Logic();
  checkMaxOnTime();

  // Update display and web interface
  updateDisplay();
  webServer.update(); // Changed from update() to handleClient()

  // Small delay to prevent excessive CPU usage
  delay(100);
}