/*
  sensor_tropfen.cpp
  Nutzen:
  - Kapselt das Auslesen des Funduino-Tropfensensors.
  Funktion:
  - Liest den Analogwert per Mittelung, prueft den Gueltigkeitsbereich
    und erkennt eine moegliche Trennung/Floating-Situation.
*/

#include "mega_gemeinsam.h"

namespace {
  constexpr long TROPFEN_MIN_ROH = 0;
  constexpr long TROPFEN_MAX_ROH = 1023;
  constexpr uint8_t TROPFEN_STICHPROBEN_ANZAHL = 5;
  constexpr int TROPFEN_FLOATING_SPREAD_SCHWELLE = 80;
  constexpr int TROPFEN_PULLUP_HOCH_SCHWELLE = 1000;
  constexpr int TROPFEN_PULLUP_DELTA_SCHWELLE = 250;
}

/*
  Zweck:
  - Initialisiert den Tropfensensor-Eingang.
  Verhalten:
  - Konfiguriert den Analogpin als INPUT.
  Rueckgabe:
  - Keine.
*/
void Tropfen_starten() {
  pinMode(TROPFEN_SENSOR_PIN, INPUT);
}

/*
  Zweck:
  - Liest den Tropfensensor als robusten Rohwert.
  Verhalten:
  - Bildet einen Mittelwert aus mehreren ADC-Stichproben.
  - Prueft Bereich und fuehrt zusaetzlich einen Floating-Check mit Pullup durch.
  Rueckgabe:
  - Rohwert 0..1023 bei Erfolg, sonst `-1`.
  - Statustext wird ueber `statusAusgabe` gesetzt.
*/
long Tropfen_leseRohwert(const char *&statusAusgabe) {
  pinMode(TROPFEN_SENSOR_PIN, INPUT);

  long summe = 0;
  int minRoh = 1023;
  int maxRoh = 0;
  for (uint8_t i = 0; i < TROPFEN_STICHPROBEN_ANZAHL; i++) {
    const int stichprobe = analogRead(TROPFEN_SENSOR_PIN);
    summe += stichprobe;
    if (stichprobe < minRoh) {
      minRoh = stichprobe;
    }
    if (stichprobe > maxRoh) {
      maxRoh = stichprobe;
    }
    delayMicroseconds(200);
  }

  const int roh = static_cast<int>(summe / TROPFEN_STICHPROBEN_ANZAHL);
  if (roh < TROPFEN_MIN_ROH || roh > TROPFEN_MAX_ROH) {
    statusAusgabe = "error_range";
    return -1;
  }

  pinMode(TROPFEN_SENSOR_PIN, INPUT_PULLUP);
  delayMicroseconds(200);
  const int pullupStichprobe = analogRead(TROPFEN_SENSOR_PIN);
  pinMode(TROPFEN_SENSOR_PIN, INPUT);

  const bool floatingDurchRauschen = (maxRoh - minRoh) >= TROPFEN_FLOATING_SPREAD_SCHWELLE;
  const bool floatingDurchPullup = (pullupStichprobe >= TROPFEN_PULLUP_HOCH_SCHWELLE) &&
                                   ((pullupStichprobe - roh) >= TROPFEN_PULLUP_DELTA_SCHWELLE);
  if (floatingDurchRauschen || floatingDurchPullup) {
    statusAusgabe = "error_not_connected";
    return -1;
  }

  statusAusgabe = "ok";
  return static_cast<long>(roh);
}
