// EnvironmentSensors.cpp
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>

class EnvironmentSensors
{
private:
    Adafruit_BME280 bme;
    BH1750 lightMeter;
    bool bmeFound = false;
    bool lightMeterFound = false;

    float temperature = 0;
    float humidity = 0;
    float pressure = 0;
    float lightLevel = 0;

public:
    bool begin()
    {
        Wire.begin(); // Start I2C

        // Initialize BME280
        bmeFound = bme.begin(0x76); // Try first address
        if (!bmeFound)
        {
            bmeFound = bme.begin(0x77); // Try alternate address
        }
        if (!bmeFound)
        {
            Serial.println("Could not find BME280 sensor!");
        }

        // Initialize BH1750
        lightMeterFound = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
        if (!lightMeterFound)
        {
            Serial.println("Could not find BH1750 sensor!");
        }

        return bmeFound || lightMeterFound; // Return true if at least one sensor works
    }

    void update()
    {
        if (bmeFound)
        {
            temperature = bme.readTemperature();
            humidity = bme.readHumidity();
            pressure = bme.readPressure() / 100.0F; // Convert to hPa
        }

        if (lightMeterFound)
        {
            lightLevel = lightMeter.readLightLevel();
        }
    }

    // Getter methods
    float getTemperature() const { return temperature; }
    float getHumidity() const { return humidity; }
    float getPressure() const { return pressure; }
    float getLightLevel() const { return lightLevel; }

    // Status methods
    bool hasBME280() const { return bmeFound; }
    bool hasBH1750() const { return lightMeterFound; }
};