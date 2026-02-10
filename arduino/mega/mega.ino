/*
  mega.ino
  - Sketch-Einstieg (setup/loop)
  - Fachlogik liegt in den Modulen unter .cpp
*/

#include "mega_shared.h"

void setup() {
  DEBUG_PORT.begin(DEBUG_BAUD);  // USB Debug
  DATA_PORT.begin(DATA_BAUD);    // UART zu Node-RED (Serial1)

  Actuators_begin();
  Sensors_begin();
  Data_begin();

  DEBUG_PORT.println("Mega Grundgeruest bereit.");
}

void loop() {
  // Serielle Kommunikation (Node-RED)
  Data_tick();

  // Platz fuer weitere zyklische Tasks
}
