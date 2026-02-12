# Arduino Mega 2560 - Kontaktanschlussplan (Pinout)

## Uebersicht
Dokumentation aller digitalen und analogen Pin-Zuweisungen fuer Sensoren und Aktoren.

---

## Serial-Verbindungen

| Pin | Funktion | Verbindung | Baud-Rate |
|-----|----------|-----------|-----------|
| USB | Serial (Debug/Programmierung) | Computer | 115200 |
| TX1 (18) | Serial1 TX | Arduino D18 -> B2 -> A2 -> Raspberry Pi RXD | 9600 |
| RX1 (19) | Serial1 RX | Raspberry Pi TXD -> A1 -> B1 -> Arduino D19 | 9600 |

---

## Pegelwandler-Kanalbelegung (A/B)

| Kanal | A-Seite | B-Seite | Zweck |
|-------|---------|---------|-------|
| A1 <-> B1 | Raspberry Pi TXD | Arduino D19 (RX1) | UART Pi -> Mega |
| A2 <-> B2 | Raspberry Pi RXD | Arduino D18 (TX1) | UART Mega -> Pi |
| A3 <-> B3 | RC522 SDA/SS | Arduino D53 | SPI CS/SS |
| A4 <-> B4 | RC522 SCK (vom User als ACK benannt) | Arduino D52 | SPI Takt |
| A5 <-> B5 | RC522 MOSI | Arduino D51 | SPI MOSI |
| A6 <-> B6 | RC522 MISO | Arduino D50 | SPI MISO |
| A7 <-> B7 | RC522 RST | Arduino D49 | RC522 Reset |

---

## Digitale Ausgaenge (Aktoren)

| Pin | Funktion | Komponente | Status | Notizen |
|-----|----------|-----------|--------|---------|
| 22 | Aktor 1 | | | |
| 23 | Aktor 2 | | | |
| 24 | Aktor 3 | | | |
| 25 | Aktor 4 | | | |
| 26 | HC-SR04 TRIG | Ultraschall-Sensor | aktiv | Trigger-Ausgang (10us Pulse) |
| 49 | RC522 RST | RFID-RC522 | aktiv | Reset-Leitung |
| 53 | RC522 SS/SDA | RFID-RC522 | aktiv | SPI Chip Select |

---

## Digitale Eingaenge (Sensoren)

| Pin | Funktion | Sensortyp | Bus | Notizen |
|-----|----------|-----------|-----|---------|
| 27 | HC-SR04 ECHO | Ultraschall-Sensor | Digital | Echo-Eingang (5V TTL) |
| 50 | RC522 MISO | RFID-RC522 | SPI | Master In Slave Out |

---

## Analoge Eingaenge (A0-A15)

| Pin | Adc-Nummer | Sensor | Spannung | Kalibrierung | Notizen |
|-----|-----------|--------|----------|-------------|---------|
| A0 | ADC0 | Funduino Tropfensensor (Signal S) | 0-5V | Offset via Dashboard (`droplet_offset_raw`) | Analog-Rohwert 0..1023 |
| A1 | ADC1 | derzeit nicht genutzt | 0-5V | | |
| A2 | ADC2 | derzeit nicht genutzt | 0-5V | | |
| A3 | ADC3 | derzeit nicht genutzt | 0-5V | | |
| A4 | ADC4 | derzeit nicht genutzt | 0-5V | | |
| A5 | ADC5 | derzeit nicht genutzt | 0-5V | | |

---

## I2C / SPI Schnittstellen

### I2C (Mega: SDA=20, SCL=21)

| Adresse | Geraet | Funktion | Notizen |
|---------|--------|----------|---------|
| | | | |

### SPI (Mega: MOSI=51, MISO=50, SCK=52, CS=53)

| Pin | Signal | Geraet | Funktion |
|-----|--------|--------|----------|
| 50 | MISO | RC522 | Daten zum Mega |
| 51 | MOSI | RC522 | Daten zum RC522 |
| 52 | SCK | RC522 | SPI Takt |
| 53 | CS/SS | RC522 | Chip Select |

---

## Stromversorgung

| Quelle | GND | 5V | 3.3V | Komponenten |
|--------|-----|----|------|-------------|
| Mega 5V | x | x | | HC-SR04, Funduino Tropfensensor, Power Board |
| Mega 3.3V | x | | x | RC522 |
| Extern | | | | |

---

## Verdrahtungsschema

```
[Arduino Mega 2560]
         |
    +----+-----------------------+
    |                            |
  [USB]                     [Serial1 UART]
                               |
                         [Raspberry Pi 4]
```

### Raspberry Pi <-> Arduino Verbindung

```
RPi GPIO    <->    Arduino Mega
(TXD, 3.3V) <->    A1/B1 <-> RX1 (Pin 19)
(RXD, 3.3V) <->    A2/B2 <-> TX1 (Pin 18)
(GND)       <->    GND
```

### HC-SR04 <-> Arduino Verbindung

```
HC-SR04      <->    Arduino Mega
VCC (5V)     <->    5V
GND          <->    GND
TRIG         <->    D26
ECHO         <->    D27
```

### RC522 <-> Arduino Verbindung

```
RC522        <->    Pegelwandler <-> Arduino Mega
SDA/SS       <->    A3/B3         <-> D53
SCK          <->    A4/B4         <-> D52
MOSI         <->    A5/B5         <-> D51
MISO         <->    A6/B6         <-> D50
RST          <->    A7/B7         <-> D49
3.3V         <->    3.3V
GND          <->    GND
```

### Funduino Tropfensensor <-> Arduino Verbindung

```
Funduino Tropfensensor   <->    Arduino Mega
+5V                      <->    5V
-GND                     <->    GND
S (Analog Out)           <->    A0
```

---

## Notizen
- Raspberry Pi RXD ist nicht 5V-tolerant: Mega TX1 immer ueber Pegelwandler/Spannungsteiler anschliessen.
- RC522 nur mit 3.3V betreiben.
- GND muss zwischen Mega, Raspberry Pi und Sensoren gemeinsam verbunden sein.
- A1..A7/B1..B7 in dieser Datei sind Kanaele des Pegelwandler-Moduls, nicht Arduino Analogpins A1..A7.
