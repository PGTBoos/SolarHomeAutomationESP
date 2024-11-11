// TimeSync.cpp
#include <Arduino.h>
#include <time.h>

class TimeSync
{
private:
    const char *ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 3600;     // Netherlands is UTC+1
    const int daylightOffset_sec = 3600; // 3600 seconds = 1 hour

public:
    TimeSync() {}

    void begin()
    {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

        // Wait for time to be set
        time_t now = 0;
        struct tm timeinfo = {0};
        int retry = 0;
        const int retry_count = 10;

        while (timeinfo.tm_year < (2024 - 1900) && ++retry < retry_count)
        {
            Serial.printf("Waiting for time sync... (%d/%d)\n", retry, retry_count);
            delay(1000);
            time(&now);
            localtime_r(&now, &timeinfo);
        }

        if (timeinfo.tm_year > (2024 - 1900))
        {
            Serial.println("Time synchronized!");
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
            Serial.printf("Current time: %s\n", time_str);
        }
        else
        {
            Serial.println("Could not sync time");
        }
    }

    String getCurrentTime()
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            return "Failed to obtain time";
        }
        char timeString[9];
        strftime(timeString, 9, "%H:%M:%S", &timeinfo);
        return String(timeString);
    }

    bool isTimeBetween(const char *startTime, const char *endTime)
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            return false;
        }

        int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;

        // Parse start time (format "HH:MM")
        int startHour, startMin;
        sscanf(startTime, "%d:%d", &startHour, &startMin);
        int startMinutes = startHour * 60 + startMin;

        // Parse end time
        int endHour, endMin;
        sscanf(endTime, "%d:%d", &endHour, &endMin);
        int endMinutes = endHour * 60 + endMin;

        if (endMinutes < startMinutes)
        { // Handles overnight periods
            return currentMinutes >= startMinutes || currentMinutes <= endMinutes;
        }

        return currentMinutes >= startMinutes && currentMinutes <= endMinutes;
    }

    // Get hours and minutes as integers
    void getCurrentHourMinute(int &hour, int &minute)
    {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            hour = timeinfo.tm_hour;
            minute = timeinfo.tm_min;
        }
        else
        {
            hour = 0;
            minute = 0;
        }
    }

    // Get current time as minutes since midnight
    int getCurrentMinutes()
    {
        int hour, minute;
        getCurrentHourMinute(hour, minute);
        return hour * 60 + minute;
    }
};