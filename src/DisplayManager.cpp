// DisplayManager.cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin not used
#define SCREEN_ADDRESS 0x3C

class DisplayManager
{
private:
    Adafruit_SSD1306 display;
    bool displayFound = false;
    int currentPage = 0;
    unsigned long lastPageChange = 0;
    const unsigned long PAGE_DURATION = 3000; // 3 seconds per page

public:
    DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

    bool begin()
    {
        // Initialize display
        displayFound = display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
        if (!displayFound)
        {
            Serial.println("Could not find SSD1306 OLED display!");
            return false;
        }

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.println("Initializing...");
        display.display();
        return true;
    }

    void showPowerPage(float importPower, float exportPower)
    {
        if (!displayFound)
            return;

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("Power Monitor");
        display.println();

        display.print("Import: ");
        display.print(importPower, 1);
        display.println("W");

        display.print("Export: ");
        display.print(exportPower, 1);
        display.println("W");

        display.display();
    }

    void showEnvironmentPage(float temp, float humidity, float light)
    {
        if (!displayFound)
            return;

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("Environment");
        display.println();

        display.print("Temp: ");
        display.print(temp, 1);
        display.println("C");

        display.print("Humidity: ");
        display.print(humidity, 0);
        display.println("%");

        display.print("Light: ");
        display.print(light, 0);
        display.println(" lux");

        display.display();
    }

    void showSwitchesPage(bool switch1, bool switch2, bool switch3,
                          const String &sw1Time, const String &sw2Time, const String &sw3Time)
    {
        if (!displayFound)
            return;

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("Switches");
        display.println();

        display.print("SW1: ");
        display.println(switch1 ? "ON " + sw1Time : "OFF");

        display.print("SW2: ");
        display.println(switch2 ? "ON " + sw2Time : "OFF");

        display.print("SW3: ");
        display.println(switch3 ? "ON " + sw3Time : "OFF");

        display.display();
    }

    void updateDisplay(float importPower, float exportPower,
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
};