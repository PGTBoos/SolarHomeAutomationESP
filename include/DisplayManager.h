#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <U8g2lib.h>
#include <Wire.h>

class DisplayManager
{
private:
    U8G2_SH1106_128X64_NONAME_F_HW_I2C display;
    bool displayFound = false;
    int currentPage = 0;
    unsigned long lastPageChange = 0;
    const unsigned long PAGE_DURATION = 500;

    void showPowerPage(float importPower, float exportPower);
    void showEnvironmentPage(float temp, float humidity, float light);
    void showSwitchesPage(bool switch1, bool switch2, bool switch3,
                          const String &sw1Time, const String &sw2Time, const String &sw3Time);
    void showInfoPage(const String &time, const String &ip, bool wifiConnected);

public:
    DisplayManager() : display(U8G2_R0, /* reset= */ U8X8_PIN_NONE) {}
    bool begin();

    // Basic display update (without info page)
    void updateDisplay(float importPower, float exportPower,
                       float temp, float humidity, float light,
                       bool sw1, bool sw2, bool sw3,
                       const String &sw1Time, const String &sw2Time, const String &sw3Time);
};

#endif