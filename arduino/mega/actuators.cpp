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
    // Active-Low Relaismodul: HIGH = aus, LOW = ein
    digitalWrite(ACTUATOR_PINS[i], HIGH);
  }
}

bool Actuators_set(uint8_t pin, bool state) {
  if (!Actuators_isValidPin(pin)) {
    return false;
  }

  // state=true bedeutet logisch EIN -> bei Active-Low physisch LOW
  digitalWrite(pin, state ? LOW : HIGH);
  return true;
}
