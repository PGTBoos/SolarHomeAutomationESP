#include "main.h"

// Global variable definitions
TimingControl timing;
Config config;
DisplayManager display;
EnvironmentSensors sensors;
HomeP1Device *p1Meter = nullptr;

HomeSocketDevice *sockets[NUM_SOCKETS] = {nullptr, nullptr, nullptr};
unsigned long lastStateChangeTime[NUM_SOCKETS] = {0, 0, 0};
bool switchForceOff[NUM_SOCKETS] = {false, false, false};

TimeSync timeSync;
WebInterface webServer;
NetworkCheck *phoneCheck = nullptr;

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

  // Print raw config file content
  Serial.println("\nRaw config file content:");
  Serial.println("------------------------");
  while (configFile.available()) {
    Serial.write(configFile.read());
  }
  Serial.println("\n------------------------");

  // Reset file pointer to start
  configFile.seek(0);

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

  for (int i = 0; i < NUM_SOCKETS; i++) {
    String key = "socket_" + String(i + 1);
    config.socket_ip[i] = doc[key].as<String>();
    Serial.printf("Loaded %s: %s\n", key.c_str(), config.socket_ip[i].c_str());
  }

  // config.socket_1 = doc["socket_1"].as<String>();
  // config.socket_2 = doc["socket_2"].as<String>();
  // config.socket_3 = doc["socket_3"].as<String>();
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
  // give the ip stack som time
  delay(200);
  yield();
  delay(300);
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

  for (int i = 0; i < NUM_SOCKETS; i++) {
    if (sockets[i] && sockets[i]->getCurrentState() &&
        (currentTime - lastStateChangeTime[i]) > config.max_on_time) {
      sockets[i]->setState(false);
      switchForceOff[i] = true;
      lastStateChangeTime[i] = currentTime;
    }
  }
}

// void updateSwitch1Logic() {
//   if (!socket1 || !p1Meter)
//     return;

//   float exportPower = p1Meter->getCurrentExport();
//   bool currentState = socket1->getCurrentState();
//   bool newState = currentState;

//   if (exportPower > config.power_on_threshold && !currentState) {
//     newState = true;
//   } else if (exportPower < config.power_off_threshold && currentState) {
//     newState = false;
//   }

//   if (newState != currentState && canChangeState(0, newState)) {
//     socket1->setState(newState);
//     lastStateChangeTime[0] = millis();
//   }
// }

// void updateSwitch2Logic() {
//   if (!socket2)
//     return;

//   int hour, minute;
//   timeSync.getCurrentHourMinute(hour, minute);
//   float light = sensors.getLightLevel();
//   bool currentState = socket2->getCurrentState();
//   bool newState = currentState;

//   // After 17:45 and light < 75 lux
//   if (hour >= 17 && minute >= 45 && light < 75) {
//     newState = true;
//   } else if (light >= 75) {
//     newState = false;
//   }

//   if (newState != currentState && canChangeState(1, newState)) {
//     socket2->setState(newState);
//     lastStateChangeTime[1] = millis();
//   }
// }

// void updateSwitch3Logic() {
//   if (!socket3)
//     return;

//   int hour, minute;
//   timeSync.getCurrentHourMinute(hour, minute);
//   float light = sensors.getLightLevel();
//   bool currentState = socket3->getCurrentState();
//   bool newState = currentState;

//   // After 17:30 and light < 50 lux
//   if (hour >= 17 && minute >= 30 && light < 50) {
//     newState = true;
//   } else if (light >= 50) {
//     newState = false;
//   }

//   if (newState != currentState && canChangeState(2, newState)) {
//     socket3->setState(newState);
//     lastStateChangeTime[2] = millis();
//   }
// }

void updateDisplay() {
  if (!p1Meter) {
    // print an error message if the p1Meter object is not initialized
    Serial.println("P1Meter object not initialized");
    // initialize the p1Meter object
    p1Meter = new HomeP1Device(config.p1_ip.c_str());
    return;
  }

  bool switchStates[NUM_SOCKETS];
  String switchTimes[NUM_SOCKETS];

  for (int i = 0; i < NUM_SOCKETS; i++) {
    switchStates[i] = sockets[i] ? sockets[i]->getCurrentState() : false;
    switchTimes[i] = String(millis() - lastStateChangeTime[i]);
  }

  display.updateDisplay(p1Meter->getCurrentImport(),
                        p1Meter->getCurrentExport(), p1Meter->getTotalImport(),
                        p1Meter->getTotalExport(), sensors.getTemperature(),
                        sensors.getHumidity(), sensors.getLightLevel(),
                        switchStates, switchTimes);
}
// void updateDisplay() {
//   if (!p1Meter)
//     return;
//   unsigned long currentMillis = millis();
//   if (currentMillis - lastTimeDisplay >= 1000) {
//     int hour, minute;
//     timeSync.getCurrentHourMinute(hour, minute);
//     Serial.printf("Current time: %02d:%02d\n", hour, minute);
//     lastTimeDisplay = currentMillis;
//   }

