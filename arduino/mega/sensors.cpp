#include "mega_shared.h"

/*
  sensors.cpp
  - Sensor-Handling
  - HC-SR04 auf D26 (TRIG) / D27 (ECHO)
*/

namespace {
  constexpr unsigned long HC_SR04_TIMEOUT_US = 30000UL;
  constexpr long HC_SR04_MIN_CM = 2;
  constexpr long HC_SR04_MAX_CM = 400;

  long readHcsr04DistanceCm(bool &ok, const char *&status) {
    // Trigger-Impuls: LOW -> HIGH (10us) -> LOW
    digitalWrite(HC_SR04_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(HC_SR04_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(HC_SR04_TRIG_PIN, LOW);

    unsigned long durationUs = pulseIn(HC_SR04_ECHO_PIN, HIGH, HC_SR04_TIMEOUT_US);
    if (durationUs == 0) {
      ok = false;
      status = "error_timeout";
      return -1;
    }

    long distanceCm = static_cast<long>(durationUs / 58UL);
    if (distanceCm < HC_SR04_MIN_CM || distanceCm > HC_SR04_MAX_CM) {
      ok = false;
      status = "error_range";
      return -1;
    }

    ok = true;
    status = "ok";
    return distanceCm;
  }
}

void Sensors_begin() {
  // Analoge Pins sind standardmaessig Input, explizit setzen fuer Klarheit
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  pinMode(HC_SR04_TRIG_PIN, OUTPUT);
  pinMode(HC_SR04_ECHO_PIN, INPUT);
  digitalWrite(HC_SR04_TRIG_PIN, LOW);
}

void Sensors_readSnapshot(SensorSnapshot &out) {
  // Platzhalterwerte fuer A0/A1 bleiben aktiv
  out.a0 = analogRead(A0);
  out.a1 = analogRead(A1);

  bool hcsr04Ok = false;
  const char *hcsr04Status = "error_unknown";
  out.hcsr04_distance_cm = readHcsr04DistanceCm(hcsr04Ok, hcsr04Status);
  out.hcsr04_status = hcsr04Status;

  out.uptime_ms = millis();
}
