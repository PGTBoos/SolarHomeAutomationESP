
-------------------
Language: Cpp
# This keeps the opening brace on the same line for functions and control statements
BraceWrapping:
  AfterControlStatement: false
  AfterFunction: false
  BeforeCatch: false
  BeforeElse: false
BreakBeforeBraces: Attach
# Other common settings you might want:
IndentWidth: 2
TabWidth: 2
UseTab: Never
AllowShortFunctionsOnASingleLine: None
-------------------
// DisplayManager.cpp
#include "DisplayManager.h"
#include <GlobalVars.h>
#include <WiFi.h>
#include <timeSync.h>

// Assuming timeSync is a global or class member variable
bool DisplayManager::begin() {
  Serial.println("\nInitializing \nOLED display...");

  if (!display.begin()) {
    Serial.println("SH1106 allocation failed");
    return false;
  }

  // Setup display parameters
  display.setDisplayRotation(U8G2_R1);
  display.setFont(u8g2_font_profont10_tr); // Default font for labels
  display.setDrawColor(1);
  display.setFontPosTop();
  display.clearBuffer();

  // Draw initial test pattern
  display.drawStr(5, 8, "Starting...");
  display.drawFrame(0, 0, display.getWidth(), display.getHeight());
  display.sendBuffer();

  displayFound = true;
  Serial.println("Display initialized successfully!");
  return true;
}

void DisplayManager::showPowerPage(float importPower, float exportPower,
                                   float totalImport, float totalExport) {
  if (!displayFound)
    return;

  display.clearBuffer();

  // Title
  display.setFont(u8g2_font_profont10_tr);
  display.drawBox(0, 0, 64, 12);
  display.setDrawColor(0);
  display.drawStr(2, 2, "Power");
  display.setDrawColor(1);

  // Function to format power value
  auto formatPower = [](float power) -> String {
    if (abs(power) >= 1000) {
      // Display in kW with 2 decimals
      return String(power / 1000.0, 2) + " kW";
    } else {
      // Display in Watt with no decimals
      return String((int)power) + " Watt";
    }
  };

  // Import
  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 17, "Import:");
  display.setFont(u8g2_font_7x14_tr);
  display.setCursor(0, 27);
  display.print(formatPower(importPower));

  // Export
  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 45, "Export:");
  display.setFont(u8g2_font_7x14_tr);
  display.setCursor(0, 55);
  display.print(formatPower(exportPower));

  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 73, "Total:");

  // daily import and export
  float dailyImport = (config.yesterday > 0)
                          ? (totalImport - config.yesterdayImport)
                          : totalImport;
  float dailyExport = (config.yesterday > 0)
                          ? (totalExport - config.yesterdayExport)
                          : totalExport;

  // Daily import (used)
  display.setFont(u8g2_font_profont10_tr);
  String totalImportStr = "-" + formatPower(dailyImport);
  int totalWidth = display.getStrWidth(totalImportStr.c_str());
  display.setCursor(64 - totalWidth, 83);
  display.print(totalImportStr);

  // Daily export (produced)
  String totalExportStr = "+" + formatPower(dailyExport);
  totalWidth = display.getStrWidth(totalExportStr.c_str());
  display.setCursor(64 - totalWidth, 93);
  display.print(totalExportStr);

  display.sendBuffer();
}

void DisplayManager::showEnvironmentPage(float temp, float humidity,
                                         float light) {
  if (!displayFound)
    return;

  display.clearBuffer();

  // Title
  display.setFont(u8g2_font_profont10_tr);
  display.drawBox(0, 0, 64, 12);
  display.setDrawColor(0);
  display.drawStr(2, 2, "Environment");
  display.setDrawColor(1);

  // Temperature
  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 17, "Temperature:");
  display.setFont(u8g2_font_7x14_tr);
  display.setCursor(0, 27);
  display.print(temp, 1);
  display.print(" C'");

  // Humidity
  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 45, "Humidity:");
  display.setFont(u8g2_font_7x14_tr);
  display.setCursor(0, 55);
  display.print(humidity, 0);
  display.print(" %");

  // Light
  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 73, "Light:");
  display.setFont(u8g2_font_7x14_tr);
  display.setCursor(0, 83);
  display.print(light, 0);
  display.print(" Lux");

  display.sendBuffer();
}

void DisplayManager::showSwitchesPage(bool switch1, bool switch2, bool switch3,
                                      const String &sw1Time,
                                      const String &sw2Time,
                                      const String &sw3Time) {
  if (!displayFound)
    return;

  display.clearBuffer();

  // Title
  display.setFont(u8g2_font_profont10_tr);
  display.drawBox(0, 0, 64, 12);
  display.setDrawColor(0);
  display.drawStr(2, 2, "Switches");
  display.setDrawColor(1);

  // Function to draw switch status
  auto drawSwitch = [&](int y, const char *name, bool state) {
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, y, name);

    display.setFont(u8g2_font_7x14_tr);
    if (state) {
      display.drawBox(25, y - 1, 25, 14);
      display.setDrawColor(0);
      display.drawStr(27, y, "ON");
      display.setDrawColor(1);
    } else {
      display.drawStr(27, y, "OFF");
    }
  };

  // Draw all switches with more spacing due to larger font
  drawSwitch(20, "SW 1:", switch1);
  drawSwitch(40, "SW 2:", switch2);
  drawSwitch(60, "SW 3:", switch3);

  display.sendBuffer();
}

void DisplayManager::showInfoPage() {
  if (!displayFound)
    return;

  display.clearBuffer();

  // Title
  display.setFont(u8g2_font_profont10_tr);
  display.drawBox(0, 0, 64, 12);
  display.setDrawColor(0);
  display.drawStr(2, 2, "System Info");
  display.setDrawColor(1);

  // Time
  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 17, "Time:");
  display.setFont(u8g2_font_7x14_tr);
  display.setCursor(0, 27);
  display.print(timeSync.getCurrentTime()); // Get time directly from TimeSync

  // WiFi Status
  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 45, "WiFi:");
  display.setFont(u8g2_font_7x14_tr);
  display.setCursor(0, 55);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("Online");

    // IP Address with alternating backgrounds, no dots
    display.setFont(u8g2_font_profont10_tr); // Slightly larger than 4x6
    display.drawStr(0, 73, "IP:");
    IPAddress ip = WiFi.localIP();

    // Calculate positions with 5 pixels per character
    const int charWidth = 5;
    const int blockWidth = charWidth * 3 + 1; // Each block is 3 digits
    const int startX = 0;
    const int y = 83;

    // Draw background blocks
    for (int i = 0; i < 4; i++) {
      if (i % 2 == 1) { // Alternate blocks
        display.drawBox(startX + (i * blockWidth), y - 1, blockWidth, 10);
      }
    }

    // Print numbers
    display.setDrawColor(1); // Normal color for odd blocks
    display.setCursor(startX, y);
    display.printf("%03d", ip[0]);

    display.setDrawColor(0); // Inverted for even blocks
    display.setCursor(startX + blockWidth, y);
    display.printf("%03d", ip[1]);

    display.setDrawColor(1); // Back to normal
    display.setCursor(startX + (blockWidth * 2), y);
    display.printf("%03d", ip[2]);

    display.setDrawColor(0); // Inverted for last block
    display.setCursor(startX + (blockWidth * 3), y);
    display.printf("%03d", ip[3]);

    display.setDrawColor(1); // Reset to normal
  } else {
    display.print("Offline");
  }

  display.sendBuffer();
}

void DisplayManager::updateDisplay(float importPower, float exportPower,
                                   float totalImport, float totalExport,
                                   float temp, float humidity, float light,
                                   bool sw1, bool sw2, bool sw3,
                                   const String &sw1Time, const String &sw2Time,
                                   const String &sw3Time) {
  if (!displayFound)
    return;

  // Rotate pages every PAGE_DURATION milliseconds
  if (millis() - lastPageChange >= PAGE_DURATION) {
    currentPage = (currentPage + 1) % 4; // Cycle through 4 pages
    lastPageChange = millis();
  } else {
    return;
  }

  // Show current page
  switch (currentPage) {
  case 0:
    showPowerPage(importPower, exportPower, totalImport, totalExport);
    break;
  case 1:
    showEnvironmentPage(temp, humidity, light);
    break;
  case 2:
    showSwitchesPage(sw1, sw2, sw3, sw1Time, sw2Time, sw3Time);
    break;
  case 3:
    showInfoPage(); // Now using the parameter-less version
    break;
  }
}
-------------------
// EnvironmentSensors.cpp
#include "EnvironmentSensor.h"

bool EnvironmentSensors::begin()
{
    Wire.begin(); // Start I2C

    // Initialize BME280
    bmeFound = bme.begin(0x76); // Try first address
    if (!bmeFound)
    {
        bmeFound = bme.begin(0x77); // Try alternate address
    }
    if (!bmeFound)
    {
        Serial.println("Could not find BME280 sensor!");
    }

    // Initialize BH1750
    lightMeterFound = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    if (!lightMeterFound)
    {
        Serial.println("Could not find BH1750 sensor!");
    }
    else
    {
        lightMeter.setMTreg(64);     // Set measurement time to 400ms
        lightMeter.readLightLevel(); // Read once to start measurement

        Serial.println("BH1750 sensor found!");
    }

    return bmeFound || lightMeterFound; // Return true if at least one sensor works
}

void EnvironmentSensors::update()
{
    if (bmeFound)
    {
        temperature = bme.readTemperature();
        humidity = bme.readHumidity();
        pressure = bme.readPressure() / 100.0F; // Convert to hPa
    }

    if (lightMeterFound)
    {
        lightLevel = lightMeter.readLightLevel();
    }
}

// Getter methods
float EnvironmentSensors::getTemperature() const
{
    return temperature;
}

