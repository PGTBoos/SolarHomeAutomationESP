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
    // Cache structure definition
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

    WebServer server;
    unsigned long lastCheck = 0;
    static const size_t BUFFER_SIZE = 1024;
    static const int CHUNK_DELAY = 5;
    static const unsigned long CHECK_INTERVAL = 30000;
    static const unsigned long ERROR_COOLDOWN = 5000;
    uint8_t *buffer;
    CachedData cached; // Instance of the cache

    bool serveFile(const String &path);
    void handleSwitch(int switchNumber);
    void updateCache(); // Declaration of the cache update function

public:
    WebInterface() : server(8080), buffer(new uint8_t[BUFFER_SIZE]) {}
    void begin();
    void update();
    ~WebInterface() { delete[] buffer; }
};