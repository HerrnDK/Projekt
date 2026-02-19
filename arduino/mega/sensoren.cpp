/*
  sensoren.cpp
  Nutzen:
  - Einheitliche Sensor-Schnittstelle fuer den Rest des Systems.
  Funktion:
  - Startet alle Sensor-Teilmodule, aggregiert Messwerte in einer gemeinsamen
    Momentaufnahme und reicht RFID-Funktionen zentral weiter.
*/

#include "mega_gemeinsam.h"

/*
  Zweck:
  - Initialisiert alle Sensor-Teilmodule zentral.
  Verhalten:
  - Ruft Startfunktionen fuer HC-SR04, Tropfen, Truebung und RFID auf.
  Rueckgabe:
  - Keine.
*/
void Sensoren_starten() {
  Hcsr04_starten();
  Tropfen_starten();
  Truebung_starten();
  Tds_starten();
  Rfid_starten();
}

/*
  Zweck:
  - Liefert eine vollstaendige Sensor-Momentaufnahme.
  Verhalten:
  - Ruft alle Sensor-Lesefunktionen auf, uebernimmt deren Statuscodes
    und setzt die Laufzeit in Millisekunden.
  Rueckgabe:
  - Keine (Ausgabe ueber Referenzparameter `ausgabe`).
*/
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

  const char *tdsStatus = "error_unknown";
  ausgabe.tds_roh = Tds_leseRohwert(tdsStatus);
  ausgabe.tds_status = tdsStatus;

  ausgabe.laufzeit_ms = millis();
}

/*
  Zweck:
  - Reicht eine RFID-UID-Anfrage an das RFID-Teilmodul weiter.
  Verhalten:
  - Delegiert Lesen und Statusermittlung direkt an `Rfid_lesenUid`.
  Rueckgabe:
  - Keine (UID/Status ueber Referenzparameter).
*/
void Sensoren_lesenRfid(char *uidAusgabe, size_t uidAusgabeLaenge, const char *&statusAusgabe) {
  Rfid_lesenUid(uidAusgabe, uidAusgabeLaenge, statusAusgabe);
}

/*
  Zweck:
  - Liefert den zuletzt bekannten RFID-Hardwarestatus.
  Verhalten:
  - Gibt den Statusstring aus dem RFID-Teilmodul durch.
  Rueckgabe:
  - Zeiger auf den aktuellen Statusstring.
*/
const char *Sensoren_holeRfidHardwareStatus() {
  return Rfid_holeHardwareStatus();
}

/*
  Zweck:
  - Liefert den letzten RFID-Probe-Status.
  Verhalten:
  - Gibt den zuletzt gespeicherten Probe-Status-Text aus dem RFID-Teilmodul durch.
  Rueckgabe:
  - Zeiger auf den Probe-Statusstring.
*/
const char *Sensoren_holeRfidProbeStatus() {
  return Rfid_holeProbeStatus();
}

/*
  Zweck:
  - Liefert das zuletzt gelesene RC522 Versionsregister.
  Verhalten:
  - Gibt den vom RFID-Teilmodul gespeicherten Registerwert durch.
  Rueckgabe:
  - 8-Bit Registerwert.
*/
uint8_t Sensoren_holeRfidVersionsRegister() {
  return Rfid_holeVersionsRegister();
}
