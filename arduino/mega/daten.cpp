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
    LED,<idx>,<state>\n -> Taster-LED schalten (idx 1..5) + Bestaetigung (type=led)
    RFID\n              -> RFID-Momentaufnahme (type=rfid)
    STEPPER_5S\n        -> Schrittmotor 1 fuer 5 Sekunden starten (type=stepper)
    STEPPER_120\n       -> Schrittmotor 1 um ca. 120 Grad starten (type=stepper)
    STEPPER2_5S\n       -> Schrittmotor 2 fuer 5 Sekunden starten (type=stepper)
    STEPPER2_120\n      -> Schrittmotor 2 um ca. 120 Grad starten (type=stepper)
*/

#include "mega_gemeinsam.h"

#include <stdlib.h>
#include <string.h>

namespace {
  constexpr size_t DATEN_KOMMANDO_MAX = 128;
  char eingabePuffer[DATEN_KOMMANDO_MAX];
  uint8_t eingabeLaenge = 0;

  SensorMomentaufnahme letzterSnapshot = {};

  void Daten_verarbeiteKommando(const char *kommando);
  bool Daten_parseActKommando(const char *kommando, uint8_t &pin, uint8_t &zustand);
  bool Daten_parseLedKommando(const char *kommando, uint8_t &index1Bis5, uint8_t &zustand);
  void Daten_schreibeSensorFelderJson(const SensorMomentaufnahme &snapshot);
  void Daten_schreibeUptimeFeldJson(unsigned long laufzeitMs);
  void Daten_sendeActBestaetigung(uint8_t pin, uint8_t zustand, bool erfolgreich);
  void Daten_sendeLedBestaetigung(uint8_t index1Bis5, uint8_t zustand, bool erfolgreich);
  void Daten_sendeSchrittmotorBestaetigung(uint8_t stepperIndex1Bis2, const char *kommando, bool erfolgreich, const char *status);
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
  Daten_schreibeSensorFelderJson(momentaufnahme);
  Daten_schreibeUptimeFeldJson(momentaufnahme.laufzeit_ms);
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
    - Schreibt den gemeinsamen Sensorblock als JSON-Felder.
    Verhalten:
    - Gibt alle Sensor-Rohwerte, Statusfelder und Schrittmotorzustand aus.
    Rueckgabe:
    - Keine.
  */
  void Daten_schreibeSensorFelderJson(const SensorMomentaufnahme &snapshot) {
    PORT_DATEN.print(",\"hcsr04_status\":\"");
    PORT_DATEN.print(snapshot.hcsr04_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"droplet_raw\":");
    PORT_DATEN.print(snapshot.tropfen_roh);
    PORT_DATEN.print(",\"droplet_status\":\"");
    PORT_DATEN.print(snapshot.tropfen_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"turbidity_raw\":");
    PORT_DATEN.print(snapshot.truebung_roh);
    PORT_DATEN.print(",\"turbidity_status\":\"");
    PORT_DATEN.print(snapshot.truebung_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"tds_raw\":");
    PORT_DATEN.print(snapshot.tds_roh);
    PORT_DATEN.print(",\"tds_status\":\"");
    PORT_DATEN.print(snapshot.tds_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"dht11_temp_c\":");
    PORT_DATEN.print(snapshot.dht11_temperatur_c);
    PORT_DATEN.print(",\"dht11_humidity_pct\":");
    PORT_DATEN.print(snapshot.dht11_luftfeuchte_prozent);
    PORT_DATEN.print(",\"dht11_status\":\"");
    PORT_DATEN.print(snapshot.dht11_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"btn_blue_pump\":");
    PORT_DATEN.print(snapshot.taster_blau_pumpe);
    PORT_DATEN.print(",\"btn_yellow_stepper1\":");
    PORT_DATEN.print(snapshot.taster_gelb_stepper1);
    PORT_DATEN.print(",\"btn_yellow_stepper2\":");
    PORT_DATEN.print(snapshot.taster_gelb_stepper2);
    PORT_DATEN.print(",\"btn_red_stop\":");
    PORT_DATEN.print(snapshot.taster_rot_stopp);
    PORT_DATEN.print(",\"btn_green_start_ack\":");
    PORT_DATEN.print(snapshot.taster_gruen_start_quit);
    PORT_DATEN.print(",\"led_blue_pump\":");
    PORT_DATEN.print(snapshot.led_blau_pumpe);
    PORT_DATEN.print(",\"led_yellow_stepper1\":");
    PORT_DATEN.print(snapshot.led_gelb_stepper1);
    PORT_DATEN.print(",\"led_yellow_stepper2\":");
    PORT_DATEN.print(snapshot.led_gelb_stepper2);
    PORT_DATEN.print(",\"led_red_stop\":");
    PORT_DATEN.print(snapshot.led_rot_stopp);
    PORT_DATEN.print(",\"led_green_start_ack\":");
    PORT_DATEN.print(snapshot.led_gruen_start_quit);
    PORT_DATEN.print(",\"stepper_position_deg\":");
    PORT_DATEN.print(snapshot.schrittmotor_position_grad);
    PORT_DATEN.print(",\"stepper_status\":\"");
    PORT_DATEN.print(snapshot.schrittmotor_status);
    PORT_DATEN.print("\"");
    PORT_DATEN.print(",\"stepper2_position_deg\":");
    PORT_DATEN.print(snapshot.schrittmotor2_position_grad);
    PORT_DATEN.print(",\"stepper2_status\":\"");
    PORT_DATEN.print(snapshot.schrittmotor2_status);
    PORT_DATEN.print("\"");
  }

