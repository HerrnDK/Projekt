#include "mega_gemeinsam.h"

namespace {
  constexpr unsigned long HC_SR04_TIMEOUT_MIKROSEKUNDEN = 30000UL;
  constexpr long HC_SR04_MIN_DISTANZ_CM = 2;
  constexpr long HC_SR04_MAX_DISTANZ_CM = 400;
}

void Hcsr04_starten() {
  pinMode(HC_SR04_TRIG_PIN, OUTPUT);
  pinMode(HC_SR04_ECHO_PIN, INPUT);
  digitalWrite(HC_SR04_TRIG_PIN, LOW);
}

long Hcsr04_leseDistanzCm(const char *&statusAusgabe) {
  digitalWrite(HC_SR04_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_SR04_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_SR04_TRIG_PIN, LOW);

  unsigned long dauerMikrosekunden = pulseIn(HC_SR04_ECHO_PIN, HIGH, HC_SR04_TIMEOUT_MIKROSEKUNDEN);
  if (dauerMikrosekunden == 0) {
    statusAusgabe = "error_timeout";
    return -1;
  }

  long distanzCm = static_cast<long>(dauerMikrosekunden / 58UL);
  if (distanzCm < HC_SR04_MIN_DISTANZ_CM || distanzCm > HC_SR04_MAX_DISTANZ_CM) {
    statusAusgabe = "error_range";
    return -1;
  }

  statusAusgabe = "ok";
  return distanzCm;
}
