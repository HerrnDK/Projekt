```instructions
---
applyTo: 'nodered/flows/*.json, components.yaml, deploy_flows.sh, DEPLOYMENT_GUIDE.md, README.md'
allowed: ["add_node","update_node","edit_file","create_file","replace_file"]
forbidden: ["add_secrets","remote_execute","exfiltrate"]
formatChecks:
  - command: "jq . nodered/flows/*.json"
    onError: "abort"
  - command: "yaml-lint components.yaml"
    onError: "warning"
deploy: false
---

ZWECK
-----
Klare, pruefbare Regeln fuer sichere, nachvollziehbare Aenderungen.

GRUNDREGELN
-----------
- Deutsch schreiben.
- Komponenten ueber `refer: <component_id>` ansprechen.
- Keine Geheimdaten hinzufuegen.
- Vor jeder Aenderung: kurze Vorschau/Diff zeigen und explizit "Anwenden" oder "Abbrechen" erfragen.
- Konsistente Formatierung.
- Auf der Raspberry Pi fuer Gesamt-Update nur den Befehl `update` verwenden
  (enthaelt `git pull`, Arduino-Upload und Flow-Deploy).
- Berechtigungen der Codex-Laufzeit koennen nicht durch Repo-Dateien erweitert werden;
  zusaetzliche Rechte muessen in der Codex-Session/Umgebung freigegeben werden.

VALIDIERUNG
-----------
- Bei Aenderungen an `nodered/flows/*.json`: `jq . nodered/flows/<file>.json`
- Bei Aenderungen an `components.yaml`: `yaml-lint components.yaml`
- Entspricht den `formatChecks` im Header.

FLOWS (nodered/flows/*.json)
----------------------------

- Erlaubte Felder: `label`, `tooltip`, `payload`, `description`, `name`, `order`, `width`, `height`, `icon`, `className`.
- Verboten: `id`, `z`, `type`, `wires`.
- CSS/Responsive-Aenderungen gelten global fuer alle Dashboard-Seiten, ausser explizit anders gewuenscht.
- Netzwerk-Logik liegt in `nodered/flows/Network.json` (separater Flow-Tab).
- Immer gemeinsam deployen: `dashboard_flow.json` + `Network.json` + `data_exchange_flow.json` + `fn_startup_test_flow.json` + `fn_parameters_flow.json` + `fn_profiles_flow.json`.
- Nach jeder Aenderung an `nodered/flows/*.json`: `./deploy_flows.sh` ausfuehren und Node-RED unter `http://192.168.0.250:1880` verwenden.

Beispiele:
```
ersetze: refer: button_update_sensors
- aendere label zu "Sensoren neu laden"
- aendere tooltip zu "Aktuelles Lesen aller Sensordaten"
- aendere payload zu "READ_ASYNC"
```

```
ersetze: nodered/flows/dashboard_flow.json
- finde Node mit label="Sensoren aktualisieren"
- aendere tooltip zu "Sensor-Lesezyklus starten"
```

COMPONENTS.YAML
---------------
Nutze logische IDs, keine Node-IDs. Aenderungen in components.yaml dokumentieren.
Bei UI/Logik-Change `version` erhoehen.

ARDUINO (arduino/mega)
----------------------
Pins im Code muessen zu PINOUT.md passen.
Nach Code-Aenderungen: Build mit `scripts/arduino_build.sh` und PINOUT aktualisieren.
Sketch-Aufteilung (Arduino Mega):
- `arduino/mega/mega.ino`: Sketch-Root mit `setup()`/`loop()`.
- Modul-Implementierungen liegen in `.cpp` Dateien im gleichen Ordner
  (z. B. `daten.cpp`, `aktoren.cpp`, `sensoren.cpp`, `sensor_<name>.cpp`).
- Funktions-Prototypen liegen zentral in `mega_gemeinsam.h`.
Serielle Regeln:
- `Serial` (RX0/TX0) bleibt fuer USB Debug/Programmierung.
- `Serial1` (RX1/TX1) fuer Node-RED/Raspberry Pi UART.
- Serial1 sendet nur JSON-Zeilen (newline-terminiert), damit `data_exchange_flow.json` stabil bleibt.
Build-Regeln (Arduino CLI):
- `scripts/arduino_build.sh` ist der Standard-Build.
- Das Script prueft/installiert `arduino:avr` automatisch, falls noetig.

PINOUT.MD
---------
PINOUT.md ist Quelle der Wahrheit fuer alle Pins.
Jede Pin-Aenderung muss hier dokumentiert werden.


DEPLOY_FLOWS.SH
---------------
- Script kombiniert alle Flow-Dateien.
- Bei neuen Flow-Dateien: File-Checks und Combine-Logik anpassen.
- Ziel-Endpoint bleibt: `POST /flows`.


DOKU
----
- Aenderungen an Flow-Struktur in `DEPLOYMENT_GUIDE.md` und `README.md` nachziehen.
- Bei Hardware-/Komponenten-Aenderungen `Hardware.md` mitpflegen.

SENSOR-MUSTER (ANALOG)
----------------------
Bei neuen Analogsensoren (z. B. A0/A1) immer dieses Muster nutzen:
1) Arduino
- Pin-Konstante in `mega_gemeinsam.h`.
- Felder in `SensorMomentaufnahme` erweitern (`<sensor>_raw`, `<sensor>_status`).
- Sensorlogik in eigenem `sensor_<name>.cpp` implementieren (Status `ok`, `error_range`, `error_not_connected` falls sinnvoll).
- Sensor in `sensoren.cpp` anbinden (Start + Snapshot).
- JSON in `daten.cpp` fuer `type=sensor`, `type=act`, `type=error` erweitern.
2) Node-RED
- `fn_parameters_flow.json`: Default/Store/Apply fuer `<sensor>_offset_raw` und `<sensor>_display_raw`.
- `dashboard_flow.json`: Status-Anzeige (Wert + Status) Desktop/Mobile und Offset-Slider Desktop/Mobile.
- `fn_startup_test_flow.json`: Validierung fuer neuen Sensor in Anlagenstatus aufnehmen.
- `components.yaml`: neue Komponenten eintragen und `version` erhoehen.
3) Doku
- `arduino/mega/PINOUT.md`, `Hardware.md`, `README.md`, `PROJEKT_ARCHITEKTUR_PLAN.md` aktualisieren.
4) Checks
- `jq . nodered/flows/*.json`
- `yaml-lint components.yaml` (wenn verfuegbar)
- Arduino Build mit `scripts/arduino_build.sh`

KONSISTENZ
----------
Ich halte diese Dateien konsistent:
- `components.yaml`
- `nodered/flows/*.json`
- `arduino/mega/*.ino`
- `arduino/mega/PINOUT.md`
- `Hardware.md`
```

