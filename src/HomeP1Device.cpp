// HomeP1Device.cpp
#include "HomeP1Device.h"

HomeP1Device::HomeP1Device(const char *ip) : baseUrl("http://" + String(ip)),
                                             lastImportPower(0),
                                             lastExportPower(0),
                                             lastReadTime(0),
                                             lastReadSuccess(false)
{
}
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

void HomeP1Device::update()
{
    if (millis() - lastReadTime >= READ_INTERVAL)
    {
        lastReadSuccess = getPowerData(lastImportPower, lastExportPower);
        lastReadTime = millis();
    }
}

bool HomeP1Device::getPowerData(float &importPower, float &exportPower)
{
    http.begin(client, baseUrl + "/api/v1/data");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();
        StaticJsonDocument<2048> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
            float power = doc["active_power_w"].as<float>();
            Serial.printf("Received P1 power data: %.2f W\n", power);
            importPower = max(power, 0);
            exportPower = max(-power, 0);
            http.end();
            return true;
        }
    }
    http.end();
    return false;
}

float HomeP1Device::getCurrentImport() const
{
    return lastImportPower;
}

float HomeP1Device::getCurrentExport() const
{
    return lastExportPower;
}

float HomeP1Device::getNetPower() const
{
    return lastImportPower - lastExportPower;
}

bool HomeP1Device::isConnected() const
{
    return lastReadSuccess;
}