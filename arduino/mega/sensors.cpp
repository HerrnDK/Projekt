#include "mega_shared.h"

/*
  sensors.cpp
  - Sensor-Handling (Platzhalter)
*/

void Sensors_begin() {
  // Analoge Pins sind standardmaessig Input, explizit setzen fuer Klarheit
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
}

void Sensors_readSnapshot(SensorSnapshot &out) {
  // Platzhalterwerte - hier spaeter echte Sensoren anbinden
  out.a0 = analogRead(A0);
  out.a1 = analogRead(A1);
  out.uptime_ms = millis();
}
