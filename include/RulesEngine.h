// SimpleRuleEngine.h
#ifndef SIMPLE_RULE_ENGINE_H
#define SIMPLE_RULE_ENGINE_H

#include "GlobalVars.h"

class SimpleRuleEngine
{
    struct SocketState
    {
        bool currentState;             // Current on/off state
        unsigned long lastStateChange; // When the current state started (millis)
        int lastChangeHour;            // Hour of last state change (for time persistence)
        int lastChangeMinute;          // Minute of last state change
        bool stateChangeProcessed;     // Flag to prevent multiple triggers in same state
    };

private:
    float current_lux;
    static const int MEMORY_SLOTS = 32;  // used in TurnUntil
    bool memory[MEMORY_SLOTS] = {false}; // used in TurnUntil

    static const int MAX_SOCKETS = 3;
    SocketState socketStates[MAX_SOCKETS];
    HomeSocketDevice *sockets[MAX_SOCKETS]; // Array instead of individual pointers

    // Helper function to get socket and validate
    HomeSocketDevice *getSocket(int socket_number);
    void updateSocketDuration(int socket_number);

public:
    // In the header:

    static const uint8_t SUNDAY = 0b00000001;
    static const uint8_t MONDAY = 0b00000010;
    static const uint8_t TUESDAY = 0b00000100;
    static const uint8_t WEDNESDAY = 0b00001000;
    static const uint8_t THURSDAY = 0b00010000;
    static const uint8_t FRIDAY = 0b00100000;
    static const uint8_t SATURDAY = 0b01000000;

    // Convenient combinations
    static const uint8_t WEEKDAYS = MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY;
    static const uint8_t WEEKEND = SATURDAY | SUNDAY;
    static const uint8_t EVERYDAY = WEEKDAYS | WEEKEND;

    int isWeekday(uint8_t dayPattern);

public:
    SimpleRuleEngine()
    {
        // Initialize socket pointers to nullptr
        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            sockets[i] = nullptr;
            socketStates[i] = {false, 0, 0, 0, true};
        }
    }
    void updateLightLevel();
    int lightSensorAbove(int lux_value);
    int lightSensorBelow(int lux_value);
    int phoneFound();
    int phoneNotFound();
    void turnOn(int socket_number, int condition);
    void turnOff(int socket_number, int condition);

    // Time functions
    int after(const char *timeStr);
    int before(const char *timeStr);
    int isOn(int socket_number);
    int isOff(int socket_number);

    // Time-based functions
    int turnOnInbetween(const char *startTime, const char *endTime);
    int turnOnAfter(const char *startTime);
    int turnOffAfter(const char *startTime);

    int turnOnBefore(const char *startTime);
    int turnOfBefore(const char *startTime);

    int hasBeenOnFor(int socket_number, int minutes);
    int hasBeenOffFor(int socket_number, int minutes);

    int setMem(int slot, unsigned long value);
    int readMem(int slot);
    int Delay(int memSlot, int triggerFunction);

    // Logical operators
    int OR(int func1, int func2);
    int AND(int func1, int func2);
    int NOT(int func);

    int TurnUntil(int memoryIndex, int turnOnCondition, int turnOffCondition);
};

#endif
