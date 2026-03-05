# Leistungsbilanz (ohne Wasserpumpe)

Stand: 2026-03-05

## Zweck
Diese Datei berechnet die Leistungsaufnahme der verbauten Komponenten.
Die Wasserpumpe ist explizit aus der Rechnung entfernt.
Der Schrittmotor-Zweig ist getrennt betrachtet:
- Motor-Nennwerte (Spule): 3.3V, 1.5A pro Phase
- Treiber-Versorgung (2x TB6600): extern 12V

Formel:

- `P [W] = U [V] * I [A]`

Hinweis:

- Werte mit `*` sind konservative Annahmen (Klon-/Modulabhaengig).

## Einzelwerte je Komponente

| Komponente | U (V) | I_min (A) | I_max (A) | P_min (W) | P_max (W) | Hinweis |
|---|---:|---:|---:|---:|---:|---|
| Raspberry Pi 4 Model B | 5.0 | 0.600 | 3.000 | 3.000 | 15.000 | 3A ist Sizing-Grenze des Netzteils |
| Arduino Mega 2560 R3 (Board) | 5.0 | 0.070* | 0.200* | 0.350 | 1.000 | Board-Eigenstrom |
| HC-SR04 | 5.0 | 0.015 | 0.020 | 0.075 | 0.100 | Datenblatt 15..20mA |
| Funduino Tropfensensor | 5.0 | 0.010* | 0.015* | 0.050 | 0.075 | Klonabhaengig |
| Truebungssensor (SEN0189) | 5.0 | 0.040 | 0.040 | 0.200 | 0.200 | 40mA |
| TDS-Sensor (SEN0244) | 5.0 | 0.003 | 0.006 | 0.015 | 0.030 | 3..6mA |
| DHT11 | 5.0 | 0.0005* | 0.0025* | 0.0025 | 0.0125 | Typisch Standby/Messbetrieb |
| Taster (5x, INPUT_PULLUP) | 5.0 | 0.0000 | 0.00125* | 0.0000 | 0.00625 | max bei 5x gleichzeitig gedrueckt (ca. 0.25mA je Taster) |
| Taster-LEDs (5x) | 5.0 | 0.0000 | 0.1000* | 0.0000 | 0.5000 | Annahme max 20mA je LED mit Vorwiderstand |
| Relaismodul 4-Kanal (Spulen) | 5.0 | 0.000 | 0.2856 | 0.000 | 1.428 | min: alle AUS, max: alle 4 EIN |
| RFID RC522 | 3.3 | 0.026* | 0.050* | 0.0858 | 0.165 | 3.3V-Pin am Mega beachten |
| Pegelwandler 5V<->3.3V | 3.3 | 0.001* | 0.006* | 0.0033 | 0.0198 | Lastabhaengig |
| Schrittmotor 42HSC8402-15B11 (pro Motor, pro Phase) | 3.3 | 0.000 | 1.500 | 0.000 | 4.950 | Motor-Nennwert pro Phase (`rated current`) |
| TB6600 #1 + Schrittmotor #1 am 12V-Netzteil | 12.0 | 0.000 | 1.000* | 0.000 | 12.000* | abgeschaetzt aus Motorleistung (2 Phasen) + Treiberverlust |
| TB6600 #2 + Schrittmotor #2 am 12V-Netzteil | 12.0 | 0.000 | 1.000* | 0.000 | 12.000* | identische Annahme wie fuer Motor 1 |

## Getrennte Rechnung

### 1) Arduino-Teil (ohne Raspberry Pi, ohne Pumpe)

5V-Zweig:

- `I_5V_min = 0.070 + 0.015 + 0.010 + 0.040 + 0.003 + 0.0005 + 0.0000 + 0.0000 + 0.0000 = 0.1385 A`
- `P_5V_min = 5.0 * 0.1385 = 0.6925 W`
- `I_5V_max = 0.200 + 0.020 + 0.015 + 0.040 + 0.006 + 0.0025 + 0.00125 + 0.1000 + 0.2856 = 0.67035 A`
- `P_5V_max = 5.0 * 0.67035 = 3.35175 W`

3.3V-Zweig:

- `I_3V3_min = 0.026 + 0.001 = 0.027 A`
- `P_3V3_min = 3.3 * 0.027 = 0.0891 W`
- `I_3V3_max = 0.050 + 0.006 = 0.056 A`
- `P_3V3_max = 3.3 * 0.056 = 0.1848 W`

Arduino gesamt:

