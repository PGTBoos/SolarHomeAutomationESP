#include "RulesEngine.h"

// Turn::OnOff implementations
int Turn::OnOff::after(const char *timeStr) {
  int currentHour, currentMinute;
  timeSync.getCurrentHourMinute(currentHour, currentMinute);
  int currentMinutes = currentHour * 60 + currentMinute;

  int startHours, startMinutes;
  sscanf(timeStr, "%d:%d", &startHours, &startMinutes);
  int startTotalMinutes = startHours * 60 + startMinutes;

  int result = (currentMinutes >= startTotalMinutes) ? 1 : 0;
  Serial.printf("Turn %s after %s: %s\n", (this == &turn->on) ? "on" : "off",
                timeStr, result ? "true" : "false");
  return result;
}

int Turn::OnOff::inbetween(const char *startTime, const char *endTime) {
  int currentHour, currentMinute;
  timeSync.getCurrentHourMinute(currentHour, currentMinute);
  int currentMinutes = currentHour * 60 + currentMinute;

  int startHours, startMinutes;
  sscanf(startTime, "%d:%d", &startHours, &startMinutes);
  int startTotalMinutes = startHours * 60 + startMinutes;

  int endHours, endMinutes;
  sscanf(endTime, "%d:%d", &endHours, &endMinutes);
  int endTotalMinutes = endHours * 60 + endMinutes;

  int result;
  if (startTotalMinutes <= endTotalMinutes) {
    result = (currentMinutes >= startTotalMinutes &&
              currentMinutes < endTotalMinutes)
                 ? 1
                 : 0;
  } else {
    result = (currentMinutes >= startTotalMinutes ||
              currentMinutes < endTotalMinutes)
                 ? 1
                 : 0;
  }

  Serial.printf("Turn %s between %s and %s: %s\n",
                (this == &turn->on) ? "on" : "off", startTime, endTime,
                result ? "true" : "false");
  return result;
}

// Find implementations for use in IP adres availability
int Find::response(const char *ip) {
  if (ip == nullptr && phoneCheck) {
    int result = phoneCheck->isDevicePresent() ? 1 : 0;
    Serial.printf("Device present: %s\n", result ? "true" : "false");
    return result;
  }
  // Implement IP-based detection here when needed
  return 0;
}

int Find::noResponse(const char *ip) {
  return 1 - response(ip);
}

// Logical implementations
int Logical::OR(int func1, int func2) {
  int result = ((func1 > 0) || (func2 > 0)) ? 1 : 0;
  Serial.printf("OR operation: %d OR %d = %d\n", func1, func2, result);
  return result;
}

int Logical::AND(int func1, int func2) {
  int result = ((func1 > 0) && (func2 > 0)) ? 1 : 0;
  Serial.printf("AND operation: %d AND %d = %d\n", func1, func2, result);
  return result;
}

int Logical::NOT(int func) {
  int result = (func <= 0) ? 1 : 0;
  Serial.printf("NOT operation: NOT %d = %d\n", func, result);
  return result;
}

int Time::random59() {
  TimeSync::TimeData t = timeSync.getTime();
  srand(t.dayOfYear);
  int result = rand() % 60;
  Serial.printf("Random59 for day %d: %d\n", t.dayOfYear, result);
  return result;
}

const char *Time::addTime(const char *timeStr, int minutes) {
  int hour, minute;
  sscanf(timeStr, "%d:%d", &hour, &minute);

  // Add minutes
  minute += minutes;

  // Handle overflow
  hour += minute / 60;
  minute = minute % 60;

  // Handle 24-hour wrap
  hour = hour % 24;

  // Format time string
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", hour, minute);

  Serial.printf("AddTime: %s + %d minutes = %s\n", timeStr, minutes,
                timeBuffer);

  return timeBuffer;
}

