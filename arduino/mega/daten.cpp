/*
  daten.cpp
  Nutzen:
  - Kommunikationsbruecke zwischen Node-RED (Raspberry Pi) und Arduino.
  Funktion:
  - Nimmt Kommandos ueber Serial1 entgegen, ruft die passenden Modul-Funktionen auf
    und sendet strukturierte JSON-Antworten zurueck.
  Protokoll (newline-terminiert):
    READ\n              -> Sensor-Momentaufnahme (type=sensor)
    ACT,<pin>,<state>\n -> Aktor schalten + Bestaetigung (type=act)
    RFID\n              -> RFID-Momentaufnahme (type=rfid)
*/

#include "mega_gemeinsam.h"

#include <stdlib.h>
#include <string.h>

namespace {
  constexpr size_t DATEN_KOMMANDO_MAX = 128;
  char eingabePuffer[DATEN_KOMMANDO_MAX];
  uint8_t eingabeLaenge = 0;

  SensorMomentaufnahme letzterSnapshot = {
    -1, "error_init",
    -1, "error_init",
    -1, "error_init",
    -1, "error_init",
    0
  };

  void Daten_verarbeiteKommando(const char *kommando);
  bool Daten_parseActKommando(const char *kommando, uint8_t &pin, uint8_t &zustand);
  void Daten_sendeActBestaetigung(uint8_t pin, uint8_t zustand, bool erfolgreich);
  void Daten_sendeFehler(const char *fehlercode);
}

/*
  Zweck:
  - Initialisiert den Kommunikationszustand des Datenmoduls.
  Verhalten:
  - Leert den Eingabepuffer und erstellt einen ersten Sensor-Snapshot
    als Baseline fuer ACK-/Fehlermeldungen.
  Rueckgabe:
  - Keine.
*/
void Daten_starten() {
  eingabeLaenge = 0;
  Sensoren_lesenMomentaufnahme(letzterSnapshot);
}

/*
  Zweck:
  - Liest fortlaufend Kommandos von `PORT_DATEN`.
  Verhalten:
  - Pufferung bis Zeilenende (`\\n`/`\\r`), anschliessend Kommandoverarbeitung.
  - Bei Ueberlaenge wird der Puffer verworfen (Overflow-Schutz).
  Rueckgabe:
  - Keine.
*/
void Daten_tick() {
  while (PORT_DATEN.available() > 0) {
    char zeichen = static_cast<char>(PORT_DATEN.read());

    if (zeichen == '\n' || zeichen == '\r') {
      if (eingabeLaenge > 0) {
        eingabePuffer[eingabeLaenge] = '\0';
        Daten_verarbeiteKommando(eingabePuffer);
        eingabeLaenge = 0;
      }
    } else {
      if (eingabeLaenge < DATEN_KOMMANDO_MAX - 1) {
        eingabePuffer[eingabeLaenge++] = zeichen;
      } else {
        // Buffer-Overflow vermeiden.
        eingabeLaenge = 0;
      }
    }
  }
}

/*
  Zweck:
  - Sendet den aktuellen Sensorzustand als JSON.
  Verhalten:
  - Liest alle Sensoren, speichert den Snapshot und schreibt eine `type=sensor`
    Antwort im stabilen Protokollformat auf `PORT_DATEN`.
  Rueckgabe:
  - Keine.
*/
void Daten_sendenSensorMomentaufnahme() {
  SensorMomentaufnahme momentaufnahme;
  Sensoren_lesenMomentaufnahme(momentaufnahme);
  letzterSnapshot = momentaufnahme;

  PORT_DATEN.print("{\"type\":\"sensor\",\"hcsr04_distance_cm\":");
  PORT_DATEN.print(momentaufnahme.hcsr04_distanz_cm);
  PORT_DATEN.print(",\"hcsr04_status\":\"");
  PORT_DATEN.print(momentaufnahme.hcsr04_status);
  PORT_DATEN.print("\"");
  PORT_DATEN.print(",\"droplet_raw\":");
  PORT_DATEN.print(momentaufnahme.tropfen_roh);
  PORT_DATEN.print(",\"droplet_status\":\"");
  PORT_DATEN.print(momentaufnahme.tropfen_status);
  PORT_DATEN.print("\"");
  PORT_DATEN.print(",\"turbidity_raw\":");
  PORT_DATEN.print(momentaufnahme.truebung_roh);
  PORT_DATEN.print(",\"turbidity_status\":\"");
  PORT_DATEN.print(momentaufnahme.truebung_status);
  PORT_DATEN.print("\"");
  PORT_DATEN.print(",\"tds_raw\":");
  PORT_DATEN.print(momentaufnahme.tds_roh);
  PORT_DATEN.print(",\"tds_status\":\"");
  PORT_DATEN.print(momentaufnahme.tds_status);
  PORT_DATEN.print("\"");
  PORT_DATEN.print(",\"uptime_ms\":");
  PORT_DATEN.print(momentaufnahme.laufzeit_ms);
  PORT_DATEN.println("}");
}

