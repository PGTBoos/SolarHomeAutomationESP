// SimpleRuleEngine.h
#ifndef SIMPLE_RULE_ENGINE_H
#define SIMPLE_RULE_ENGINE_H

#include "GlobalVars.h"

class SimpleRuleEngine
{
private:
    float current_lux;

public:
    SimpleRuleEngine();
    void updateLightLevel();
    int lightSensorAbove(int lux_value);
    int lightSensorBelow(int lux_value);
    int pingFound();
    int pingNotFound();
    void turnOn(int socket_number, int condition);
    void turnOff(int socket_number, int condition);
};

#endif
