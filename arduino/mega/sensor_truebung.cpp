#include "mega_gemeinsam.h"

namespace {
  constexpr long TRUEBUNG_MIN_ROH = 0;
  constexpr long TRUEBUNG_MAX_ROH = 1023;
  constexpr uint8_t TRUEBUNG_STICHPROBEN_ANZAHL = 5;
  constexpr int TRUEBUNG_FLOATING_SPREAD_SCHWELLE = 220;
  constexpr int TRUEBUNG_ROH_HOCH_SCHWELLE = 850;
  constexpr int TRUEBUNG_PULLUP_HOCH_SCHWELLE = 1015;
  constexpr int TRUEBUNG_PULLUP_DELTA_SCHWELLE = 450;
  constexpr uint8_t TRUEBUNG_TRENNUNG_BESTAETIGUNG_SNAPSHOTS = 3;
  uint8_t truebungTrennungVerdachtZaehler = 0;
}

void Truebung_starten() {
  pinMode(TRUEBUNG_SENSOR_PIN, INPUT);
}

long Truebung_leseRohwert(const char *&statusAusgabe) {
  pinMode(TRUEBUNG_SENSOR_PIN, INPUT);

  long summe = 0;
  int minRoh = 1023;
  int maxRoh = 0;
  for (uint8_t i = 0; i < TRUEBUNG_STICHPROBEN_ANZAHL; i++) {
    const int stichprobe = analogRead(TRUEBUNG_SENSOR_PIN);
    summe += stichprobe;
    if (stichprobe < minRoh) {
      minRoh = stichprobe;
    }
    if (stichprobe > maxRoh) {
      maxRoh = stichprobe;
    }
    delayMicroseconds(200);
  }

  const int roh = static_cast<int>(summe / TRUEBUNG_STICHPROBEN_ANZAHL);
  if (roh < TRUEBUNG_MIN_ROH || roh > TRUEBUNG_MAX_ROH) {
    statusAusgabe = "error_range";
    return -1;
  }

  pinMode(TRUEBUNG_SENSOR_PIN, INPUT_PULLUP);
  delayMicroseconds(200);
  const int pullupStichprobe = analogRead(TRUEBUNG_SENSOR_PIN);
  pinMode(TRUEBUNG_SENSOR_PIN, INPUT);

  const bool floatingDurchRauschen = (maxRoh - minRoh) >= TRUEBUNG_FLOATING_SPREAD_SCHWELLE;
  const bool floatingDurchPullup = (roh >= TRUEBUNG_ROH_HOCH_SCHWELLE) &&
                                   (pullupStichprobe >= TRUEBUNG_PULLUP_HOCH_SCHWELLE) &&
                                   ((pullupStichprobe - roh) >= TRUEBUNG_PULLUP_DELTA_SCHWELLE);
  const bool trennungVermutet = floatingDurchRauschen && floatingDurchPullup;

  if (trennungVermutet) {
    if (truebungTrennungVerdachtZaehler < TRUEBUNG_TRENNUNG_BESTAETIGUNG_SNAPSHOTS) {
      truebungTrennungVerdachtZaehler++;
    }
  } else {
    truebungTrennungVerdachtZaehler = 0;
  }

  if (truebungTrennungVerdachtZaehler >= TRUEBUNG_TRENNUNG_BESTAETIGUNG_SNAPSHOTS) {
    statusAusgabe = "error_not_connected";
    return -1;
  }

  statusAusgabe = "ok";
  return static_cast<long>(roh);
}
