// HomeSocketDevice.cpp
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
    HomeSocketDevice(const char *ip) : baseUrl("http://" + String(ip)),
                                       lastKnownState(false),
                                       lastReadTime(0),
                                       lastReadSuccess(false) {}

    void update()
    {
        if (millis() - lastReadTime >= READ_INTERVAL)
        {
            lastKnownState = getState();
            lastReadTime = millis();
        }
    }

    bool setState(bool state)
    {
        http.begin(client, baseUrl + "/api/v1/state");
        http.addHeader("Content-Type", "application/json");

        String payload = "{\"power_on\":" + String(state ? "true" : "false") + "}";
        int httpCode = http.PUT(payload);

        lastReadSuccess = (httpCode == HTTP_CODE_OK);
        if (lastReadSuccess)
        {
            lastKnownState = state;
        }

        http.end();
        return lastReadSuccess;
    }

    bool getState()
    {
        http.begin(client, baseUrl + "/api/v1/state");
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error)
            {
                lastKnownState = doc["power_on"];
                lastReadSuccess = true;
                http.end();
                return lastKnownState;
            }
        }

        lastReadSuccess = false;
        http.end();
        return lastKnownState;
    }

    bool isConnected() const { return lastReadSuccess; }
    bool getCurrentState() const { return lastKnownState; }
};