  /*
    Zweck:
    - Schreibt das Uptime-Feld im Protokollformat.
    Verhalten:
    - Ergaenzt `uptime_ms` als eigenes JSON-Feld.
    Rueckgabe:
    - Keine.
  */
  void Daten_schreibeUptimeFeldJson(unsigned long laufzeitMs) {
    PORT_DATEN.print(",\"uptime_ms\":");
    PORT_DATEN.print(laufzeitMs);
  }

  /*
    Zweck:
    - Verteilt ein empfangenes Kommando auf die passende Aktion.
    Verhalten:
    - Unterstuetzt `READ`, `RFID`, `ACT,<pin>,<state>`, `LED,<idx>,<state>`,
      `STEPPER_5S`, `STEPPER_120`, `STEPPER2_5S`, `STEPPER2_120`.
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

    if (strncmp(kommando, "LED,", 4) == 0) {
      uint8_t index1Bis5 = 0;
      uint8_t zustand = 0;
      if (Daten_parseLedKommando(kommando, index1Bis5, zustand)) {
        const bool erfolgreich = Bedienelemente_setzeLed(index1Bis5, zustand != 0);
        Daten_sendeLedBestaetigung(index1Bis5, zustand, erfolgreich);
        if (!erfolgreich) {
          PORT_DEBUG.println("LED: ungueltiger Index");
        }
      } else {
        PORT_DEBUG.println("LED: Parse-Fehler");
        Daten_sendeFehler("led_parse_error");
      }
      return;
    }

    if (strcmp(kommando, "STEPPER_5S") == 0) {
      const char *status = "error_unknown";
      const bool erfolgreich = Schrittmotor_startDrehenZeitMsIndex(1, 5000UL, status);
      Daten_sendeSchrittmotorBestaetigung(1, "run_5s", erfolgreich, status);
      return;
    }

    if (strcmp(kommando, "STEPPER_120") == 0) {
      const char *status = "error_unknown";
      const bool erfolgreich = Schrittmotor_startDrehenGradIndex(1, 120L, status);
      Daten_sendeSchrittmotorBestaetigung(1, "rotate_120", erfolgreich, status);
      return;
    }

    if (strcmp(kommando, "STEPPER2_5S") == 0) {
      const char *status = "error_unknown";
      const bool erfolgreich = Schrittmotor_startDrehenZeitMsIndex(2, 5000UL, status);
      Daten_sendeSchrittmotorBestaetigung(2, "run_5s", erfolgreich, status);
      return;
    }

    if (strcmp(kommando, "STEPPER2_120") == 0) {
      const char *status = "error_unknown";
      const bool erfolgreich = Schrittmotor_startDrehenGradIndex(2, 120L, status);
      Daten_sendeSchrittmotorBestaetigung(2, "rotate_120", erfolgreich, status);
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
    - Parst und validiert ein `LED` Kommando.
    Verhalten:
    - Erwartet exakt `LED,<index>,<state>` mit `index` in `1..5`
      und `state` in `{0,1}`.
    Rueckgabe:
    - `true` bei gueltiger Eingabe, sonst `false`.
  */
  bool Daten_parseLedKommando(const char *kommando, uint8_t &index1Bis5, uint8_t &zustand) {
    const char *werteStart = kommando + 4;
    char *ende = nullptr;

    long indexWert = strtol(werteStart, &ende, 10);
    if (ende == werteStart || *ende != ',') {
      return false;
    }

    long zustandWert = strtol(ende + 1, &ende, 10);
    if (*ende != '\0') {
      return false;
    }

    if (indexWert < 1 || indexWert > 5) {
      return false;
    }

    if (zustandWert != 0 && zustandWert != 1) {
      return false;
    }

    index1Bis5 = static_cast<uint8_t>(indexWert);
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
    Daten_schreibeSensorFelderJson(letzterSnapshot);
    Daten_schreibeUptimeFeldJson(letzterSnapshot.laufzeit_ms);
    PORT_DATEN.println("}");
  }

