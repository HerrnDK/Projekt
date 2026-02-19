#pragma once

#include <Arduino.h>

// Serielle Ports und Baudraten
constexpr unsigned long BAUDRATE_DEBUG = 115200;
constexpr unsigned long BAUDRATE_DATEN = 9600;

extern HardwareSerial &PORT_DEBUG;
extern HardwareSerial &PORT_DATEN;

// Aktoren (siehe PINOUT.md)
extern const uint8_t AKTOR_PINS[];
extern const uint8_t AKTOR_ANZAHL;

// Struktur fuer Sensordaten
struct SensorMomentaufnahme {
  long hcsr04_distanz_cm;
  const char *hcsr04_status;
  long tropfen_roh;
  const char *tropfen_status;
  long truebung_roh;
  const char *truebung_status;
  unsigned long laufzeit_ms;
};

// HC-SR04 Ultraschallsensor
constexpr uint8_t HC_SR04_TRIG_PIN = 26;
constexpr uint8_t HC_SR04_ECHO_PIN = 27;
// Funduino Tropfensensor (Analog)
constexpr uint8_t TROPFEN_SENSOR_PIN = A0;
// Wassertruebungssensor (Analog)
constexpr uint8_t TRUEBUNG_SENSOR_PIN = A1;

// RFID RC522 (SPI)
constexpr uint8_t RC522_PIN_SS = 53;
constexpr uint8_t RC522_PIN_RST = 49;
constexpr size_t RFID_UID_MAX_LAENGE = 32;

// Modul-APIs
void Aktoren_starten();
bool Aktoren_setzen(uint8_t pin, bool zustand);

void Sensoren_starten();
void Sensoren_lesenMomentaufnahme(SensorMomentaufnahme &ausgabe);
void Sensoren_lesenRfid(char *uidAusgabe, size_t uidAusgabeLaenge, const char *&statusAusgabe);
const char *Sensoren_holeRfidHardwareStatus();
const char *Sensoren_holeRfidProbeStatus();
uint8_t Sensoren_holeRfidVersionsRegister();

void Daten_starten();
void Daten_tick();
void Daten_sendenSensorMomentaufnahme();
void Daten_sendenRfidMomentaufnahme();
