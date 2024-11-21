// TimeSync.h
#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>
#include <time.h>
#include <WiFi.h>

class TimeSync
{
private:
    const char *ntpServer = "nl.pool.ntp.org";    // Netherlands NTP pool
    const char *ntpServer2 = "0.nl.pool.ntp.org"; // Specific Dutch server
    const char *ntpServer3 = "1.nl.pool.ntp.org"; // Backup Dutch server
    const long gmtOffset_sec = 3600;              // Netherlands is UTC+1
    const int daylightOffset_sec = 3600;          // DST when applicable
    bool timeInitialized = false;

public:
    TimeSync() {}
    bool begin();
    void getCurrentHourMinute(int &hour, int &minute);
    String getCurrentTime();
    bool isTimeBetween(const char *startTime, const char *endTime);
    int getCurrentMinutes();
    bool isTimeSet() const { return timeInitialized; }

    int getDayOfWeek();  // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
    int getWeekNumber(); // 1-53
    int getMonth();      // 1-12
    int getYear();       // Full year (e.g., 2024)

    // Optional: helper method to get all time components at once
    struct TimeData
    {
        int year;      // Full year (2024)
        int month;     // 1-12
        int dayOfWeek; // 1-7 (Mon-Sun)
        int weekNum;   // 1-53
        int hour;      // 0-23
        int minute;    // 0-59
    };
    TimeData getTime(); // One function to get everything
};

#endif