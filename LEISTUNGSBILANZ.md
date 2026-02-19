# Leistungsbilanz (ohne Wasserpumpe)

Stand: 2026-02-19

## Zweck
Diese Datei berechnet die Leistungsaufnahme der verbauten Komponenten.
Die Wasserpumpe ist explizit aus der Rechnung entfernt.

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
| Relaismodul 4-Kanal (Spulen) | 5.0 | 0.000 | 0.2856 | 0.000 | 1.428 | min: alle AUS, max: alle 4 EIN |
| RFID RC522 | 3.3 | 0.026* | 0.050* | 0.0858 | 0.165 | 3.3V-Pin am Mega beachten |
| Pegelwandler 5V<->3.3V | 3.3 | 0.001* | 0.006* | 0.0033 | 0.0198 | Lastabhaengig |

## Getrennte Rechnung

### 1) Arduino-Teil (ohne Raspberry Pi, ohne Pumpe)

5V-Zweig:

- `I_5V_min = 0.070 + 0.015 + 0.010 + 0.040 + 0.003 + 0.000 = 0.138 A`
- `P_5V_min = 5.0 * 0.138 = 0.690 W`
- `I_5V_max = 0.200 + 0.020 + 0.015 + 0.040 + 0.006 + 0.2856 = 0.5666 A`
- `P_5V_max = 5.0 * 0.5666 = 2.833 W`

3.3V-Zweig:

- `I_3V3_min = 0.026 + 0.001 = 0.027 A`
- `P_3V3_min = 3.3 * 0.027 = 0.0891 W`
- `I_3V3_max = 0.050 + 0.006 = 0.056 A`
- `P_3V3_max = 3.3 * 0.056 = 0.1848 W`

Arduino gesamt:

- `P_ARDUINO_min = 0.690 + 0.0891 = 0.7791 W`
- `P_ARDUINO_max = 2.833 + 0.1848 = 3.0178 W`

### 2) Raspberry-Pi-Teil (getrennt)

- `P_PI_min = 5.0 * 0.600 = 3.000 W`
- `P_PI_max = 5.0 * 3.000 = 15.000 W`

## Min-/Max-Zusammenfassung

| Teilsystem | P_min (W) | P_max (W) |
|---|---:|---:|
| Arduino (ohne Pumpe) | 0.7791 | 3.0178 |
| Raspberry Pi | 3.0000 | 15.0000 |
| Gesamt (ohne Pumpe) | 3.7791 | 18.0178 |

## Netzteil-Empfehlung (ohne Pumpe)

- Raspberry Pi separat: 5V / 3A (offizielle Empfehlung).
- Arduino-Zweig separat:
  - max 5V-Strom: `0.5666 A`
  - mit 30% Reserve: `0.5666 * 1.3 = 0.7366 A`
  - Empfehlung: mindestens 5V / 1A nur fuer Arduino+Sensorik+Relais.

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
- NXP MFRC522 Datenblatt:  
  https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf
- Songle SRD-05VDC-SL-C:  
  https://www.datasheetq.com/en/pdf-html/715120/ETC/2page/SRD-05VDC-SL-C.html

