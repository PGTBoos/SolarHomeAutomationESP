// DisplayManager.cpp
#include "DisplayManager.h"
#include <WiFi.h>
#include <timeSync.h>

// Assuming timeSync is a global or class member variable
extern TimeSync *timeSync;

bool DisplayManager::begin()
{
    Serial.println("\nInitializing \nOLED display...");

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

void DisplayManager::showInfoPage()
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
    display.print(timeSync->getCurrentTime()); // Get time directly from TimeSync

    // WiFi Status
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, 45, "WiFi:");
    display.setFont(u8g2_font_7x14_tr);
    display.setCursor(0, 55);
    if (WiFi.status() == WL_CONNECTED)
    {
        display.print("Online");

        // IP Address with alternating backgrounds, no dots
        display.setFont(u8g2_font_profont10_tr); // Slightly larger than 4x6
        display.drawStr(0, 73, "IP:");
        IPAddress ip = WiFi.localIP();

        // Calculate positions with 5 pixels per character
        const int charWidth = 5;
        const int blockWidth = charWidth * 3 + 1; // Each block is 3 digits
        const int startX = 0;
        const int y = 83;

        // Draw background blocks
        for (int i = 0; i < 4; i++)
        {
            if (i % 2 == 1)
            { // Alternate blocks
                display.drawBox(startX + (i * blockWidth), y - 1, blockWidth, 10);
            }
        }

        // Print numbers
        display.setDrawColor(1); // Normal color for odd blocks
        display.setCursor(startX, y);
        display.printf("%03d", ip[0]);

        display.setDrawColor(0); // Inverted for even blocks
        display.setCursor(startX + blockWidth, y);
        display.printf("%03d", ip[1]);

        display.setDrawColor(1); // Back to normal
        display.setCursor(startX + (blockWidth * 2), y);
        display.printf("%03d", ip[2]);

        display.setDrawColor(0); // Inverted for last block
        display.setCursor(startX + (blockWidth * 3), y);
        display.printf("%03d", ip[3]);

        display.setDrawColor(1); // Reset to normal
    }
    else
    {
        display.print("Offline");
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
        currentPage = (currentPage + 1) % 4; // Cycle through 4 pages
        lastPageChange = millis();
    }
    else
    {
        return;
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
        showInfoPage(); // Now using the parameter-less version
        break;
    }
}