//   // Calculate time differences
//   String sw1Time = String((unsigned long)(millis() -
//   lastStateChangeTime[0])); String sw2Time = String((unsigned long)(millis()
//   - lastStateChangeTime[1])); String sw3Time = String((unsigned
//   long)(millis() - lastStateChangeTime[2]));

//   display.updateDisplay(p1Meter->getCurrentImport(),
//                         p1Meter->getCurrentExport(),
//                         p1Meter->getTotalImport(), p1Meter->getTotalExport(),
//                         sensors.getTemperature(), sensors.getHumidity(),
//                         sensors.getLightLevel(), socket1 ?
//                         socket1->getCurrentState() : false, socket2 ?
//                         socket2->getCurrentState() : false, socket3 ?
//                         socket3->getCurrentState() : false, String(millis() -
//                         lastStateChangeTime[0]), String(millis() -
//                         lastStateChangeTime[1]), String(millis() -
//                         lastStateChangeTime[2]));
// }

void setup() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  Serial.begin(115200);

  // First try to initialize I2C properly
  bool wireInitialized = false;
  for (int i = 0; i < 3; i++) {
    Serial.printf("\nAttempting Wire initialization (attempt %d/3)...\n",
                  i + 1);

    Wire.end(); // Make sure we start clean
    delay(50);

    if (!Wire.begin()) {
      Serial.println("Wire.begin() failed!");
      continue;
    }

    delay(50);

    // Try to set clock speed
    Wire.setClock(100000);

    // Scan for actual devices
    byte error, address;
    int deviceCount = 0;

    Serial.println("Scanning I2C bus for devices...");
    for (address = 1; address < 127; address++) {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();

      if (error == 0) {
        Serial.printf("Found device at address 0x%02X\n", address);
        deviceCount++;
      } else if (error != 2) { // Ignore error 2 (NACK on address) as that's
                               // normal for unused addresses
        Serial.printf("Error %d at address 0x%02X\n", error, address);
      }
    }

    Serial.printf("I2C scan complete: found %d devices\n", deviceCount);
    if (deviceCount > 0) {
      wireInitialized = true;
      break;
    }

    delay(100);
  }

  if (!wireInitialized) {
    Serial.println("FATAL: Failed to initialize Wire after 3 attempts!");
  }

  // Phone check initialization
  if (config.phone_ip != "" && config.phone_ip != "0" &&
      config.phone_ip != "null") {
    phoneCheck = new NetworkCheck(config.phone_ip.c_str());
    Serial.println("Phone check initialized at: " + config.phone_ip);
  }

  // SPIFFS initialization
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

  // Only proceed with display and sensors if Wire initialized successfully
  if (wireInitialized) {
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
  }

  // Network related initialization
  connectWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    timeSync.begin();
    webServer.begin();
    for (int i = 0; i < NUM_SOCKETS; i++) {
      if (config.socket_ip[i] != "" && config.socket_ip[i] != "0" &&
          config.socket_ip[i] != "null") {
        sockets[i] = new HomeSocketDevice(config.socket_ip[i].c_str());
        Serial.printf("Socket %d initialized at: %s\n", i + 1,
                      config.socket_ip[i].c_str());
      }
    }
  }
}

// void setup() {
//   WiFi.persistent(false);
//   WiFi.mode(WIFI_STA);
//   WiFi.setSleep(false);

//   Serial.begin(115200);

//   if (config.phone_ip != "" && config.phone_ip != "0" &&
//       config.phone_ip != "null") {
//     phoneCheck = new NetworkCheck(config.phone_ip.c_str());
//     Serial.println("Phone check initialized at: " + config.phone_ip);
//   }

//   if (!SPIFFS.begin(true)) {
//     Serial.println("SPIFFS Mount Failed");
//     Serial.println("Trying to format SPIFFS...");
//     if (SPIFFS.format()) {
//       Serial.println("SPIFFS formatted successfully");
//       if (SPIFFS.begin(true)) {
//         Serial.println("SPIFFS mounted successfully after format");
//       } else {
//         Serial.println("SPIFFS mount failed even after format");
//       }
//     } else {
//       Serial.println("SPIFFS format failed");
//     }
//   } else {
//     Serial.println("SPIFFS mounted successfully");
//   }

