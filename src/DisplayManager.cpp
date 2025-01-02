// DisplayManager.cpp
#include "DisplayManager.h"

bool DisplayManager::begin()
{
    Serial.println("\nInitializing OLED display...");

    if (!display.begin())
    {
        Serial.println("SH1106 allocation failed");
        return false;
    }

    // Setup display parameters
    display.setDisplayRotation(U8G2_R1);
    display.setFont(u8g2_font_profont10_tr); // Default font for labels
    display.setDrawColor(1);
    display.setFontPosTop();
    display.clearBuffer();

    // Draw initial test pattern
    display.drawStr(0, 0, "Display Ready");
    display.drawFrame(0, 0, display.getWidth(), display.getHeight());
    display.sendBuffer();

    displayFound = true;
    Serial.println("Display initialized successfully!");
    return true;
}

void DisplayManager::showPowerPage(float importPower, float exportPower)
{
    if (!displayFound)
        return;

    display.clearBuffer();

    // Title
    display.setFont(u8g2_font_profont10_tr);
    display.drawBox(0, 0, 64, 12);
    display.setDrawColor(0);
    display.drawStr(2, 2, "Power");
    display.setDrawColor(1);

    // Function to format power value
    auto formatPower = [](float power) -> String
    {
        if (abs(power) >= 1000)
        {
            // Display in kW with 2 decimals
            return String(power / 1000.0, 2) + " kW";
        }
        else
        {
            // Display in Watt with no decimals
            return String((int)power) + " Watt";
        }
    };

    // Import
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 17, "Import:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 27);
    display.print(formatPower(importPower));

    // Export
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 45, "Export:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 55);
    display.print(formatPower(exportPower));

    display.sendBuffer();
}

void DisplayManager::showEnvironmentPage(float temp, float humidity, float light)
{
    if (!displayFound)
        return;

    display.clearBuffer();

    // Title
    display.setFont(u8g2_font_profont10_tr);
    display.drawBox(0, 0, 64, 12);
    display.setDrawColor(0);
    display.drawStr(2, 2, "Environment");
    display.setDrawColor(1);

    // Temperature
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 17, "Temperature:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 27);
    display.print(temp, 1);
    display.print(" C'");

    // Humidity
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 45, "Humidity:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 55);
    display.print(humidity, 0);
    display.print(" %");

    // Light
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 73, "Light:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 83);
    display.print(light, 0);
    display.print(" Lux");

    display.sendBuffer();
}

void DisplayManager::showSwitchesPage(bool switch1, bool switch2, bool switch3,
                                      const String &sw1Time, const String &sw2Time,
                                      const String &sw3Time)
{
    if (!displayFound)
        return;

    display.clearBuffer();

    // Title
    display.setFont(u8g2_font_profont10_tr);
    display.drawBox(0, 0, 64, 12);
    display.setDrawColor(0);
    display.drawStr(2, 2, "Switches");
    display.setDrawColor(1);

    // Function to draw switch status
    auto drawSwitch = [&](int y, const char *name, bool state)
    {
        display.setFont(u8g2_font_profont10_tr);
        display.drawStr(0, y, name);

        display.setFont(u8g2_font_7x14_tr);
        if (state)
        {
            display.drawBox(25, y - 1, 25, 14);
            display.setDrawColor(0);
            display.drawStr(27, y, "ON");
            display.setDrawColor(1);
        }
        else
        {
            display.drawStr(27, y, "OFF");
        }
    };

    // Draw all switches with more spacing due to larger font
    drawSwitch(20, "SW 1:", switch1);
    drawSwitch(40, "SW 2:", switch2);
    drawSwitch(60, "SW 3:", switch3);

    display.sendBuffer();
}

void DisplayManager::showInfoPage(const String &time, const String &ip, bool wifiConnected)
{
    if (!displayFound)
        return;

    display.clearBuffer();

    // Title
    display.setFont(u8g2_font_profont10_tr);
    display.drawBox(0, 0, 64, 12);
    display.setDrawColor(0);
    display.drawStr(2, 2, "System Info");
    display.setDrawColor(1);

    // Time
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 17, "Time:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 27);
    display.print(time);

    // WiFi Status
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 45, "WiFi:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 55);
    if (wifiConnected)
    {
        display.print("Online");
    }
    else
    {
        display.print("Offline");
    }

    // IP Address
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 73, "IP:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 83);
    // Split IP into two lines if needed
    if (ip.length() > 10)
    {
        String ip1 = ip.substring(0, ip.lastIndexOf('.'));
        String ip2 = ip.substring(ip.lastIndexOf('.'));
        display.print(ip1);
        display.setCursor(0, 98);
        display.print(ip2);
    }
    else
    {
        display.print(ip);
    }

    display.sendBuffer();
}

void DisplayManager::updateDisplay(float importPower, float exportPower,
                                   float temp, float humidity, float light,
                                   bool sw1, bool sw2, bool sw3,
                                   const String &sw1Time, const String &sw2Time, const String &sw3Time)
{
    if (!displayFound)
        return;

    // Rotate pages every PAGE_DURATION milliseconds
    if (millis() - lastPageChange >= PAGE_DURATION)
    {
        currentPage = (currentPage + 1) % 3; // Cycle through 3 pages
        lastPageChange = millis();
    }

    // Show current page
    switch (currentPage)
    {
    case 0:
        showPowerPage(importPower, exportPower);
        break;
    case 1:
        showEnvironmentPage(temp, humidity, light);
        break;
    case 2:
        showSwitchesPage(sw1, sw2, sw3, sw1Time, sw2Time, sw3Time);
        break;
    case 3:
        // This is the info page, but we need to pass the correct parameters, time and IP
        showInfoPage("12:00", wifi);
        break;
    }
}