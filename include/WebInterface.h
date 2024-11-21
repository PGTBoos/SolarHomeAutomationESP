#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include "HomeP1Device.h"

using WebServer = ::WebServer;

class WebInterface
{
private:
    WebServer server;
    unsigned long lastCheck = 0;
    static const size_t BUFFER_SIZE = 1024;            // 1KB buffer
    static const int CHUNK_DELAY = 5;                  // 5ms delay between chunks
    static const unsigned long CHECK_INTERVAL = 30000; // 30 seconds
    static const unsigned long ERROR_COOLDOWN = 5000;  // 5 second cooldown

    uint8_t *buffer;

    bool serveFile(const String &path);
    void handleSwitch(int switchNumber);

    HomeP1Device *p1Device;

public:
    // Constructor with port 8080
    WebInterface(HomeP1Device *p1) : p1Device(p1) {}
    WebInterface() : server(8080), buffer(nullptr)
    {
        buffer = new uint8_t[BUFFER_SIZE];
        if (!buffer)
        {
            Serial.println("Failed to allocate buffer!");
        }
    }

    // Destructor
    ~WebInterface()
    {
        if (buffer)
        {
            delete[] buffer;
        }
    }

    void begin();
    void update();
};

#endif