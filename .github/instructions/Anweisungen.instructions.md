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
Ich arbeite hier strikt nach klaren, pruefbaren Regeln, damit Aenderungen sicher, nachvollziehbar und wiederholbar sind.

SCHNELLSTART
------------
- Deutsch schreiben.
- Komponenten ueber `refer: <component_id>` ansprechen.
- In Flows nur erlaubte Felder aendern.
- Nach jeder Aenderung validieren: `jq`, `yaml-lint`.
- Netzwerk-Logik liegt in `nodered/flows/Network.json` (separater Flow-Tab).
- `Network.json` immer zusammen mit `dashboard_flow.json` + `data_exchange_flow.json` deployen.
- Konsistente Formatierung.
- Nicht aendern: `id`, `type`, `z`, `wires`, Geheimdaten.
- Vor jeder Aenderung: zuerst eine kurze Vorschau/Diff der geplanten Aenderungen zeigen und explizit um "Anwenden" oder "Abbrechen" bitten.

TEIL 1: FLOWS
-------------

Erlaubte Felder: `label`, `tooltip`, `payload`, `description`, `name`, `order`, `width`, `height`, `icon`, `className`.
Verboten: `id`, `z`, `type`, `wires`.
Alle CSS/Responsive-Aenderungen gelten global fuer alle Dashboard-Seiten, ausser es wird explizit anders gewuenscht.
Nach jeder Aenderung an `nodered/flows/*.json`: `./deploy_flows.sh` ausfuehren und dabei Node-RED unter `http://192.168.0.250:1880` verwenden, um die Flows automatisch zu aktualisieren.

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

Validierung:
- `jq . nodered/flows/<file>.json`


TEIL 1B: NETWORK.JSON
---------------------
- `Network.json` ist ein separater Flow-Tab (Netzwerkverbindung).
- Abhaengigkeiten: UI-Gruppen/Tabs liegen in `dashboard_flow.json`.
- Immer gemeinsam deployen: `dashboard_flow.json` + `Network.json` + `data_exchange_flow.json`.

TEIL 2: COMPONENTS.YAML
-----------------------
Nutze logische IDs, keine Node-IDs. Aenderungen in components.yaml dokumentieren.
Bei UI/Logik-Change `version` erhoehen.
Validierung:
- `yaml-lint components.yaml`

TEIL 3: MEGA.INO
----------------
Pins im Code muessen zu PINOUT.md passen.
Nach Code-Aenderungen sicherstellen: kompiliert, PINOUT aktualisiert.

TEIL 4: PINOUT.MD
-----------------
PINOUT.md ist Quelle der Wahrheit fuer alle Pins.
Jede Pin-Aenderung muss hier dokumentiert werden.


TEIL 5: DEPLOY_FLOWS.SH
-----------------------
- Script kombiniert alle Flow-Dateien.
- Bei neuen Flow-Dateien: File-Checks und Combine-Logik anpassen.
- Ziel-Endpoint bleibt: `POST /flows`.


TEIL 6: DOKU
------------
- Aenderungen an Flow-Struktur in `DEPLOYMENT_GUIDE.md` und `README.md` nachziehen.

ZUSAMMENFASSUNG
---------------
Ich nutze klare Referenzen, halte Regeln ein und sorge fuer Konsistenz zwischen:
- components.yaml
- nodered/flows/*.json
- arduino/mega/mega.ino
- arduino/mega/PINOUT.md
```
