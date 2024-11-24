// SimpleRuleEngine.cpp
#include "RulesEngine.h"

SimpleRuleEngine::SimpleRuleEngine() : current_lux(0) {}

int SimpleRuleEngine::TurnUntil(int memoryIndex, int turnOnCondition, int turnOffCondition)
{
    if (memoryIndex < 0 || memoryIndex >= MEMORY_SLOTS)
    {
        Serial.printf("Memory slot %d out of range!\n", memoryIndex);
        return 0;
    }

    if (!memory[memoryIndex] && turnOnCondition)
    {
        memory[memoryIndex] = true;
    }
    else if (memory[memoryIndex] && turnOffCondition)
    {
        memory[memoryIndex] = false;
    }

    return memory[memoryIndex] ? 1 : 0;
}

void SimpleRuleEngine::updateLightLevel()
{
    current_lux = sensors.getLightLevel();
    Serial.printf("Current light level: %.1f lux\n", current_lux);
}

int SimpleRuleEngine::lightSensorAbove(int lux_value)
{
    int result = (current_lux > lux_value) ? 1 : 0;
    Serial.printf("Light > %d lux: %s\n", lux_value, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::lightSensorBelow(int lux_value)
{
    int result = (current_lux < lux_value) ? 1 : 0;
    Serial.printf("Light < %d lux: %s\n", lux_value, result ? "true" : "false");
    return result;
}
int SimpleRuleEngine::after(const char *timeStr)
{
    int hour, minute;
    sscanf(timeStr, "%d:%d", &hour, &minute);

    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);

    int currentMins = currentHour * 60 + currentMinute;
    int targetMins = hour * 60 + minute;

    int result = (currentMins >= targetMins) ? 1 : 0;
    Serial.printf("After %s: %s\n", timeStr, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::before(const char *timeStr)
{
    int hour, minute;
    sscanf(timeStr, "%d:%d", &hour, &minute);

    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);

    int currentMins = currentHour * 60 + currentMinute;
    int targetMins = hour * 60 + minute;

    int result = (currentMins < targetMins) ? 1 : 0;
    Serial.printf("Before %s: %s\n", timeStr, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::isOn(int socket_number)
{
    HomeSocketDevice *socket = nullptr;
    switch (socket_number)
    {
    case 1:
        socket = socket1;
        break;
    case 2:
        socket = socket2;
        break;
    case 3:
        socket = socket3;
        break;
    }
    return socket ? (socket->getCurrentState() ? 1 : 0) : 0;
}

int SimpleRuleEngine::isOff(int socket_number)
{
    return 1 - isOn(socket_number);
}

// still need to code the turnOnAfter, turnOffAfter, turnOnBefore, and turnOffBefore functions
// they should be a int function as well ? or not ?.
// ea they take current time and compare it to the timeStr

void SimpleRuleEngine::turnOnAfter(int socket_number, const char *timeStr)
{
    turnOn(socket_number, after(timeStr));
}

void SimpleRuleEngine::turnOffAfter(int socket_number, const char *timeStr)
{
    turnOff(socket_number, after(timeStr));
}

void SimpleRuleEngine::turnOnBefore(int socket_number, const char *timeStr)
{
    turnOn(socket_number, before(timeStr));
}

void SimpleRuleEngine::turnOffBefore(int socket_number, const char *timeStr)
{
    turnOff(socket_number, before(timeStr));
}

int SimpleRuleEngine::pingFound()
{
    if (phoneCheck)
    {
        int result = phoneCheck->isDevicePresent() ? 1 : 0;
        Serial.printf("Phone present: %s\n", result ? "true" : "false");
        return result;
    }
    Serial.println("Phone check not configured");
    return 0;
}

int SimpleRuleEngine::pingNotFound()
{
    int result = 1 - pingFound();
    Serial.printf("Phone absent: %s\n", result ? "true" : "false");
    return result;
}

void SimpleRuleEngine::turnOn(int socket_number, int condition)
{
    Serial.printf("\nEvaluating ON rule for socket %d\n", socket_number);
    Serial.printf("Condition result: %s\n", condition ? "true" : "false");

    HomeSocketDevice *socket = nullptr;
    switch (socket_number)
    {
    case 1:
        socket = socket1;
        break;
    case 2:
        socket = socket2;
        break;
    case 3:
        socket = socket3;
        break;
    }

    if (condition && socket)
    {
        Serial.printf("→ Turning ON socket %d\n", socket_number);
        socket->setState(true);
    }
    else
    {
        Serial.printf("→ No action for socket %d\n", socket_number);
    }
    Serial.println("----------------------------------------");
}

void SimpleRuleEngine::turnOff(int socket_number, int condition)
{
    Serial.printf("\nEvaluating OFF rule for socket %d\n", socket_number);
    Serial.printf("Condition result: %s\n", condition ? "true" : "false");

    HomeSocketDevice *socket = nullptr;
    switch (socket_number)
    {
    case 1:
        socket = socket1;
        break;
    case 2:
        socket = socket2;
        break;
    case 3:
        socket = socket3;
        break;
    }

    if (condition && socket)
    {
        Serial.printf("→ Turning OFF socket %d\n", socket_number);
        socket->setState(false);
    }
    else
    {
        Serial.printf("→ No action for socket %d\n", socket_number);
    }
    Serial.println("----------------------------------------");
}
