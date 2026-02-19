#include "mega_gemeinsam.h"

/*
  sensoren.cpp
  - zentrale Fassade fuer alle Sensor-Module
  - konkrete Sensorlogik liegt in separaten Dateien pro Sensor
*/

void Sensoren_starten() {
  Hcsr04_starten();
  Tropfen_starten();
  Truebung_starten();
  Rfid_starten();
}

void Sensoren_lesenMomentaufnahme(SensorMomentaufnahme &ausgabe) {
  const char *hcsr04Status = "error_unknown";
  ausgabe.hcsr04_distanz_cm = Hcsr04_leseDistanzCm(hcsr04Status);
  ausgabe.hcsr04_status = hcsr04Status;

  const char *tropfenStatus = "error_unknown";
  ausgabe.tropfen_roh = Tropfen_leseRohwert(tropfenStatus);
  ausgabe.tropfen_status = tropfenStatus;

  const char *truebungStatus = "error_unknown";
  ausgabe.truebung_roh = Truebung_leseRohwert(truebungStatus);
  ausgabe.truebung_status = truebungStatus;

  ausgabe.laufzeit_ms = millis();
}

void Sensoren_lesenRfid(char *uidAusgabe, size_t uidAusgabeLaenge, const char *&statusAusgabe) {
  Rfid_lesenUid(uidAusgabe, uidAusgabeLaenge, statusAusgabe);
}

const char *Sensoren_holeRfidHardwareStatus() {
  return Rfid_holeHardwareStatus();
}

const char *Sensoren_holeRfidProbeStatus() {
  return Rfid_holeProbeStatus();
}

uint8_t Sensoren_holeRfidVersionsRegister() {
  return Rfid_holeVersionsRegister();
}