float EnvironmentSensors::getHumidity() const
{
    return humidity;
}

float EnvironmentSensors::getPressure() const
{
    return pressure;
}

float EnvironmentSensors::getLightLevel() const
{
    return lightLevel;
}

// Status methods
bool EnvironmentSensors::hasBME280() const
{
    return bmeFound;
}

bool EnvironmentSensors::hasBH1750() const
{
    return lightMeterFound;
}
-------------------
// HomeP1Device.cpp
#include "HomeP1Device.h"

HomeP1Device::HomeP1Device(const char *ip) : baseUrl("http://" + String(ip)),
                                             lastImportPower(0),
                                             lastExportPower(0),
                                             lastTotalImport(0),
                                             lastTotalExport(0),
                                             lastReadTime(0),
                                             lastReadSuccess(false)
{
}
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

void HomeP1Device::update()
{
    if (millis() - lastReadTime >= READ_INTERVAL)
    {
        lastReadSuccess = getPowerData(lastImportPower, lastExportPower);
        lastReadTime = millis();
    }
}

bool HomeP1Device::getPowerData(float &importPower, float &exportPower)
{
    http.begin(client, baseUrl + "/api/v1/data");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();
        StaticJsonDocument<2048> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
            float power = doc["active_power_w"].as<float>();
            lastTotalImport = doc["total_power_import_kwh"].as<float>();
            lastTotalExport = doc["total_power_export_kwh"].as<float>();

            Serial.printf("Received P1 power data: %.2f W\n", power);
            Serial.printf("Today total import: %.2f kWh\n", lastTotalImport);
            Serial.printf("Today total export: %.2f kWh\n", lastTotalExport);

            importPower = max(power, 0);
            exportPower = max(-power, 0);
            http.end();
            return true;
        }
    }
    http.end();
    return false;
}

float HomeP1Device::getCurrentImport() const
{
    return lastImportPower;
}

float HomeP1Device::getCurrentExport() const
{
    return lastExportPower;
}

float HomeP1Device::getTotalImport() const
{
    return lastTotalImport;
}

float HomeP1Device::getTotalExport() const
{
    return lastTotalExport;
}

float HomeP1Device::getNetPower() const
{
    return lastImportPower - lastExportPower;
}

bool HomeP1Device::isConnected() const
{
    return lastReadSuccess;
}
-------------------
#include "HomeSocketDevice.h"

HomeSocketDevice::HomeSocketDevice(const char *ip) : baseUrl("http://" + String(ip)),
                                                     lastKnownState(false),
                                                     lastReadTime(0),
                                                     lastReadSuccess(false),
                                                     consecutiveFailures(0),
                                                     deviceIP(ip),
                                                     lastLogTime(0)
{
    Serial.printf("Initializing socket device at IP: %s\n", ip);
}

void HomeSocketDevice::update()
{
    unsigned long currentTime = millis();

    // Only try updating if enough time has passed since last attempt
    if (currentTime - lastReadTime < READ_INTERVAL)
    {
        return;
    }

    // Calculate backoff time based on failures (max 60 seconds)
    unsigned long backoffTime = min(consecutiveFailures * 5000UL, 60000UL);
    if (currentTime - lastReadTime < backoffTime)
    {
        return;
    }

    // Regular status check
    if (!getState())
    {
        consecutiveFailures++;
        if (currentTime - lastLogTime >= 30000)
        {
            Serial.printf("PowerSocket > %s > Status > Offline (retry in %lu sec)\n",
                          deviceIP.c_str(),
                          backoffTime / 1000);
            lastLogTime = currentTime;
        }
    }
    else
    {
        if (consecutiveFailures > 0)
        {
            Serial.printf("PowerSocket > %s > Status > Back online\n", deviceIP.c_str());
            lastLogTime = currentTime;
        }
        consecutiveFailures = 0;
    }

    lastReadTime = currentTime;
}

bool HomeSocketDevice::makeHttpRequest(const String &endpoint, const String &method, const String &payload, String &response)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    WiFiClient newClient;
    HTTPClient http;
    newClient.setTimeout(5000);

    String fullUrl = baseUrl + "/api/v1/state";

    if (!http.begin(newClient, fullUrl))
    {
        return false;
    }

    http.setTimeout(5000);
    http.setReuse(false);

    int httpCode;
    if (method == "GET")
    {
        httpCode = http.GET();
    }
    else if (method == "PUT")
    {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.PUT(payload);
    }

    bool success = (httpCode == HTTP_CODE_OK);
    if (success)
    {
        response = http.getString();
    }

    http.end();
    return success;
}

// for raw output enable below code
// bool HomeSocketDevice::makeHttpRequest(const String &endpoint, const String &method, const String &payload, String &response)
// {
//     if (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.printf("Power socket > No WiFi\n");
//         return false;
//     }

//     WiFiClient newClient;
//     HTTPClient http;
//     newClient.setTimeout(5000);

//     String fullUrl = baseUrl + "/api/v1/state";

//     if (!http.begin(newClient, fullUrl))
//     {
//         Serial.printf("Power socket > %s > Connection failed\n", fullUrl.c_str());
//         return false;
//     }

//     http.setTimeout(5000);
//     http.setReuse(false);

//     int httpCode;
//     if (method == "GET")
//     {
//         httpCode = http.GET();
//     }
//     else if (method == "PUT")
//     {
//         http.addHeader("Content-Type", "application/json");
//         httpCode = http.PUT(payload);
//     }

//     if (httpCode == HTTP_CODE_OK)
//     {
//         response = http.getString();
//         Serial.printf("Power socket > %s > %d > %s\n", fullUrl.c_str(), httpCode, response.c_str());
//     }
//     else if (httpCode == -1)
//     {
//         Serial.printf("Power socket > %s > %d > disconnected\n", fullUrl.c_str(), httpCode);
//     }
//     else
//     {
//         Serial.printf("Power socket > %s > %d > HTTP error\n", fullUrl.c_str(), httpCode);
//     }

//     http.end();
//     return (httpCode == HTTP_CODE_OK);
// }

bool HomeSocketDevice::getState()
{
    String response;
    if (!makeHttpRequest("/api/v1/state", "GET", "", response))
    {
        Serial.printf("PowerSocket > %s/api/v1/state > Get > HTTP error\n", deviceIP.c_str());
        lastReadSuccess = false;
        return false;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error)
    {
        Serial.printf("PowerSocket > %s/api/v1/state > Get > JSON error\n", deviceIP.c_str());
        lastReadSuccess = false;
        return false;
    }

    lastKnownState = doc["power_on"] | false;
    Serial.printf("PowerSocket > %s/api/v1/state > Get > is %s\n",
                  deviceIP.c_str(),
                  lastKnownState ? "on" : "off");
    lastReadSuccess = true;
    return true;
}

bool HomeSocketDevice::setState(bool state)
{
    StaticJsonDocument<200> doc;
    doc["power_on"] = state;
    String jsonString;
    serializeJson(doc, jsonString);

    String response;
    if (!makeHttpRequest("/api/v1/state", "PUT", jsonString, response))
    {
        Serial.printf("PowerSocket > %s/api/v1/state > Put > HTTP error\n", deviceIP.c_str());
        return false;
    }

    lastKnownState = state;
    Serial.printf("PowerSocket > %s/api/v1/state > Put > turn %s\n",
                  deviceIP.c_str(),
                  state ? "on" : "off");
    return true;
}
-------------------
{
    "folders": [
		{
			"name": "HomeSystem",
			"path": ".."
		}
	]
}
-------------------
#include "main.h"

// Global variable definitions
TimingControl timing;
Config config;
DisplayManager display;
EnvironmentSensors sensors;
HomeP1Device *p1Meter = nullptr;
HomeSocketDevice *socket1 = nullptr;
HomeSocketDevice *socket2 = nullptr;
HomeSocketDevice *socket3 = nullptr;
TimeSync timeSync;
WebInterface webServer;
NetworkCheck *phoneCheck = nullptr;
unsigned long lastStateChangeTime[3] = {0, 0, 0};
bool switchForceOff[3] = {false, false, false};
unsigned long lastTimeDisplay = 0;

