#ifndef RULES_ENGINE_H
#define RULES_ENGINE_H

#include "GlobalVars.h"
#include "HomeSocketDevice.h"
#include <string>

// Forward declarations
class SimpleRuleEngine;
class Turn;
class Find;
class Logical;
class Time;
class Sensor;
class Memory;
class State;

// Base class for command groups
class CommandBase
{
protected:
    SimpleRuleEngine *engine;

public:
    CommandBase(SimpleRuleEngine *e) : engine(e) {}
};

// Command Classes
class Turn : public CommandBase
{
public:
    class OnOff
    {
    public:
        OnOff(Turn *t) : turn(t) {}
        int after(const char *timeStr);
        int before(const char *timeStr);
        int inbetween(const char *startTime, const char *endTime);

    private:
        Turn *turn;
    };

    Turn(SimpleRuleEngine *e) : CommandBase(e), on(this), off(this) {}
    OnOff on;
    OnOff off;
};

class Find : public CommandBase
{
public:
    Find(SimpleRuleEngine *e) : CommandBase(e) {}
    int response(const char *ip = nullptr);
    int noResponse(const char *ip = nullptr);
};

class Logical : public CommandBase
{
public:
    Logical(SimpleRuleEngine *e) : CommandBase(e) {}
    int OR(int func1, int func2);
    int AND(int func1, int func2);
    int NOT(int func);
};

class Time : public CommandBase
{
public:
    Time(SimpleRuleEngine *e) : CommandBase(e) {}
    int after(const char *timeStr);
    int before(const char *timeStr);
};

class Sensor : public CommandBase
{
public:
    Sensor(SimpleRuleEngine *e) : CommandBase(e) {}
    void updateLight();
    int lightAbove(int lux_value);
    int lightBelow(int lux_value);
};

class Memory : public CommandBase
{
public:
    Memory(SimpleRuleEngine *e) : CommandBase(e) {}
    int set(int slot, unsigned long value);
    int read(int slot);
    int delay(int memSlot, int triggerFunction);
    int until(int memoryIndex, int turnOnCondition, int turnOffCondition);
};

class State : public CommandBase
{
public:
    State(SimpleRuleEngine *e) : CommandBase(e) {}
    int isOn(int socket_number);
    int isOff(int socket_number);
    int hasBeenOnFor(int socket_number, int minutes);
    int hasBeenOffFor(int socket_number, int minutes);
};

// Main Engine Class
class SimpleRuleEngine
{
    friend class CommandBase;
    friend class Turn;
    friend class Find;
    friend class Logical;
    friend class Time;
    friend class Sensor;
    friend class Memory;
    friend class State;

    struct SocketState
    {
        bool currentState;
        unsigned long lastStateChange;
        int lastChangeHour;
        int lastChangeMinute;
        bool stateChangeProcessed;
    };

public:
    static const int MEMORY_SLOTS = 32;
    static const int MAX_SOCKETS = 3;

private:
    float current_lux;
    bool memorySlots[MEMORY_SLOTS] = {false};
    SocketState socketStates[MAX_SOCKETS];
    HomeSocketDevice *sockets[MAX_SOCKETS] = {nullptr};

    HomeSocketDevice *getSocket(int socket_number);
    void updateSocketDuration(int socket_number);

public:
    void initSocket(int socket_number, const char *ip)
    {
        if (socket_number >= 1 && socket_number <= MAX_SOCKETS)
        {
            int idx = socket_number - 1;
            if (sockets[idx])
            {
                delete sockets[idx];
            }
            sockets[idx] = new HomeSocketDevice(ip);
            socketStates[idx] = {false, 0, 0, 0, true};
        }
    }

    ~SimpleRuleEngine()
    {
        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            if (sockets[i])
            {
                delete sockets[i];
                sockets[i] = nullptr;
            }
        }
    }

    void update()
    {
        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            if (sockets[i])
            {
                sockets[i]->update();
            }
        }
    }

    // Command groups with cleaner names
    Turn turn;
    Find find;
    Logical logical;
    Time time;
    Sensor sensor;
    Memory memory;
    State state;

    // Weekday constants
    static const uint8_t SUNDAY = 0b00000001;
    static const uint8_t MONDAY = 0b00000010;
    static const uint8_t TUESDAY = 0b00000100;
    static const uint8_t WEDNESDAY = 0b00001000;
    static const uint8_t THURSDAY = 0b00010000;
    static const uint8_t FRIDAY = 0b00100000;
    static const uint8_t SATURDAY = 0b01000000;

    static const uint8_t WEEKDAYS = MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY;
    static const uint8_t WEEKEND = SATURDAY | SUNDAY;
    static const uint8_t EVERYDAY = WEEKDAYS | WEEKEND;

    SimpleRuleEngine() : turn(this),
                         find(this),
                         logical(this),
                         time(this),
                         sensor(this),
                         memory(this),
                         state(this)
    {
        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            sockets[i] = nullptr;
            socketStates[i] = {false, 0, 0, 0, true};
        }
    }

    // Core control functions
    void turnOn(int socket_number, int condition);
    void turnOff(int socket_number, int condition);
    int isWeekday(uint8_t dayPattern);
};

#endif