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
  long hcsr04_distance_cm;
  const char *hcsr04_status;
  unsigned long uptime_ms;
};

// HC-SR04 Ultraschallsensor
constexpr uint8_t HC_SR04_TRIG_PIN = 26;
constexpr uint8_t HC_SR04_ECHO_PIN = 27;

// RFID RC522 (SPI)
constexpr uint8_t RC522_SS_PIN = 53;
constexpr uint8_t RC522_RST_PIN = 49;
constexpr size_t RFID_UID_MAX_LEN = 32;

// Modul-APIs
void Actuators_begin();
bool Actuators_set(uint8_t pin, bool state);

void Sensors_begin();
void Sensors_readSnapshot(SensorSnapshot &out);
void Sensors_readRfid(char *uidOut, size_t uidOutLen, const char *&statusOut);

void Data_begin();
void Data_tick();
void Data_sendSensorSnapshot();
void Data_sendRfidSnapshot();