  /*
    Zweck:
    - Sendet die Bestaetigung auf einen LED-Schaltbefehl.
    Verhalten:
    - Schreibt ein `type=led` JSON inklusive Zielindex und Schaltergebnis.
    Rueckgabe:
    - Keine.
  */
  void Daten_sendeLedBestaetigung(uint8_t index1Bis5, uint8_t zustand, bool erfolgreich) {
    PORT_DATEN.print("{\"type\":\"led\",\"ok\":");
    PORT_DATEN.print(erfolgreich ? 1 : 0);
    PORT_DATEN.print(",\"index\":");
    PORT_DATEN.print(index1Bis5);
    PORT_DATEN.print(",\"state\":");
    PORT_DATEN.print(zustand ? 1 : 0);
    PORT_DATEN.print(",\"uptime_ms\":");
    PORT_DATEN.print(millis());
    PORT_DATEN.println("}");
  }

  /*
    Zweck:
    - Sendet die Bestaetigung fuer einen Schrittmotor-Befehl.
    Verhalten:
    - Uebertraegt Ergebnis, Kommando und aktuellen Positions-/Statuswert.
    Rueckgabe:
    - Keine.
  */
  void Daten_sendeSchrittmotorBestaetigung(uint8_t stepperIndex1Bis2, const char *kommando, bool erfolgreich, const char *status) {
    PORT_DATEN.print("{\"type\":\"stepper\",\"ok\":");
    PORT_DATEN.print(erfolgreich ? 1 : 0);
    PORT_DATEN.print(",\"stepper_id\":");
    PORT_DATEN.print(stepperIndex1Bis2);
    PORT_DATEN.print(",\"cmd\":\"");
    PORT_DATEN.print(kommando);
    PORT_DATEN.print("\",\"stepper_position_deg\":");
    PORT_DATEN.print(Schrittmotor_holePositionGradIndex(stepperIndex1Bis2));
    PORT_DATEN.print(",\"stepper_status\":\"");
    PORT_DATEN.print(status);
    PORT_DATEN.print("\",\"uptime_ms\":");
    PORT_DATEN.print(millis());
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
    Daten_schreibeSensorFelderJson(letzterSnapshot);
    Daten_schreibeUptimeFeldJson(letzterSnapshot.laufzeit_ms);
    PORT_DATEN.println("}");
  }
}
