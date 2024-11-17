// WebServer.h
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WebServer.h> // ESP32 WebServer

using WebServer = ::WebServer; // This tells the compiler explicitly what WebServer is

class WebInterface
{
private:
    WebServer server; // Now it knows exactly which WebServer to use
    bool serveFile(const String &path);
    void handleSwitch(int switchNumber);

public:
    WebInterface() : server(8080) {}
    void begin();
    void update();
};

#endif