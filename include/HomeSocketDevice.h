// HomeSocketDevice.h
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

public:
    HomeSocketDevice(const char *ip);
    void update();
    bool setState(bool state);
    bool getState();
    bool isConnected() const;
    bool getCurrentState() const;
};

#endif