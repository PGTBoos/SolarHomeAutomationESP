// HomeP1Device.cpp
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include "HomeSocketDevice.h"

class HomeP1Device
{
private:
    HTTPClient http;
    WiFiClient client;
    String baseUrl;
    float lastImportPower;
    float lastExportPower;
    unsigned long lastReadTime;
    const unsigned long READ_INTERVAL = 1000;
    const unsigned long HTTP_TIMEOUT = 5000;
    bool lastReadSuccess;

    bool makeRequest(const String &endpoint, const String &method, const String &payload = "")
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            lastReadSuccess = false;
            return false;
        }

        http.begin(client, baseUrl + endpoint);
        http.setTimeout(HTTP_TIMEOUT);

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

        String response = "";
        if (httpCode == HTTP_CODE_OK)
        {
            response = http.getString();
        }

        http.end();
        lastReadSuccess = (httpCode == HTTP_CODE_OK);

        if (lastReadSuccess && response.length() > 0)
        {
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, response);

            if (!error)
            {
                float power = doc["active_power_w"].as<float>();
                if (power < 0)
                {
                    lastImportPower = 0;
                    lastExportPower = -power;
                }
                else
                {
                    lastImportPower = power;
                    lastExportPower = 0;
                }
            }
        }

        return lastReadSuccess;
    }

public:
    HomeP1Device(const char *ip) : baseUrl("http://" + String(ip)),
                                   lastImportPower(0),
                                   lastExportPower(0),
                                   lastReadTime(0),
                                   lastReadSuccess(false) {}

    void update()
    {
        if (millis() - lastReadTime >= READ_INTERVAL)
        {
            makeRequest("/api/v1/data", "GET");
            lastReadTime = millis();
        }
    }

    float getCurrentImport() const { return lastImportPower; }
    float getCurrentExport() const { return lastExportPower; }
    float getNetPower() const { return lastImportPower - lastExportPower; }
    bool isConnected() const { return lastReadSuccess; }
};