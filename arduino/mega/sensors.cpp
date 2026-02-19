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
  - Sensor-Handling fuer HC-SR04, Tropfensensor, Truebungssensor und RFID RC522
  - HC-SR04: D26 (TRIG) / D27 (ECHO)
  - Tropfensensor: A0
  - Truebungssensor: A1
  - RC522: SPI mit SS D53 und RST D49
*/

namespace {
  constexpr unsigned long HC_SR04_TIMEOUT_MIKROSEKUNDEN = 30000UL;
  constexpr long HC_SR04_MIN_DISTANZ_CM = 2;
  constexpr long HC_SR04_MAX_DISTANZ_CM = 400;

  constexpr long TROPFEN_MIN_ROH = 0;
  constexpr long TROPFEN_MAX_ROH = 1023;
  constexpr uint8_t TROPFEN_STICHPROBEN_ANZAHL = 5;
  constexpr int TROPFEN_FLOATING_SPREAD_SCHWELLE = 80;
  constexpr int TROPFEN_PULLUP_HOCH_SCHWELLE = 1000;
  constexpr int TROPFEN_PULLUP_DELTA_SCHWELLE = 250;

  constexpr long TRUEBUNG_MIN_ROH = 0;
  constexpr long TRUEBUNG_MAX_ROH = 1023;
  constexpr uint8_t TRUEBUNG_STICHPROBEN_ANZAHL = 5;
  // Beim Truebungssensor ist das Analogsignal deutlich rauschiger als beim Tropfensensor.
  // Deshalb sind die Schwellen konservativer und der Fehler wird erst nach Bestaetigung gesetzt.
  constexpr int TRUEBUNG_FLOATING_SPREAD_SCHWELLE = 220;
  constexpr int TRUEBUNG_ROH_HOCH_SCHWELLE = 850;
  constexpr int TRUEBUNG_PULLUP_HOCH_SCHWELLE = 1015;
  constexpr int TRUEBUNG_PULLUP_DELTA_SCHWELLE = 450;
  constexpr uint8_t TRUEBUNG_TRENNUNG_BESTAETIGUNG_SNAPSHOTS = 3;
  uint8_t truebungTrennungVerdachtZaehler = 0;

  MFRC522 rfidLeser(RC522_PIN_SS, RC522_PIN_RST);
  const char *rfidHardwareZustand = "error_not_initialized";
  bool rfidLeserKonfiguriert = false;
  byte rfidVersionsRegister = 0x00;
  MFRC522::StatusCode rfidProbeStatus = MFRC522::STATUS_TIMEOUT;

  // Wandelt MFRC522-Statuscodes in stabile Klartexte fuer JSON um.
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

  // Aktiviert Antenne/Gain fuer stabile RFID-Erkennung.
  void konfiguriereRfidLeser() {
    rfidLeser.PCD_AntennaOn();
    rfidLeser.PCD_SetAntennaGain(MFRC522::RxGain_max);
    rfidLeserKonfiguriert = true;
  }

  // Prueft, ob eine Karte neu praesent ist oder aufgeweckt werden kann.
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

  // Liest VersionReg und prueft, ob der Leser auf SPI antwortet.
  bool istRfidLeserErkannt() {
    rfidVersionsRegister = rfidLeser.PCD_ReadRegister(MFRC522::VersionReg);
    return rfidVersionsRegister != 0x00 && rfidVersionsRegister != 0xFF;
  }

  // Aktualisiert den RFID-Hardwarezustand inkl. Re-Init bei Laufzeit-Reconnect.
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

