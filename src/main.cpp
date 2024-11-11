#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// libs without headers..i'm lazy

#include "DisplayManager.cpp"
#include "EnvironmentSensors.cpp"
#include "HomeP1Device.cpp"
#include "HomeSocketDevice.cpp"
#include "TimeSync.cpp"
#include "WebInterface.cpp"

// Configuration structure
struct Config {
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
HomeP1Device* p1Meter = nullptr;
HomeSocketDevice* socket1 = nullptr;
HomeSocketDevice* socket2 = nullptr;
HomeSocketDevice* socket3 = nullptr;
TimeSync timeSync;
WebInterface webInterface;

// Timing variables
unsigned long lastStateChangeTime[3] = {0, 0, 0};
bool switchForceOff[3] = {false, false, false};

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
    unsigned long timeSinceChange = currentTime - lastStateChangeTime[switchIndex];
    
    if (newState) {  // Turning ON
        if (switchForceOff[switchIndex] && timeSinceChange < config.min_off_time) {
            return false;
        }
        switchForceOff[switchIndex] = false;
    } else {  // Turning OFF
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
        if (i == 0 && socket1) currentState = socket1->getCurrentState();
        if (i == 1 && socket2) currentState = socket2->getCurrentState();
        if (i == 2 && socket3) currentState = socket3->getCurrentState();
        
        if (currentState && (currentTime - lastStateChangeTime[i]) > config.max_on_time) {
            if (i == 0 && socket1) socket1->setState(false);
            if (i == 1 && socket2) socket2->setState(false);
            if (i == 2 && socket3) socket3->setState(false);
            switchForceOff[i] = true;
            lastStateChangeTime[i] = currentTime;
        }
    }
}

void updateSwitch1Logic() {
    if (!socket1 || !p1Meter) return;
    
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
    if (!socket2) return;
    
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
    if (!socket3) return;
    
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
    if (!p1Meter) return;
    
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
        String(millis() - lastStateChangeTime[2])
    );
}

void setup() {
    Serial.begin(115200);
    
    // Load configuration
    if (!loadConfiguration()) {
        Serial.println("Using default configuration");
    }
    
    // Initialize components
    if (!display.begin()) {
        Serial.println("Display initialization failed!");
    }
    
    if (!sensors.begin()) {
        Serial.println("Sensor initialization failed!");
    }
    
    connectWiFi();
    
    // Initialize network components after WiFi connection
    if (WiFi.status() == WL_CONNECTED) {
        p1Meter = new HomeP1Device(config.p1_ip.c_str());
        socket1 = new HomeSocketDevice(config.socket_1.c_str());
        socket2 = new HomeSocketDevice(config.socket_2.c_str());
        socket3 = new HomeSocketDevice(config.socket_3.c_str());
        
        timeSync.begin();
        webInterface.begin();
    }
    
    // Initial state
    for (int i = 0; i < 3; i++) {
        lastStateChangeTime[i] = millis();
        switchForceOff[i] = false;
    }
}

void loop() {
    // Update sensor readings
    sensors.update();
    if (p1Meter) p1Meter->update();
    if (socket1) socket1->update();
    if (socket2) socket2->update();
    if (socket3) socket3->update();
    
    // Update automation logic
    updateSwitch1Logic();
    updateSwitch2Logic();
    updateSwitch3Logic();
    checkMaxOnTime();
    
    // Update display and web interface
    updateDisplay();
    webInterface.update();
    
    // Small delay to prevent excessive CPU usage
    delay(100);
}
