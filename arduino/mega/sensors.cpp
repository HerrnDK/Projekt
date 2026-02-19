#include "mega_shared.h"

#include <SPI.h>
#ifndef MFRC522_SPICLOCK
#define MFRC522_SPICLOCK (1000000u)
#endif
#include <MFRC522.h>
#include <stdio.h>
#include <string.h>

/*
  sensors.cpp
  - Sensor-Handling
  - HC-SR04 auf D26 (TRIG) / D27 (ECHO)
  - Funduino Tropfensensor auf A0 (Analog-Rohwert)
  - Wassertruebungssensor auf A1 (Analog-Rohwert)
  - RFID RC522 via SPI (SS D53, RST D49)
*/

namespace {
  constexpr unsigned long HC_SR04_TIMEOUT_US = 30000UL;
  constexpr long HC_SR04_MIN_CM = 2;
  constexpr long HC_SR04_MAX_CM = 400;
  constexpr long DROPLET_MIN_RAW = 0;
  constexpr long DROPLET_MAX_RAW = 1023;
  constexpr uint8_t DROPLET_SAMPLE_COUNT = 5;
  constexpr int DROPLET_FLOATING_SPREAD_THRESHOLD = 80;
  constexpr int DROPLET_PULLUP_HIGH_THRESHOLD = 1000;
  constexpr int DROPLET_PULLUP_DELTA_THRESHOLD = 250;
  constexpr long TURBIDITY_MIN_RAW = 0;
  constexpr long TURBIDITY_MAX_RAW = 1023;
  constexpr uint8_t TURBIDITY_SAMPLE_COUNT = 5;
  constexpr int TURBIDITY_FLOATING_SPREAD_THRESHOLD = 80;
  constexpr int TURBIDITY_PULLUP_HIGH_THRESHOLD = 1000;
  constexpr int TURBIDITY_PULLUP_DELTA_THRESHOLD = 250;

  MFRC522 rfidReader(RC522_SS_PIN, RC522_RST_PIN);
  const char *rfidHardwareStatus = "error_not_initialized";
  bool rfidReaderConfigured = false;
  byte rfidVersionReg = 0x00;
  MFRC522::StatusCode rfidProbeStatus = MFRC522::STATUS_TIMEOUT;

  const char *rfidStatusCodeToText(MFRC522::StatusCode code) {
    switch (code) {
      case MFRC522::STATUS_OK: return "STATUS_OK";
      case MFRC522::STATUS_ERROR: return "STATUS_ERROR";
      case MFRC522::STATUS_COLLISION: return "STATUS_COLLISION";
      case MFRC522::STATUS_TIMEOUT: return "STATUS_TIMEOUT";
      case MFRC522::STATUS_NO_ROOM: return "STATUS_NO_ROOM";
      case MFRC522::STATUS_INTERNAL_ERROR: return "STATUS_INTERNAL_ERROR";
      case MFRC522::STATUS_INVALID: return "STATUS_INVALID";
      case MFRC522::STATUS_CRC_WRONG: return "STATUS_CRC_WRONG";
      case MFRC522::STATUS_MIFARE_NACK: return "STATUS_MIFARE_NACK";
      default: return "STATUS_UNKNOWN";
    }
  }

  void configureRfidReader() {
    // Improve detection stability for weak tags/modules.
    rfidReader.PCD_AntennaOn();
    rfidReader.PCD_SetAntennaGain(MFRC522::RxGain_max);
    rfidReaderConfigured = true;
  }

  bool isCardPresentOrWakeup() {
    if (rfidReader.PICC_IsNewCardPresent()) {
      rfidProbeStatus = MFRC522::STATUS_OK;
      return true;
    }

    // Also detect cards that are already in field / halted.
    byte atqa[2] = {0, 0};
    byte atqaSize = sizeof(atqa);
    MFRC522::StatusCode wakeStatus = rfidReader.PICC_WakeupA(atqa, &atqaSize);
    rfidProbeStatus = wakeStatus;
    return wakeStatus == MFRC522::STATUS_OK || wakeStatus == MFRC522::STATUS_COLLISION;
  }

  bool isRfidReaderDetected() {
    rfidVersionReg = rfidReader.PCD_ReadRegister(MFRC522::VersionReg);
    return rfidVersionReg != 0x00 && rfidVersionReg != 0xFF;
  }

