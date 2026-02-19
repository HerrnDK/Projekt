# Leistungsbilanz (Datenblattbasiert)

Stand: 2026-02-19

## Zweck
Diese Datei dokumentiert Spannung, Strom und Leistung der im Projekt verwendeten Komponenten.
Berechnung:

- `P [W] = U [V] * I [A]`

Wichtig:

- Bei Klon-Modulen (z. B. Funduino/FC-37) schwanken Datenblattwerte je Hersteller.
- Fuer die 5V-Wasserpumpe ist der exakte Strom vom Pumpen-Typenschild/Datenblatt einzutragen.
- Werte mit `*` sind konservative Annahmen, wenn das konkrete Datenblatt keinen eindeutigen Betriebsstrom angibt.

## Einzelkomponenten

| Komponente | Anzahl | Versorgung | I_typ (A) | I_max (A) | P_typ (W) | P_max (W) | Hinweis |
|---|---:|---:|---:|---:|---:|---:|---|
| Raspberry Pi 4 Model B | 1 | 5.0V | 0.600 | 3.000 | 3.000 | 15.000 | 3.0A ist Netzteil-Empfehlung (Sizing), nicht Dauerlast |
| Arduino Mega 2560 R3 (Board-Basis) | 1 | 5.0V | 0.070* | 0.200* | 0.350 | 1.000 | Board-Eigenstrom nicht eindeutig im Mega-Board-Datenblatt |
| HC-SR04 | 1 | 5.0V | 0.015 | 0.020 | 0.075 | 0.100 | Working current 15mA, max laut User-Manual 20mA |
| Funduino Tropfensensor (FC-37/YL-83) | 1 | 5.0V | 0.010* | 0.015* | 0.050 | 0.075 | Klonmodul, Stromaufnahme oft nicht sauber spezifiziert |
| Truebungssensor (DFRobot SEN0189) | 1 | 5.0V | 0.040 | 0.040 | 0.200 | 0.200 | 40mA (MAX) |
| TDS-Sensor (DFRobot SEN0244) | 1 | 5.0V | 0.0045 | 0.006 | 0.0225 | 0.030 | 3..6mA |
| Relaismodul (SRD-05VDC-SL-C), 1 Relais EIN | 1 von 4 | 5.0V | 0.0714 | 0.0714 | 0.357 | 0.357 | Coil 0.36W bei 5V |
| Relaismodul (SRD-05VDC-SL-C), 4 Relais EIN | 4 von 4 | 5.0V | 0.2856 | 0.2856 | 1.428 | 1.428 | `4 * 71.4mA` |
| RFID RC522 (MFRC522-Modul) | 1 | 3.3V | 0.026* | 0.050* | 0.0858 | 0.165 | Mega-3.3V-Pin ist mit 50mA begrenzt |
| Pegelwandler 5V<->3.3V (BSS138-Typ) | 1 | 5.0V/3.3V | 0.001* | 0.006* | 0.004* | 0.030* | Lastabhaengig, meist klein |
| Wasserpumpe 5V (Relais 1 Last) | 1 | 5.0V | `I_pumpe_typ` | `I_pumpe_max` | `5 * I_pumpe_typ` | `5 * I_pumpe_max` | Muss aus Pumpen-Datenblatt eingesetzt werden |

## Zusammenrechnung

### 1) Arduino-/Sensor-/Relais-Zweig (ohne Raspberry Pi)

Typisch (1 Relais EIN, ohne Pumpe):

- `I_5V_typ_ohne_pumpe = 0.070 + 0.015 + 0.010 + 0.040 + 0.0045 + 0.0714 = 0.2109 A`
- `P_5V_typ_ohne_pumpe = 5.0 * 0.2109 = 1.0545 W`

Maximal (4 Relais EIN, ohne Pumpe):

- `I_5V_max_ohne_pumpe = 0.200 + 0.020 + 0.015 + 0.040 + 0.006 + 0.2856 = 0.5666 A`
- `P_5V_max_ohne_pumpe = 5.0 * 0.5666 = 2.8330 W`

3.3V-Zweig (RC522 + Pegelwandler):

- `I_3V3_typ = 0.027 A`
- `P_3V3_typ = 3.3 * 0.027 = 0.0891 W`
- `I_3V3_max = 0.056 A`
- `P_3V3_max = 3.3 * 0.056 = 0.1848 W`

Mit Pumpe:

- `P_arduino_typ_mit_pumpe = 1.0545 + 0.0891 + 5 * I_pumpe_typ`
- `P_arduino_max_mit_pumpe = 2.8330 + 0.1848 + 5 * I_pumpe_max`

Beispiel (`I_pumpe_typ = 0.60A`, `I_pumpe_max = 1.00A`):

- `P_arduino_typ_mit_pumpe = 1.0545 + 0.0891 + 3.000 = 4.1436 W`
- `P_arduino_max_mit_pumpe = 2.8330 + 0.1848 + 5.000 = 8.0178 W`

### 2) Gesamtsystem inkl. Raspberry Pi

Typisch:

- `P_gesamt_typ = P_pi_typ + P_arduino_typ_mit_pumpe`
- `P_gesamt_typ = 3.000 + (1.0545 + 0.0891 + 5 * I_pumpe_typ)`
- `P_gesamt_typ = 4.1436 + 5 * I_pumpe_typ`

Maximal (Sizing):

- `P_gesamt_max = P_pi_max + P_arduino_max_mit_pumpe`
- `P_gesamt_max = 15.000 + (2.8330 + 0.1848 + 5 * I_pumpe_max)`
- `P_gesamt_max = 18.0178 + 5 * I_pumpe_max`

Beispiel mit `I_pumpe_max = 1.00A`:

- `P_gesamt_max = 23.0178 W`

## Empfehlung Netzteile

- Raspberry Pi 4: offizielles 5V/3A Netzteil.
- Arduino + Sensorik + Relais + Pumpe:
  - Strombedarf mindestens `I_5V_max_ohne_pumpe + I_pumpe_max`.
  - Mit 30% Reserve: `I_netzteil >= 1.3 * (0.5666 + I_pumpe_max)`.
  - Beispiel `I_pumpe_max = 1.00A` -> `I_netzteil >= 2.04A`, empfohlen 5V/3A.

## Quellen

- Raspberry Pi Dokumentation (Typical power requirements):  
  https://www.raspberrypi.com/documentation/computers/raspberry-pi.html
- Arduino Mega 2560 Rev3 (Tech Specs):  
  https://store.arduino.cc/products/arduino-mega-2560-rev3
- HC-SR04 User Manual (ELECFREAKS/Cytron):  
  https://www.elecfreaks.com/download/HC-SR04.pdf
- DFRobot SEN0189 (Truebungssensor):  
  https://wiki.dfrobot.com/Turbidity_sensor_SKU__SEN0189
- DFRobot SEN0244 (TDS):  
  https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
- NXP MFRC522 Datenblatt:  
  https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf
- Songle SRD-05VDC-SL-C Coil-Daten (0.36W, 71.4mA bei 5V):  
  https://www.datasheetq.com/en/pdf-html/715120/ETC/2page/SRD-05VDC-SL-C.html
- FC-37/YL-83 Modul-Spezifikationen (Klon-abh., 3.3..5V, >15mA Treiberfaehigkeit):  
  https://abc-rc.pl/en/products/fc-37-rain-sensor-water-detector-for-arduino-3-5v-6308.html

