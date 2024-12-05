// SimpleRuleEngine.cpp
#include "RulesEngine.h"

HomeSocketDevice *SimpleRuleEngine::getSocket(int socket_number)
{
    if (socket_number < 1 || socket_number > MAX_SOCKETS)
    {
        Serial.printf("Invalid socket number: %d\n", socket_number);
        return nullptr;
    }
    return sockets[socket_number - 1];
}

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

int SimpleRuleEngine::OR(int func1, int func2)
{
    int result = ((func1 > 0) || (func2 > 0)) ? 1 : 0;
    Serial.printf("OR operation: %d OR %d = %d\n", func1, func2, result);
    return result;
}

int SimpleRuleEngine::AND(int func1, int func2)
{
    int result = ((func1 > 0) && (func2 > 0)) ? 1 : 0;
    Serial.printf("AND operation: %d AND %d = %d\n", func1, func2, result);
    return result;
}

int SimpleRuleEngine::NOT(int func)
{
    int result = (func <= 0) ? 1 : 0;
    Serial.printf("NOT operation: NOT %d = %d\n", func, result);
    return result;
}

int SimpleRuleEngine::turnOnInbetween(const char *startTime, const char *endTime)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);
    int currentMinutes = currentHour * 60 + currentMinute;

    // Parse start time
    int startHours, startMinutes;
    sscanf(startTime, "%d:%d", &startHours, &startMinutes);
    int startTotalMinutes = startHours * 60 + startMinutes;

    // Parse end time
    int endHours, endMinutes;
    sscanf(endTime, "%d:%d", &endHours, &endMinutes);
    int endTotalMinutes = endHours * 60 + endMinutes;

    int result;
    // Handle cases where the period crosses midnight
    if (startTotalMinutes <= endTotalMinutes)
    {
        // Normal case (e.g., 21:00 to 23:00)
        result = (currentMinutes >= startTotalMinutes && currentMinutes < endTotalMinutes) ? 1 : 0;
    }
    else
    {
        // Crosses midnight (e.g., 23:00 to 06:00)
        result = (currentMinutes >= startTotalMinutes || currentMinutes < endTotalMinutes) ? 1 : 0;
    }

    Serial.printf("Time between %s and %s: %s\n", startTime, endTime, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::turnOnBefore(const char *timeStr)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);
    int currentMinutes = currentHour * 60 + currentMinute;

    // Parse input time string (format: "HH:MM")
    int hours, minutes;
    sscanf(timeStr, "%d:%d", &hours, &minutes);
    int targetMinutes = hours * 60 + minutes;

    int result = (currentMinutes < targetMinutes) ? 1 : 0;
    Serial.printf("Turn on before %s: %s\n", timeStr, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::turnOnAfter(const char *startTime)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);
    int currentMinutes = currentHour * 60 + currentMinute;

    // Parse start time
    int startHours, startMinutes;
    sscanf(startTime, "%d:%d", &startHours, &startMinutes);
    int startTotalMinutes = startHours * 60 + startMinutes;

    int result = (currentMinutes >= startTotalMinutes) ? 1 : 0;
    Serial.printf("Turn on after %s: %s\n", startTime, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::turnOffAfter(const char *startTime)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);
    int currentMinutes = currentHour * 60 + currentMinute;

    // Parse start time
    int startHours, startMinutes;
    sscanf(startTime, "%d:%d", &startHours, &startMinutes);
    int startTotalMinutes = startHours * 60 + startMinutes;

    int result = (currentMinutes < startTotalMinutes) ? 1 : 0; // Return 1 BEFORE the time, 0 after
    Serial.printf("Turn off after %s: %s\n", startTime, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::isWeekday(uint8_t dayPattern)
{
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);

    // Get current weekday (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    int today = timeinfo.tm_wday; // 0-6, Sunday=0

    // Convert weekday to our bit pattern (1 << 0 for Sunday, 1 << 1 for Monday, etc)
    uint8_t todayBit = (1 << today);

    int result = (dayPattern & todayBit) ? 1 : 0;
    Serial.printf("Day check (pattern: 0x%02X): %s\n", dayPattern, result ? "true" : "false");
    return result;
}

int SimpleRuleEngine::setMem(int slot, unsigned long value)
{
    if (slot < 0 || slot >= MEMORY_SLOTS)
    {
        Serial.printf("Memory slot %d out of range!\n", slot);
        return 0;
    }

    memory[slot] = value;
    Serial.printf("Set memory slot %d to %lu\n", slot, value);
    return 1;
}

int SimpleRuleEngine::readMem(int slot)
{
    if (slot < 0 || slot >= MEMORY_SLOTS)
    {
        Serial.printf("Memory slot %d out of range!\n", slot);
        return 0;
    }

    unsigned long value = memory[slot];
    Serial.printf("Read memory slot %d: %lu\n", slot, value);
    return value;
}