  void refreshRfidHardwareStatus() {
    // Reader kann waehrend Laufzeit ab-/angesteckt werden.
    if (isRfidReaderDetected()) {
      if (!rfidReaderConfigured) {
        configureRfidReader();
      }
      rfidHardwareStatus = "ok";
      return;
    }

    // Ein Re-Init-Versuch erlaubt Wiedererkennung nach Reconnect.
    rfidReader.PCD_Init();
    rfidReaderConfigured = false;
    delay(2);
    if (isRfidReaderDetected()) {
      configureRfidReader();
      rfidHardwareStatus = "ok";
    } else {
      rfidHardwareStatus = "error_not_detected";
    }
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

  long readDropletRaw(bool &ok, const char *&status) {
    pinMode(DROPLET_SENSOR_PIN, INPUT);

    long sum = 0;
    int minRaw = 1023;
    int maxRaw = 0;
    for (uint8_t i = 0; i < DROPLET_SAMPLE_COUNT; i++) {
      const int sample = analogRead(DROPLET_SENSOR_PIN);
      sum += sample;
      if (sample < minRaw) {
        minRaw = sample;
      }
      if (sample > maxRaw) {
        maxRaw = sample;
      }
      delayMicroseconds(200);
    }

    const int raw = static_cast<int>(sum / DROPLET_SAMPLE_COUNT);
    if (raw < DROPLET_MIN_RAW || raw > DROPLET_MAX_RAW) {
      ok = false;
      status = "error_range";
      return -1;
    }

    // Floating-Check: internen Pullup kurz aktivieren.
    // Bei abgezogenem Sensor springt der ADC-Wert deutlich gegen 1023.
    pinMode(DROPLET_SENSOR_PIN, INPUT_PULLUP);
    delayMicroseconds(200);
    const int pullupSample = analogRead(DROPLET_SENSOR_PIN);
    pinMode(DROPLET_SENSOR_PIN, INPUT);

    const bool floatingByNoise = (maxRaw - minRaw) >= DROPLET_FLOATING_SPREAD_THRESHOLD;
    const bool floatingByPullup = (pullupSample >= DROPLET_PULLUP_HIGH_THRESHOLD) &&
                                  ((pullupSample - raw) >= DROPLET_PULLUP_DELTA_THRESHOLD);
    if (floatingByNoise || floatingByPullup) {
      ok = false;
      status = "error_not_connected";
      return -1;
    }

    ok = true;
    status = "ok";
    return static_cast<long>(raw);
  }

  long readTurbidityRaw(bool &ok, const char *&status) {
    pinMode(TURBIDITY_SENSOR_PIN, INPUT);

    long sum = 0;
    int minRaw = 1023;
    int maxRaw = 0;
    for (uint8_t i = 0; i < TURBIDITY_SAMPLE_COUNT; i++) {
      const int sample = analogRead(TURBIDITY_SENSOR_PIN);
      sum += sample;
      if (sample < minRaw) {
        minRaw = sample;
      }
      if (sample > maxRaw) {
        maxRaw = sample;
      }
      delayMicroseconds(200);
    }

    const int raw = static_cast<int>(sum / TURBIDITY_SAMPLE_COUNT);
    if (raw < TURBIDITY_MIN_RAW || raw > TURBIDITY_MAX_RAW) {
      ok = false;
      status = "error_range";
      return -1;
    }

    // Floating-Check: internen Pullup kurz aktivieren.
    // Bei abgezogenem Sensor springt der ADC-Wert deutlich gegen 1023.
    pinMode(TURBIDITY_SENSOR_PIN, INPUT_PULLUP);
    delayMicroseconds(200);
    const int pullupSample = analogRead(TURBIDITY_SENSOR_PIN);
    pinMode(TURBIDITY_SENSOR_PIN, INPUT);

    const bool floatingByNoise = (maxRaw - minRaw) >= TURBIDITY_FLOATING_SPREAD_THRESHOLD;
    const bool floatingByPullup = (pullupSample >= TURBIDITY_PULLUP_HIGH_THRESHOLD) &&
                                  ((pullupSample - raw) >= TURBIDITY_PULLUP_DELTA_THRESHOLD);
    if (floatingByNoise || floatingByPullup) {
      ok = false;
      status = "error_not_connected";
      return -1;
    }

    ok = true;
    status = "ok";
    return static_cast<long>(raw);
  }
}

void Sensors_begin() {
  pinMode(HC_SR04_TRIG_PIN, OUTPUT);
  pinMode(HC_SR04_ECHO_PIN, INPUT);
  pinMode(DROPLET_SENSOR_PIN, INPUT);
  pinMode(TURBIDITY_SENSOR_PIN, INPUT);
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

  bool dropletOk = false;
  const char *dropletStatus = "error_unknown";
  out.droplet_raw = readDropletRaw(dropletOk, dropletStatus);
  out.droplet_status = dropletStatus;

  bool turbidityOk = false;
  const char *turbidityStatus = "error_unknown";
  out.turbidity_raw = readTurbidityRaw(turbidityOk, turbidityStatus);
  out.turbidity_status = turbidityStatus;

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
    rfidProbeStatus = MFRC522::STATUS_ERROR;
    statusOut = rfidHardwareStatus;
    return;
  }

  if (!isCardPresentOrWakeup()) {
    if (rfidProbeStatus != MFRC522::STATUS_TIMEOUT) {
      statusOut = "probe_error";
    }
    return;
  }

  if (!rfidReader.PICC_ReadCardSerial()) {
    delay(2);
    if (!rfidReader.PICC_ReadCardSerial()) {
      rfidProbeStatus = MFRC522::STATUS_ERROR;
      statusOut = "read_error";
      return;
    }
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

const char *Sensors_getRfidProbeStatus() {
  return rfidStatusCodeToText(rfidProbeStatus);
}

uint8_t Sensors_getRfidVersionReg() {
  return rfidVersionReg;
}