bool loadConfiguration() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return false;
  }

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();

  if (error) {
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
  // config.phone_ip = doc["phone_ip"].as<String>();

  return true;
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

bool canChangeState(int switchIndex, bool newState) {
  unsigned long currentTime = millis();
  unsigned long timeSinceChange =
      currentTime - lastStateChangeTime[switchIndex];

  if (newState) { // Turning ON
    if (switchForceOff[switchIndex] && timeSinceChange < config.min_off_time) {
      return false;
    }
    switchForceOff[switchIndex] = false;
  } else { // Turning OFF
    if (timeSinceChange < config.min_on_time) {
      return false;
    }
  }

  return true;
}

void checkMaxOnTime() {
  unsigned long currentTime = millis();

  for (int i = 0; i < 3; i++) {
    bool currentState = false;
    if (i == 0 && socket1)
      currentState = socket1->getCurrentState();
    if (i == 1 && socket2)
      currentState = socket2->getCurrentState();
    if (i == 2 && socket3)
      currentState = socket3->getCurrentState();

    if (currentState &&
        (currentTime - lastStateChangeTime[i]) > config.max_on_time) {
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

void updateSwitch1Logic() {
  if (!socket1 || !p1Meter)
    return;

  float exportPower = p1Meter->getCurrentExport();
  bool currentState = socket1->getCurrentState();
  bool newState = currentState;

  if (exportPower > config.power_on_threshold && !currentState) {
    newState = true;
  } else if (exportPower < config.power_off_threshold && currentState) {
    newState = false;
  }

  if (newState != currentState && canChangeState(0, newState)) {
    socket1->setState(newState);
    lastStateChangeTime[0] = millis();
  }
}

void updateSwitch2Logic() {
  if (!socket2)
    return;

  int hour, minute;
  timeSync.getCurrentHourMinute(hour, minute);
  float light = sensors.getLightLevel();
  bool currentState = socket2->getCurrentState();
  bool newState = currentState;

  // After 17:45 and light < 75 lux
  if (hour >= 17 && minute >= 45 && light < 75) {
    newState = true;
  } else if (light >= 75) {
    newState = false;
  }

  if (newState != currentState && canChangeState(1, newState)) {
    socket2->setState(newState);
    lastStateChangeTime[1] = millis();
  }
}

void updateSwitch3Logic() {
  if (!socket3)
    return;

  int hour, minute;
  timeSync.getCurrentHourMinute(hour, minute);
  float light = sensors.getLightLevel();
  bool currentState = socket3->getCurrentState();
  bool newState = currentState;

  // After 17:30 and light < 50 lux
  if (hour >= 17 && minute >= 30 && light < 50) {
    newState = true;
  } else if (light >= 50) {
    newState = false;
  }

  if (newState != currentState && canChangeState(2, newState)) {
    socket3->setState(newState);
    lastStateChangeTime[2] = millis();
  }
}

void updateDisplay() {
  if (!p1Meter)
    return;
  unsigned long currentMillis = millis();
  if (currentMillis - lastTimeDisplay >= 1000) {
    int hour, minute;
    timeSync.getCurrentHourMinute(hour, minute);
    Serial.printf("Current time: %02d:%02d\n", hour, minute);
    lastTimeDisplay = currentMillis;
  }

  // Calculate time differences
  String sw1Time = String((unsigned long)(millis() - lastStateChangeTime[0]));
  String sw2Time = String((unsigned long)(millis() - lastStateChangeTime[1]));
  String sw3Time = String((unsigned long)(millis() - lastStateChangeTime[2]));

  display.updateDisplay(p1Meter->getCurrentImport(),
                        p1Meter->getCurrentExport(), p1Meter->getTotalImport(),
                        p1Meter->getTotalExport(), sensors.getTemperature(),
                        sensors.getHumidity(), sensors.getLightLevel(),
                        socket1 ? socket1->getCurrentState() : false,
                        socket2 ? socket2->getCurrentState() : false,
                        socket3 ? socket3->getCurrentState() : false,
                        String(millis() - lastStateChangeTime[0]),
                        String(millis() - lastStateChangeTime[1]),
                        String(millis() - lastStateChangeTime[2]));
}

void setup() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  Serial.begin(115200);

  if (config.phone_ip != "" && config.phone_ip != "0" &&
      config.phone_ip != "null") {
    phoneCheck = new NetworkCheck(config.phone_ip.c_str());
    Serial.println("Phone check initialized at: " + config.phone_ip);
  }

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    Serial.println("Trying to format SPIFFS...");
    if (SPIFFS.format()) {
      Serial.println("SPIFFS formatted successfully");
      if (SPIFFS.begin(true)) {
        Serial.println("SPIFFS mounted successfully after format");
      } else {
        Serial.println("SPIFFS mount failed even after format");
      }
    } else {
      Serial.println("SPIFFS format failed");
    }
  } else {
    Serial.println("SPIFFS mounted successfully");
  }

  if (!loadConfiguration()) {
    Serial.println("Using default configuration");
  }

  Wire.setClock(100000);
  Wire.begin();

  if (display.begin()) {
    Serial.println("Display initialized successfully");
  } else {
    Serial.println("Display not connected or initialization failed!");
  }

  if (sensors.begin()) {
    Serial.println("Environmental sensors initialized successfully");
  } else {
    Serial.println(
        "Environmental sensors not connected or initialization failed!");
  }

  connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Config values:");
    Serial.println("P1 IP: " + config.p1_ip);
    Serial.println("Socket 1: " + config.socket_1);
    Serial.println("Socket 2: " + config.socket_2);
    Serial.println("Socket 3: " + config.socket_3);
    Serial.println("Phone IP:" + config.phone_ip);

    if (config.p1_ip != "" && config.p1_ip != "0" && config.p1_ip != "null") {
      p1Meter = new HomeP1Device(config.p1_ip.c_str());
      Serial.println("P1 Meter initialized at: " + config.p1_ip);
    }

    if (config.socket_1 != "" && config.socket_1 != "0" &&
        config.socket_1 != "null") {
      socket1 = new HomeSocketDevice(config.socket_1.c_str());
      Serial.println("Socket 1 initialized at: " + config.socket_1);
    }

    if (config.socket_2 != "" && config.socket_2 != "0" &&
        config.socket_2 != "null") {
      socket2 = new HomeSocketDevice(config.socket_2.c_str());
      Serial.println("Socket 2 initialized at: " + config.socket_2);
    }

    if (config.socket_3 != "" && config.socket_3 != "0" &&
        config.socket_3 != "null") {
      socket3 = new HomeSocketDevice(config.socket_3.c_str());
      Serial.println("Socket 3 initialized at: " + config.socket_3);
    }

    timeSync.begin();
    webServer.begin();
  }

  // Initialize timing and state
  unsigned long startTime = millis();
}

void reconnectWiFi() {
  unsigned long currentMillis = millis();

  if (WiFi.status() != WL_CONNECTED &&
      (currentMillis - timing.lastWiFiCheck >= timing.WIFI_CHECK_INTERVAL ||
       timing.lastWiFiCheck == 0)) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());
    timing.lastWiFiCheck = currentMillis;
  }
}

static int yesterday;
static uint8_t operationOrder = 0;

void loop() {
  unsigned long currentMillis = millis();

  // Use static counter to sequence ALL operations

  File file;

  switch (operationOrder) {
  case 0:
    file = SPIFFS.open("/daily_totals.json", "r");
    if (file) {
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, file);
      file.close();

      if (!error) {
        config.yesterday = doc["day"] | 0;
        config.yesterdayImport = doc["import"] | 0.0f;
        config.yesterdayExport = doc["export"] | 0.0f;

        Serial.println("\nLoaded previous day totals:");
        Serial.printf("Day: %d\n", config.yesterday);
        Serial.printf("Import: %.2f kWh\n", config.yesterdayImport);
        Serial.printf("Export: %.2f kWh\n", config.yesterdayExport);
      } else {
        Serial.println("Error parsing daily totals file");
        config.yesterday = 0;
        config.yesterdayImport = 0;
        config.yesterdayExport = 0;
      }
    } else {
      Serial.println("No previous day totals found");
      config.yesterday = 0;
      config.yesterdayImport = 0;
      config.yesterdayExport = 0;
    }
    operationOrder = 5;
    break;

  case 5: // Environmental sensor (I2C) - Temperature/Humidity
    if (currentMillis - timing.lastEnvSensorUpdate >=
        timing.ENV_SENSOR_INTERVAL) {
      sensors.update(); // Assuming this method exists, if not we use
                        // sensors.update()
      timing.lastEnvSensorUpdate = currentMillis;
      operationOrder = 10;
      yield();
      delay(1);
    }
    break;

  case 10:
    if (WiFi.status() != WL_CONNECTED) {
      reconnectWiFi();
      yield();
      delay(8000);
      reconnectWiFi();
      // operationOrder = 10; // Go back to start if no WiFi
    } else {
      operationOrder = 12;
    }
    break;

  case 12: // Light sensor (I2C)
    if (currentMillis - timing.lastLightSensorUpdate >=
        timing.LIGHT_SENSOR_INTERVAL) {
      sensors.update(); // Assuming this method exists, if not we use
                        // sensors.update()
      timing.lastLightSensorUpdate = currentMillis;
      operationOrder = 20;
      yield();
      delay(1);
    }
    break;

  case 20: // Display update (I2C)
    if (currentMillis - timing.lastDisplayUpdate >= timing.DISPLAY_INTERVAL) {
      updateDisplay();
      timing.lastDisplayUpdate = currentMillis;
      operationOrder = 30;
      yield();
      delay(1);
    }
    break;

  case 30: // P1 meter (Network)
    if (p1Meter &&
        (currentMillis - timing.lastP1Update >= timing.P1_INTERVAL)) {
      p1Meter->update();
      timing.lastP1Update = currentMillis;
      operationOrder = 40;
      yield();
      delay(50);
    } else {
      operationOrder = 40;
    }
    break;

  case 40: // Socket 1 (Network)
    if (socket1 &&
        (currentMillis - timing.lastSocket1Update >= timing.SOCKET_INTERVAL)) {
      socket1->update();
      timing.lastSocket1Update = currentMillis;
      if (p1Meter) {
        updateSwitch1Logic();
      }
      operationOrder = 50;
      yield();
      delay(50);
    } else {
      operationOrder = 50;
    }
    break;

  case 50: // Socket 2 (Network)
    if (socket2 &&
        (currentMillis - timing.lastSocket2Update >= timing.SOCKET_INTERVAL)) {
      socket2->update();
      timing.lastSocket2Update = currentMillis;
      updateSwitch2Logic();
      operationOrder = 60;
      yield();
      delay(50);
    } else {
      operationOrder = 60;
    }
    break;

  case 60: // Socket 3 (Network)
    if (socket3 &&
        (currentMillis - timing.lastSocket3Update >= timing.SOCKET_INTERVAL)) {
      socket3->update();
      timing.lastSocket3Update = currentMillis;
      updateSwitch3Logic();
      operationOrder = 70;
      yield();
      delay(50);
    } else {
      operationOrder = 70;
    }
    break;

  case 70: // Max on time check (no I2C or network)
    checkMaxOnTime();
    operationOrder = 80;
    break;

  case 80: // Web server (Network)
    webServer.update();
    operationOrder = 90; // Back to start
    yield();
    break;

  case 90: // Phone presence check
    if (phoneCheck && (currentMillis - timing.lastPhoneCheck >=
                       timing.PHONE_CHECK_INTERVAL)) {
      if (phoneCheck->isDevicePresent()) {
        Serial.println("Phone is detected");
        // Add your logic for when phone is present
      } else {
        Serial.println("Phone is not detected");
        // Add your logic for when phone is absent
      }
      timing.lastPhoneCheck = currentMillis;
      operationOrder = 100;
      yield();
      delay(50); // Give some time between network operations
    } else {
      operationOrder = 100;
    }
    break;

  case 100: {
    if (!p1Meter) {
      operationOrder = 5;
      break;
    }

    static int lastSavedDay = config.yesterday; // Keep track of last saved day
    int currentDay = timeSync.getTime().dayOfYear;

    // Only save if it's a new day and we haven't saved for this day yet
    if (currentDay != lastSavedDay && timeSync.getTime().hour == 0 &&
        timeSync.getTime().minute == 0) {
      StaticJsonDocument<128> doc;
      doc["day"] = currentDay;
      doc["import"] = p1Meter->getTotalImport();
      doc["export"] = p1Meter->getTotalExport();

      File file = SPIFFS.open("/daily_totals.json", "w");
      if (file) {
        serializeJson(doc, file);
        file.close();
        Serial.printf("Saved day %d totals to SPIFFS:\n", currentDay);
        Serial.printf("Import: %.2f kWh\n", doc["import"].as<float>());
        Serial.printf("Export: %.2f kWh\n", doc["export"].as<float>());

        // Update config values and lastSavedDay
        config.yesterday = currentDay;
        config.yesterdayImport = doc["import"].as<float>();
        config.yesterdayExport = doc["export"].as<float>();
        lastSavedDay = currentDay;
      } else {
        Serial.println("Failed to open daily_totals.json for writing");
      }
    }
    operationOrder = 5;
    break;
  }
  default:
    Serial.printf("ERROR: Invalid operation order: %d\n", operationOrder);
    operationOrder = 5; // Reset to beginning
    break;
  }
}
-------------------
**Some C++ reminders of the C++ syntax and concepts used in the code:**
-----

