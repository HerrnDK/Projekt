#include "mega_gemeinsam.h"

HardwareSerial &PORT_DEBUG = Serial;
HardwareSerial &PORT_DATEN = Serial1;

const uint8_t AKTOR_PINS[] = {22, 23, 24, 25};
const uint8_t AKTOR_ANZAHL = sizeof(AKTOR_PINS) / sizeof(AKTOR_PINS[0]);
