// TimeSync.cpp
#include "TimeSync.h"

bool TimeSync::begin()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected - cannot sync time");
        return false;
    }

    // Configure NTP with Dutch servers
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2, ntpServer3);

    Serial.println("Attempting to sync with Dutch NTP servers...");

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 20;  // Number of retries
    const int retry_delay = 500; // ms between retries

    while (!getLocalTime(&timeinfo) && ++retry < retry_count)
    {
        Serial.printf("NTP Sync attempt %d/%d\n", retry, retry_count);
        if (retry == 5)
        {
            Serial.println("Initial NTP servers not responding, trying backup servers...");
        }
        delay(retry_delay);
    }

    if (getLocalTime(&timeinfo))
    {
        char time_str[25];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
        Serial.println("✓ Time synchronized successfully!");
        Serial.printf("Current time: %s\n", time_str);
        Serial.printf("Timezone: UTC+%d\n", (gmtOffset_sec + daylightOffset_sec) / 3600);
        timeInitialized = true;
        return true;
    }
    else
    {
        Serial.println("× Failed to sync time after multiple attempts");
        Serial.println("Diagnostic information:");
        Serial.printf("WiFi status: %d\n", WiFi.status());
        Serial.printf("WiFi SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("WiFi IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.println("Please check:");
        Serial.println("1. WiFi connection is stable");
        Serial.println("2. NTP ports (123 UDP) aren't blocked");
        Serial.println("3. DNS resolution is working");
        timeInitialized = false;
        return false;
    }
}

void TimeSync::getCurrentHourMinute(int &hour, int &minute)
{
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
        hour = timeinfo.tm_hour;
        minute = timeinfo.tm_min;
        Serial.printf("Current time: %02d:%02d\n", hour, minute);
    }
    else
    {
        hour = 12;
        minute = 0;
        Serial.println("⚠ Could not get current time, using default (12:00)");
        timeInitialized = false; // Mark time as not synchronized
    }
}

String TimeSync::getCurrentTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("⚠ Failed to obtain time");
        timeInitialized = false; // Mark time as not synchronized
        return "Time not set";
    }

    char timeString[9];
    strftime(timeString, 9, "%H:%M:%S", &timeinfo);
    return String(timeString);
}

bool TimeSync::isTimeBetween(const char *startTime, const char *endTime)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("⚠ Failed to get time for comparison");
        timeInitialized = false; // Mark time as not synchronized
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

    // Debug time information
    Serial.printf("Time check - Current: %02d:%02d (%d min), ",
                  timeinfo.tm_hour, timeinfo.tm_min, currentMinutes);
    Serial.printf("Start: %02d:%02d (%d min), ",
                  startHour, startMin, startMinutes);
    Serial.printf("End: %02d:%02d (%d min)\n",
                  endHour, endMin, endMinutes);

    if (endMinutes < startMinutes)
    { // Handles overnight periods
        bool isInRange = currentMinutes >= startMinutes || currentMinutes <= endMinutes;
        Serial.printf("Overnight period check: %s\n", isInRange ? "true" : "false");
        return isInRange;
    }

    bool isInRange = currentMinutes >= startMinutes && currentMinutes <= endMinutes;
    Serial.printf("Same-day period check: %s\n", isInRange ? "true" : "false");
    return isInRange;
}

int TimeSync::getCurrentMinutes()
{
    int hour, minute;
    getCurrentHourMinute(hour, minute);
    return hour * 60 + minute;
}

TimeSync::TimeData TimeSync::getTime()
{
    TimeData t = {0};
    struct tm timeinfo;

    if (getLocalTime(&timeinfo))
    {
        t.year = timeinfo.tm_year + 1900;
        t.month = timeinfo.tm_mon + 1;
        // Convert to 1-7 where Monday=1 and Sunday=7
        t.dayOfWeek = timeinfo.tm_wday == 0 ? 7 : timeinfo.tm_wday;
        t.hour = timeinfo.tm_hour;
        t.minute = timeinfo.tm_min;
        t.weekNum = ((timeinfo.tm_yday + 7 - timeinfo.tm_wday) / 7) + 1;
        // day of the year
        t.dayOfYear = timeinfo.tm_yday;
    }
    else
    {
        Serial.println("Failed to get time");
    }
    return t;
}