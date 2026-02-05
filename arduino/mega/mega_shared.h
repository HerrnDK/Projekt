#pragma once

#include <Arduino.h>

// Serielle Ports und Baudraten
constexpr unsigned long DEBUG_BAUD = 115200;
constexpr unsigned long DATA_BAUD = 9600;

extern HardwareSerial &DEBUG_PORT;
extern HardwareSerial &DATA_PORT;

// Beispiel-Aktuatoren (siehe PINOUT.md)
extern const uint8_t ACTUATOR_PINS[];
extern const uint8_t ACTUATOR_COUNT;

// Struktur fuer Sensordaten
struct SensorSnapshot {
  int a0;
  int a1;
  unsigned long uptime_ms;
};

// Modul-APIs
void Actuators_begin();
bool Actuators_set(uint8_t pin, bool state);

void Sensors_begin();
void Sensors_readSnapshot(SensorSnapshot &out);

void Data_begin();
void Data_tick();
void Data_sendSensorSnapshot();
