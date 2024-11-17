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
};

#endif