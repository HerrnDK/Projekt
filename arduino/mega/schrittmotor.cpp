/*
  schrittmotor.cpp
  Nutzen:
  - Kapselt die Ansteuerung von zwei TB6600-kompatiblen Schrittmotor-Treibern.
  Funktion:
  - Stellt Startbefehle fuer Drehung nach Zeit oder Winkel pro Motor bereit,
    fuehrt die Schrittimpulse nicht-blockierend im `tick()` aus und
    liefert Positions-/Statuswerte je Schrittmotor.
*/

#include "mega_gemeinsam.h"

namespace {
  constexpr bool SCHRITTMOTOR_RICHTUNG_VORWAERTS = true;
  constexpr bool SCHRITTMOTOR_ENABLE_ACTIVE_LOW = true;
  constexpr unsigned long SCHRITTMOTOR_SCHRITT_INTERVALL_US = 1200UL;
  constexpr unsigned int SCHRITTMOTOR_PULS_BREITE_US = 8;

  struct SchrittmotorZustand {
    uint8_t stepPin;
    uint8_t dirPin;
    uint8_t enaPin;
    bool laeuft;
    bool zeitmodus;
    bool richtungVorwaerts;
    unsigned long zeitendeMs;
    unsigned long letzterSchrittUs;
    long restSchritte;
    long positionsSchritte;
    const char *status;
  };

  SchrittmotorZustand schrittmotoren[SCHRITTMOTOR_ANZAHL] = {
    {SCHRITTMOTOR1_STEP_PIN, SCHRITTMOTOR1_DIR_PIN, SCHRITTMOTOR1_ENA_PIN, false, false, SCHRITTMOTOR_RICHTUNG_VORWAERTS, 0, 0, 0, 0, "idle"},
    {SCHRITTMOTOR2_STEP_PIN, SCHRITTMOTOR2_DIR_PIN, SCHRITTMOTOR2_ENA_PIN, false, false, SCHRITTMOTOR_RICHTUNG_VORWAERTS, 0, 0, 0, 0, "idle"}
  };

  bool Schrittmotor_istGueltigerIndex(uint8_t index1Bis2) {
    return index1Bis2 >= 1 && index1Bis2 <= SCHRITTMOTOR_ANZAHL;
  }

  SchrittmotorZustand *Schrittmotor_holeZustand(uint8_t index1Bis2) {
    if (!Schrittmotor_istGueltigerIndex(index1Bis2)) {
      return nullptr;
    }
    return &schrittmotoren[index1Bis2 - 1];
  }

  void Schrittmotor_setzeEnable(SchrittmotorZustand &motor, bool aktiv) {
    if (SCHRITTMOTOR_ENABLE_ACTIVE_LOW) {
      digitalWrite(motor.enaPin, aktiv ? LOW : HIGH);
    } else {
      digitalWrite(motor.enaPin, aktiv ? HIGH : LOW);
    }
  }

  void Schrittmotor_stoppe(SchrittmotorZustand &motor) {
    motor.laeuft = false;
    motor.zeitmodus = false;
    motor.restSchritte = 0;
    Schrittmotor_setzeEnable(motor, false);
    motor.status = "idle";
  }

  void Schrittmotor_schrittImpuls(SchrittmotorZustand &motor) {
    digitalWrite(motor.stepPin, HIGH);
    delayMicroseconds(SCHRITTMOTOR_PULS_BREITE_US);
    digitalWrite(motor.stepPin, LOW);

    motor.positionsSchritte += motor.richtungVorwaerts ? 1 : -1;
    motor.status = "running";
  }
}

/*
  Zweck:
  - Initialisiert beide TB6600 Treiber.
  Verhalten:
  - Setzt STEP/DIR/ENA Pins auf OUTPUT und deaktiviert beide Treiber.
  Rueckgabe:
  - Keine.
*/
void Schrittmotor_starten() {
  for (uint8_t i = 0; i < SCHRITTMOTOR_ANZAHL; i++) {
    SchrittmotorZustand &motor = schrittmotoren[i];
    pinMode(motor.stepPin, OUTPUT);
    pinMode(motor.dirPin, OUTPUT);
    pinMode(motor.enaPin, OUTPUT);

    digitalWrite(motor.stepPin, LOW);
    digitalWrite(motor.dirPin, SCHRITTMOTOR_RICHTUNG_VORWAERTS ? HIGH : LOW);
    Schrittmotor_setzeEnable(motor, false);
    Schrittmotor_stoppe(motor);
  }
}

