#include "mega_shared.h"

#include <SPI.h>
#include <MFRC522.h>
#include <stdio.h>
#include <string.h>

/*
  sensors.cpp
  - Sensor-Handling
  - HC-SR04 auf D26 (TRIG) / D27 (ECHO)
  - RFID RC522 via SPI (SS D53, RST D49)
*/

namespace {
  constexpr unsigned long HC_SR04_TIMEOUT_US = 30000UL;
  constexpr long HC_SR04_MIN_CM = 2;
  constexpr long HC_SR04_MAX_CM = 400;

  MFRC522 rfidReader(RC522_SS_PIN, RC522_RST_PIN);
  const char *rfidHardwareStatus = "error_not_initialized";

  bool isRfidReaderDetected() {
    const byte version = rfidReader.PCD_ReadRegister(MFRC522::VersionReg);
    return version != 0x00 && version != 0xFF;
  }

  void refreshRfidHardwareStatus() {
    // Reader kann waehrend Laufzeit ab-/angesteckt werden.
    if (isRfidReaderDetected()) {
      rfidHardwareStatus = "ok";
      return;
    }

    // Ein Re-Init-Versuch erlaubt Wiedererkennung nach Reconnect.
    rfidReader.PCD_Init();
    delay(2);
    rfidHardwareStatus = isRfidReaderDetected() ? "ok" : "error_not_detected";
  }

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
  pinMode(HC_SR04_TRIG_PIN, OUTPUT);
  pinMode(HC_SR04_ECHO_PIN, INPUT);
  digitalWrite(HC_SR04_TRIG_PIN, LOW);

  SPI.begin();
  rfidReader.PCD_Init();
  delay(4);
  refreshRfidHardwareStatus();
}

void Sensors_readSnapshot(SensorSnapshot &out) {
  bool hcsr04Ok = false;
  const char *hcsr04Status = "error_unknown";
  out.hcsr04_distance_cm = readHcsr04DistanceCm(hcsr04Ok, hcsr04Status);
  out.hcsr04_status = hcsr04Status;

  out.uptime_ms = millis();
}

void Sensors_readRfid(char *uidOut, size_t uidOutLen, const char *&statusOut) {
  if (uidOut == nullptr || uidOutLen == 0) {
    statusOut = "buffer_error";
    return;
  }

  uidOut[0] = '\0';
  statusOut = "no_card";

  refreshRfidHardwareStatus();

  if (strcmp(rfidHardwareStatus, "ok") != 0) {
    statusOut = rfidHardwareStatus;
    return;
  }

  if (!rfidReader.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfidReader.PICC_ReadCardSerial()) {
    statusOut = "read_error";
    return;
  }

  bool truncated = false;
  size_t pos = 0;
  statusOut = "ok";

  for (byte i = 0; i < rfidReader.uid.size; i++) {
    if (i > 0) {
      if (pos + 1 >= uidOutLen) {
        truncated = true;
        break;
      }
      uidOut[pos++] = ':';
    }

    if (pos + 2 >= uidOutLen) {
      truncated = true;
      break;
    }

    int written = snprintf(uidOut + pos, uidOutLen - pos, "%02X", rfidReader.uid.uidByte[i]);
    if (written != 2) {
      statusOut = "uid_format_error";
      uidOut[0] = '\0';
      break;
    }
    pos += static_cast<size_t>(written);
  }

  uidOut[(pos < uidOutLen) ? pos : (uidOutLen - 1)] = '\0';

  if (truncated) {
    statusOut = "uid_truncated";
  } else if (uidOut[0] == '\0' && strcmp(statusOut, "uid_format_error") != 0) {
    statusOut = "uid_empty";
  }

  rfidReader.PICC_HaltA();
  rfidReader.PCD_StopCrypto1();
}

const char *Sensors_getRfidHardwareStatus() {
  return rfidHardwareStatus;
}
