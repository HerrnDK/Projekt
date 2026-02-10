# Logbuch

Dieses Logbuch dokumentiert Fortschritte seit dem letzten Eintrag.
Regelungen für Einträge:
- Jeder Eintrag hat einen Zeitstempel als Überschrift (z. B. `YYYY-MM-DD HH:MM TZ`).
- Jeder Eintrag enthält die Abschnitte `Erreicht`, `Nächstes Ziel` und `Bestehende Probleme`.
- Inhalte sind kurz, konkret und bezogen auf Änderungen seit dem letzten Eintrag.
- Jeder neue Eintrag übernimmt die letzten Ziele und Probleme die nicht beseitigt wurden.

## 2026-01-31

Erreicht:
- Anschluss und Inbetriebnahme des Raspberry Pi mit Touch-Display.
- Installation Node-red + Addons für QR-Code generation, Dashboard Elemente
- Installation Netzwerktools wie nmcli für die Steuerung der Wlan Verbindung
- Autostart vom Browser im Kioskmodus
- Anlegen der Ersten Dashboards wie Welcome, Wifi Verbindung, Sensoren, Parametrierung
- Automatisierung der Netzwerkverbindung (Kein Wlan Verbunden dann AP an, dann über Wifi Dashboard Anmeldung an Lokales Wifi Netzwerk möglich)

Nächstes Ziel:
- 

Bestehende Probleme:
- Trotz Touch Display keine On Screen Tastatur aufgrund der nutzung des Browsers im Kiosk Modus

## 2026-02-05 20:54 UTC

Erreicht:
- Anlegen eines Github Projekts
- Zentrales Logbuch `LOGBOOK.md` angelegt und Erststruktur mit Zeitstempel erstellt.
- Deployment-Guide und Deploy-Skript nach `nodered/flows/` verschoben.
- Node-RED-Flow-Dateien ergänzt/aktualisiert (`nodered/flows/Network.json` hinzugefügt, `nodered/flows/dashboard_flow.json` aktualisiert).
- `components.yaml` aus dem Repo-Root entfernt; neue Datei unter `nodered/flows/components.yaml` hinzugefügt.

Nächstes Ziel:
- Pinout-Dokumentation konkretisieren (Sensoren/Aktoren, I2C/SPI-Adressen, analoge Eingänge befüllen).
- Data exchange zwischen Arduino und Node-RED.
- Flows für Automatisierungen anlegen.

Bestehende Probleme:
- (Beschreibung bestehender Probleme)

## 2026-02-05

Erreicht:
- Arduino-Sketch in Wrapper (.ino) + Implementierungen (.cpp/.h) aufgeteilt, damit `arduino-cli` stabil kompiliert.
- Gemeinsame Basis in `arduino/mega/mega_shared.h/.cpp` eingefuehrt (Ports, Pins, Structs).
- Serial1-Protokoll erweitert: JSON-Ausgaben mit `type` (`sensor`, `act`, `error`) inkl. ACK und Fehlercodes.
- `scripts/arduino_build.sh` hinzugefuegt (auto-installiert `arduino:avr` Core bei Bedarf).
- `.gitignore` angelegt und `bin/` ausgeschlossen.
- `README.md` auf aktuelle Struktur/Protokoll/Build/Deploy aktualisiert.
- `nodered/flows/DEPLOYMENT_GUIDE.md` auf Deploy via `git pull` auf der Pi + `deploy_flows.sh` umgestellt.
- `nodered/flows/data_exchange_flow.json` Labels fuer Klarheit aktualisiert.
- `arduino-cli` lokal installiert und Build erfolgreich ausgefuehrt.

Nächstes Ziel:
- Pinout-Dokumentation konkretisieren (Sensoren/Aktoren, I2C/SPI-Adressen, analoge Eingänge befuellen).
- Git Pull auf der Pi ausfuehren und Flows mit `./nodered/flows/deploy_flows.sh` deployen.
- Serial1-Kommunikation am echten Aufbau testen (READ/ACT, JSON im Dashboard).
- Flows fuer Automatisierungen ergaenzen.

Bestehende Probleme:
- Node-RED von der Entwicklungsumgebung aus nicht erreichbar (Deploy hier nicht moeglich).
- On-Screen-Tastatur im Kioskmodus weiterhin nicht verfuegbar.

## 2026-02-07

