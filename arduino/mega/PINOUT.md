# Arduino Mega 2560 - Kontaktanschlussplan (Pinout)

## Übersicht
Dokumentation aller digitalen und analogen Pin-Zuweisungen für Sensoren und Aktoren.

---

## Serial-Verbindungen

| Pin | Funktion | Verbindung | Baud-Rate |
|-----|----------|-----------|-----------|
| USB | Serial (Debug/Programmierung) | Computer | 115200 |
| TX1 (18) | Serial1 TX (5V) | Pegelwandler 5V->3.3V -> Raspberry Pi RXD | 9600 |
| RX1 (19) | Serial1 RX | Raspberry Pi TXD (3.3V, direkt moeglich) | 9600 |

---

## Digitale Ausgänge (Aktoren)

| Pin | Funktion | Komponente | Status | Notizen |
|-----|----------|-----------|--------|---------|
| 22 | Aktor 1 | | | |
| 23 | Aktor 2 | | | |
| 24 | Aktor 3 | | | |
| 25 | Aktor 4 | | | |
| 26 | HC-SR04 TRIG | Ultraschall-Sensor | aktiv | Trigger-Ausgang (10us Pulse) |

---

## Digitale Eingänge (Sensoren)

| Pin | Funktion | Sensortyp | I2C/SPI/Analog | Notizen |
|-----|----------|-----------|---|---------|
| 27 | HC-SR04 ECHO | Ultraschall-Sensor | Digital | Echo-Eingang (5V TTL) |
| | | | | |
| | | | | |

---

## Analoge Eingänge (A0-A15)

| Pin | Adc-Nummer | Sensor | Spannung | Kalibrierung | Notizen |
|-----|-----------|--------|----------|-------------|---------|
| A0 | ADC0 | | 0-5V | | |
| A1 | ADC1 | | 0-5V | | |
| A2 | ADC2 | | 0-5V | | |
| A3 | ADC3 | | 0-5V | | |
| A4 | ADC4 | | 0-5V | | |
| A5 | ADC5 | | 0-5V | | |

---

## I2C / SPI Schnittstellen

### I2C (Mega: SDA=20, SCL=21)

| Adresse | Gerät | Funktion | Notizen |
|---------|-------|----------|---------|
| | | | |
| | | | |

### SPI (Mega: MOSI=51, MISO=50, SCK=52, CS=53)

| Pin | CS | Gerät | Funktion |
|-----|----|----|----------|
| 53 | | | |

---

## Stromversorgung

| Quelle | GND | 5V | 3.3V | Komponenten |
|--------|-----|----|----|-------------|
| Mega 5V | | | | Power Board |
| Extern | | | | |

---

## Verdrahtungsschema

```
[Arduino Mega 2560]
         |
    +----+----+
    |         |
  [USB]    [Serial1 UART]
              |
        [Raspberry Pi 4]
```

### Raspberry Pi ↔ Arduino Verbindung

```
RPi GPIO    ↔    Arduino Mega
(TXD, 3.3V) ↔    RX1 (Pin 19)
(RXD, 3.3V) ↔    TX1 (Pin 18) ueber Pegelwandler 5V->3.3V
(GND)       ↔    GND
```

### HC-SR04 ↔ Arduino Verbindung

```
HC-SR04      ↔    Arduino Mega
VCC (5V)     ↔    5V
GND          ↔    GND
TRIG         ↔    D26
ECHO         ↔    D27
```

---

## Schnellreferenz: Arduino Mega 2560 Pins

- **Digital Pins:** 0-53 (bevorzugt: 22-53 für Projekte, 0-1 für Serial, 2-3 für Interrupts)
- **PWM Pins:** 2-13, 44-46
- **Analog Pins:** A0-A15 (entspricht Pins 54-69)
- **I2C:** SDA (Pin 20), SCL (Pin 21)
- **SPI:** MOSI (51), MISO (50), SCK (52), CS (53)

---

## Notizen
- Alle Spannungen standardmäßig 5V (außer anders angegeben)
- GND muss zwischen Mega und Raspberry Pi verbunden sein
- Raspberry Pi RXD ist nicht 5V-tolerant: Mega TX1 immer ueber Pegelwandler/Spannungsteiler anschliessen
- Sensoren/Aktoren mit Motorentreibern verwenden für höhere Ströme
