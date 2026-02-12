# Arduino Mega 2560 + Node-RED (Raspberry Pi)

Dieses Repository enthaelt das Grundgeruest fuer die serielle Kommunikation
zwischen einem Arduino Mega 2560 R3 und einem Raspberry Pi mit Node-RED.

## Wiring (Serial1)
- **Arduino Mega 2560** Serial1:
  - TX1 (Pin 18, 5V) -> Pegelwandler 5V->3.3V -> Raspberry Pi RX
  - RX1 (Pin 19) <- Raspberry Pi TX (3.3V)
  - GND -> GND

> WICHTIG: Zwischen Mega TX1 (5V) und Raspberry Pi RX muss ein Pegelwandler
> (oder mindestens ein geeigneter Spannungsteiler) verwendet werden.
> Raspberry Pi GPIO ist nicht 5V-tolerant.

> Der USB-Port (Serial RX0/TX0) bleibt fuer Programmierung und Debug frei.

## Wiring (HC-SR04)
- HC-SR04 VCC -> 5V
- HC-SR04 GND -> GND
- HC-SR04 TRIG -> D26
- HC-SR04 ECHO -> D27

## Wiring (RFID RC522)
- RC522 SDA/SS -> D53 (SS)
- RC522 SCK -> D52
- RC522 MOSI -> D51
- RC522 MISO -> D50
- RC522 RST -> D49
- RC522 3.3V -> 3.3V
- RC522 GND -> GND

> Hinweis: RC522 muss mit 3.3V betrieben werden.

## Wiring (Funduino Tropfensensor)
- Tropfensensor +5V -> 5V
- Tropfensensor -GND -> GND
- Tropfensensor S (Analog) -> A0

## Wiring (4-Kanal Relaismodul)
- Relaismodul VCC -> 5V
- Relaismodul GND -> GND
- Relaismodul IN1 -> D22 (Relais 1: Wasserpumpe)
- Relaismodul IN2 -> D23 (Relais 2: Reserve)
- Relaismodul IN3 -> D24 (Relais 3: Reserve)
- Relaismodul IN4 -> D25 (Relais 4: Reserve)

> Hinweis: Das eingesetzte Relaismodul ist active-low (gegen GND schaltend):
> `ACT,...,1` = Relais EIN = IN-Pin auf `LOW` (0V),
> `ACT,...,0` = Relais AUS = IN-Pin auf `HIGH` (5V).

## Protokoll (Serial1, newline-terminiert)
- `READ` -> Arduino sendet JSON Sensor-Snapshot
- `ACT,<pin>,<state>` -> Aktor schalten, JSON-ACK
- `RFID` -> RFID Snapshot lesen (UID + Status inkl. Modulzustand)

JSON-Formate (Beispiele):
- Sensor: `{"type":"sensor","hcsr04_distance_cm":42,"hcsr04_status":"ok","droplet_raw":512,"droplet_status":"ok","uptime_ms":7890}`
- Act: `{"type":"act","ok":1,"pin":22,"state":1,"hcsr04_distance_cm":42,"hcsr04_status":"ok","droplet_raw":512,"droplet_status":"ok","uptime_ms":7890}`
- Error: `{"type":"error","code":"unknown_command","hcsr04_distance_cm":-1,"hcsr04_status":"error_timeout","droplet_raw":-1,"droplet_status":"error_range","uptime_ms":7890}`
- RFID: `{"type":"rfid","rfid_uid":"DE:AD:BE:EF","rfid_status":"ok","rfid_hw_status":"ok","rfid_probe_status":"STATUS_OK","rfid_version_reg":"0x92","uptime_ms":7890}`

HC-SR04 Statuswerte:
- `ok` gemessene Distanz ist gueltig
- `error_timeout` kein Echo innerhalb Timeout
- `error_range` gemessene Distanz ausserhalb 2..400 cm

Tropfensensor Statuswerte:
- `ok` analoger Rohwert ist gueltig (0..1023)
- `error_range` analoger Rohwert ausserhalb des gueltigen ADC-Bereichs
- `error_not_connected` Sensorleitung ist vermutlich abgezogen/floating

RFID Statuswerte:
- `ok` gueltige UID gelesen
- `no_card` kein RFID Chip praesent
- `probe_error` Reader-Probe fehlgeschlagen (z. B. Signal-/SPI-Problem)
- `read_error` Lesen fehlgeschlagen
- `uid_truncated` UID zu lang fuer Payload-Buffer

RFID Hardwarestatus (`rfid_hw_status`):
- `ok` RC522 Modul antwortet
- `error_not_detected` RC522 Modul nicht erkannt
- `error_not_initialized` RC522 noch nicht initialisiert

RFID Diagnose:
- `rfid_probe_status` letzte REQA/WUPA Probe (`STATUS_OK`, `STATUS_TIMEOUT`, ...)
- `rfid_version_reg` Inhalt von `VersionReg` (typisch `0x91` oder `0x92`)

## Repo-Struktur (wichtige Dateien)
- `arduino/mega/`
  - `mega.ino` Sketch-Root mit `setup()`/`loop()`
  - `mega_shared.h` gemeinsame Typen/Prototypen
  - `mega_shared.cpp` gemeinsame Definitionen
  - `data.cpp` Implementierung Serial1-Protokoll
  - `actuators.cpp` Implementierung Aktoren
  - `sensors.cpp` Implementierung Sensoren
  - `PINOUT.md` Quelle der Wahrheit fuer Pins