int SimpleRuleEngine::Delay(int memSlot, int triggerFunction)
{
    unsigned long currentMillis = millis();

    // If trigger function is true and timer isn't running
    if (triggerFunction && readMem(memSlot) == 0)
    {
        setMem(memSlot, currentMillis);
        Serial.printf("Starting delay in slot %d\n", memSlot);
        return 0;
    }

    // If timer is running
    if (readMem(memSlot) > 0)
    {
        const unsigned long DELAY_PERIOD = 5 * 60 * 1000; // 5 minutes in milliseconds

        if (currentMillis - readMem(memSlot) >= DELAY_PERIOD)
        {
            setMem(memSlot, 0); // Reset timer
            Serial.printf("Delay completed in slot %d\n", memSlot);
            return 1;
        }
        Serial.printf("Delay still running in slot %d\n", memSlot);
        return 0;
    }

    return 0;
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

int SimpleRuleEngine::isOn(int socket_number)
{
    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket)
        return 0;

    updateSocketDuration(socket_number);
    return socket->getCurrentState() ? 1 : 0;
}

int SimpleRuleEngine::isOff(int socket_number)
{
    return 1 - isOn(socket_number);
}

void SimpleRuleEngine::updateSocketDuration(int socket_number)
{
    if (socket_number < 1 || socket_number > MAX_SOCKETS)
        return;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    // Get current time
    int currentHour, currentMinute;
    timeSync.getCurrentHourMinute(currentHour, currentMinute);

    // If state changed, update timestamps
    HomeSocketDevice *socket = getSocket(socket_number);
    if (socket && socket->getCurrentState() != state.currentState)
    {
        state.currentState = socket->getCurrentState();
        state.lastStateChange = millis();
        state.lastChangeHour = currentHour;
        state.lastChangeMinute = currentMinute;
        state.stateChangeProcessed = false;

        Serial.printf("Socket %d state changed to: %s at %02d:%02d\n",
                      socket_number, state.currentState ? "ON" : "OFF",
                      currentHour, currentMinute);
    }
}

void SimpleRuleEngine::turnOn(int socket_number, int condition)
{
    Serial.printf("\nEvaluating ON rule for socket %d\n", socket_number);
    Serial.printf("Condition result: %s\n", condition ? "true" : "false");

    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket)
        return;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    // Only turn on if not already on and condition is true
    if (condition && (!socket->getCurrentState() || !state.stateChangeProcessed))
    {
        Serial.printf("→ Turning ON socket %d\n", socket_number);
        socket->setState(true);
        state.stateChangeProcessed = true;
    }
    else
    {
        Serial.printf("→ No action for socket %d\n", socket_number);
    }

    updateSocketDuration(socket_number);
    Serial.println("----------------------------------------");
}

void SimpleRuleEngine::turnOff(int socket_number, int condition)
{
    Serial.printf("\nEvaluating OFF rule for socket %d\n", socket_number);
    Serial.printf("Condition result: %s\n", condition ? "true" : "false");

    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket)
        return;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    // Only turn off if not already off and condition is true
    if (condition && (socket->getCurrentState() || !state.stateChangeProcessed))
    {
        Serial.printf("→ Turning OFF socket %d\n", socket_number);
        socket->setState(false);
        state.stateChangeProcessed = true;
    }
    else
    {
        Serial.printf("→ No action for socket %d\n", socket_number);
    }

    updateSocketDuration(socket_number);
    Serial.println("----------------------------------------");
}

int SimpleRuleEngine::hasBeenOnFor(int socket_number, int minutes)
{
    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket || !socket->getCurrentState())
        return 0;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    unsigned long duration = (millis() - state.lastStateChange) / (60 * 1000);
    int result = (duration >= minutes) ? 1 : 0;

    Serial.printf("Socket %d has been ON for %lu minutes (target: %d): %s\n",
                  socket_number, duration, minutes, result ? "true" : "false");

    return result;
}

int SimpleRuleEngine::hasBeenOffFor(int socket_number, int minutes)
{
    HomeSocketDevice *socket = getSocket(socket_number);
    if (!socket || socket->getCurrentState())
        return 0;

    int idx = socket_number - 1;
    SocketState &state = socketStates[idx];

    unsigned long duration = (millis() - state.lastStateChange) / (60 * 1000);
    int result = (duration >= minutes) ? 1 : 0;

    Serial.printf("Socket %d has been OFF for %lu minutes (target: %d): %s\n",
                  socket_number, duration, minutes, result ? "true" : "false");

    return result;
}