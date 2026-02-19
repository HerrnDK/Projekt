#include "mega_shared.h"

#include <stdlib.h>
#include <string.h>

/*
  data.cpp
  - Bruecke zwischen Node-RED und Arduino-Funktionen ueber Serial1
  - Erwartete ASCII-Kommandos (newline-terminiert):
    READ\n              -> sendet JSON Sensor-Momentaufnahme (type=sensor)
    ACT,<pin>,<state>\n -> schaltet Aktor + JSON-Bestaetigung (type=act)
    RFID\n              -> sendet RFID-Momentaufnahme (type=rfid)
*/

namespace {
  constexpr size_t DATEN_KOMMANDO_MAX = 128;
  char eingabePuffer[DATEN_KOMMANDO_MAX];
  uint8_t eingabeLaenge = 0;

  SensorMomentaufnahme letzterSnapshot = {-1, "error_init", -1, "error_init", -1, "error_init", 0};

  void Daten_verarbeiteKommando(const char *kommando);
  bool Daten_parseActKommando(const char *kommando, uint8_t &pin, uint8_t &zustand);
  void Daten_sendeActBestaetigung(uint8_t pin, uint8_t zustand, bool erfolgreich);
  void Daten_sendeFehler(const char *fehlercode);
}

/*
  Initialisiert das Datenmodul.
  - setzt den Eingabepuffer zurueck
  - liest einmal alle Sensoren, damit bei ACT/ERROR eine gueltige Baseline vorliegt
*/
void Daten_starten() {
  eingabeLaenge = 0;
  Sensoren_lesenMomentaufnahme(letzterSnapshot);
}

/*
  Zyklischer Poll fuer den seriellen Eingang.
  - sammelt Zeichen bis Newline
  - uebergibt komplette Kommandos an den Parser
  - verwirft ueberlange Eingaben (Overflow-Schutz)
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
  Liest alle Sensoren und sendet eine JSON-Zeile an Node-RED.
  Das JSON-Format bleibt absichtlich stabil, damit bestehende Flows kompatibel bleiben.
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
  PORT_DATEN.print(",\"uptime_ms\":");
  PORT_DATEN.print(momentaufnahme.laufzeit_ms);
  PORT_DATEN.println("}");
}

/*
  Liest den RFID-Status und sendet eine JSON-Zeile an Node-RED.
  Neben UID und Lesestatus werden auch Hardware-/Probe-Infos uebertragen.
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
    Zentrale Kommandoverarbeitung fuer READ / ACT / RFID.
    Unbekannte oder fehlerhafte Kommandos werden als Fehler-JSON gemeldet.
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
    Parst ein ACT-Kommando im Format ACT,<pin>,<state>.
    state ist nur 0 oder 1.
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
    Sendet eine ACT-Bestaetigung als JSON.
    Der letzte Sensor-Snapshot wird mitgesendet, damit UI-Felder nicht leer werden.
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
    PORT_DATEN.print(",\"uptime_ms\":");
    PORT_DATEN.print(letzterSnapshot.laufzeit_ms);
    PORT_DATEN.println("}");
  }

  /*
    Sendet ein Fehler-JSON mit Fehlercode und letztem Sensor-Snapshot.
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
    PORT_DATEN.print(",\"uptime_ms\":");
    PORT_DATEN.print(letzterSnapshot.laufzeit_ms);
    PORT_DATEN.println("}");
  }
}
