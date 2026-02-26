/*
  schrittmotor.cpp
  Nutzen:
  - Kapselt die Ansteuerung eines TB6600-kompatiblen Schrittmotor-Treibers.
  Funktion:
  - Stellt Startbefehle fuer Drehung nach Zeit oder Winkel bereit,
    fuehrt die Schrittimpulse nicht-blockierend im `tick()` aus und
    stellt die aktuelle Soll-Position in Grad bereit.
*/

#include "mega_gemeinsam.h"

namespace {
  constexpr bool SCHRITTMOTOR_RICHTUNG_VORWAERTS = true;
  constexpr bool SCHRITTMOTOR_ENABLE_ACTIVE_LOW = true;
  constexpr unsigned long SCHRITTMOTOR_SCHRITT_INTERVALL_US = 1200UL;
  constexpr unsigned int SCHRITTMOTOR_PULS_BREITE_US = 8;

  bool schrittmotorLaeuft = false;
  bool schrittmotorZeitmodus = false;
  bool schrittmotorRichtungVorwaerts = SCHRITTMOTOR_RICHTUNG_VORWAERTS;

  unsigned long schrittmotorZeitendeMs = 0;
  unsigned long letzterSchrittUs = 0;
  long restSchritte = 0;
  long positionsSchritte = 0;

  const char *schrittmotorStatus = "idle";

  void Schrittmotor_setzeEnable(bool aktiv) {
    if (SCHRITTMOTOR_ENABLE_ACTIVE_LOW) {
      digitalWrite(SCHRITTMOTOR_ENA_PIN, aktiv ? LOW : HIGH);
    } else {
      digitalWrite(SCHRITTMOTOR_ENA_PIN, aktiv ? HIGH : LOW);
    }
  }

  void Schrittmotor_stoppe() {
    schrittmotorLaeuft = false;
    schrittmotorZeitmodus = false;
    restSchritte = 0;
    Schrittmotor_setzeEnable(false);
    schrittmotorStatus = "idle";
  }

  void Schrittmotor_schrittImpuls() {
    digitalWrite(SCHRITTMOTOR_STEP_PIN, HIGH);
    delayMicroseconds(SCHRITTMOTOR_PULS_BREITE_US);
    digitalWrite(SCHRITTMOTOR_STEP_PIN, LOW);

    positionsSchritte += schrittmotorRichtungVorwaerts ? 1 : -1;
    schrittmotorStatus = "running";
  }
}

/*
  Zweck:
  - Initialisiert den TB6600 Treiber.
  Verhalten:
  - Setzt STEP/DIR/ENA Pins auf OUTPUT und deaktiviert den Treiber zunaechst.
  Rueckgabe:
  - Keine.
*/
void Schrittmotor_starten() {
  pinMode(SCHRITTMOTOR_STEP_PIN, OUTPUT);
  pinMode(SCHRITTMOTOR_DIR_PIN, OUTPUT);
  pinMode(SCHRITTMOTOR_ENA_PIN, OUTPUT);

  digitalWrite(SCHRITTMOTOR_STEP_PIN, LOW);
  digitalWrite(SCHRITTMOTOR_DIR_PIN, SCHRITTMOTOR_RICHTUNG_VORWAERTS ? HIGH : LOW);
  Schrittmotor_setzeEnable(false);
  Schrittmotor_stoppe();
}

/*
  Zweck:
  - Startet eine zeitgesteuerte Drehung.
  Verhalten:
  - Schaltet den Treiber ein und dreht fuer die angegebene Dauer vorwaerts.
  Rueckgabe:
  - `true` bei erfolgreichem Start, sonst `false`.
  - `statusAusgabe` liefert Fehler-/Statuscode.
*/
bool Schrittmotor_startDrehenZeitMs(unsigned long dauerMs, const char *&statusAusgabe) {
  if (schrittmotorLaeuft) {
    statusAusgabe = "busy";
    return false;
  }

  if (dauerMs == 0) {
    statusAusgabe = "invalid_duration";
    return false;
  }

  schrittmotorZeitmodus = true;
  schrittmotorLaeuft = true;
  schrittmotorRichtungVorwaerts = SCHRITTMOTOR_RICHTUNG_VORWAERTS;
  schrittmotorZeitendeMs = millis() + dauerMs;
  letzterSchrittUs = 0;
  restSchritte = 0;

  digitalWrite(SCHRITTMOTOR_DIR_PIN, schrittmotorRichtungVorwaerts ? HIGH : LOW);
  Schrittmotor_setzeEnable(true);
  schrittmotorStatus = "running";
  statusAusgabe = "ok";
  return true;
}

