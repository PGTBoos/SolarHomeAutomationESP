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