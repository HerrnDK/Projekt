/*
  mega.ino
  - Sketch-Einstieg (setup/loop)
  - Fachlogik liegt in den Modulen unter .cpp
*/

#include "mega_gemeinsam.h"

void setup() {
  PORT_DEBUG.begin(BAUDRATE_DEBUG);  // USB-Debugschnittstelle
  PORT_DATEN.begin(BAUDRATE_DATEN);  // UART zu Node-RED (Serial1)

  Aktoren_starten();
  Sensoren_starten();
  Daten_starten();

  PORT_DEBUG.println("Mega Grundgeruest bereit.");
}

void loop() {
  // Serielle Kommunikation (Node-RED)
  Daten_tick();

  // Platz fuer weitere zyklische Tasks
}