/*
  Zweck:
  - Startet eine winkelfeste Drehung.
  Verhalten:
  - Berechnet die dafuer noetigen Schritte und fuehrt sie im Tick aus.
  Rueckgabe:
  - `true` bei erfolgreichem Start, sonst `false`.
  - `statusAusgabe` liefert Fehler-/Statuscode.
*/
bool Schrittmotor_startDrehenGrad(long grad, const char *&statusAusgabe) {
  if (schrittmotorLaeuft) {
    statusAusgabe = "busy";
    return false;
  }

  if (grad <= 0) {
    statusAusgabe = "invalid_degree";
    return false;
  }

  const long schritte = ((SCHRITTMOTOR_SCHRITTE_PRO_UMDREHUNG * grad) + 180L) / 360L;
  if (schritte <= 0) {
    statusAusgabe = "invalid_degree";
    return false;
  }

  schrittmotorZeitmodus = false;
  schrittmotorLaeuft = true;
  schrittmotorRichtungVorwaerts = SCHRITTMOTOR_RICHTUNG_VORWAERTS;
  schrittmotorZeitendeMs = 0;
  letzterSchrittUs = 0;
  restSchritte = schritte;

  digitalWrite(SCHRITTMOTOR_DIR_PIN, schrittmotorRichtungVorwaerts ? HIGH : LOW);
  Schrittmotor_setzeEnable(true);
  schrittmotorStatus = "running";
  statusAusgabe = "ok";
  return true;
}

/*
  Zweck:
  - Zyklische, nicht-blockierende Schritt-Ausfuehrung.
  Verhalten:
  - Gibt gemaess Intervall genau einen Schrittimpuls aus, prueft Ende
    nach Zeit oder Restschritten und stoppt den Treiber sauber.
  Rueckgabe:
  - Keine.
*/
void Schrittmotor_tick() {
  if (!schrittmotorLaeuft) {
    return;
  }

  if (schrittmotorZeitmodus) {
    if (static_cast<long>(millis() - schrittmotorZeitendeMs) >= 0) {
      Schrittmotor_stoppe();
      return;
    }
  } else if (restSchritte <= 0) {
    Schrittmotor_stoppe();
    return;
  }

  const unsigned long jetztUs = micros();
  if ((jetztUs - letzterSchrittUs) < SCHRITTMOTOR_SCHRITT_INTERVALL_US) {
    return;
  }
  letzterSchrittUs = jetztUs;

  Schrittmotor_schrittImpuls();
  if (!schrittmotorZeitmodus) {
    restSchritte--;
    if (restSchritte <= 0) {
      Schrittmotor_stoppe();
    }
  }
}

/*
  Zweck:
  - Liefert die aktuelle Soll-Position in Grad.
  Verhalten:
  - Rechnet die interne Schrittposition modulo Umdrehung auf 0..359 Grad um.
  Rueckgabe:
  - Ganzzahliger Positionswert in Grad.
*/
long Schrittmotor_holePositionGrad() {
  const long schritteProUmdrehung = SCHRITTMOTOR_SCHRITTE_PRO_UMDREHUNG;
  if (schritteProUmdrehung <= 0) {
    return 0;
  }

  long modSchritte = positionsSchritte % schritteProUmdrehung;
  if (modSchritte < 0) {
    modSchritte += schritteProUmdrehung;
  }

  return (modSchritte * 360L) / schritteProUmdrehung;
}

/*
  Zweck:
  - Liefert den aktuellen Schrittmotor-Status.
  Verhalten:
  - Gibt `running` oder `idle` zurueck.
  Rueckgabe:
  - Zeiger auf Statusstring.
*/
const char *Schrittmotor_holeStatus() {
  return schrittmotorStatus;
}