// Time implementations
int Time::after(const char *timeStr) {
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

int Time::before(const char *timeStr) {
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

// Sensor implementations
void Sensor::updateLight() {
  engine->current_lux = sensors.getLightLevel();
  Serial.printf("Current light level: %.1f lux\n", engine->current_lux);
}

int Sensor::lightAbove(int lux_value) {
  int result = (engine->current_lux > lux_value) ? 1 : 0;
  Serial.printf("Light > %d lux: %s\n", lux_value, result ? "true" : "false");
  return result;
}

int Sensor::lightBelow(int lux_value) {
  int result = (engine->current_lux < lux_value) ? 1 : 0;
  Serial.printf("Light < %d lux: %s\n", lux_value, result ? "true" : "false");
  return result;
}

// Memory implementations
int Memory::set(int slot, unsigned long value) {
  if (slot < 0 || slot >= SimpleRuleEngine::MEMORY_SLOTS) {
    Serial.printf("Memory slot %d out of range!\n", slot);
    return 0;
  }

  engine->memorySlots[slot] = value;
  Serial.printf("Set memory slot %d to %lu\n", slot, value);
  return 1;
}

int Memory::read(int slot) {
  if (slot < 0 || slot >= SimpleRuleEngine::MEMORY_SLOTS) {
    Serial.printf("Memory slot %d out of range!\n", slot);
    return 0;
  }

  unsigned long value = engine->memorySlots[slot];
  Serial.printf("Read memory slot %d: %lu\n", slot, value);
  return value;
}

int Memory::delay(int memSlot, int triggerFunction) {
  unsigned long currentMillis = millis();

  if (triggerFunction && read(memSlot) == 0) {
    set(memSlot, currentMillis);
    Serial.printf("Starting delay in slot %d\n", memSlot);
    return 0;
  }

  if (read(memSlot) > 0) {
    const unsigned long DELAY_PERIOD = 5 * 60 * 1000; // 5 minutes

    if (currentMillis - read(memSlot) >= DELAY_PERIOD) {
      set(memSlot, 0);
      Serial.printf("Delay completed in slot %d\n", memSlot);
      return 1;
    }
    Serial.printf("Delay still running in slot %d\n", memSlot);
  }
  return 0;
}

int Memory::until(int memoryIndex, int turnOnCondition, int turnOffCondition) {
  if (memoryIndex < 0 || memoryIndex >= SimpleRuleEngine::MEMORY_SLOTS) {
    Serial.printf("Memory slot %d out of range!\n", memoryIndex);
    return 0;
  }

  if (!engine->memorySlots[memoryIndex] && turnOnCondition) {
    engine->memorySlots[memoryIndex] = true;
  } else if (engine->memorySlots[memoryIndex] && turnOffCondition) {
    engine->memorySlots[memoryIndex] = false;
  }

  return engine->memorySlots[memoryIndex] ? 1 : 0;
}

// State implementations
int State::isOn(int socket_number) {
  HomeSocketDevice *socket = engine->getSocket(socket_number);
  if (!socket)
    return 0;

  engine->updateSocketDuration(socket_number);
  return socket->getCurrentState() ? 1 : 0;
}

int State::isOff(int socket_number) {
  return 1 - isOn(socket_number);
}

int State::hasBeenOnFor(int socket_number, int minutes) {
  HomeSocketDevice *socket = engine->getSocket(socket_number);
  if (!socket || !socket->getCurrentState())
    return 0;

  int idx = socket_number - 1;
  auto &state = engine->socketStates[idx];

  unsigned long duration = (millis() - state.lastStateChange) / (60 * 1000);
  int result = (duration >= minutes) ? 1 : 0;

  Serial.printf("Socket %d has been ON for %lu minutes (target: %d): %s\n",
                socket_number, duration, minutes, result ? "true" : "false");

  return result;
}

int State::hasBeenOffFor(int socket_number, int minutes) {
  HomeSocketDevice *socket = engine->getSocket(socket_number);
  if (!socket || socket->getCurrentState())
    return 0;

  int idx = socket_number - 1;
  auto &state = engine->socketStates[idx];

  unsigned long duration = (millis() - state.lastStateChange) / (60 * 1000);
  int result = (duration >= minutes) ? 1 : 0;

  Serial.printf("Socket %d has been OFF for %lu minutes (target: %d): %s\n",
                socket_number, duration, minutes, result ? "true" : "false");

  return result;
}

// Core SimpleRuleEngine implementations
HomeSocketDevice *SimpleRuleEngine::getSocket(int socket_number) {
  if (socket_number < 1 || socket_number > MAX_SOCKETS) {
    Serial.printf("Invalid socket number: %d\n", socket_number);
    return nullptr;
  }
  return sockets[socket_number - 1];
}

void SimpleRuleEngine::updateSocketDuration(int socket_number) {
  if (socket_number < 1 || socket_number > MAX_SOCKETS)
    return;

  int idx = socket_number - 1;
  SocketState &state = socketStates[idx];

  int currentHour, currentMinute;
  timeSync.getCurrentHourMinute(currentHour, currentMinute);

  HomeSocketDevice *socket = getSocket(socket_number);
  if (socket && socket->getCurrentState() != state.currentState) {
    state.currentState = socket->getCurrentState();
    state.lastStateChange = millis();
    state.lastChangeHour = currentHour;
    state.lastChangeMinute = currentMinute;
    state.stateChangeProcessed = false;

    Serial.printf("Socket %d state changed to: %s at %02d:%02d\n",
                  socket_number, state.currentState ? "ON" : "OFF", currentHour,
                  currentMinute);
  }
}

void SimpleRuleEngine::turnOn(int socket_number, int condition) {
  Serial.printf("\nEvaluating ON rule for socket %d\n", socket_number);
  Serial.printf("Condition result: %s\n", condition ? "true" : "false");

  HomeSocketDevice *socket = getSocket(socket_number);
  if (!socket)
    return;

  int idx = socket_number - 1;
  SocketState &state = socketStates[idx];

  if (condition &&
      (!socket->getCurrentState() || !state.stateChangeProcessed)) {
    Serial.printf("→ Turning ON socket %d\n", socket_number);
    socket->setState(true);
    state.stateChangeProcessed = true;
  } else {
    Serial.printf("→ No action for socket %d\n", socket_number);
  }

  updateSocketDuration(socket_number);
  Serial.println("----------------------------------------");
}

void SimpleRuleEngine::turnOff(int socket_number, int condition) {
  Serial.printf("\nEvaluating OFF rule for socket %d\n", socket_number);
  Serial.printf("Condition result: %s\n", condition ? "true" : "false");

  HomeSocketDevice *socket = getSocket(socket_number);
  if (!socket)
    return;

  int idx = socket_number - 1;
  SocketState &state = socketStates[idx];

  if (condition && (socket->getCurrentState() || !state.stateChangeProcessed)) {
    Serial.printf("→ Turning OFF socket %d\n", socket_number);
    socket->setState(false);
    state.stateChangeProcessed = true;
  } else {
    Serial.printf("→ No action for socket %d\n", socket_number);
  }

  updateSocketDuration(socket_number);
  Serial.println("----------------------------------------");
}

int SimpleRuleEngine::isWeekday(uint8_t dayPattern) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return 0;
  }

  int today = timeinfo.tm_wday; // 0-6, Sunday=0
  uint8_t todayBit = (1 << today);

  int result = (dayPattern & todayBit) ? 1 : 0;
  Serial.printf("Day check (pattern: 0x%02X): %s\n", dayPattern,
                result ? "true" : "false");
  return result;
}