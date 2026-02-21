/*
  aktoren.cpp
  Nutzen:
  - Kapselt die komplette Relais-/Aktorsteuerung.
  Funktion:
  - Initialisiert alle freigegebenen Aktor-Pins und setzt Schaltbefehle
    fuer die Relais (active-low) sicher um.
*/

#include "mega_gemeinsam.h"

namespace {
  /*
    Zweck:
    - Validiert, ob ein Pin als Aktor freigegeben ist.
    Verhalten:
    - Vergleicht den Ziel-Pin mit allen Eintraegen in `AKTOR_PINS`.
    Rueckgabe:
    - `true`, wenn der Pin erlaubt ist, sonst `false`.
  */
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
  Zweck:
  - Bringt alle Relais beim Start in einen definierten Zustand.
  Verhalten:
  - Konfiguriert jeden Aktor-Pin als Ausgang und setzt ihn auf `HIGH` (AUS bei active-low).
  Rueckgabe:
  - Keine.
*/
void Aktoren_starten() {
  for (uint8_t i = 0; i < AKTOR_ANZAHL; i++) {
    pinMode(AKTOR_PINS[i], OUTPUT);
    // Active-Low Relaismodul: HIGH = aus, LOW = ein
    digitalWrite(AKTOR_PINS[i], HIGH);
  }
}

/*
  Zweck:
  - Schaltet genau einen freigegebenen Aktor-Pin.
  Verhalten:
  - Prueft den Pin gegen die Freigabeliste und setzt active-low:
    `zustand=true` -> LOW (EIN), `zustand=false` -> HIGH (AUS).
  Rueckgabe:
  - `true`, wenn erfolgreich gesetzt.
  - `false`, wenn der Pin nicht freigegeben ist.
*/
bool Aktoren_setzen(uint8_t pin, bool zustand) {
  if (!Aktoren_istGueltigerPin(pin)) {
    return false;
  }

  // zustand=true bedeutet logisch EIN -> bei Active-Low physisch LOW
  digitalWrite(pin, zustand ? LOW : HIGH);
  return true;
}
