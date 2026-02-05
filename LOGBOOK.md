# Logbuch

Dieses Logbuch dokumentiert Fortschritte seit dem letzten Eintrag.
Regelungen für Einträge:
- Jeder Eintrag hat einen Zeitstempel als Überschrift (z. B. `YYYY-MM-DD` oder `YYYY-MM-DD HH:MM TZ`).
- Jeder Eintrag enthält die Abschnitte `Erreicht`, `Nächstes Ziel` und `Bestehende Probleme`.
- Inhalte sind kurz, konkret und bezogen auf Änderungen seit dem letzten Eintrag.

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
