// SimpleRuleEngine.h
#ifndef SIMPLE_RULE_ENGINE_H
#define SIMPLE_RULE_ENGINE_H

#include "GlobalVars.h"

class SimpleRuleEngine
{
private:
    float current_lux;
    static const int MEMORY_SLOTS = 32;  // used in TurnUntil
    bool memory[MEMORY_SLOTS] = {false}; // used in TurnUntil

public:
    SimpleRuleEngine();
    void updateLightLevel();
    int lightSensorAbove(int lux_value);
    int lightSensorBelow(int lux_value);
    int pingFound();
    int pingNotFound();
    void turnOn(int socket_number, int condition);
    void turnOff(int socket_number, int condition);

    // Time functions
    int after(const char *timeStr);
    int before(const char *timeStr);
    int isOn(int socket_number);
    int isOff(int socket_number);

    void turnOnAfter(int socket_number, const char *timeStr);
    void turnOffAfter(int socket_number, const char *timeStr);
    void turnOnBefore(int socket_number, const char *timeStr);
    void turnOffBefore(int socket_number, const char *timeStr);

    int TurnUntil(int memoryIndex, int turnOnCondition, int turnOffCondition);
};

#endif
