// DisplayManager.cpp
#include "DisplayManager.h"
#include <GlobalVars.h>
#include <WiFi.h>
#include <timeSync.h>

// Assuming timeSync is a global or class member variable
bool DisplayManager::begin() {
  Serial.println("\nInitializing \nOLED display...");

  if (!display.begin()) {
    Serial.println("SH1106 allocation failed");
    return false;
  }
  display.setContrast(64);
  // Setup display parameters
  display.setDisplayRotation(U8G2_R1);
  display.setFont(u8g2_font_profont10_tr); // Default font for labels
  display.setDrawColor(1);
  display.setFontPosTop();
  display.clearBuffer();

  // Draw initial test pattern
  display.drawStr(5, 8, "Starting.!!");
  display.drawFrame(0, 0, display.getWidth(), display.getHeight());
  display.sendBuffer();

  displayFound = true;
  currentPage = 0;
  lastPageChange = millis() - PAGE_DURATION;
  Serial.println("Display initialized successfully!");

  return true;
}

void DisplayManager::showPowerPage(float importPower, float exportPower,
                                   float totalImport, float totalExport) {
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
  auto formatPower = [](float power) -> String {
    if (abs(power) >= 1000) {
      // Display in kW with 2 decimals
      return String(power / 1000.0, 2) + " kW";
    } else {
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

  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 73, "Total:");

  // daily import and export
  float dailyImport = (config.yesterday > 0)
                          ? (totalImport - config.yesterdayImport)
                          : totalImport;
  float dailyExport = (config.yesterday > 0)
                          ? (totalExport - config.yesterdayExport)
                          : totalExport;

  // Daily import (used)
  display.setFont(u8g2_font_profont10_tr);
  String totalImportStr = "-" + String(dailyImport, 1) + " kWh";
  int totalWidth = display.getStrWidth(totalImportStr.c_str());
  display.setCursor(64 - totalWidth, 83);
  display.print(totalImportStr);

  // Daily export (produced)
  String totalExportStr = "+" + String(dailyExport, 1) + " kWh";
  totalWidth = display.getStrWidth(totalExportStr.c_str());
  display.setCursor(64 - totalWidth, 93);
  display.print(totalExportStr);

  display.sendBuffer();
}

void DisplayManager::showEnvironmentPage(float temp, float humidity,
                                         float light) {
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

void DisplayManager::showSwitchesPage(const bool switches[],
                                      const String switchTimes[]) {
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
  auto drawSwitch = [&](int y, const char *name, bool state) {
    display.setFont(u8g2_font_profont10_tr);
    display.drawStr(0, y, name);

    display.setFont(u8g2_font_7x14_tr);
    if (state) {
      display.drawBox(25, y - 1, 25, 14);
      display.setDrawColor(0);
      display.drawStr(27, y, "ON");
      display.setDrawColor(1);
    } else {
      display.drawStr(27, y, "OFF");
    }
  };

  for (int i = 0; i < NUM_SOCKETS; i++) {
    char name[10];
    snprintf(name, sizeof(name), "SW %d:", i + 1);
    drawSwitch(20 + (i * 20), name, switches[i]);
  }

  display.sendBuffer();
}

void DisplayManager::showInfoPage() {
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
  display.print(timeSync.getCurrentTime()); // Get time directly from TimeSync

  // WiFi Status
  display.setFont(u8g2_font_profont10_tr);
  display.drawStr(0, 45, "WiFi:");
  display.setFont(u8g2_font_7x14_tr);
  display.setCursor(30, 45); // Moved cursor right to align with "WiFi:"
  if (WiFi.status() == WL_CONNECTED) {
    display.print("Yes");

    // IP Address
    display.setFont(u8g2_font_robot_de_niro_tn);
    IPAddress ip = WiFi.localIP();

    // Calculate positions with 5 pixels per character
    const int charWidth = 5;
    const int blockWidth = charWidth * 3 + 1; // Each block is 3 digits
    const int startX = 0;
    const int y = 60; // Moved up to be closer to WiFi status

    // Print numbers
    display.setDrawColor(1);
    display.setCursor(startX, y);
    display.printf("%03d", ip[0]);

    display.setCursor(startX + blockWidth, y);
    display.printf("%03d", ip[1]);

    display.setCursor(startX + (blockWidth * 2), y);
    display.printf("%03d", ip[2]);

    display.setCursor(startX + (blockWidth * 3), y);
    display.printf("%03d", ip[3]);
  } else {
    display.print("Off");
  }

  // RAM Info
  display.setFont(u8g2_font_profont10_tr);
  uint32_t totalRam = ESP.getHeapSize() / 1024; // Total RAM in KB
  uint32_t freeRam = ESP.getFreeHeap() / 1024;  // Free RAM in KB
  display.drawStr(0, 75, "RAM:");
  display.setFont(u8g2_font_robot_de_niro_tn);
  display.setCursor(25,
                    75); // Moved from 25 to 30 to give more space after "RAM:"
  display.printf("%d", totalRam);
  display.setCursor(48,
                    75); // Moved from 45 to 55 to accommodate larger numbers
  display.printf("(%d)", freeRam);

  display.sendBuffer();
}

void DisplayManager::updateDisplay(float importPower, float exportPower,
                                   float totalImport, float totalExport,
                                   float temp, float humidity, float light,
                                   const bool switches[],
                                   const String switchTimes[]) {
  // if (!displayFound)
  //   Serial.println("Display not found");
  // return;
  Serial.printf("UpdateDisplay called - currentPage: %d\n", currentPage);
  // Rotate pages every PAGE_DURATION milliseconds
  if (millis() - lastPageChange >= PAGE_DURATION) {
    currentPage = (currentPage + 1) % 4; // Cycle through 4 pages
    lastPageChange = millis();
  } else {
    return;
  }

  // Show current page
  switch (currentPage) {
  case 0:
    showPowerPage(importPower, exportPower, totalImport, totalExport);
    break;
  case 1:
    showEnvironmentPage(temp, humidity, light);
    break;
  case 2:
    showSwitchesPage(switches, switchTimes);
    break;
  case 3:
    showInfoPage(); // Now using the parameter-less version
    break;
  }
}