1. **Pointers and Dynamic Allocation**:
```cpp
HomeP1Device* p1Meter = nullptr;  // Pointer declaration, initialized to null
p1Meter = new HomeP1Device(ip);   // Dynamic allocation with 'new'
```

2. **References and Value Parameters**:
```cpp
// Example from timing functions
void updateDisplay() {
    String& timeString;     // Reference (uses &)
    float value = p1Meter->getCurrentExport();  // Value copy
}
```

3. **Struct Definition**:
```cpp
struct Config {
    String wifi_ssid;      // Member variables
    String wifi_password;
    float power_on_threshold;
    // etc...
} config;  // Direct instance creation
```

4. **Null Checks and Short-Circuit Evaluation**:
```cpp
if (!socket1 || !p1Meter) return;  // Short-circuit OR operator
if (newState != currentState && canChangeState(0, newState)) {  // Short-circuit AND
```

5. **Ternary Operator**:
```cpp
// Example from display updates
socket1 ? socket1->getCurrentState() : false;  // Conditional ? true_value : false_value
```

6. **Scope Resolution Operator (::)**:
```cpp
WiFi.status() == WL_CONNECTED  // Class::constant or Class::method
```

7. **String to C-String Conversion**:
```cpp
config.wifi_ssid.c_str()  // Convert String to const char* for C functions
```

8. **Constant Values**:
```cpp
const unsigned long MIN_ON_TIME = 300;  // Type-specific constants
```

9. **Member Function Calls**:
```cpp
sensors.update();         // Direct member function call with .
p1Meter->getCurrentExport();  // Pointer member function call with ->
```

10. **Type Casting and Default Values**:
```cpp
config.min_on_time = doc["min_on_time"] | 300UL;  // UL suffix for unsigned long
```

11. **Array Syntax**:
```cpp
unsigned long lastStateChangeTime[3] = {0, 0, 0};  // Array initialization
```

12. **Prefix vs Postfix Operators**:
```cpp
attempts++;  // Postfix increment
++attempts;  // Prefix increment (slightly more efficient)
```

Would you like me to explain any of these concepts in more detail? Or would you like to see more examples of specific C++ features?

-------------------
// NetworkCheck.cpp
#include "NetworkCheck.h"

NetworkCheck::NetworkCheck(const char *ip) : deviceIP(ip),
                                             lastKnownState(false),
                                             lastCheckTime(0),
                                             consecutiveFailures(0)
{
    Serial.printf("Network > %s > Check initialized\n", ip);
}

bool NetworkCheck::isDevicePresent()
{
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime < CHECK_INTERVAL)
    {
        return lastKnownState;
    }

    lastCheckTime = currentTime;
    bool pingResult = pingDevice();

    if (pingResult)
    {
        if (!lastKnownState)
        { // Device just became available
            Serial.printf("Network > %s > Device detected\n", deviceIP.c_str());
        }
        consecutiveFailures = 0;
    }
    else
    {
        consecutiveFailures++;
        if (lastKnownState)
        { // Device just became unavailable
            Serial.printf("Network > %s > Device lost\n", deviceIP.c_str());
        }
    }

    lastKnownState = pingResult;
    return pingResult;
}

bool NetworkCheck::pingDevice()
{
    bool success = Ping.ping(deviceIP.c_str(), 1); // 1 ping attempt
    if (success)
    {
        Serial.printf("Network > %s > Ping response: %.2fms\n",
                      deviceIP.c_str(),
                      Ping.averageTime());
    }
    return success;
}
-------------------
// SimpleRuleEngine.cpp
#include "RulesEngine.h"

HomeSocketDevice *SimpleRuleEngine::getSocket(int socket_number)
{
    if (socket_number < 1 || socket_number > MAX_SOCKETS)
    {
        Serial.printf("Invalid socket number: %d\n", socket_number);
        return nullptr;
    }
    return sockets[socket_number - 1];
}

int SimpleRuleEngine::TurnUntil(int memoryIndex, int turnOnCondition, int turnOffCondition)
{
    if (memoryIndex < 0 || memoryIndex >= MEMORY_SLOTS)
    {
        Serial.printf("Memory slot %d out of range!\n", memoryIndex);
        return 0;
    }

    if (!memory[memoryIndex] && turnOnCondition)
    {
        memory[memoryIndex] = true;
    }
    else if (memory[memoryIndex] && turnOffCondition)
    {
        memory[memoryIndex] = false;
    }

    return memory[memoryIndex] ? 1 : 0;
}

void SimpleRuleEngine::updateLightLevel()
{
    current_lux = sensors.getLightLevel();
    Serial.printf("Current light level: %.1f lux\n", current_lux);
}

