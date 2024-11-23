// SimpleRuleEngine.cpp
#include "RulesEngine.h"

SimpleRuleEngine::SimpleRuleEngine() : current_lux(0) {}

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