Erreicht:
- Dokumentation zur UART-Pegelwandlung ergänzt (`README.md`, `Hardware.md`, `arduino/mega/PINOUT.md`, `nodered/flows/DEPLOYMENT_GUIDE.md`).
- Architektur-/Ablaufplan für "ein Flow pro Funktion" als `nodered/flows/FLOW_ARCHITEKTUR_PLAN.md` angelegt.
- Neue Funktionslogik `nodered/flows/fn_startup_test_flow.json` umgesetzt (Startup-Sensortest, Anlagenstatus bereit/stoerung, Boot-Initialstatus, zyklische Prüfung).
- Deploy-Skript `nodered/flows/deploy_flows.sh` erweitert:
  - robustere Pfadbehandlung (kein doppeltes `nodered/flows`),
  - Einbindung von `fn_startup_test_flow.json` in Checks und Flow-Kombination.
- Netzwerk-/Welcome-Flow erweitert:
  - dynamische QR-Anzeige (Setup: AP+Login, bei WLAN-Verbindung: zentraler Dashboard-QR),
  - WiFi-Tab zeigt URLs für Debug und Dashboard.
- LAN-Debug-URL auf kabelgebundene Schnittstelle umgestellt (`end0`), inkl. Resolver-Node in `Network.json`.
- Reihenfolge im WiFi-Tab angepasst:
  - SSID,
  - Passwort direkt darunter,
  - danach Buttons in Reihenfolge Verbinden -> Trennen -> Neu laden (Desktop und Mobile).
- `nodered/flows/components.yaml` konsistent mit neuen Netzwerk-/Startup-Komponenten erweitert.

Nächstes Ziel:
- Pinout-Dokumentation konkretisieren (Sensoren/Aktoren, I2C/SPI-Adressen, analoge Eingänge befuellen).
- Git Pull auf der Pi ausfuehren und Flows mit `./nodered/flows/deploy_flows.sh` deployen.
- Serial1-Kommunikation am echten Aufbau testen (READ/ACT, JSON im Dashboard).
- Nächsten Funktionsflow nach gleichem Muster aus dem Architekturplan anlegen (z. B. Aktor-Funktion separat).
- Optionale Bereinigung alter Legacy-QR-Nodes in `Network.json` (falls nach Stabilisierung nicht mehr benötigt).

Bestehende Probleme:
- Node-RED von der Entwicklungsumgebung aus nicht erreichbar (Deploy hier nicht moeglich).
- `yaml-lint` ist in der aktuellen Umgebung nicht installiert (YAML-Formatcheck nur eingeschränkt möglich).
- On-Screen-Tastatur im Kioskmodus weiterhin nicht verfuegbar.

## 2026-02-10 18:09 UTC

Erreicht:
- Arduino-Mega-Struktur auf einen klaren Sketch-Einstieg vereinfacht (`arduino/mega/mega.ino` mit `setup()`/`loop()`).
- Ueberfluessige Wrapper-/Altdateien entfernt (`arduino/mega/main.ino`, `arduino/mega/main.cpp`, `arduino/mega/data.ino`, `arduino/mega/funktion_*.ino`).
- Moduldateien einheitlich benannt (`arduino/mega/actuators.cpp`, `arduino/mega/sensors.cpp` statt `funktion_*`).
- Dokumentation auf neue Arduino-Struktur aktualisiert (`README.md`, `.github/instructions/Anweisungen.instructions.md`).
- `AGENTS.md` im Repo-Root angelegt und eine klare Regel-Hierarchie zu `.github/instructions/Anweisungen.instructions.md` definiert.
- Upload-Workflow um `scripts/arduino_watch_upload.sh` erweitert und in der Doku ergaenzt.
- Dashboard-Flow weiter angepasst (`nodered/flows/dashboard_flow.json`).

Nächstes Ziel:
- Pinout-Dokumentation konkretisieren (Sensoren/Aktoren, I2C/SPI-Adressen, analoge Eingänge befuellen).
- Git Pull auf der Pi ausfuehren und Flows mit `./nodered/flows/deploy_flows.sh` deployen.
- Serial1-Kommunikation am echten Aufbau testen (READ/ACT, JSON im Dashboard).
- Nächsten Funktionsflow nach gleichem Muster aus dem Architekturplan anlegen (z. B. Aktor-Funktion separat).
- Optionale Bereinigung alter Legacy-QR-Nodes in `Network.json` (falls nach Stabilisierung nicht mehr benötigt).
- Arduino-Watch-Upload im Zielsystem im Realbetrieb pruefen (Änderung -> Auto-Compile -> Auto-Upload).

Bestehende Probleme:
- Node-RED von der Entwicklungsumgebung aus nicht erreichbar (Deploy hier nicht moeglich).
- `yaml-lint` ist in der aktuellen Umgebung nicht installiert (YAML-Formatcheck nur eingeschränkt möglich).
- On-Screen-Tastatur im Kioskmodus weiterhin nicht verfuegbar.