  // Liest die Distanz des HC-SR04 in Zentimetern und liefert Statuscodes.
  long leseHcsr04DistanzCm(bool &messungGueltig, const char *&status) {
    // Trigger-Impuls: LOW -> HIGH (10us) -> LOW
    digitalWrite(HC_SR04_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(HC_SR04_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(HC_SR04_TRIG_PIN, LOW);

    unsigned long dauerMikrosekunden = pulseIn(HC_SR04_ECHO_PIN, HIGH, HC_SR04_TIMEOUT_MIKROSEKUNDEN);
    if (dauerMikrosekunden == 0) {
      messungGueltig = false;
      status = "error_timeout";
      return -1;
    }

    long distanzCm = static_cast<long>(dauerMikrosekunden / 58UL);
    if (distanzCm < HC_SR04_MIN_DISTANZ_CM || distanzCm > HC_SR04_MAX_DISTANZ_CM) {
      messungGueltig = false;
      status = "error_range";
      return -1;
    }

    messungGueltig = true;
    status = "ok";
    return distanzCm;
  }

  // Liest den Tropfensensor als gemittelten Rohwert inkl. Floating-Erkennung.
  long leseTropfenRohwert(bool &messungGueltig, const char *&status) {
    pinMode(TROPFEN_SENSOR_PIN, INPUT);

    long summe = 0;
    int minRoh = 1023;
    int maxRoh = 0;
    for (uint8_t i = 0; i < TROPFEN_STICHPROBEN_ANZAHL; i++) {
      const int stichprobe = analogRead(TROPFEN_SENSOR_PIN);
      summe += stichprobe;
      if (stichprobe < minRoh) {
        minRoh = stichprobe;
      }
      if (stichprobe > maxRoh) {
        maxRoh = stichprobe;
      }
      delayMicroseconds(200);
    }

    const int roh = static_cast<int>(summe / TROPFEN_STICHPROBEN_ANZAHL);
    if (roh < TROPFEN_MIN_ROH || roh > TROPFEN_MAX_ROH) {
      messungGueltig = false;
      status = "error_range";
      return -1;
    }

    // Floating-Check: internen Pullup kurz aktivieren.
    pinMode(TROPFEN_SENSOR_PIN, INPUT_PULLUP);
    delayMicroseconds(200);
    const int pullupStichprobe = analogRead(TROPFEN_SENSOR_PIN);
    pinMode(TROPFEN_SENSOR_PIN, INPUT);

    const bool floatingDurchRauschen = (maxRoh - minRoh) >= TROPFEN_FLOATING_SPREAD_SCHWELLE;
    const bool floatingDurchPullup = (pullupStichprobe >= TROPFEN_PULLUP_HOCH_SCHWELLE) &&
                                     ((pullupStichprobe - roh) >= TROPFEN_PULLUP_DELTA_SCHWELLE);
    if (floatingDurchRauschen || floatingDurchPullup) {
      messungGueltig = false;
      status = "error_not_connected";
      return -1;
    }

    messungGueltig = true;
    status = "ok";
    return static_cast<long>(roh);
  }

  // Liest den Truebungssensor als Rohwert inkl. bestaetigter Trennungs-Erkennung.
  long leseTruebungRohwert(bool &messungGueltig, const char *&status) {
    pinMode(TRUEBUNG_SENSOR_PIN, INPUT);

    long summe = 0;
    int minRoh = 1023;
    int maxRoh = 0;
    for (uint8_t i = 0; i < TRUEBUNG_STICHPROBEN_ANZAHL; i++) {
      const int stichprobe = analogRead(TRUEBUNG_SENSOR_PIN);
      summe += stichprobe;
      if (stichprobe < minRoh) {
        minRoh = stichprobe;
      }
      if (stichprobe > maxRoh) {
        maxRoh = stichprobe;
      }
      delayMicroseconds(200);
    }

    const int roh = static_cast<int>(summe / TRUEBUNG_STICHPROBEN_ANZAHL);
    if (roh < TRUEBUNG_MIN_ROH || roh > TRUEBUNG_MAX_ROH) {
      messungGueltig = false;
      status = "error_range";
      return -1;
    }

    // Floating-Check: internen Pullup kurz aktivieren.
    pinMode(TRUEBUNG_SENSOR_PIN, INPUT_PULLUP);
    delayMicroseconds(200);
    const int pullupStichprobe = analogRead(TRUEBUNG_SENSOR_PIN);
    pinMode(TRUEBUNG_SENSOR_PIN, INPUT);

    const bool floatingDurchRauschen = (maxRoh - minRoh) >= TRUEBUNG_FLOATING_SPREAD_SCHWELLE;
    const bool floatingDurchPullup = (roh >= TRUEBUNG_ROH_HOCH_SCHWELLE) &&
                                     (pullupStichprobe >= TRUEBUNG_PULLUP_HOCH_SCHWELLE) &&
                                     ((pullupStichprobe - roh) >= TRUEBUNG_PULLUP_DELTA_SCHWELLE);
    const bool trennungVermutet = floatingDurchRauschen && floatingDurchPullup;

    if (trennungVermutet) {
      if (truebungTrennungVerdachtZaehler < TRUEBUNG_TRENNUNG_BESTAETIGUNG_SNAPSHOTS) {
        truebungTrennungVerdachtZaehler++;
      }
    } else {
      truebungTrennungVerdachtZaehler = 0;
    }

    if (truebungTrennungVerdachtZaehler >= TRUEBUNG_TRENNUNG_BESTAETIGUNG_SNAPSHOTS) {
      messungGueltig = false;
      status = "error_not_connected";
      return -1;
    }

    messungGueltig = true;
    status = "ok";
    return static_cast<long>(roh);
  }
}

/*
  Initialisiert alle Sensor-Schnittstellen.
  - setzt Pinmodi
  - startet SPI und initialisiert den RC522
*/
void Sensoren_starten() {
  pinMode(HC_SR04_TRIG_PIN, OUTPUT);
  pinMode(HC_SR04_ECHO_PIN, INPUT);
  pinMode(TROPFEN_SENSOR_PIN, INPUT);
  pinMode(TRUEBUNG_SENSOR_PIN, INPUT);
  digitalWrite(HC_SR04_TRIG_PIN, LOW);

  SPI.begin();
  rfidLeser.PCD_Init();
  delay(4);
  aktualisiereRfidHardwareZustand();
}

/*
  Liest alle Sensoren und fuellt eine gemeinsame Momentaufnahme.
*/
void Sensoren_lesenMomentaufnahme(SensorMomentaufnahme &ausgabe) {
  bool hcsr04Gueltig = false;
  const char *hcsr04Status = "error_unknown";
  ausgabe.hcsr04_distanz_cm = leseHcsr04DistanzCm(hcsr04Gueltig, hcsr04Status);
  ausgabe.hcsr04_status = hcsr04Status;

  bool tropfenGueltig = false;
  const char *tropfenStatus = "error_unknown";
  ausgabe.tropfen_roh = leseTropfenRohwert(tropfenGueltig, tropfenStatus);
  ausgabe.tropfen_status = tropfenStatus;

  bool truebungGueltig = false;
  const char *truebungStatus = "error_unknown";
  ausgabe.truebung_roh = leseTruebungRohwert(truebungGueltig, truebungStatus);
  ausgabe.truebung_status = truebungStatus;

  ausgabe.laufzeit_ms = millis();
}

/*
  Liest die aktuelle RFID-Karte.
  - liefert UID im Buffer
  - liefert Statuscode ueber Referenzparameter
  - behandelt Kartenabwesenheit, Probe- und Lesefehler robust
*/
void Sensoren_lesenRfid(char *uidAusgabe, size_t uidAusgabeLaenge, const char *&statusAusgabe) {
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
  Liefert den zuletzt bekannten RFID-Hardwarezustand.
*/
const char *Sensoren_holeRfidHardwareStatus() {
  return rfidHardwareZustand;
}

/*
  Liefert den letzten Probe-Status (REQA/WUPA) als Klartext.
*/
const char *Sensoren_holeRfidProbeStatus() {
  return rfidStatusCodeZuText(rfidProbeStatus);
}

/*
  Liefert den Inhalt des RC522 VersionReg fuer Diagnosezwecke.
*/
uint8_t Sensoren_holeRfidVersionsRegister() {
  return rfidVersionsRegister;
}