int SimpleRuleEngine::lightSensorAbove(int lux_value)
{
    int result = (current_lux > lux_value) ? 1 : 0;
    Serial.printf("Light > %d lux: %s\n", lux_value, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::lightSensorBelow(int lux_value)
{
    int result = (current_lux < lux_value) ? 1 : 0;
    Serial.printf("Light < %d lux: %s\n", lux_value, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::after(const char *timeStr)
{
    int hour, minute;
    sscanf(timeStr, "%d:%d", &hour, &minute);

    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);

    int currentMins = currentHour * 60 + currentMinute;
    int targetMins = hour * 60 + minute;

    int result = (currentMins >= targetMins) ? 1 : 0;
    Serial.printf("After %s: %s\n", timeStr, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::before(const char *timeStr)
{
    int hour, minute;
    sscanf(timeStr, "%d:%d", &hour, &minute);

    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);

    int currentMins = currentHour * 60 + currentMinute;
    int targetMins = hour * 60 + minute;

    int result = (currentMins < targetMins) ? 1 : 0;
    Serial.printf("Before %s: %s\n", timeStr, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::OR(int func1, int func2)
{
    int result = ((func1 > 0) || (func2 > 0)) ? 1 : 0;
    Serial.printf("OR operation: %d OR %d = %d\n", func1, func2, result);
    return result;
}

int SimpleRuleEngine::AND(int func1, int func2)
{
    int result = ((func1 > 0) && (func2 > 0)) ? 1 : 0;
    Serial.printf("AND operation: %d AND %d = %d\n", func1, func2, result);
    return result;
}

int SimpleRuleEngine::NOT(int func)
{
    int result = (func <= 0) ? 1 : 0;
    Serial.printf("NOT operation: NOT %d = %d\n", func, result);
    return result;
}

int SimpleRuleEngine::turnOnInbetween(const char *startTime, const char *endTime)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);
    int currentMinutes = currentHour * 60 + currentMinute;

    // Parse start time
    int startHours, startMinutes;
    sscanf(startTime, "%d:%d", &startHours, &startMinutes);
    int startTotalMinutes = startHours * 60 + startMinutes;

    // Parse end time
    int endHours, endMinutes;
    sscanf(endTime, "%d:%d", &endHours, &endMinutes);
    int endTotalMinutes = endHours * 60 + endMinutes;

    int result;
    // Handle cases where the period crosses midnight
    if (startTotalMinutes <= endTotalMinutes)
    {
        // Normal case (e.g., 21:00 to 23:00)
        result = (currentMinutes >= startTotalMinutes && currentMinutes < endTotalMinutes) ? 1 : 0;
    }
    else
    {
        // Crosses midnight (e.g., 23:00 to 06:00)
        result = (currentMinutes >= startTotalMinutes || currentMinutes < endTotalMinutes) ? 1 : 0;
    }

    Serial.printf("Time between %s and %s: %s\n", startTime, endTime, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::turnOnBefore(const char *timeStr)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);
    int currentMinutes = currentHour * 60 + currentMinute;

    // Parse input time string (format: "HH:MM")
    int hours, minutes;
    sscanf(timeStr, "%d:%d", &hours, &minutes);
    int targetMinutes = hours * 60 + minutes;

    int result = (currentMinutes < targetMinutes) ? 1 : 0;
    Serial.printf("Turn on before %s: %s\n", timeStr, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::turnOnAfter(const char *startTime)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);
    int currentMinutes = currentHour * 60 + currentMinute;

    // Parse start time
    int startHours, startMinutes;
    sscanf(startTime, "%d:%d", &startHours, &startMinutes);
    int startTotalMinutes = startHours * 60 + startMinutes;

    int result = (currentMinutes >= startTotalMinutes) ? 1 : 0;
    Serial.printf("Turn on after %s: %s\n", startTime, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::turnOffAfter(const char *startTime)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);
    int currentMinutes = currentHour * 60 + currentMinute;

    // Parse start time
    int startHours, startMinutes;
    sscanf(startTime, "%d:%d", &startHours, &startMinutes);
    int startTotalMinutes = startHours * 60 + startMinutes;

    int result = (currentMinutes < startTotalMinutes) ? 1 : 0; // Return 1 BEFORE the time, 0 after
    Serial.printf("Turn off after %s: %s\n", startTime, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::isWeekday(uint8_t dayPattern)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);

    // Get current weekday (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    int today = timeinfo.tm_wday; // 0-6, Sunday=0

    // Convert weekday to our bit pattern (1 << 0 for Sunday, 1 << 1 for Monday, etc)
    uint8_t todayBit = (1 << today);

    int result = (dayPattern & todayBit) ? 1 : 0;
    Serial.printf("Day check (pattern: 0x%02X): %s\n", dayPattern, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::setMem(int slot, unsigned long value)
{
    if (slot < 0 || slot >= MEMORY_SLOTS)
    {
        Serial.printf("Memory slot %d out of range!\n", slot);
        return 0;
    }

    memory[slot] = value;
    Serial.printf("Set memory slot %d to %lu\n", slot, value);
    return 1;
}

int SimpleRuleEngine::readMem(int slot)
{
    if (slot < 0 || slot >= MEMORY_SLOTS)
    {
        Serial.printf("Memory slot %d out of range!\n", slot);
        return 0;
    }

    unsigned long value = memory[slot];
    Serial.printf("Read memory slot %d: %lu\n", slot, value);
    return value;
}

int SimpleRuleEngine::Delay(int memSlot, int triggerFunction)
{
    unsigned long currentMillis = millis();

    // If trigger function is true and timer isn't running
    if (triggerFunction && readMem(memSlot) == 0)
    {
        setMem(memSlot, currentMillis);
        Serial.printf("Starting delay in slot %d\n", memSlot);
        return 0;
    }

    // If timer is running
    if (readMem(memSlot) > 0)
    {
        const unsigned long DELAY_PERIOD = 5 * 60 * 1000; // 5 minutes in milliseconds

        if (currentMillis - readMem(memSlot) >= DELAY_PERIOD)
        {
            setMem(memSlot, 0); // Reset timer
            Serial.printf("Delay completed in slot %d\n", memSlot);
            return 1;
        }
        Serial.printf("Delay still running in slot %d\n", memSlot);
        return 0;
    }

    return 0;
}

int SimpleRuleEngine::phoneFound()
{
    if (phoneCheck)
    {
        int result = phoneCheck->isDevicePresent() ? 1 : 0;
        Serial.printf("Phone present: %s\n", result ? "true" : "false");
        return result;
    }
    Serial.println("Phone check not configured");
    return 0;
}

int SimpleRuleEngine::phoneNotFound()
{
    int result = 1 - phoneFound();
    Serial.printf("Phone absent: %s\n", result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::isOn(int socket_number)
{
    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket)
        return 0;

    updateSocketDuration(socket_number);
    return socket->getCurrentState() ? 1 : 0;
}

int SimpleRuleEngine::isOff(int socket_number)
{
    return 1 - isOn(socket_number);
}

void SimpleRuleEngine::updateSocketDuration(int socket_number)
{
    if (socket_number < 1 || socket_number > MAX_SOCKETS)
        return;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    // Get current time
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);

    // If state changed, update timestamps
    HomeSocketDevice *socket = getSocket(socket_number);
    if (socket && socket->getCurrentState() != state.currentState)
    {
        state.currentState = socket->getCurrentState();
        state.lastStateChange = millis();
        state.lastChangeHour = currentHour;
        state.lastChangeMinute = currentMinute;
        state.stateChangeProcessed = false;

        Serial.printf("Socket %d state changed to: %s at %02d:%02d\n",
                      socket_number, state.currentState ? "ON" : "OFF",
                      currentHour, currentMinute);
    }
}

void SimpleRuleEngine::turnOn(int socket_number, int condition)
{
    Serial.printf("\nEvaluating ON rule for socket %d\n", socket_number);
    Serial.printf("Condition result: %s\n", condition ? "true" : "false");

    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket)
        return;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    // Only turn on if not already on and condition is true
    if (condition && (!socket->getCurrentState() || !state.stateChangeProcessed))
    {
        Serial.printf("â†’ Turning ON socket %d\n", socket_number);
        socket->setState(true);
        state.stateChangeProcessed = true;
    }
    else
    {
        Serial.printf("â†’ No action for socket %d\n", socket_number);
    }

    updateSocketDuration(socket_number);
    Serial.println("----------------------------------------");
}

void SimpleRuleEngine::turnOff(int socket_number, int condition)
{
    Serial.printf("\nEvaluating OFF rule for socket %d\n", socket_number);
    Serial.printf("Condition result: %s\n", condition ? "true" : "false");

    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket)
        return;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    // Only turn off if not already off and condition is true
    if (condition && (socket->getCurrentState() || !state.stateChangeProcessed))
    {
        Serial.printf("â†’ Turning OFF socket %d\n", socket_number);
        socket->setState(false);
        state.stateChangeProcessed = true;
    }
    else
    {
        Serial.printf("â†’ No action for socket %d\n", socket_number);
    }

    updateSocketDuration(socket_number);
    Serial.println("----------------------------------------");
}

int SimpleRuleEngine::hasBeenOnFor(int socket_number, int minutes)
{
    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket || !socket->getCurrentState())
        return 0;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    unsigned long duration = (millis() - state.lastStateChange) / (60 * 1000);
    int result = (duration >= minutes) ? 1 : 0;

    Serial.printf("Socket %d has been ON for %lu minutes (target: %d): %s\n",
                  socket_number, duration, minutes, result ? "true" : "false");

    return result;
}

int SimpleRuleEngine::hasBeenOffFor(int socket_number, int minutes)
{
    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket || socket->getCurrentState())
        return 0;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    unsigned long duration = (millis() - state.lastStateChange) / (60 * 1000);
    int result = (duration >= minutes) ? 1 : 0;

    Serial.printf("Socket %d has been OFF for %lu minutes (target: %d): %s\n",
                  socket_number, duration, minutes, result ? "true" : "false");

    return result;
}
-------------------
// TimeSync.cpp
#include "TimeSync.h"

bool TimeSync::begin()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected - cannot sync time");
        return false;
    }

    // Configure NTP with Dutch servers
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2, ntpServer3);

    Serial.println("Attempting to sync with Dutch NTP servers...");

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 20;  // Number of retries
    const int retry_delay = 500; // ms between retries

    while (!getLocalTime(&timeinfo) && ++retry < retry_count)
    {
        Serial.printf("NTP Sync attempt %d/%d\n", retry, retry_count);
        if (retry == 5)
        {
            Serial.println("Initial NTP servers not responding, trying backup servers...");
        }
        delay(retry_delay);
    }

    if (getLocalTime(&timeinfo))
    {
        char time_str[25];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
        Serial.println("âœ“ Time synchronized successfully!");
        Serial.printf("Current time: %s\n", time_str);
        Serial.printf("Timezone: UTC+%d\n", (gmtOffset_sec + daylightOffset_sec) / 3600);
        timeInitialized = true;
        return true;
    }
    else
    {
        Serial.println("Ã— Failed to sync time after multiple attempts");
        Serial.println("Diagnostic information:");
        Serial.printf("WiFi status: %d\n", WiFi.status());
        Serial.printf("WiFi SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("WiFi IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.println("Please check:");
        Serial.println("1. WiFi connection is stable");
        Serial.println("2. NTP ports (123 UDP) aren't blocked");
        Serial.println("3. DNS resolution is working");
        timeInitialized = false;
        return false;
    }
}

void TimeSync::getCurrentHourMinute(int &hour, int &minute)
{
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
        hour = timeinfo.tm_hour;
        minute = timeinfo.tm_min;
        Serial.printf("Current time: %02d:%02d\n", hour, minute);
    }
    else
    {
        hour = 12;
        minute = 0;
        Serial.println("âš  Could not get current time, using default (12:00)");
        timeInitialized = false; // Mark time as not synchronized
    }
}

String TimeSync::getCurrentTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("âš  Failed to obtain time");
        timeInitialized = false; // Mark time as not synchronized
        return "Time not set";
    }

    char timeString[9];
    strftime(timeString, 9, "%H:%M:%S", &timeinfo);
    return String(timeString);
}

