/*
  sensor_tds.cpp
  Nutzen:
  - Kapselt das Auslesen des Ocean-TDS-Meter-Sensors.
  Funktion:
  - Liest den Analogwert per Mittelung und erkennt moegliche
    Trennungs-/Floating-Zustaende robust ueber Pullup-Pruefung.
*/

#include "mega_gemeinsam.h"

namespace {
  constexpr long TDS_MIN_ROH = 0;
  constexpr long TDS_MAX_ROH = 1023;
  constexpr uint8_t TDS_STICHPROBEN_ANZAHL = 7;
  constexpr int TDS_FLOATING_SPREAD_SCHWELLE = 140;
  constexpr int TDS_PULLUP_HOCH_SCHWELLE = 1000;
  constexpr int TDS_PULLUP_DELTA_SCHWELLE = 280;
  constexpr uint8_t TDS_TRENNUNG_BESTAETIGUNG_SNAPSHOTS = 2;
  uint8_t tdsTrennungVerdachtZaehler = 0;
}

/*
  Zweck:
  - Initialisiert den TDS-Sensor-Eingang.
  Verhalten:
  - Konfiguriert den Analogpin als INPUT.
  Rueckgabe:
  - Keine.
*/
void Tds_starten() {
  pinMode(TDS_SENSOR_PIN, INPUT);
}

/*
  Zweck:
  - Liest den TDS-Sensor als robusten Rohwert.
  Verhalten:
  - Bildet einen Mittelwert aus mehreren ADC-Stichproben.
  - Fuehrt Bereichs- und Floating-Checks durch.
  - Meldet Trennung/Floating erst nach Bestaetigung ueber mehrere Snapshots.
  Rueckgabe:
  - Rohwert 0..1023 bei Erfolg, sonst `-1`.
  - Statustext wird ueber `statusAusgabe` gesetzt.
*/
long Tds_leseRohwert(const char *&statusAusgabe) {
  pinMode(TDS_SENSOR_PIN, INPUT);

  long summe = 0;
  int minRoh = 1023;
  int maxRoh = 0;
  for (uint8_t i = 0; i < TDS_STICHPROBEN_ANZAHL; i++) {
    const int stichprobe = analogRead(TDS_SENSOR_PIN);
    summe += stichprobe;
    if (stichprobe < minRoh) {
      minRoh = stichprobe;
    }
    if (stichprobe > maxRoh) {
      maxRoh = stichprobe;
    }
    delayMicroseconds(200);
  }

  const int roh = static_cast<int>(summe / TDS_STICHPROBEN_ANZAHL);
  if (roh < TDS_MIN_ROH || roh > TDS_MAX_ROH) {
    statusAusgabe = "error_range";
    return -1;
  }

  pinMode(TDS_SENSOR_PIN, INPUT_PULLUP);
  delayMicroseconds(200);
  const int pullupStichprobe = analogRead(TDS_SENSOR_PIN);
  pinMode(TDS_SENSOR_PIN, INPUT);

  const bool floatingDurchRauschen = (maxRoh - minRoh) >= TDS_FLOATING_SPREAD_SCHWELLE;
  const bool floatingDurchPullup = (pullupStichprobe >= TDS_PULLUP_HOCH_SCHWELLE) &&
                                   ((pullupStichprobe - roh) >= TDS_PULLUP_DELTA_SCHWELLE);
  const bool trennungVermutet = floatingDurchRauschen && floatingDurchPullup;

  if (trennungVermutet) {
    if (tdsTrennungVerdachtZaehler < TDS_TRENNUNG_BESTAETIGUNG_SNAPSHOTS) {
      tdsTrennungVerdachtZaehler++;
    }
  } else {
    tdsTrennungVerdachtZaehler = 0;
  }

  if (tdsTrennungVerdachtZaehler >= TDS_TRENNUNG_BESTAETIGUNG_SNAPSHOTS) {
    statusAusgabe = "error_not_connected";
    return -1;
  }

  statusAusgabe = "ok";
  return static_cast<long>(roh);
}
