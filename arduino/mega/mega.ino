/*
  Arduino Mega 2560 R3 - Serial protocol for Node-RED (Raspberry Pi 4)

  Wiring:
    - Use Serial1 (pins 18=TX1, 19=RX1) for RX/TX to Raspberry Pi UART.
    - Keep Serial (USB) for programming/debug.

  Protocol (ASCII, newline terminated):
    - READ\n           -> Arduino responds with a single JSON line of sensor data.
    - ACT,<pin>,<state>\n
        <pin>   = digital pin number (e.g. 22)
        <state> = 0 or 1
        Response: OK,<pin>,<state>

  Notes:
    - Replace the example sensor reads with your real sensors later.
    - Default: 9600 baud on Serial1 (match on the Pi).
*/

#include <Arduino.h>

// Example pins for actuators (adjust later)
const uint8_t ACTUATOR_PINS[] = {22, 23, 24, 25};
const uint8_t ACTUATOR_COUNT = sizeof(ACTUATOR_PINS) / sizeof(ACTUATOR_PINS[0]);

String inputBuffer;

void setup() {
  Serial.begin(115200);    // USB debug/programming
  Serial1.begin(9600);     // UART to Raspberry Pi

  for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
    pinMode(ACTUATOR_PINS[i], OUTPUT);
    digitalWrite(ACTUATOR_PINS[i], LOW);
  }

  Serial.println("Mega Serial Control ready.");
}

bool isActuatorPin(uint8_t pin) {
  for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
    if (ACTUATOR_PINS[i] == pin) {
      return true;
    }
  }
  return false;
}

void sendSensorSnapshot() {
  // Placeholder values - replace with real sensor readings later
  int analog0 = analogRead(A0);
  int analog1 = analogRead(A1);

  // Example JSON response (single line)
  Serial1.print("{\"a0\":");
  Serial1.print(analog0);
  Serial1.print(",\"a1\":");
  Serial1.print(analog1);
  Serial1.print(",\"uptime_ms\":");
  Serial1.print(millis());
  Serial1.println("}");
}

void handleCommand(const String &cmd) {
  if (cmd == "READ") {
    sendSensorSnapshot();
    return;
  }

  if (cmd.startsWith("ACT,")) {
    // Format: ACT,<pin>,<state>
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);

    if (secondComma > 0) {
      int pin = cmd.substring(firstComma + 1, secondComma).toInt();
      int state = cmd.substring(secondComma + 1).toInt();

      if (isActuatorPin(pin)) {
        digitalWrite(pin, state ? HIGH : LOW);
        Serial1.print("OK,");
        Serial1.print(pin);
        Serial1.print(",");
        Serial1.println(state ? 1 : 0);
        return;
      }
    }
  }

  Serial1.println("ERR,UNKNOWN_COMMAND");
}

void loop() {
  while (Serial1.available() > 0) {
    char c = static_cast<char>(Serial1.read());
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        handleCommand(inputBuffer);
        inputBuffer = "";
      }
    } else {
      inputBuffer += c;
      if (inputBuffer.length() > 128) {
        inputBuffer = ""; // prevent runaway buffer
      }
    }
  }
}
