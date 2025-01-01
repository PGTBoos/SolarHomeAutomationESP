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
    display.setFont(u8g2_font_6x10_tr); // Choose a suitable font
    display.setDrawColor(1);            // White on black
    display.setFontPosTop();            // Upper left corner is reference
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
    display.drawStr(0, 0, "Power Monitor");

    display.setCursor(0, 20);
    display.print("Import: ");
    display.print(importPower, 1);
    display.print("W");

    display.setCursor(0, 35);
    display.print("Export: ");
    display.print(exportPower, 1);
    display.print("W");

    display.sendBuffer();
}

void DisplayManager::showEnvironmentPage(float temp, float humidity, float light)
{
    if (!displayFound)
        return;

    display.clearBuffer();
    display.drawStr(0, 0, "Environment");

    display.setCursor(0, 20);
    display.print("Temp: ");
    display.print(temp, 1);
    display.print("C");

    display.setCursor(0, 35);
    display.print("Humidity: ");
    display.print(humidity, 0);
    display.print("%");

    display.setCursor(0, 50);
    display.print("Light: ");
    display.print(light, 0);
    display.print(" lux");

    display.sendBuffer();
}

void DisplayManager::showSwitchesPage(bool switch1, bool switch2, bool switch3,
                                      const String &sw1Time, const String &sw2Time, const String &sw3Time)
{
    if (!displayFound)
        return;

    display.clearBuffer();
    display.drawStr(0, 0, "Switches");

    display.setCursor(0, 20);
    display.print("SW1: ");
    display.print(switch1 ? "ON " + sw1Time : "OFF");

    display.setCursor(0, 35);
    display.print("SW2: ");
    display.print(switch2 ? "ON " + sw2Time : "OFF");

    display.setCursor(0, 50);
    display.print("SW3: ");
    display.print(switch3 ? "ON " + sw3Time : "OFF");

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
    }
}