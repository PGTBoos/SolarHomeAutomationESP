// HomeWizardDevices.h
// HomeP1Device.cpp
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
    unsigned long lastReadTime;
    const unsigned long READ_INTERVAL = 1000; // Read every second
    bool lastReadSuccess;

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
            lastReadSuccess = getPowerData(lastImportPower, lastExportPower);
            lastReadTime = millis();
        }
    }

    bool getPowerData(float &importPower, float &exportPower)
    {
        http.begin(client, baseUrl + "/api/v1/data");
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error)
            {
                float power = doc["active_power_w"].as<float>();
                if (power < 0)
                {
                    importPower = 0;
                    exportPower = -power;
                }
                else
                {
                    importPower = power;
                    exportPower = 0;
                }
                http.end();
                return true;
            }
        }
        http.end();
        return false;
    }

    // Getter methods
    float getCurrentImport() const { return lastImportPower; }
    float getCurrentExport() const { return lastExportPower; }
    float getNetPower() const { return lastImportPower - lastExportPower; }
    bool isConnected() const { return lastReadSuccess; }
};