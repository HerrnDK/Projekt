#include "mega_shared.h"

#include <string.h>
#include <stdlib.h>

/*
  data.cpp
  - Uebersetzung der Daten fuer Node-RED (Serial1)
  - Erwartet ASCII-Kommandos (newline-terminiert):
    READ\n              -> sendet JSON-Snapshot (type=sensor)
    ACT,<pin>,<state>\n -> schaltet Aktor + JSON-ACK (type=act)
*/

namespace {
  constexpr size_t DATA_CMD_MAX = 128;
  char inputBuffer[DATA_CMD_MAX];
  uint8_t inputLen = 0;

  SensorSnapshot lastSnapshot = {-1, "error_init", 0};

  void Data_handleCommand(const char *cmd);
  bool Data_parseAct(const char *cmd, uint8_t &pin, uint8_t &state);
  void Data_sendActAck(uint8_t pin, uint8_t state, bool ok);
  void Data_sendError(const char *code);
}

void Data_begin() {
  inputLen = 0;
  Sensors_readSnapshot(lastSnapshot);
}

void Data_tick() {
  while (DATA_PORT.available() > 0) {
    char c = static_cast<char>(DATA_PORT.read());

    if (c == '\n' || c == '\r') {
      if (inputLen > 0) {
        inputBuffer[inputLen] = '\0';
        Data_handleCommand(inputBuffer);
        inputLen = 0;
      }
    } else {
      if (inputLen < DATA_CMD_MAX - 1) {
        inputBuffer[inputLen++] = c;
      } else {
        // Buffer-Overflow vermeiden
        inputLen = 0;
      }
    }
  }
}

void Data_sendSensorSnapshot() {
  SensorSnapshot snapshot;
  Sensors_readSnapshot(snapshot);
  lastSnapshot = snapshot;

  // JSON-Zeile fuer Node-RED (data_exchange_flow.json erwartet JSON)
  DATA_PORT.print("{\"type\":\"sensor\",\"hcsr04_distance_cm\":");
  DATA_PORT.print(snapshot.hcsr04_distance_cm);
  DATA_PORT.print(",\"hcsr04_status\":\"");
  DATA_PORT.print(snapshot.hcsr04_status);
  DATA_PORT.print("\"");
  DATA_PORT.print(",\"uptime_ms\":");
  DATA_PORT.print(snapshot.uptime_ms);
  DATA_PORT.println("}");
}

namespace {
  void Data_handleCommand(const char *cmd) {
    if (strcmp(cmd, "READ") == 0) {
      Data_sendSensorSnapshot();
      return;
    }

    if (strncmp(cmd, "ACT,", 4) == 0) {
      uint8_t pin = 0;
      uint8_t state = 0;
      if (Data_parseAct(cmd, pin, state)) {
        bool ok = Actuators_set(pin, state != 0);
        Data_sendActAck(pin, state, ok);
        if (!ok) {
          DEBUG_PORT.println("ACT: ungueltiger Pin");
        }
      } else {
        DEBUG_PORT.println("ACT: Parse-Fehler");
        Data_sendError("act_parse_error");
      }
      return;
    }

    DEBUG_PORT.print("Unbekanntes Kommando: ");
    DEBUG_PORT.println(cmd);
    Data_sendError("unknown_command");
  }

  bool Data_parseAct(const char *cmd, uint8_t &pin, uint8_t &state) {
    // Erwartet: ACT,<pin>,<state>
    const char *p = cmd + 4;
    char *end = nullptr;

    long pinVal = strtol(p, &end, 10);
    if (end == p || *end != ',') {
      return false;
    }

    long stateVal = strtol(end + 1, &end, 10);
    if (*end != '\0') {
      return false;
    }

    if (pinVal < 0 || pinVal > 255) {
      return false;
    }

    if (stateVal != 0 && stateVal != 1) {
      return false;
    }

    pin = static_cast<uint8_t>(pinVal);
    state = static_cast<uint8_t>(stateVal);
    return true;
  }

  void Data_sendActAck(uint8_t pin, uint8_t state, bool ok) {
    // Letzten Snapshot mitsenden, damit UI-Felder nicht leer werden
    DATA_PORT.print("{\"type\":\"act\",\"ok\":");
    DATA_PORT.print(ok ? 1 : 0);
    DATA_PORT.print(",\"pin\":");
    DATA_PORT.print(pin);
    DATA_PORT.print(",\"state\":");
    DATA_PORT.print(state ? 1 : 0);
    DATA_PORT.print(",\"hcsr04_distance_cm\":");
    DATA_PORT.print(lastSnapshot.hcsr04_distance_cm);
    DATA_PORT.print(",\"hcsr04_status\":\"");
    DATA_PORT.print(lastSnapshot.hcsr04_status);
    DATA_PORT.print("\"");
    DATA_PORT.print(",\"uptime_ms\":");
    DATA_PORT.print(lastSnapshot.uptime_ms);
    DATA_PORT.println("}");
  }

  void Data_sendError(const char *code) {
    DATA_PORT.print("{\"type\":\"error\",\"code\":\"");
    DATA_PORT.print(code);
    DATA_PORT.print("\",\"hcsr04_distance_cm\":");
    DATA_PORT.print(lastSnapshot.hcsr04_distance_cm);
    DATA_PORT.print(",\"hcsr04_status\":\"");
    DATA_PORT.print(lastSnapshot.hcsr04_status);
    DATA_PORT.print("\"");
    DATA_PORT.print(",\"uptime_ms\":");
    DATA_PORT.print(lastSnapshot.uptime_ms);
    DATA_PORT.println("}");
  }
}
