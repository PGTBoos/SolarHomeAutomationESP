// DisplayManager.h
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

class DisplayManager
{
private:
    Adafruit_SSD1306 display;
    bool displayFound = false;
    int currentPage = 0;
    unsigned long lastPageChange = 0;
    const unsigned long PAGE_DURATION = 3000;

    void showPowerPage(float importPower, float exportPower);
    void showEnvironmentPage(float temp, float humidity, float light);
    void showSwitchesPage(bool switch1, bool switch2, bool switch3,
                          const String &sw1Time, const String &sw2Time, const String &sw3Time);

public:
    DisplayManager();
    bool begin();
    void updateDisplay(float importPower, float exportPower,
                       float temp, float humidity, float light,
                       bool sw1, bool sw2, bool sw3,
                       const String &sw1Time, const String &sw2Time, const String &sw3Time);
};

#endif