- `P_ARDUINO_min = 0.6925 + 0.0891 = 0.7816 W`
- `P_ARDUINO_max = 3.35175 + 0.1848 = 3.53655 W`

### 2) Raspberry-Pi-Teil (getrennt)

- `P_PI_min = 5.0 * 0.600 = 3.000 W`
- `P_PI_max = 5.0 * 3.000 = 15.000 W`

### 3) Schrittmotor/TB6600-Zweig (getrennt, 2 Motoren)

- Geplantes Modell: `42HSC8402-15B11` (noch zu verifizieren)
- Treiber: TB6600
- Versorgung: externes `12V` Netzteil

Berechnungsschema je Motor:

- Motor pro Phase:
  - `U_phase = 3.3V`
  - `I_phase = 1.5A`
  - `P_phase = 3.3 * 1.5 = 4.95W`
- Motor geschaetzt bei 2 aktiven Phasen:
  - `P_motor_max = 2 * 4.95 = 9.9W`
- TB6600-12V-Eingang (nahezu konservativ):
  - `I_12V_ideal = 9.9 / 12.0 = 0.825A`
  - `I_12V_max_annahme = 1.0A`
  - `P_12V_max = 12.0 * 1.0 = 12.0W`

Gesamt fuer 2 Motoren/Treiber:

- `I_12V_max_annahme_gesamt = 2 * 1.0 = 2.0A`
- `P_12V_max_gesamt = 12.0 * 2.0 = 24.0W`

Hinweis:
- `rated current` ist Phasenstrom (nicht direkt der 12V-Netzteilstrom).
- Der TB6600 ist ein Chopper-Treiber; die Motor-Nennspannung ist nicht gleich
  der Treiber-Versorgungsspannung.

## Min-/Max-Zusammenfassung

| Teilsystem | P_min (W) | P_max (W) |
|---|---:|---:|
| Arduino (ohne Pumpe) | 0.7816 | 3.5366 |
| Raspberry Pi | 3.0000 | 15.0000 |
| Gesamt (ohne Pumpe) | 3.7816 | 18.5366 |
| Externer 12V TB6600/Schrittmotor-Zweig (2x) | 0.0000 | 24.0000 |
| Gesamt inkl. TB6600/Schrittmotor-Zweig | 3.7816 | 42.5366 |

## Netzteil-Empfehlung (ohne Pumpe)

- Raspberry Pi separat: 5V / 3A (offizielle Empfehlung).
- Arduino-Zweig separat:
  - max 5V-Strom: `0.67035 A`
  - mit 30% Reserve: `0.67035 * 1.3 = 0.8715 A`
  - Empfehlung: mindestens 5V / 1A nur fuer Arduino+Sensorik+Relais+Taster/LEDs.

## Netzteil-Empfehlung (12V Schrittmotor-Zweig)

- Basis: `I_12V_max_annahme_gesamt = 2.0 A` (2x TB6600 + 2x Schrittmotor)
- Nennleistung ohne Reserve:
  - `P_12V_netzteil_nenn = 12.0 * 2.0 = 24.0 W`
- Mit 30% Reserve:
  - `P_12V_netzteil_min = 24.0 * 1.3 = 31.2 W`
- Empfehlung:
  - mindestens `12V / 3A` (36W)
  - empfohlen `12V / 4A` (48W) fuer ausreichende Dynamikreserve

## Quellen

- Raspberry Pi Dokumentation:  
  https://www.raspberrypi.com/documentation/computers/raspberry-pi.html
- Arduino Mega 2560 Rev3:  
  https://store.arduino.cc/products/arduino-mega-2560-rev3
- HC-SR04 Datenblatt:  
  https://www.elecfreaks.com/download/HC-SR04.pdf
- DFRobot SEN0189 (Truebung):  
  https://wiki.dfrobot.com/Turbidity_sensor_SKU__SEN0189
- DFRobot SEN0244 (TDS):  
  https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
- DHT11 Datenblatt (Typreferenz):  
  https://components101.com/sites/default/files/component_datasheet/DHT11-Temperature-Sensor.pdf
- NXP MFRC522 Datenblatt:  
  https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf
- Songle SRD-05VDC-SL-C:  
  https://www.datasheetq.com/en/pdf-html/715120/ETC/2page/SRD-05VDC-SL-C.html
- Modellbezeichnung Schrittmotor (Projektannahme):  
  42HSC8402-15B11 (konkretes Datenblatt noch offen)
- NEMA17 Referenzwerte (allgemein, nicht modellspezifisch):  
  https://reprap.org/wiki/NEMA_17_Stepper_motor
