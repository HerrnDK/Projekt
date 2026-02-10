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

## Protokoll (Serial1, newline-terminiert)
- `READ` -> Arduino sendet JSON Sensor-Snapshot
- `ACT,<pin>,<state>` -> Aktor schalten, JSON-ACK

JSON-Formate (Beispiele):
- Sensor: `{"type":"sensor","a0":123,"a1":456,"uptime_ms":7890}`
- Act: `{"type":"act","ok":1,"pin":22,"state":1,"a0":123,"a1":456,"uptime_ms":7890}`
- Error: `{"type":"error","code":"unknown_command","a0":123,"a1":456,"uptime_ms":7890}`

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
  - `dashboard_flow.json` UI + Sensoren/Aktoren
  - `Network.json` Netzwerk-Tab
  - `data_exchange_flow.json` Serial-I/O Arduino
  - `components.yaml` logische Komponentenreferenzen
  - `deploy_flows.sh` Flow-Deploy Script (POST /flows)
  - `DEPLOYMENT_GUIDE.md` Deployment-Doku
- `scripts/arduino_build.sh` lokaler Build via Arduino CLI
- `.github/instructions/Anweisungen.instructions.md` Arbeitsregeln
- `LOGBOOK.md` Arbeitslog (nur nach expliziter Aufforderung aktualisieren)

## Arduino IDE
- Oeffne `arduino/mega/mega.ino`.
- Die `.cpp/.h` Dateien werden automatisch mitgebaut.

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
- Mit `inotifywait` reagiert es sofort, sonst nutzt es Polling als Fallback.

## Deployment (Node-RED auf dem Pi)
- Flows werden direkt auf dem Raspberry Pi aktualisiert:
  1. `git pull` im Repo auf dem Pi
  2. `./nodered/flows/deploy_flows.sh`

## Next Steps (wenn Sensoren/Aktoren bekannt sind)
- Platzhalterwerte (A0/A1) durch echte Sensoren ersetzen.
- `ACTUATOR_PINS` und UI-Buttons erweitern.
- Baudrate ggf. anpassen.
