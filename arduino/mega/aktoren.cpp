#include "mega_gemeinsam.h"

/*
  aktoren.cpp
  - Verwaltung der Relais-/Aktorpins
*/

namespace {
  // Prueft, ob ein Pin in der zentralen Aktorliste vorhanden ist.
  bool Aktoren_istGueltigerPin(uint8_t pin) {
    for (uint8_t i = 0; i < AKTOR_ANZAHL; i++) {
      if (AKTOR_PINS[i] == pin) {
        return true;
      }
    }
    return false;
  }
}

/*
  Initialisiert alle Aktoren beim Start.
  - setzt alle konfigurierten Pins auf OUTPUT
  - setzt alle Relais in den sicheren AUS-Zustand
*/
void Aktoren_starten() {
  for (uint8_t i = 0; i < AKTOR_ANZAHL; i++) {
    pinMode(AKTOR_PINS[i], OUTPUT);
    // Active-Low Relaismodul: HIGH = aus, LOW = ein
    digitalWrite(AKTOR_PINS[i], HIGH);
  }
}

/*
  Schaltet einen Aktor logisch ein oder aus.
  - zustand=true  bedeutet EIN
  - zustand=false bedeutet AUS
  Rueckgabe:
  - true  => Pin war gueltig und wurde gesetzt
  - false => Pin ist kein freigegebener Aktor-Pin
*/
bool Aktoren_setzen(uint8_t pin, bool zustand) {
  if (!Aktoren_istGueltigerPin(pin)) {
    return false;
  }

  // zustand=true bedeutet logisch EIN -> bei Active-Low physisch LOW
  digitalWrite(pin, zustand ? LOW : HIGH);
  return true;
}
