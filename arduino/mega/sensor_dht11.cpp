/*
  sensor_dht11.cpp
  Nutzen:
  - Kapselt das Auslesen des DHT11 Sensors fuer Temperatur und Luftfeuchte.
  Funktion:
  - Fuehrt den DHT11 Single-Wire Handshake aus, liest 40 Datenbits,
    prueft die Checksumme und liefert Werte + Status fuer die Systemsnapshot-JSON.
*/

#include "mega_gemeinsam.h"

#include <string.h>

namespace {
  constexpr unsigned long DHT11_START_LOW_MS = 20UL;
  constexpr unsigned long DHT11_START_HIGH_US = 30UL;
  constexpr unsigned long DHT11_TIMEOUT_US = 120UL;
  constexpr unsigned long DHT11_MIN_MESSINTERVALL_MS = 1200UL;
  constexpr unsigned long DHT11_BIT_ONE_SCHWELLE_US = 45UL;

  long letzteTemperaturC = -1;
  long letzteLuftfeuchteProzent = -1;
  const char *letzterStatus = "error_init";
  unsigned long letzteMessungMs = 0;

  bool Dht11_warteAufPegel(uint8_t pegel, unsigned long timeoutUs) {
    const unsigned long startUs = micros();
    while (digitalRead(DHT11_SENSOR_PIN) != pegel) {
      if ((micros() - startUs) > timeoutUs) {
        return false;
      }
    }
    return true;
  }

  bool Dht11_leseFrame(uint8_t daten[5], const char *&statusAusgabe) {
    for (uint8_t i = 0; i < 5; i++) {
      daten[i] = 0;
    }

    // Startsignal laut DHT11 Protokoll.
    pinMode(DHT11_SENSOR_PIN, OUTPUT);
    digitalWrite(DHT11_SENSOR_PIN, LOW);
    delay(DHT11_START_LOW_MS);
    digitalWrite(DHT11_SENSOR_PIN, HIGH);
    delayMicroseconds(DHT11_START_HIGH_US);
    pinMode(DHT11_SENSOR_PIN, INPUT_PULLUP);

    // Antwortsignal des Sensors: LOW -> HIGH -> LOW.
    if (!Dht11_warteAufPegel(LOW, DHT11_TIMEOUT_US)) {
      statusAusgabe = "error_not_connected";
      return false;
    }
    if (!Dht11_warteAufPegel(HIGH, DHT11_TIMEOUT_US)) {
      statusAusgabe = "error_timeout";
      return false;
    }
    if (!Dht11_warteAufPegel(LOW, DHT11_TIMEOUT_US)) {
      statusAusgabe = "error_timeout";
      return false;
    }

    // 40 Bit lesen: jedes Bit hat HIGH-Zeit, die 0/1 kodiert.
    for (uint8_t bitIndex = 0; bitIndex < 40; bitIndex++) {
      if (!Dht11_warteAufPegel(HIGH, DHT11_TIMEOUT_US)) {
        statusAusgabe = "error_timeout";
        return false;
      }

      const unsigned long bitHighStartUs = micros();
      if (!Dht11_warteAufPegel(LOW, DHT11_TIMEOUT_US)) {
        statusAusgabe = "error_timeout";
        return false;
      }

      const unsigned long highDauerUs = micros() - bitHighStartUs;
      const uint8_t bitWert = (highDauerUs > DHT11_BIT_ONE_SCHWELLE_US) ? 1 : 0;
      daten[bitIndex / 8] = static_cast<uint8_t>((daten[bitIndex / 8] << 1) | bitWert);
    }

    statusAusgabe = "ok";
    return true;
  }
}

/*
  Zweck:
  - Initialisiert den DHT11 Datenpin.
  Verhalten:
  - Setzt den Pin in den Ruhezustand mit Pullup.
  Rueckgabe:
  - Keine.
*/
void Dht11_starten() {
  pinMode(DHT11_SENSOR_PIN, INPUT_PULLUP);
  letzteTemperaturC = -1;
  letzteLuftfeuchteProzent = -1;
  letzterStatus = "error_init";
  letzteMessungMs = 0;
}

/*
  Zweck:
  - Liest Temperatur und Luftfeuchte vom DHT11.
  Verhalten:
  - Beachtet das minimale Messintervall (Caching),
    prueft Checksumme und Wertebereich.
  Rueckgabe:
  - Temperatur/Luftfeuchte als Ganzzahl ueber Referenzen.
  - Status ueber `statusAusgabe` (`ok`, `error_not_connected`, `error_timeout`,
    `error_checksum`, `error_range`).
*/
void Dht11_lesen(long &temperaturCAusgabe, long &luftfeuchteProzentAusgabe, const char *&statusAusgabe) {
  const unsigned long jetztMs = millis();
  const bool darfMessungUeberspringen = (letzterStatus != nullptr) &&
                                        (strcmp(letzterStatus, "error_init") != 0) &&
                                        ((jetztMs - letzteMessungMs) < DHT11_MIN_MESSINTERVALL_MS);

  if (darfMessungUeberspringen) {
    temperaturCAusgabe = letzteTemperaturC;
    luftfeuchteProzentAusgabe = letzteLuftfeuchteProzent;
    statusAusgabe = letzterStatus;
    return;
  }

  uint8_t daten[5];
  const char *leseStatus = "error_timeout";
  if (!Dht11_leseFrame(daten, leseStatus)) {
    letzteTemperaturC = -1;
    letzteLuftfeuchteProzent = -1;
    letzterStatus = leseStatus;
    letzteMessungMs = jetztMs;
    temperaturCAusgabe = letzteTemperaturC;
    luftfeuchteProzentAusgabe = letzteLuftfeuchteProzent;
    statusAusgabe = letzterStatus;
    return;
  }

  const uint8_t checksumme = static_cast<uint8_t>(daten[0] + daten[1] + daten[2] + daten[3]);
  if (daten[4] != checksumme) {
    letzteTemperaturC = -1;
    letzteLuftfeuchteProzent = -1;
    letzterStatus = "error_checksum";
    letzteMessungMs = jetztMs;
    temperaturCAusgabe = letzteTemperaturC;
    luftfeuchteProzentAusgabe = letzteLuftfeuchteProzent;
    statusAusgabe = letzterStatus;
    return;
  }

  const long luftfeuchte = static_cast<long>(daten[0]);
  const long temperatur = static_cast<long>(daten[2]);

  if (temperatur < 0 || temperatur > 60 || luftfeuchte < 0 || luftfeuchte > 100) {
    letzteTemperaturC = -1;
    letzteLuftfeuchteProzent = -1;
    letzterStatus = "error_range";
    letzteMessungMs = jetztMs;
    temperaturCAusgabe = letzteTemperaturC;
    luftfeuchteProzentAusgabe = letzteLuftfeuchteProzent;
    statusAusgabe = letzterStatus;
    return;
  }

  letzteTemperaturC = temperatur;
  letzteLuftfeuchteProzent = luftfeuchte;
  letzterStatus = "ok";
  letzteMessungMs = jetztMs;

  temperaturCAusgabe = letzteTemperaturC;
  luftfeuchteProzentAusgabe = letzteLuftfeuchteProzent;
  statusAusgabe = letzterStatus;
}
