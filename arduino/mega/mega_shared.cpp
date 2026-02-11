#include "mega_shared.h"

HardwareSerial &DEBUG_PORT = Serial;
HardwareSerial &DATA_PORT = Serial1;

const uint8_t ACTUATOR_PINS[] = {22, 23, 24, 25};
const uint8_t ACTUATOR_COUNT = sizeof(ACTUATOR_PINS) / sizeof(ACTUATOR_PINS[0]);
