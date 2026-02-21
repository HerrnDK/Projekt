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

## 2026-02-11 23:10 UTC

Erreicht:
- Dashboard-Uptime auf `HH:MM:SS` umgestellt (aus `uptime_ms` wird `uptime_hms` erzeugt und angezeigt).
- Update-Zyklus fuer Sensorabfrage auf 2 Sekunden gesetzt (`fn_startup_test_flow.json`, Inject-Repeat von `20` auf `2`).
- Parameter-Dashboard aufgeraeumt und strukturiert:
  - Beispiel-Ansteuerungen (Pin 22/23 Buttons Desktop + Mobile) entfernt.
  - HC-SR04-Parameter als Slider (`-5 .. +5 cm`) fuer Desktop und Mobile eingebaut.
  - Offset-Anzeige (`HC-SR04 Korrektur (cm)`) fuer Desktop und Mobile hinzugefuegt.
- Neuen separaten Funktionsflow `nodered/flows/fn_parameters_flow.json` eingefuehrt:
  - Boot-Initialisierung des Offsets,
  - Speichern/Validieren von Slider-Werten in `global.hcsr04_offset_cm`,
  - Berechnung `hcsr04_distance_display_cm` fuer die Anzeige.
- Datenrouting der Flows umgestellt:
  - `data_exchange_flow.json` verteilt Sensor-JSON jetzt an mehrere Funktionsflows (Startup + Parameter).
  - Dashboard empfängt Sensordaten fuer die Anzeige ueber den Parameter-Flow (korrigierte Distanzwerte).
- Deployment und Doku aktualisiert:
  - `deploy_flows.sh` um `fn_parameters_flow.json` erweitert (Checks + Combine).
  - `README.md`, `DEPLOYMENT_GUIDE.md`, `FLOW_ARCHITEKTUR_PLAN.md`, `components.yaml` und `.github/instructions/Anweisungen.instructions.md` auf die neue Struktur angepasst.
- Validierung ausgefuehrt:
  - JSON-Check erfolgreich (`jq . nodered/flows/*.json`).
  - Link-Referenzen der kombinierten Flows geprueft (`link-check: ok`).

Nächstes Ziel:
- Pinout-Dokumentation konkretisieren (Sensoren/Aktoren, I2C/SPI-Adressen, analoge Eingänge befuellen).
- Naechsten Funktionsflow nach gleichem Muster aus dem Architekturplan anlegen (z. B. weitere Sensor-/Aktor-Logik).
- Optionale Bereinigung alter Legacy-QR-Nodes in `Network.json` (falls nach Stabilisierung nicht mehr benoetigt).
- Arduino-Watch-Upload im Zielsystem im Realbetrieb pruefen (Aenderung -> Auto-Compile -> Auto-Upload).

Bestehende Probleme:
- Node-RED von der Entwicklungsumgebung aus nicht erreichbar (Deploy hier nicht moeglich).
- `yaml-lint` ist in der aktuellen Umgebung nicht installiert (YAML-Formatcheck nur eingeschränkt moeglich).
- On-Screen-Tastatur im Kioskmodus weiterhin nicht verfuegbar.

## 2026-02-12 20:15 UTC

Erreicht:
- RFID RC522 Funktionskette stabilisiert: Hardware-/Probe-Status im Status-Dashboard sichtbar, Profil-Flow mit 3 Buttons (`Lesen`, `Profil 1`, `Profil 2`) und Loesch-/Anlernlogik je Slot umgesetzt.
- Tropfensensor (Funduino) voll integriert:
  - Arduino Snapshot um `droplet_raw` + `droplet_status` erweitert,
  - Dashboard-Statusfelder (Desktop/Mobile) ergaenzt,
  - Startup-Test auf HC-SR04 + Tropfensensor erweitert,
  - Parametrierung mit eigenem Offset-Slider umgesetzt.
- Fehlerfall "Sensor abgezogen" fuer Tropfensensor abgesichert (`error_not_connected`) und Anzeige so angepasst, dass Fehlerwerte nicht mehr als gueltiger Messwert erscheinen.
- Relaisfunktion umgesetzt (4-Kanal Modul an D22-D25):
  - Parametrierung: je Relais ein Toggle-Button (Desktop/Mobile),
  - Status-Dashboard: Relais 1-4 nur als ON/OFF Anzeige,
  - Datenpfad ueber `ACT,<pin>,<state>` mit ACK-Auswertung.
- Relaislogik auf active-low angepasst (gegen GND schaltend):
  - logisch EIN -> LOW (0V),
  - logisch AUS -> HIGH (5V),
  - sicherer Bootzustand auf AUS gesetzt.
- Dokumentation konsistent nachgezogen (`README.md`, `arduino/mega/PINOUT.md`, `Hardware.md`, `FLOW_ARCHITEKTUR_PLAN.md`, `DEPLOYMENT_GUIDE.md`, `components.yaml`).
- Bedienkomfort auf der Pi verbessert: Shell-Befehl `update` fuer `git pull` + Arduino Upload + Flow-Deploy eingerichtet.

