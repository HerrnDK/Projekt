/*
  sensor_rfid.cpp
  Nutzen:
  - Kapselt die komplette RC522-RFID-Kommunikation.
  Funktion:
  - Initialisiert SPI/Reader, erkennt Karten-UIDs und liefert
    zusaetzliche Diagnosewerte (Hardwarestatus, Probe-Status, Versionsregister).
*/

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

  /*
    Zweck:
    - Uebersetzt MFRC522-Statuscodes in stabile Textwerte.
    Verhalten:
    - Mappt bekannte Codes auf feste Strings fuer JSON-Diagnose.
    Rueckgabe:
    - Zeiger auf den passenden Statusstring.
  */
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

  /*
    Zweck:
    - Aktiviert und optimiert den RFID-Empfang.
    Verhalten:
    - Schaltet die Antenne ein und setzt maximale Empfaengerverstaerkung.
    Rueckgabe:
    - Keine.
  */
  void konfiguriereRfidLeser() {
    rfidLeser.PCD_AntennaOn();
    rfidLeser.PCD_SetAntennaGain(MFRC522::RxGain_max);
    rfidLeserKonfiguriert = true;
  }

  /*
    Zweck:
    - Prueft, ob aktuell eine RFID-Karte praesent ist.
    Verhalten:
    - Nutzt zuerst "new card present", danach WakeupA als Fallback.
    - Aktualisiert dabei den internen Probe-Status.
    Rueckgabe:
    - `true`, wenn eine Karte erkannt bzw. aufgeweckt wurde.
  */
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

  /*
    Zweck:
    - Prueft die Erreichbarkeit des RC522 ueber SPI.
    Verhalten:
    - Liest `VersionReg` und wertet 0x00/0xFF als nicht erkannt.
    Rueckgabe:
    - `true`, wenn das Modul plausibel antwortet.
  */
  bool istRfidLeserErkannt() {
    rfidVersionsRegister = rfidLeser.PCD_ReadRegister(MFRC522::VersionReg);
    return rfidVersionsRegister != 0x00 && rfidVersionsRegister != 0xFF;
  }

  /*
    Zweck:
    - Aktualisiert den globalen RFID-Hardwarestatus.
    Verhalten:
    - Prueft Reader-Erkennung, fuehrt bei Bedarf Re-Init aus
      und setzt den resultierenden Statusstring.
    Rueckgabe:
    - Keine.
  */
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

/*
  Zweck:
  - Startet das RFID-Teilmodul beim Boot.
  Verhalten:
  - Initialisiert SPI und RC522 und fuehrt einen ersten Hardwarecheck durch.
  Rueckgabe:
  - Keine.
*/
void Rfid_starten() {
  SPI.begin();
  rfidLeser.PCD_Init();
  delay(4);
  aktualisiereRfidHardwareZustand();
}

/*
  Zweck:
  - Liest die UID einer praesentierten RFID-Karte.
  Verhalten:
  - Prueft Hardwarestatus, fuehrt Presence/Probe-Check durch und formatiert UID.
  - Setzt fuer alle Fehlerfaelle einen stabilen Statustext.
  Rueckgabe:
  - Keine (UID/Status ueber Referenzparameter).
*/
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

/*
  Zweck:
  - Liefert den aktuellen RFID-Hardwarestatus.
  Verhalten:
  - Gibt den zuletzt ermittelten Zustandstext direkt zurueck.
  Rueckgabe:
  - Zeiger auf den Statusstring.
*/
const char *Rfid_holeHardwareStatus() {
  return rfidHardwareZustand;
}

/*
  Zweck:
  - Liefert den letzten Probe-Status als Text.
  Verhalten:
  - Konvertiert den gespeicherten MFRC522-Statuscode in einen String.
  Rueckgabe:
  - Zeiger auf den Probe-Statusstring.
*/
const char *Rfid_holeProbeStatus() {
  return rfidStatusCodeZuText(rfidProbeStatus);
}

/*
  Zweck:
  - Liefert das zuletzt gelesene RC522-Versionsregister.
  Verhalten:
  - Gibt den internen Registerpuffer unveraendert zurueck.
  Rueckgabe:
  - 8-Bit Registerwert.
*/
uint8_t Rfid_holeVersionsRegister() {
  return rfidVersionsRegister;
}