/*
  Zweck:
  - Startet eine zeitgesteuerte Drehung fuer einen ausgewaehlten Motor.
  Verhalten:
  - Schaltet den Treiber ein und dreht fuer die angegebene Dauer vorwaerts.
  Rueckgabe:
  - `true` bei erfolgreichem Start, sonst `false`.
  - `statusAusgabe` liefert Fehler-/Statuscode.
*/
bool Schrittmotor_startDrehenZeitMsIndex(uint8_t index1Bis2, unsigned long dauerMs, const char *&statusAusgabe) {
  SchrittmotorZustand *motor = Schrittmotor_holeZustand(index1Bis2);
  if (motor == nullptr) {
    statusAusgabe = "invalid_stepper";
    return false;
  }

  if (motor->laeuft) {
    statusAusgabe = "busy";
    return false;
  }

  if (dauerMs == 0) {
    statusAusgabe = "invalid_duration";
    return false;
  }

  motor->zeitmodus = true;
  motor->laeuft = true;
  motor->richtungVorwaerts = SCHRITTMOTOR_RICHTUNG_VORWAERTS;
  motor->zeitendeMs = millis() + dauerMs;
  motor->letzterSchrittUs = 0;
  motor->restSchritte = 0;

  digitalWrite(motor->dirPin, motor->richtungVorwaerts ? HIGH : LOW);
  Schrittmotor_setzeEnable(*motor, true);
  motor->status = "running";
  statusAusgabe = "ok";
  return true;
}

/*
  Zweck:
  - Startet eine winkelfeste Drehung fuer einen ausgewaehlten Motor.
  Verhalten:
  - Berechnet die dafuer noetigen Schritte und fuehrt sie im Tick aus.
  Rueckgabe:
  - `true` bei erfolgreichem Start, sonst `false`.
  - `statusAusgabe` liefert Fehler-/Statuscode.
*/
bool Schrittmotor_startDrehenGradIndex(uint8_t index1Bis2, long grad, const char *&statusAusgabe) {
  SchrittmotorZustand *motor = Schrittmotor_holeZustand(index1Bis2);
  if (motor == nullptr) {
    statusAusgabe = "invalid_stepper";
    return false;
  }

  if (motor->laeuft) {
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

  motor->zeitmodus = false;
  motor->laeuft = true;
  motor->richtungVorwaerts = SCHRITTMOTOR_RICHTUNG_VORWAERTS;
  motor->zeitendeMs = 0;
  motor->letzterSchrittUs = 0;
  motor->restSchritte = schritte;

  digitalWrite(motor->dirPin, motor->richtungVorwaerts ? HIGH : LOW);
  Schrittmotor_setzeEnable(*motor, true);
  motor->status = "running";
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
  for (uint8_t i = 0; i < SCHRITTMOTOR_ANZAHL; i++) {
    SchrittmotorZustand &motor = schrittmotoren[i];
    if (!motor.laeuft) {
      continue;
    }

    if (motor.zeitmodus) {
      if (static_cast<long>(millis() - motor.zeitendeMs) >= 0) {
        Schrittmotor_stoppe(motor);
        continue;
      }
    } else if (motor.restSchritte <= 0) {
      Schrittmotor_stoppe(motor);
      continue;
    }

    const unsigned long jetztUs = micros();
    if ((jetztUs - motor.letzterSchrittUs) < SCHRITTMOTOR_SCHRITT_INTERVALL_US) {
      continue;
    }
    motor.letzterSchrittUs = jetztUs;

    Schrittmotor_schrittImpuls(motor);
    if (!motor.zeitmodus) {
      motor.restSchritte--;
      if (motor.restSchritte <= 0) {
        Schrittmotor_stoppe(motor);
      }
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
long Schrittmotor_holePositionGradIndex(uint8_t index1Bis2) {
  SchrittmotorZustand *motor = Schrittmotor_holeZustand(index1Bis2);
  if (motor == nullptr) {
    return 0;
  }

  const long schritteProUmdrehung = SCHRITTMOTOR_SCHRITTE_PRO_UMDREHUNG;
  if (schritteProUmdrehung <= 0) {
    return 0;
  }

  long modSchritte = motor->positionsSchritte % schritteProUmdrehung;
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
const char *Schrittmotor_holeStatusIndex(uint8_t index1Bis2) {
  SchrittmotorZustand *motor = Schrittmotor_holeZustand(index1Bis2);
  if (motor == nullptr) {
    return "invalid_stepper";
  }
  return motor->status;
}

/*
  Zweck:
  - Legacy-API fuer Motor 1 (abwaertskompatibel).
*/
bool Schrittmotor_startDrehenZeitMs(unsigned long dauerMs, const char *&statusAusgabe) {
  return Schrittmotor_startDrehenZeitMsIndex(1, dauerMs, statusAusgabe);
}

/*
  Zweck:
  - Legacy-API fuer Motor 1 (abwaertskompatibel).
*/
bool Schrittmotor_startDrehenGrad(long grad, const char *&statusAusgabe) {
  return Schrittmotor_startDrehenGradIndex(1, grad, statusAusgabe);
}

/*
  Zweck:
  - Legacy-API fuer Motor 1 (abwaertskompatibel).
*/
long Schrittmotor_holePositionGrad() {
  return Schrittmotor_holePositionGradIndex(1);
}

/*
  Zweck:
  - Legacy-API fuer Motor 1 (abwaertskompatibel).
*/
const char *Schrittmotor_holeStatus() {
  return Schrittmotor_holeStatusIndex(1);
}