bool TimeSync::isTimeBetween(const char *startTime, const char *endTime)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("âš  Failed to get time for comparison");
        timeInitialized = false; // Mark time as not synchronized
        return false;
    }

    int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    // Parse start time (format "HH:MM")
    int startHour, startMin;
    sscanf(startTime, "%d:%d", &startHour, &startMin);
    int startMinutes = startHour * 60 + startMin;

    // Parse end time
    int endHour, endMin;
    sscanf(endTime, "%d:%d", &endHour, &endMin);
    int endMinutes = endHour * 60 + endMin;

    // Debug time information
    Serial.printf("Time check - Current: %02d:%02d (%d min), ",
                  timeinfo.tm_hour, timeinfo.tm_min, currentMinutes);
    Serial.printf("Start: %02d:%02d (%d min), ",
                  startHour, startMin, startMinutes);
    Serial.printf("End: %02d:%02d (%d min)\n",
                  endHour, endMin, endMinutes);

    if (endMinutes < startMinutes)
    { // Handles overnight periods
        bool isInRange = currentMinutes >= startMinutes || currentMinutes <= endMinutes;
        Serial.printf("Overnight period check: %s\n", isInRange ? "true" : "false");
        return isInRange;
    }

    bool isInRange = currentMinutes >= startMinutes && currentMinutes <= endMinutes;
    Serial.printf("Same-day period check: %s\n", isInRange ? "true" : "false");
    return isInRange;
}

int TimeSync::getCurrentMinutes()
{
    int hour, minute;
    getCurrentHourMinute(hour, minute);
    return hour * 60 + minute;
}

TimeSync::TimeData TimeSync::getTime()
{
    TimeData t = {0};
    struct tm timeinfo;

    if (getLocalTime(&timeinfo))
    {
        t.year = timeinfo.tm_year + 1900;
        t.month = timeinfo.tm_mon + 1;
        // Convert to 1-7 where Monday=1 and Sunday=7
        t.dayOfWeek = timeinfo.tm_wday == 0 ? 7 : timeinfo.tm_wday;
        t.hour = timeinfo.tm_hour;
        t.minute = timeinfo.tm_min;
        t.weekNum = ((timeinfo.tm_yday + 7 - timeinfo.tm_wday) / 7) + 1;
        // day of the year
        t.dayOfYear = timeinfo.tm_yday;
    }
    else
    {
        Serial.println("Failed to get time");
    }
    return t;
}
-------------------
// WebServer.cpp
#include "WebInterface.h"

String WebInterface::getContentType(const String &path)
{
    if (path.endsWith(".html"))
        return "text/html";
    else if (path.endsWith(".css"))
        return "text/css";
    else if (path.endsWith(".js"))
        return "application/javascript";
    else if (path.endsWith(".json"))
        return "application/json";
    else if (path.endsWith(".ico"))
        return "image/x-icon";
    return "text/plain";
}

bool WebInterface::serveFromCache(const String &path)
{
    for (int i = 0; i < MAX_CACHED_FILES; i++)
    {
        if (cachedFiles[i].path == path && cachedFiles[i].data != nullptr)
        {
            Serial.printf("Web > Serving %s from RAM cache\n", path.c_str());
            server.sendHeader("Content-Type", getContentType(path));
            server.sendHeader("Content-Length", String(cachedFiles[i].size));
            server.sendHeader("Cache-Control", "no-cache");
            server.send(200, getContentType(path), "");
            server.client().write(cachedFiles[i].data, cachedFiles[i].size);
            return true;
        }
    }
    return false;
}

void WebInterface::cacheFile(const String &path, File &file)
{
    static int cacheIndex = 0;

    if (cachedFiles[cacheIndex].data != nullptr)
    {
        delete[] cachedFiles[cacheIndex].data;
        cachedFiles[cacheIndex].data = nullptr;
    }

    size_t fileSize = file.size();
    cachedFiles[cacheIndex].data = new uint8_t[fileSize];
    if (cachedFiles[cacheIndex].data)
    {
        file.read(cachedFiles[cacheIndex].data, fileSize);
        cachedFiles[cacheIndex].path = path;
        cachedFiles[cacheIndex].size = fileSize;
        Serial.printf("Web > Cached %s in RAM (%u bytes)\n", path.c_str(), fileSize);

        cacheIndex = (cacheIndex + 1) % MAX_CACHED_FILES;
    }
}

void WebInterface::updateCache()
{
    if (p1Meter)
    {
        cached.import_power = p1Meter->getCurrentImport();
        cached.export_power = p1Meter->getCurrentExport();
    }
    cached.temperature = sensors.getTemperature();
    cached.humidity = sensors.getHumidity();
    cached.light = sensors.getLightLevel();

    cached.socket_states[0] = socket1 ? socket1->getCurrentState() : false;
    cached.socket_states[1] = socket2 ? socket2->getCurrentState() : false;
    cached.socket_states[2] = socket3 ? socket3->getCurrentState() : false;

    for (int i = 0; i < 3; i++)
    {
        cached.socket_durations[i] = millis() - lastStateChangeTime[i];
    }
}

void WebInterface::begin()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    server.client().setNoDelay(true);

    Serial.printf("Total space: %d bytes\n", SPIFFS.totalBytes());
    Serial.printf("Used space: %d bytes\n", SPIFFS.usedBytes());

    // Serve the main page at root URL
    server.on("/", HTTP_GET, [this]()
              { serveFile("/data/index.html"); });

    // API endpoint for getting data
    server.on("/data", HTTP_GET, [this]()
              {
        server.sendHeader("Content-Type", "application/json");
        server.sendHeader("Access-Control-Allow-Origin", "*");

        StaticJsonDocument<2048> doc;
        doc["import_power"] = cached.import_power;
        doc["export_power"] = cached.export_power;
        doc["temperature"] = cached.temperature;
        doc["humidity"] = cached.humidity;
        doc["light"] = cached.light;

        JsonArray switches = doc.createNestedArray("switches");
        for(int i = 0; i < 3; i++) {
            JsonObject sw = switches.createNestedObject();
            sw["state"] = cached.socket_states[i];
            sw["duration"] = String(cached.socket_durations[i]/1000) + "s";
        }

        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response); });

    // API endpoints for controlling switches
    server.on("/switch/1", HTTP_POST, [this]()
              { handleSwitch(1); });
    server.on("/switch/2", HTTP_POST, [this]()
              { handleSwitch(2); });
    server.on("/switch/3", HTTP_POST, [this]()
              { handleSwitch(3); });

    // Handle any other static files
    server.onNotFound([this]()
                      {
        if (!serveFile(server.uri())) {
            server.send(404, "text/plain", "Not found");
        } });

    server.begin();
    Serial.println("Web server started on IP: " + WiFi.localIP().toString());
}

void WebInterface::update()
{
    static unsigned long lastWebUpdate = 0;
    static unsigned long lastClientCheck = 0;
    unsigned long now = millis();

    // Handle web clients first
    server.handleClient();

    // Only reset if we haven't seen any activity for a longer period
    if (server.client() && server.client().connected())
    {
        lastWebUpdate = now; // Reset timeout if we have an active client
        lastClientCheck = now;
    }
    else if (now - lastClientCheck >= 1000)
    { // Check connection status every second
        lastClientCheck = now;
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.printf("Web > Status: No active clients (uptime: %lus)\n",
                          (now - lastWebUpdate) / 1000);
        }
    }

    // Only reset if really needed (increase to 2 minutes)
    if (now - lastWebUpdate > 120000)
    { // 2 minutes
        Serial.println("Web > Watchdog: Server inactive, attempting reset");
        server.close();
        delay(100); // Give it time to close
        server.begin();
        Serial.println("Web > Server reset complete");
        lastWebUpdate = now;
    }

    // Update cache periodically
    static unsigned long lastCacheUpdate = 0;
    if (now - lastCacheUpdate >= 1000)
    {
        updateCache();
        lastCacheUpdate = now;
    }

    // WiFi check
    if (now - lastCheck >= CHECK_INTERVAL)
    {
        lastCheck = now;
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Web > WiFi connection lost - attempting reconnect");
            WiFi.reconnect();
        }
    }

    yield();
}

bool WebInterface::serveFile(const String &path)
{
    Serial.printf("Web > Attempting to serve: %s\n", path.c_str());

    if (!buffer)
    {
        Serial.println("Web > Error: Buffer not allocated!");
        return false;
    }

    // Try cache first
    if (serveFromCache(path))
    {
        Serial.println("Web > Served from cache successfully");
        return true;
    }

    File file = SPIFFS.open(path, "r");
    if (!file)
    {
        Serial.printf("Web > Error: Failed to open %s\n", path.c_str());
        return false;
    }

    size_t fileSize = file.size();
    Serial.printf("Web > Serving file from SPIFFS: %s (%u bytes)\n", path.c_str(), fileSize);

    String contentType = getContentType(path);
    server.sendHeader("Content-Type", contentType);
    server.sendHeader("Content-Length", String(fileSize));
    server.sendHeader("Connection", "close");
    server.sendHeader("Cache-Control", "no-cache");
    server.setContentLength(fileSize);
    server.send(200, contentType, "");

    size_t totalBytesSent = 0;
    while (totalBytesSent < fileSize)
    {
        if (!server.client().connected())
        {
            Serial.println("Web > Error: Client disconnected during transfer");
            file.close();
            return false;
        }

        size_t bytesRead = file.read(buffer, min(BUFFER_SIZE, fileSize - totalBytesSent));
        if (bytesRead == 0)
        {
            Serial.println("Web > Error: Failed to read file");
            break;
        }

        size_t bytesWritten = server.client().write(buffer, bytesRead);
        if (bytesWritten != bytesRead)
        {
            Serial.printf("Web > Warning: Partial write %u/%u bytes\n", bytesWritten, bytesRead);
            delay(50);
            continue;
        }

        totalBytesSent += bytesWritten;
        if (totalBytesSent % (BUFFER_SIZE * 4) == 0)
        {
            Serial.printf("Web > Progress: %u/%u bytes sent\n", totalBytesSent, fileSize);
        }
        delay(1);
        yield();
    }

    file.close();

    if (totalBytesSent == fileSize)
    {
        Serial.println("Web > File served successfully");
        // Try to cache the file for next time
        file = SPIFFS.open(path, "r");
        if (file)
        {
            cacheFile(path, file);
            file.close();
        }
        return true;
    }
    else
    {
        Serial.printf("Web > Error: Only sent %u/%u bytes\n", totalBytesSent, fileSize);
        return false;
    }
}