/*
  Zweck:
  - Sendet den aktuellen RFID-Zustand als JSON.
  Verhalten:
  - Liest UID/Status ueber das RFID-Modul und uebertraegt zusaetzlich
    Hardwarestatus, Probe-Status und Versionsregister.
  Rueckgabe:
  - Keine.
*/
void Daten_sendenRfidMomentaufnahme() {
  char uid[RFID_UID_MAX_LAENGE];
  const char *rfidStatus = "error_init";
  Sensoren_lesenRfid(uid, sizeof(uid), rfidStatus);
  const char *rfidHardwareStatus = Sensoren_holeRfidHardwareStatus();
  const char *rfidProbeStatus = Sensoren_holeRfidProbeStatus();
  const uint8_t rfidVersionsRegister = Sensoren_holeRfidVersionsRegister();

  PORT_DATEN.print("{\"type\":\"rfid\",\"rfid_uid\":\"");
  PORT_DATEN.print(uid);
  PORT_DATEN.print("\",\"rfid_status\":\"");
  PORT_DATEN.print(rfidStatus);
  PORT_DATEN.print("\",\"rfid_hw_status\":\"");
  PORT_DATEN.print(rfidHardwareStatus);
  PORT_DATEN.print("\",\"rfid_probe_status\":\"");
  PORT_DATEN.print(rfidProbeStatus);
  PORT_DATEN.print("\",\"rfid_version_reg\":\"0x");
  if (rfidVersionsRegister < 0x10) {
    PORT_DATEN.print('0');
  }
  PORT_DATEN.print(rfidVersionsRegister, HEX);
  PORT_DATEN.print("\",\"uptime_ms\":");
  PORT_DATEN.print(millis());
  PORT_DATEN.println("}");
}

namespace {
  /*
    Zweck:
    - Verteilt ein empfangenes Kommando auf die passende Aktion.
    Verhalten:
    - Unterstuetzt `READ`, `RFID` und `ACT,<pin>,<state>`.
    - Unbekannte oder fehlerhafte Kommandos erzeugen ein Fehler-JSON.
    Rueckgabe:
    - Keine.
  */
  void Daten_verarbeiteKommando(const char *kommando) {
    if (strcmp(kommando, "READ") == 0) {
      Daten_sendenSensorMomentaufnahme();
      return;
    }

    if (strcmp(kommando, "RFID") == 0) {
      Daten_sendenRfidMomentaufnahme();
      return;
    }

    if (strncmp(kommando, "ACT,", 4) == 0) {
      uint8_t pin = 0;
      uint8_t zustand = 0;
      if (Daten_parseActKommando(kommando, pin, zustand)) {
        bool erfolgreich = Aktoren_setzen(pin, zustand != 0);
        Daten_sendeActBestaetigung(pin, zustand, erfolgreich);
        if (!erfolgreich) {
          PORT_DEBUG.println("ACT: ungueltiger Pin");
        }
      } else {
        PORT_DEBUG.println("ACT: Parse-Fehler");
        Daten_sendeFehler("act_parse_error");
      }
      return;
    }

    PORT_DEBUG.print("Unbekanntes Kommando: ");
    PORT_DEBUG.println(kommando);
    Daten_sendeFehler("unknown_command");
  }

