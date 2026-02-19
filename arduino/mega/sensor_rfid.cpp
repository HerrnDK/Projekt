#include "mega_gemeinsam.h"

#include <SPI.h>
#ifndef MFRC522_SPICLOCK
#define MFRC522_SPICLOCK (1000000u)
#endif
#include <MFRC522.h>
#include <stdio.h>
#include <string.h>

namespace {
  MFRC522 rfidLeser(RC522_PIN_SS, RC522_PIN_RST);
  const char *rfidHardwareZustand = "error_not_initialized";
  bool rfidLeserKonfiguriert = false;
  byte rfidVersionsRegister = 0x00;
  MFRC522::StatusCode rfidProbeStatus = MFRC522::STATUS_TIMEOUT;

  const char *rfidStatusCodeZuText(MFRC522::StatusCode code) {
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

  void konfiguriereRfidLeser() {
    rfidLeser.PCD_AntennaOn();
    rfidLeser.PCD_SetAntennaGain(MFRC522::RxGain_max);
    rfidLeserKonfiguriert = true;
  }

  bool istKarteVorhandenOderAufwecken() {
    if (rfidLeser.PICC_IsNewCardPresent()) {
      rfidProbeStatus = MFRC522::STATUS_OK;
      return true;
    }

    byte atqa[2] = {0, 0};
    byte atqaGroesse = sizeof(atqa);
    MFRC522::StatusCode wakeStatus = rfidLeser.PICC_WakeupA(atqa, &atqaGroesse);
    rfidProbeStatus = wakeStatus;
    return wakeStatus == MFRC522::STATUS_OK || wakeStatus == MFRC522::STATUS_COLLISION;
  }

  bool istRfidLeserErkannt() {
    rfidVersionsRegister = rfidLeser.PCD_ReadRegister(MFRC522::VersionReg);
    return rfidVersionsRegister != 0x00 && rfidVersionsRegister != 0xFF;
  }

  void aktualisiereRfidHardwareZustand() {
    if (istRfidLeserErkannt()) {
      if (!rfidLeserKonfiguriert) {
        konfiguriereRfidLeser();
      }
      rfidHardwareZustand = "ok";
      return;
    }

    rfidLeser.PCD_Init();
    rfidLeserKonfiguriert = false;
    delay(2);
    if (istRfidLeserErkannt()) {
      konfiguriereRfidLeser();
      rfidHardwareZustand = "ok";
    } else {
      rfidHardwareZustand = "error_not_detected";
    }
  }
}

void Rfid_starten() {
  SPI.begin();
  rfidLeser.PCD_Init();
  delay(4);
  aktualisiereRfidHardwareZustand();
}

void Rfid_lesenUid(char *uidAusgabe, size_t uidAusgabeLaenge, const char *&statusAusgabe) {
  if (uidAusgabe == nullptr || uidAusgabeLaenge == 0) {
    statusAusgabe = "buffer_error";
    return;
  }

  uidAusgabe[0] = '\0';
  statusAusgabe = "no_card";

  aktualisiereRfidHardwareZustand();

  if (strcmp(rfidHardwareZustand, "ok") != 0) {
    rfidProbeStatus = MFRC522::STATUS_ERROR;
    statusAusgabe = rfidHardwareZustand;
    return;
  }

  if (!istKarteVorhandenOderAufwecken()) {
    if (rfidProbeStatus != MFRC522::STATUS_TIMEOUT) {
      statusAusgabe = "probe_error";
    }
    return;
  }

  if (!rfidLeser.PICC_ReadCardSerial()) {
    delay(2);
    if (!rfidLeser.PICC_ReadCardSerial()) {
      rfidProbeStatus = MFRC522::STATUS_ERROR;
      statusAusgabe = "read_error";
      return;
    }
  }

  bool abgeschnitten = false;
  size_t pos = 0;
  statusAusgabe = "ok";

  for (byte i = 0; i < rfidLeser.uid.size; i++) {
    if (i > 0) {
      if (pos + 1 >= uidAusgabeLaenge) {
        abgeschnitten = true;
        break;
      }
      uidAusgabe[pos++] = ':';
    }

    if (pos + 2 >= uidAusgabeLaenge) {
      abgeschnitten = true;
      break;
    }

    int geschrieben = snprintf(uidAusgabe + pos, uidAusgabeLaenge - pos, "%02X", rfidLeser.uid.uidByte[i]);
    if (geschrieben != 2) {
      statusAusgabe = "uid_format_error";
      uidAusgabe[0] = '\0';
      break;
    }
    pos += static_cast<size_t>(geschrieben);
  }

  uidAusgabe[(pos < uidAusgabeLaenge) ? pos : (uidAusgabeLaenge - 1)] = '\0';

  if (abgeschnitten) {
    statusAusgabe = "uid_truncated";
  } else if (uidAusgabe[0] == '\0' && strcmp(statusAusgabe, "uid_format_error") != 0) {
    statusAusgabe = "uid_empty";
  }

  rfidLeser.PICC_HaltA();
  rfidLeser.PCD_StopCrypto1();
}

const char *Rfid_holeHardwareStatus() {
  return rfidHardwareZustand;
}

const char *Rfid_holeProbeStatus() {
  return rfidStatusCodeZuText(rfidProbeStatus);
}

uint8_t Rfid_holeVersionsRegister() {
  return rfidVersionsRegister;
}