//   if (!loadConfiguration()) {
//     Serial.println("Using default configuration");
//   }

//   Wire.setClock(100000);
//   Wire.begin();

//   if (display.begin()) {
//     Serial.println("Display initialized successfully");
//   } else {
//     Serial.println("Display not connected or initialization failed!");
//   }

//   if (sensors.begin()) {
//     Serial.println("Environmental sensors initialized successfully");
//   } else {
//     Serial.println(
//         "Environmental sensors not connected or initialization failed!");
//   }

//   connectWiFi();
//   if (WiFi.status() == WL_CONNECTED) {
//     for (int i = 0; i < NUM_SOCKETS; i++) {
//       if (config.socket_ip[i] != "" && config.socket_ip[i] != "0" &&
//           config.socket_ip[i] != "null") {
//         sockets[i] = new HomeSocketDevice(config.socket_ip[i].c_str());
//         Serial.printf("Socket %d initialized at: %s\n", i + 1,
//                       config.socket_ip[i].c_str());
//       }
//     }

//     timeSync.begin();
//     webServer.begin();
//   }

//   // Initialize timing and state
//   unsigned long startTime = millis();
// }

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
static uint16_t operationOrder = 0;

void loop() {
  unsigned long currentMillis = millis();

  // Use static counter to sequence for ALL operations

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
      yield();
      delay(1);
    }
    operationOrder = 10;
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
      yield();
      delay(1);
    }
    operationOrder = 20;
    break;

  case 20: // Display update (I2C)

    if (currentMillis - timing.lastDisplayUpdate >= timing.DISPLAY_INTERVAL) {
      Serial.printf("Display case 20 - time since last update: %lu ms\n",
                    currentMillis - timing.lastDisplayUpdate);
      Serial.println("Updating display...");
      updateDisplay();
      timing.lastDisplayUpdate = currentMillis;
      yield();
      delay(17);
      Serial.println("Display update complete");
    }
    operationOrder = 30;
    break;

  case 30: // P1 meter (Network)
    if (p1Meter &&
        (currentMillis - timing.lastP1Update >= timing.P1_INTERVAL)) {
      p1Meter->update();
      timing.lastP1Update = currentMillis;
      yield();
      delay(49);
    }
    operationOrder = 40;
    break;

  case 40: // Socket updates (Network)
    for (int i = 0; i < NUM_SOCKETS; i++) {
      if (sockets[i] && (currentMillis - timing.lastSocketUpdates[i] >=
                         timing.SOCKET_INTERVAL)) {
        sockets[i]->update();
        timing.lastSocketUpdates[i] = currentMillis;
        // Handle socket-specific logic
        if (i == 0 && p1Meter) {
          // updateSwitch1Logic();
        } else if (i == 1) {
          //  updateSwitch2Logic();
        } else if (i == 2) {
          // updateSwitch3Logic();
        }
        yield();
        delay(50);
      }
    }
    operationOrder = 70;
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
      operationOrder = 1000;
      break;
    }

    static int lastSavedDay = config.yesterday;
    int currentDay = timeSync.getTime().dayOfYear;

    if (lastSavedDay == 0) {
      lastSavedDay = currentDay;
      config.yesterday = currentDay;
      config.yesterdayImport = p1Meter->getTotalImport();
      config.yesterdayExport = p1Meter->getTotalExport();

      // Save initial values
      StaticJsonDocument<128> doc;
      doc["day"] = currentDay;
      doc["import"] = config.yesterdayImport;
      doc["export"] = config.yesterdayExport;

      File file = SPIFFS.open("/daily_totals.json", "w");
      if (file) {
        serializeJson(doc, file);
        file.close();
        Serial.printf(
            "Initialized day totals - Day: %d, Import: %.3f, Export: %.3f\n",
            currentDay, config.yesterdayImport, config.yesterdayExport);
      }
    }

    // Only check for day change - remove the exact midnight check
    if (currentDay != lastSavedDay) {
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
      }
    }
    operationOrder = 1000;
    break;
  }

    // lets do switching logic above 1000

  case 1000: {
    operationOrder = 5;
    // uf lux is below 10 and it is after 17:45, turn on socket 1
    // and socket one is connected
    // and can ping computer

    break;
  }
  default:
    Serial.printf("ERROR: Invalid operation order: %d\n", operationOrder);
    operationOrder = 5; // Reset to beginning
    break;
  }
}