Naechstes Ziel:
- End-to-End Test auf der Zielhardware durchfuehren (Relais 1 Pumpe + Relais 2-4 Reserve, Statusrueckmeldung ON/OFF im Dashboard pruefen).
- Optional: Relaisanzeige um stoerungssichere Rueckmelde-Logik erweitern (z. B. separates Fehlerkennzeichen bei ACT-Fehlern).
- Optionale Bereinigung alter Legacy-QR-Nodes in `Network.json` (falls nach Stabilisierung nicht mehr benoetigt).
- Arduino-Watch-Upload im Zielsystem im Realbetrieb pruefen (Aenderung -> Auto-Compile -> Auto-Upload).

Bestehende Probleme:
- Node-RED von der Entwicklungsumgebung aus nicht erreichbar (Deploy hier nicht moeglich).
- `yaml-lint` ist in der aktuellen Umgebung nicht installiert (YAML-Formatcheck nur eingeschraenkt moeglich).
- On-Screen-Tastatur im Kioskmodus weiterhin nicht verfuegbar.

## 2026-02-19

Erreicht:
- Arduino-Code weiter modularisiert und vereinheitlicht:
  - Sensorik in eigene Module aufgeteilt (`sensor_hcsr04.cpp`, `sensor_tropfen.cpp`, `sensor_truebung.cpp`, `sensor_tds.cpp`, `sensor_rfid.cpp`).
  - Gemeinsame Schnittstelle in `mega_gemeinsam.h/.cpp` konsolidiert.
  - Dateikopf-Kommentare in Arduino-Dateien ergaenzt (Nutzen/Funktion je Datei).
- Sprachkonsistenz verbessert:
  - Bezeichner und Dokumentation weitgehend auf deutsche Begriffe umgestellt.
  - Doppelte Alt-/Neu-Dateien bereinigt, um Build-Konflikte (multiple definition) zu vermeiden.
- RFID-Funktion stabilisiert und erweitert:
  - Diagnosewerte (`rfid_hw_status`, `rfid_probe_status`, `rfid_version_reg`) durchgaengig nutzbar.
  - Profil-Logik mit 3 Buttons (`Lesen`, `Profil 1 anlernen/loeschen`, `Profil 2 anlernen/loeschen`) finalisiert.
  - Statusanzeige im Dashboard konsistent angebunden.
- Wassertruebungssensor voll integriert:
  - Arduino-Messwert + Status in Snapshot/JSON.
  - Dashboard-Anzeige (Wert/Status) in Desktop und Mobile.
  - Offset-Parametrierung inkl. Slider.
  - Fehlerlogik fuer `error_not_connected` angepasst.
- Ocean-TDS-Sensor voll integriert:
  - Arduino-Modul `sensor_tds.cpp` mit robuster Auslese- und Trennungslogik.
  - Snapshot/JSON um `tds_raw` und `tds_status` erweitert.
  - Startup-Test und Anlagenstatuspruefung um TDS erweitert.
  - Dashboard-Anzeigen (Wert/Status) und Offset-Slider (Desktop/Mobile) ergaenzt.
  - Komponentenregister (`nodered/flows/components.yaml`) inkl. Versionsanhebung aktualisiert.
- Anlagenstatus-Text verbessert:
  - Stoerungsanzeige fokussiert auf den betroffenen Sensor statt pauschaler Sammelmeldung.
- Architektur- und UML/Flow-Dokumentation ueberarbeitet:
  - Mermaid-Fehler fuer GitHub-Rendering korrigiert.
  - `PROJEKT_ARCHITEKTUR_PLAN.md` als zentrale, durchnummerierte Dokumentation erweitert.
  - Gesamtflussdiagramm fuer End-to-End-Ablauf hinzugefuegt.
- Betriebs- und Projektdoku aktualisiert:
  - `README.md`, `Hardware.md`, `arduino/mega/PINOUT.md`, `nodered/flows/DEPLOYMENT_GUIDE.md` auf neuen Stand gebracht.
  - Hinweise in `.github/instructions/Anweisungen.instructions.md` um Update-Workflow (`update`) und Berechtigungsgrenzen ergaenzt.
- Leistungsberechnung neu erstellt:
  - `LEISTUNGSBILANZ.md` angelegt.
  - Bilanz ohne Wasserpumpe aufgeteilt in Arduino-Teil und Raspberry-Pi-Teil.
  - Min-/Max-Leistungswerte sowie Gesamt-Min/Max dokumentiert.

Naechstes Ziel:
- End-to-End Test auf Zielhardware fuer alle Sensoren inkl. TDS und Truebung dokumentieren (Messplaene + Soll/Ist).
- Optional: TDS/Truebung mit realen Kalibrierpunkten (Referenzfluessigkeiten) verifizieren und Offset-Grenzen feinjustieren.
- Optional: Netzteil-/Stromkonzept mit echten Messwerten (Multimeter/USB-Power-Meter) gegen `LEISTUNGSBILANZ.md` validieren.
- Optional: Legacy-/Altknoten in `Network.json` weiter aufraeumen, falls im Betrieb nicht benoetigt.

Bestehende Probleme:
- Node-RED von der Entwicklungsumgebung aus nicht erreichbar (Deploy hier nicht moeglich).
- `yaml-lint` ist in der aktuellen Umgebung nicht installiert (YAML-Formatcheck nur eingeschraenkt moeglich).
- On-Screen-Tastatur im Kioskmodus weiterhin nicht verfuegbar.
