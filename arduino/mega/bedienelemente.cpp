/*
  bedienelemente.cpp
  Nutzen:
  - Kapselt die 5 Taster-Eingaenge und 5 LED-Ausgaenge.
  Funktion:
  - Initialisiert die Hardware, liest Tasterzustaende robust aus und
    setzt LED-Zustaende fuer Dashboard-Kommandos.
*/

#include "mega_gemeinsam.h"

namespace {
  constexpr uint8_t TASTER_PINS[BEDIENELEMENTE_ANZAHL] = {
    TASTER_BLAU_PUMPE_PIN,
    TASTER_GELB_STEPPER1_PIN,
    TASTER_GELB_STEPPER2_PIN,
    TASTER_ROT_STOPP_PIN,
    TASTER_GRUEN_START_QUIT_PIN
  };

  constexpr uint8_t LED_PINS[BEDIENELEMENTE_ANZAHL] = {
    LED_BLAU_PUMPE_PIN,
    LED_GELB_STEPPER1_PIN,
    LED_GELB_STEPPER2_PIN,
    LED_ROT_STOPP_PIN,
    LED_GRUEN_START_QUIT_PIN
  };

  uint8_t ledStatus[BEDIENELEMENTE_ANZAHL] = {0, 0, 0, 0, 0};

  bool Bedienelemente_istGueltigerIndex(uint8_t index1Bis5) {
    return index1Bis5 >= 1 && index1Bis5 <= BEDIENELEMENTE_ANZAHL;
  }
}

/*
  Zweck:
  - Initialisiert Taster und LEDs in definiertem Grundzustand.
  Verhalten:
  - Taster laufen mit internem Pullup (LOW = gedrueckt).
  - LEDs werden als OUTPUT konfiguriert und beim Start ausgeschaltet.
  Rueckgabe:
  - Keine.
*/
void Bedienelemente_starten() {
  for (uint8_t i = 0; i < BEDIENELEMENTE_ANZAHL; i++) {
    pinMode(TASTER_PINS[i], INPUT_PULLUP);
  }

  for (uint8_t i = 0; i < BEDIENELEMENTE_ANZAHL; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
    ledStatus[i] = 0;
  }
}

/*
  Zweck:
  - Liest die aktuellen Tasterzustaende.
  Verhalten:
  - Gibt fuer jeden Taster `1` bei gedrueckt und `0` bei losgelassen aus.
  Rueckgabe:
  - Keine (Ausgabe ueber Arrayparameter).
*/
void Bedienelemente_leseTaster(uint8_t ausgabe[BEDIENELEMENTE_ANZAHL]) {
  for (uint8_t i = 0; i < BEDIENELEMENTE_ANZAHL; i++) {
    const bool gedrueckt = (digitalRead(TASTER_PINS[i]) == LOW);
    ausgabe[i] = gedrueckt ? 1 : 0;
  }
}

/*
  Zweck:
  - Liefert die zuletzt gesetzten LED-Zustaende.
  Verhalten:
  - Gibt fuer jede LED `1` (ein) oder `0` (aus) zurueck.
  Rueckgabe:
  - Keine (Ausgabe ueber Arrayparameter).
*/
void Bedienelemente_leseLeds(uint8_t ausgabe[BEDIENELEMENTE_ANZAHL]) {
  for (uint8_t i = 0; i < BEDIENELEMENTE_ANZAHL; i++) {
    ausgabe[i] = ledStatus[i];
  }
}

/*
  Zweck:
  - Schaltet eine LED gezielt ein oder aus.
  Verhalten:
  - Indexbereich ist 1..5 entsprechend der Tasterreihenfolge.
  - Bei ungueltigem Index wird nichts geaendert.
  Rueckgabe:
  - `true` bei Erfolg, sonst `false`.
*/
bool Bedienelemente_setzeLed(uint8_t index1Bis5, bool ein) {
  if (!Bedienelemente_istGueltigerIndex(index1Bis5)) {
    return false;
  }

  const uint8_t idx = index1Bis5 - 1;
  digitalWrite(LED_PINS[idx], ein ? HIGH : LOW);
  ledStatus[idx] = ein ? 1 : 0;
  return true;
}
