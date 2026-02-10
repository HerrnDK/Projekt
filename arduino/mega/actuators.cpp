#include "mega_shared.h"

/*
  actuators.cpp
  - Aktoren-Handling
*/

namespace {
  bool Actuators_isValidPin(uint8_t pin) {
    for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
      if (ACTUATOR_PINS[i] == pin) {
        return true;
      }
    }
    return false;
  }
}

void Actuators_begin() {
  for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
    pinMode(ACTUATOR_PINS[i], OUTPUT);
    digitalWrite(ACTUATOR_PINS[i], LOW);
  }
}

bool Actuators_set(uint8_t pin, bool state) {
  if (!Actuators_isValidPin(pin)) {
    return false;
  }

  digitalWrite(pin, state ? HIGH : LOW);
  return true;
}