- `nodered/flows/`
  - `dashboard_flow.json` UI + Sensoranzeigen + Parametrierung
  - `Network.json` Netzwerk-Tab
  - `data_exchange_flow.json` Serial-I/O Arduino
  - `fn_parameters_flow.json` Parameter-Logik (HC-SR04 + Tropfensensor Offset + Relaissteuerung)
  - `fn_profiles_flow.json` RFID Profile-Logik (Anlernen + Profilzuweisung)
  - `components.yaml` logische Komponentenreferenzen
  - `deploy_flows.sh` Flow-Deploy Script (POST /flows)
  - `DEPLOYMENT_GUIDE.md` Deployment-Doku
- `scripts/arduino_build.sh` lokaler Build via Arduino CLI
- `.github/instructions/Anweisungen.instructions.md` Arbeitsregeln
- `LOGBOOK.md` Arbeitslog (nur nach expliziter Aufforderung aktualisieren)

## Arduino IDE
- Oeffne `arduino/mega/mega.ino`.
- Die `.cpp/.h` Dateien werden automatisch mitgebaut.

## Arduino Libraries
- Fuer RFID RC522 wird die Bibliothek `MFRC522` benoetigt (Library Manager oder Arduino CLI).

## Build (Arduino CLI)
- Compile (Mega 2560):
  - `bin/arduino-cli compile --fqbn arduino:avr:mega arduino/mega`
  - oder: `./scripts/arduino_build.sh` (installiert den Core automatisch, falls noetig)

## Auto-Upload bei Dateiaenderung (Pi -> USB)
- Fuer "Speichern -> direkt flashen" nutze:
  - `./scripts/arduino_watch_upload.sh`
- Optional mit festem Port:
  - `ARDUINO_PORT=/dev/ttyACM0 ./scripts/arduino_watch_upload.sh`
- Nur einmal bauen + flashen (ohne Watch):
  - `./scripts/arduino_watch_upload.sh --once`

Hinweise:
- Das Script beobachtet `arduino/mega` auf `*.ino`, `*.cpp`, `*.c`, `*.h`, `*.hpp`.
- Bei jeder Aenderung: Compile via `scripts/arduino_build.sh`, danach Upload via `arduino-cli upload`.
- Falls `arduino-cli` fehlt, wird es automatisch lokal nach `./bin/arduino-cli` installiert.
- Mit `inotifywait` reagiert es sofort, sonst nutzt es Polling als Fallback.

## Deployment (Node-RED auf dem Pi)
- Flows werden direkt auf dem Raspberry Pi aktualisiert:
  1. `cd ~/.node-red` (falls das dein Repo-Root ist), dann `git pull`
  2. `./nodered/flows/deploy_flows.sh`

## Parametrierung (Dashboard)
- Im Tab `Projekt-Parametrierung` gibt es einen Slider `HC-SR04 Korrektur (cm)` mit Bereich `-5 .. +5`.
- Es gibt zusaetzlich einen Slider `Tropfensensor Offset (raw)` mit Bereich `-300 .. +300`.
- Im Tab `Projekt-Parametrierung` gibt es zusaetzlich 4 Relais-Buttons:
  - `Relais 1 (Pumpe)` schaltet D22
  - `Relais 2 (Reserve)` schaltet D23
  - `Relais 3 (Reserve)` schaltet D24
  - `Relais 4 (Reserve)` schaltet D25
- Die Offsets werden in `fn_parameters_flow.json` gespeichert (`global.hcsr04_offset_cm`, `global.droplet_offset_raw`).
- Die Anzeigen nutzen die korrigierten Werte `hcsr04_distance_display_cm` und `droplet_display_raw`.

## Profile (Dashboard)
- Im Tab `Profile` gibt es drei Schaltflaechen:
  - `Lesen` (sendet `RFID_READ`)
  - `Profil 1 anlernen/loeschen` (sendet `RFID_LEARN_P1`)
  - `Profil 2 anlernen/loeschen` (sendet `RFID_LEARN_P2`)
- Es werden genau zwei UID-Slots gepflegt:
  - UID in Slot 1 -> `Profil 1`
  - UID in Slot 2 -> `Profil 2`
- Wenn ein Profil bereits belegt ist, loescht der jeweilige Profil-Button die Bindung.
- Bereits bekannte Chips aktivieren direkt ihr hinterlegtes Profil.
- Die erkannte UID, das aktive Profil und der RFID Modulstatus werden live angezeigt.
- Im Tab `Projekt-info` unter `Status / Sensoren` werden zusaetzlich `RFID RC522 Status`, `Tropfensensor Status` und die 4 Relais-Zustaende (`ON`/`OFF`) angezeigt.

## Next Steps (wenn Sensoren/Aktoren bekannt sind)
- Weitere Sensoren nach gleichem Muster in `sensors.cpp` ergaenzen.
- `ACTUATOR_PINS` und UI-Buttons bei Bedarf erweitern.
- Baudrate ggf. anpassen.


