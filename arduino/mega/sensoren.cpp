/*
  sensoren.cpp
  Nutzen:
  - Einheitliche Sensor-Schnittstelle fuer den Rest des Systems.
  Funktion:
  - Startet alle Sensor-Teilmodule, aggregiert Messwerte in einer gemeinsamen
    Momentaufnahme und reicht RFID-Funktionen zentral weiter.
*/

#include "mega_gemeinsam.h"

namespace {
  /*
    Zweck:
    - Vereinheitlicht das Lesen von Sensorwert + Status.
    Verhalten:
    - Initialisiert den Status mit `error_unknown`, ruft die Lesefunktion auf
      und schreibt Ergebnis sowie finalen Status in die Zielvariablen.
    Rueckgabe:
    - Keine.
  */
  void Sensoren_leseWertMitStatus(long &zielWert, const char *&zielStatus, long (*lesefunktion)(const char *&)) {
    const char *status = "error_unknown";
    zielWert = lesefunktion(status);
    zielStatus = status;
  }
}

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
  Dht11_starten();
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
  Sensoren_leseWertMitStatus(ausgabe.hcsr04_distanz_cm, ausgabe.hcsr04_status, Hcsr04_leseDistanzCm);
  Sensoren_leseWertMitStatus(ausgabe.tropfen_roh, ausgabe.tropfen_status, Tropfen_leseRohwert);
  Sensoren_leseWertMitStatus(ausgabe.truebung_roh, ausgabe.truebung_status, Truebung_leseRohwert);
  Sensoren_leseWertMitStatus(ausgabe.tds_roh, ausgabe.tds_status, Tds_leseRohwert);

  const char *dht11Status = "error_unknown";
  Dht11_lesen(ausgabe.dht11_temperatur_c, ausgabe.dht11_luftfeuchte_prozent, dht11Status);
  ausgabe.dht11_status = dht11Status;

  uint8_t taster[BEDIENELEMENTE_ANZAHL] = {0, 0, 0, 0, 0};
  uint8_t leds[BEDIENELEMENTE_ANZAHL] = {0, 0, 0, 0, 0};
  Bedienelemente_leseTaster(taster);
  Bedienelemente_leseLeds(leds);
  ausgabe.taster_blau_pumpe = taster[0];
  ausgabe.taster_gelb_stepper1 = taster[1];
  ausgabe.taster_gelb_stepper2 = taster[2];
  ausgabe.taster_rot_stopp = taster[3];
  ausgabe.taster_gruen_start_quit = taster[4];
  ausgabe.led_blau_pumpe = leds[0];
  ausgabe.led_gelb_stepper1 = leds[1];
  ausgabe.led_gelb_stepper2 = leds[2];
  ausgabe.led_rot_stopp = leds[3];
  ausgabe.led_gruen_start_quit = leds[4];

  ausgabe.schrittmotor_position_grad = Schrittmotor_holePositionGradIndex(1);
  ausgabe.schrittmotor_status = Schrittmotor_holeStatusIndex(1);
  ausgabe.schrittmotor2_position_grad = Schrittmotor_holePositionGradIndex(2);
  ausgabe.schrittmotor2_status = Schrittmotor_holeStatusIndex(2);

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