void WebInterface::handleSwitch(int switchNumber)
{
    if (!server.hasArg("plain"))
    {
        server.send(400, "text/plain", "Body not received");
        return;
    }

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (error)
    {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    bool state = doc["state"];
    server.sendHeader("Content-Type", "application/json");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"success\":true}");
}
-------------------
<!DOCTYPE html>
<html>

<head>
    <title>Home Energy Dashboard</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background: #f5f5f5;
        }

        .grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }

        .card {
            border: 1px solid #ccc;
            border-radius: 8px;
            padding: 15px;
            margin-bottom: 20px;
            background: white;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }

        .switch-card {
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .value {
            font-size: 24px;
            font-weight: bold;
            color: #2196F3;
        }

        .label {
            color: #666;
        }

        /* Toggle switch styling */
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }

        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .toggle-slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }

        .toggle-slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }

        input:checked+.toggle-slider {
            background-color: #2196F3;
        }

        input:checked+.toggle-slider:before {
            transform: translateX(26px);
        }

        .status {
            font-size: 14px;
            color: #666;
        }

        .details {
            font-size: 12px;
            color: #888;
            margin-top: 5px;
        }

        .status-icon {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }

        .status-on {
            background-color: #4CAF50;
        }

        .status-off {
            background-color: #ccc;
        }

        .time-info {
            position: fixed;
            bottom: 10px;
            right: 10px;
            background: rgba(255, 255, 255, 0.9);
            padding: 8px 15px;
            border-radius: 4px;
            font-size: 12px;
            color: #666;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }

        .update-recent {
            color: #4CAF50;
        }

        .update-old {
            color: #f44336;
        }
    </style>
</head>

<body>
    <div class="card">
        <h2>Power Monitor</h2>
        <div class="grid">
            <div>
                <div class="label">Import</div>
                <div class="value" id="import-power">-- W</div>
            </div>
            <div>
                <div class="label">Export</div>
                <div class="value" id="export-power">-- W</div>
            </div>
        </div>
    </div>

    <div class="card">
        <h2>Environment</h2>
        <div class="grid">
            <div>
                <div class="label">Temperature</div>
                <div class="value" id="temperature">--Â°C</div>
            </div>
            <div>
                <div class="label">Humidity</div>
                <div class="value" id="humidity">--%</div>
            </div>
            <div>
                <div class="label">Light</div>
                <div class="value" id="light">-- lux</div>
            </div>
        </div>
    </div>

    <div class="card switch-card">
        <div>
            <h3><span class="status-icon status-off"></span>Switch 1 (Solar Excess)</h3>
            <div class="status" id="switch1-status">Off</div>
            <div class="details">Auto: Power export > 500W</div>
        </div>
        <label class="toggle-switch">
            <input type="checkbox" id="switch1" onchange="toggleSwitch(1, this.checked)">
            <span class="toggle-slider"></span>
        </label>
    </div>

    <div class="card switch-card">
        <div>
            <h3><span class="status-icon status-off"></span>Switch 2 (Evening Light)</h3>
            <div class="status" id="switch2-status">Off</div>
            <div class="details">Auto: Light < 75 lux (After 17:45)</div>
                    <div class="details" id="switch2-offtime">Today's off time: --:--</div>
            </div>
            <label class="toggle-switch">
                <input type="checkbox" id="switch2" onchange="toggleSwitch(2, this.checked)">
                <span class="toggle-slider"></span>
            </label>
        </div>

        <div class="card switch-card">
            <div>
                <h3><span class="status-icon status-off"></span>Switch 3 (Evening Light)</h3>
                <div class="status" id="switch3-status">Off</div>
                <div class="details">Auto: Light < 50 lux (After 17:30)</div>
                        <div class="details" id="switch3-offtime">Today's off time: --:--</div>
                </div>
                <label class="toggle-switch">
                    <input type="checkbox" id="switch3" onchange="toggleSwitch(3, this.checked)">
                    <span class="toggle-slider"></span>
                </label>
            </div>

            <div class="time-info">
                <div>Current Time: <span id="current-time">--:--:--</span></div>
                <div>Last Update: <span id="last-update" class="update-recent">--:--:--</span></div>
            </div>

            <script>
                let lastUpdateTime = null;

                // Function to toggle switches
                function toggleSwitch(switchNumber, state) {
                    fetch(`/switch/${switchNumber}`, {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json',
                        },
                        body: JSON.stringify({ state: state })
                    })
                        .then(response => response.json())
                        .then(data => {
                            updateStatusIcon(switchNumber, state);
                        })
                        .catch(error => {
                            console.error('Error:', error);
                            // Revert switch state on error
                            document.getElementById(`switch${switchNumber}`).checked = !state;
                        });
                }

                // Update the status icon for a switch
                function updateStatusIcon(switchNumber, state) {
                    const icon = document.querySelector(`#switch${switchNumber}-status`).previousElementSibling.querySelector('.status-icon');
                    icon.className = 'status-icon ' + (state ? 'status-on' : 'status-off');
                }

                // Format duration string
                function formatDuration(seconds) {
                    if (seconds === 0) return "Off";
                    const hours = Math.floor(seconds / 3600);
                    const minutes = Math.floor((seconds % 3600) / 60);
                    return `On for ${hours}h ${minutes}m`;
                }

                // Update time displays
                function updateTimeDisplay() {
                    const now = new Date();
                    document.getElementById('current-time').textContent =
                        now.toLocaleTimeString();

                    if (lastUpdateTime) {
                        const lastUpdate = document.getElementById('last-update');
                        const timeDiff = now - lastUpdateTime;
                        lastUpdate.textContent = lastUpdateTime.toLocaleTimeString();
                        lastUpdate.className = timeDiff > 5000 ? 'update-old' : 'update-recent';
                    }
                }

                // Main data update function
                function updateData() {
                    fetch('/data')
                        .then(response => response.json())
                        .then(data => {
                            // Update power values
                            document.getElementById('import-power').textContent = `${data.import_power} W`;
                            document.getElementById('export-power').textContent = `${data.export_power} W`;

                            // Update environmental data
                            document.getElementById('temperature').textContent = `${data.temperature}Â°C`;
                            document.getElementById('humidity').textContent = `${data.humidity}%`;
                            document.getElementById('light').textContent = `${data.light} lux`;

                            // Update switches
                            data.switches.forEach((switch_data, index) => {
                                const switchNum = index + 1;
                                const checkbox = document.getElementById(`switch${switchNum}`);
                                const statusDiv = document.getElementById(`switch${switchNum}-status`);

                                checkbox.checked = switch_data.state;
                                statusDiv.textContent = formatDuration(switch_data.duration);
                                updateStatusIcon(switchNum, switch_data.state);

                                // Update off times for light switches
                                if (switchNum > 1 && switch_data.off_time) {
                                    document.getElementById(`switch${switchNum}-offtime`).textContent =
                                        `Today's off time: ${switch_data.off_time}`;
                                }
                            });

                            lastUpdateTime = new Date();
                            updateTimeDisplay();
                        })
                        .catch(error => {
                            console.error('Error:', error);
                            document.getElementById('last-update').className = 'update-old';
                        });
                }

                // Update time display every second
                setInterval(updateTimeDisplay, 1000);

                // Update data every 2 seconds
                setInterval(updateData, 2000);

                // Initial update
                updateData();
            </script>
</body>

</html>
-------------------
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <Arduino.h>
#include "TimeSync.h"
class DisplayManager
{
private:
    U8G2_SH1106_128X64_NONAME_F_HW_I2C display;
    bool displayFound = false;
    int currentPage = 0;
    unsigned long lastPageChange = 0;
    const unsigned long PAGE_DURATION = 500;

    void showPowerPage(float importPower, float exportPower, float totalImport, float totalExport);
    void showEnvironmentPage(float temp, float humidity, float light);
    void showSwitchesPage(bool switch1, bool switch2, bool switch3,
                          const String &sw1Time, const String &sw2Time, const String &sw3Time);
    void showInfoPage();

public:
    DisplayManager() : display(U8G2_R0, /* reset= */ U8X8_PIN_NONE) {}
    bool begin();

    // Basic display update (without info page)
    void updateDisplay(float importPower, float exportPower, float totalImport, float totalExport,
                       float temp, float humidity, float light,
                       bool sw1, bool sw2, bool sw3,
                       const String &sw1Time, const String &sw2Time, const String &sw3Time);
};

#endif
-------------------
// EnvironmentSensor.h
#ifndef ENVIRONMENT_SENSORS_H
#define ENVIRONMENT_SENSORS_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>

class EnvironmentSensors
{
private:
    Adafruit_BME280 bme;
    BH1750 lightMeter;
    bool bmeFound = false;
    bool lightMeterFound = false;

    float temperature = 0;
    float humidity = 0;
    float pressure = 0;
    float lightLevel = 0;

public:
    bool begin();
    void update();
    float getTemperature() const;
    float getHumidity() const;
    float getPressure() const;
    float getLightLevel() const;
    bool hasBME280() const;
    bool hasBH1750() const;
};