  /*
    Zweck:
    - Parst und validiert ein `ACT` Kommando.
    Verhalten:
    - Erwartet exakt `ACT,<pin>,<state>` mit `state` in `{0,1}`.
    - Prueft Wertebereich und Format.
    Rueckgabe:
    - `true` bei gueltiger Eingabe, sonst `false`.
  */
  bool Daten_parseActKommando(const char *kommando, uint8_t &pin, uint8_t &zustand) {
    const char *werteStart = kommando + 4;
    char *ende = nullptr;

    long pinWert = strtol(werteStart, &ende, 10);
    if (ende == werteStart || *ende != ',') {
      return false;
    }

    long zustandWert = strtol(ende + 1, &ende, 10);
    if (*ende != '\0') {
      return false;
    }

    if (pinWert < 0 || pinWert > 255) {
      return false;
    }

    if (zustandWert != 0 && zustandWert != 1) {
      return false;
    }

    pin = static_cast<uint8_t>(pinWert);
    zustand = static_cast<uint8_t>(zustandWert);
    return true;
  }

  /*
    Zweck:
    - Sendet die Bestaetigung auf einen Aktor-Schaltbefehl.
    Verhalten:
    - Schreibt ein `type=act` JSON inklusive Schaltergebnis und letztem Sensor-Snapshot.
    Rueckgabe:
    - Keine.
  */
  void Daten_sendeActBestaetigung(uint8_t pin, uint8_t zustand, bool erfolgreich) {
    PORT_DATEN.print("{\"type\":\"act\",\"ok\":");
    PORT_DATEN.print(erfolgreich ? 1 : 0);
    PORT_DATEN.print(",\"pin\":");
    PORT_DATEN.print(pin);
    PORT_DATEN.print(",\"state\":");
    PORT_DATEN.print(zustand ? 1 : 0);
    PORT_DATEN.print(",\"hcsr04_distance_cm\":");
    PORT_DATEN.print(letzterSnapshot.hcsr04_distanz_cm);
    PORT_DATEN.print(",\"hcsr04_status\":\"");
    PORT_DATEN.print(letzterSnapshot.hcsr04_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"droplet_raw\":");
    PORT_DATEN.print(letzterSnapshot.tropfen_roh);
    PORT_DATEN.print(",\"droplet_status\":\"");
    PORT_DATEN.print(letzterSnapshot.tropfen_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"turbidity_raw\":");
    PORT_DATEN.print(letzterSnapshot.truebung_roh);
    PORT_DATEN.print(",\"turbidity_status\":\"");
    PORT_DATEN.print(letzterSnapshot.truebung_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"tds_raw\":");
    PORT_DATEN.print(letzterSnapshot.tds_roh);
    PORT_DATEN.print(",\"tds_status\":\"");
    PORT_DATEN.print(letzterSnapshot.tds_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"uptime_ms\":");
    PORT_DATEN.print(letzterSnapshot.laufzeit_ms);
    PORT_DATEN.println("}");
  }

  /*
    Zweck:
    - Meldet Kommunikations- oder Befehlsfehler an Node-RED.
    Verhalten:
    - Schreibt ein `type=error` JSON mit Fehlercode und letztem Sensor-Snapshot.
    Rueckgabe:
    - Keine.
  */
  void Daten_sendeFehler(const char *fehlercode) {
    PORT_DATEN.print("{\"type\":\"error\",\"code\":\"");
    PORT_DATEN.print(fehlercode);
    PORT_DATEN.print("\",\"hcsr04_distance_cm\":");
    PORT_DATEN.print(letzterSnapshot.hcsr04_distanz_cm);
    PORT_DATEN.print(",\"hcsr04_status\":\"");
    PORT_DATEN.print(letzterSnapshot.hcsr04_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"droplet_raw\":");
    PORT_DATEN.print(letzterSnapshot.tropfen_roh);
    PORT_DATEN.print(",\"droplet_status\":\"");
    PORT_DATEN.print(letzterSnapshot.tropfen_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"turbidity_raw\":");
    PORT_DATEN.print(letzterSnapshot.truebung_roh);
    PORT_DATEN.print(",\"turbidity_status\":\"");
    PORT_DATEN.print(letzterSnapshot.truebung_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"tds_raw\":");
    PORT_DATEN.print(letzterSnapshot.tds_roh);
    PORT_DATEN.print(",\"tds_status\":\"");
    PORT_DATEN.print(letzterSnapshot.tds_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"uptime_ms\":");
    PORT_DATEN.print(letzterSnapshot.laufzeit_ms);
    PORT_DATEN.println("}");
  }
}