#endif
-------------------
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

    float yesterdayImport;
    float yesterdayExport;
    int yesterday;

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
    const unsigned long ENV_SENSOR_INTERVAL = 500;    // 10 seconds
    const unsigned long LIGHT_SENSOR_INTERVAL = 500;  // 10 seconds
    const unsigned long DISPLAY_INTERVAL = 1500;      // 1 second
    const unsigned long P1_INTERVAL = 1000;           // 1 second
    const unsigned long SOCKET_INTERVAL = 5000;       // 5 seconds
    const unsigned long WIFI_CHECK_INTERVAL = 30000;  // 30 seconds
    const unsigned long PHONE_CHECK_INTERVAL = 60000; // 60 seconds

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
-------------------
// HomeP1Device.h
#ifndef HOME_P1_DEVICE_H
#define HOME_P1_DEVICE_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

class HomeP1Device
{
private:
    HTTPClient http;
    WiFiClient client;
    String baseUrl;
    float lastImportPower;
    float lastExportPower;

    float lastTotalImport;
    float lastTotalExport;

    unsigned long lastReadTime;
    const unsigned long READ_INTERVAL = 1000;
    const unsigned long HTTP_TIMEOUT = 5000;
    bool lastReadSuccess;
    bool getPowerData(float &importPower, float &exportPower);
    bool makeRequest(const String &endpoint, const String &method, const String &payload = "");

public:
    HomeP1Device(const char *ip);
    void update();
    float getCurrentImport() const;
    float getCurrentExport() const;
    float getNetPower() const;
    bool isConnected() const;
    float getTotalImport() const;
    float getTotalExport() const;
};

#endif
-------------------
#ifndef HOME_SOCKET_DEVICE_H
#define HOME_SOCKET_DEVICE_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

class HomeSocketDevice
{
private:
    HTTPClient http;
    WiFiClient client;
    String baseUrl;
    bool lastKnownState;
    unsigned long lastReadTime;
    const unsigned long READ_INTERVAL = 1000;
    bool lastReadSuccess;

    int consecutiveFailures;
    String deviceIP; // Store IP for better logging
    bool makeHttpRequest(const String &endpoint, const String &method, const String &payload, String &response);

    unsigned long lastLogTime; // For controlling log frequency

public:
    HomeSocketDevice(const char *ip);
    void update();
    bool setState(bool state);
    bool getState();
    bool isConnected() const { return consecutiveFailures == 0; }
    bool getCurrentState() const { return lastKnownState; }
};

#endif
-------------------
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
-------------------
// NetworkCheck.h
#ifndef NETWORK_CHECK_H
#define NETWORK_CHECK_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>

class NetworkCheck
{
private:
    String deviceIP;
    bool lastKnownState;
    unsigned long lastCheckTime;
    const unsigned long CHECK_INTERVAL = 60000; // Check every minute
    int consecutiveFailures;

    bool pingDevice();

public:
    NetworkCheck(const char *ip);
    bool isDevicePresent();
};

#endif
-------------------

This directory is intended for project header files.

A header file is a file containing C declarations and macro definitions
to be shared between several project source files. You request the use of a
header file in your project source file (C, C++, etc) located in `src` folder
by including it, with the C preprocessing directive `#include'.

```src/main.c

#include "header.h"

int main (void)
{
 ...
}
```

Including a header file produces the same results as copying the header file
into each source file that needs it. Such copying would be time-consuming
and error-prone. With a header file, the related declarations appear
in only one place. If they need to be changed, they can be changed in one
place, and programs that include the header file will automatically use the
new version when next recompiled. The header file eliminates the labor of
finding and changing all the copies as well as the risk that a failure to
find one copy will result in inconsistencies within a program.

In C, the usual convention is to give header files names that end with `.h'.
It is most portable to use only letters, digits, dashes, and underscores in
header file names, and at most one dot.

Read more about using header files in official GCC documentation:

* Include Syntax
* Include Operation
* Once-Only Headers
* Computed Includes

https://gcc.gnu.org/onlinedocs/cpp/Header-Files.html

-------------------
// SimpleRuleEngine.h
#ifndef SIMPLE_RULE_ENGINE_H
#define SIMPLE_RULE_ENGINE_H

#include "GlobalVars.h"

class SimpleRuleEngine
{
    struct SocketState
    {
        bool currentState;             // Current on/off state
        unsigned long lastStateChange; // When the current state started (millis)
        int lastChangeHour;            // Hour of last state change (for time persistence)
        int lastChangeMinute;          // Minute of last state change
        bool stateChangeProcessed;     // Flag to prevent multiple triggers in same state
    };

private:
    float current_lux;
    static const int MEMORY_SLOTS = 32;  // used in TurnUntil
    bool memory[MEMORY_SLOTS] = {false}; // used in TurnUntil

    static const int MAX_SOCKETS = 3;
    SocketState socketStates[MAX_SOCKETS];
    HomeSocketDevice *sockets[MAX_SOCKETS]; // Array instead of individual pointers

    // Helper function to get socket and validate
    HomeSocketDevice *getSocket(int socket_number);
    void updateSocketDuration(int socket_number);

public:
    // In the header:

    static const uint8_t SUNDAY = 0b00000001;
    static const uint8_t MONDAY = 0b00000010;
    static const uint8_t TUESDAY = 0b00000100;
    static const uint8_t WEDNESDAY = 0b00001000;
    static const uint8_t THURSDAY = 0b00010000;
    static const uint8_t FRIDAY = 0b00100000;
    static const uint8_t SATURDAY = 0b01000000;

    // Convenient combinations
    static const uint8_t WEEKDAYS = MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY;
    static const uint8_t WEEKEND = SATURDAY | SUNDAY;
    static const uint8_t EVERYDAY = WEEKDAYS | WEEKEND;

    int isWeekday(uint8_t dayPattern);

public:
    SimpleRuleEngine()
    {
        // Initialize socket pointers to nullptr
        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            sockets[i] = nullptr;
            socketStates[i] = {false, 0, 0, 0, true};
        }
    }
    void updateLightLevel();
    int lightSensorAbove(int lux_value);
    int lightSensorBelow(int lux_value);
    int phoneFound();
    int phoneNotFound();
    void turnOn(int socket_number, int condition);
    void turnOff(int socket_number, int condition);

    // Time functions
    int after(const char *timeStr);
    int before(const char *timeStr);
    int isOn(int socket_number);
    int isOff(int socket_number);

    // Time-based functions
    int turnOnInbetween(const char *startTime, const char *endTime);
    int turnOnAfter(const char *startTime);
    int turnOffAfter(const char *startTime);

    int turnOnBefore(const char *startTime);
    int turnOfBefore(const char *startTime);

    int hasBeenOnFor(int socket_number, int minutes);
    int hasBeenOffFor(int socket_number, int minutes);

    int setMem(int slot, unsigned long value);
    int readMem(int slot);
    int Delay(int memSlot, int triggerFunction);

    // Logical operators
    int OR(int func1, int func2);
    int AND(int func1, int func2);
    int NOT(int func);

    int TurnUntil(int memoryIndex, int turnOnCondition, int turnOffCondition);
};

#endif

-------------------
// TimeSync.h
#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>
#include <time.h>
#include <WiFi.h>

class TimeSync
{
private:
    const char *ntpServer = "nl.pool.ntp.org";    // Netherlands NTP pool
    const char *ntpServer2 = "0.nl.pool.ntp.org"; // Specific Dutch server
    const char *ntpServer3 = "1.nl.pool.ntp.org"; // Backup Dutch server
    const long gmtOffset_sec = 3600;              // Netherlands is UTC+1
    const int daylightOffset_sec = 3600;          // DST when applicable
    bool timeInitialized = false;

public:
    TimeSync() {}
    bool begin();
    void getCurrentHourMinute(int &hour, int &minute);
    String getCurrentTime();
    bool isTimeBetween(const char *startTime, const char *endTime);
    int getCurrentMinutes();
    bool isTimeSet() const { return timeInitialized; }

    int getDayOfWeek();  // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
    int getWeekNumber(); // 1-53
    int getMonth();      // 1-12
    int getYear();       // Full year (e.g., 2024)

    // Optional: helper method to get all time components at once
    struct TimeData
    {
        int year;      // Full year (2024)
        int month;     // 1-12
        int dayOfWeek; // 1-7 (Mon-Sun)
        int weekNum;   // 1-53
        int hour;      // 0-23
        int minute;    // 0-59
        int dayOfYear; // 0-365
    };
    TimeData getTime(); // One function to get everything
};

#endif
-------------------
// WebInterface.h
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include "GlobalVars.h"

using WebServer = ::WebServer;

class WebInterface
{
private:
    struct CachedData
    {
        float import_power = 0;
        float export_power = 0;
        float temperature = 0;
        float humidity = 0;
        float light = 0;
        bool socket_states[3] = {false, false, false};
        unsigned long socket_durations[3] = {0, 0, 0};
    };

    struct FileCache
    {
        String path;
        uint8_t *data = nullptr;
        size_t size = 0;
    };

    WebServer server;
    unsigned long lastCheck = 0;
    static const size_t BUFFER_SIZE = 1024;
    static const int CHUNK_DELAY = 5;
    static const unsigned long CHECK_INTERVAL = 30000;
    static const unsigned long ERROR_COOLDOWN = 5000;
    static const int MAX_CACHED_FILES = 2;

    uint8_t *buffer;
    CachedData cached;
    FileCache cachedFiles[MAX_CACHED_FILES];

    void updateCache();
    String getContentType(const String &path);
    bool serveFromCache(const String &path);
    void cacheFile(const String &path, File &file);
    bool serveFile(const String &path);
    void handleSwitch(int switchNumber);

public:
    WebInterface() : server(8080), buffer(new uint8_t[BUFFER_SIZE]) {}
    void begin();
    void update();
    ~WebInterface()
    {
        delete[] buffer;
        for (int i = 0; i < MAX_CACHED_FILES; i++)
        {
            if (cachedFiles[i].data != nullptr)
            {
                delete[] cachedFiles[i].data;
            }
        }
    }
};
