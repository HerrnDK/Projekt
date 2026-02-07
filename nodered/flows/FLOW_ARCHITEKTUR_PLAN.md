# Node-RED Ablaufplan: Ein Flow pro Funktion

## Ziel
- Pro Fachfunktion ein eigener Node-RED-Flow (eigener Tab).
- Gemeinsame Kommunikationsschicht bleibt zentral in `data_exchange_flow.json`.
- Funktionsflows senden/empfangen nur ueber Link-Nodes zur Kommunikationsschicht.

## Aktuelle Basis (Stand im Repo)
- Kommunikationsschicht: `nodered/flows/data_exchange_flow.json`
- UI/Fachlogik (gemischt): `nodered/flows/dashboard_flow.json`
- Netzwerklogik: `nodered/flows/Network.json`

## Zielarchitektur (Soll)
- `nodered/flows/data_exchange_flow.json` (Transport, JSON-Parse, Routing)
- `nodered/flows/fn_sensoren_flow.json` (READ ausloesen, Sensorwerte nutzen)
- `nodered/flows/fn_aktoren_flow.json` (ACT-Kommandos aufbauen, Ack auswerten)
- `nodered/flows/fn_<weitere_funktion>.json` (je Funktion ein Tab)
- `nodered/flows/Network.json` (separat, unveraendert als Netzwerk-Flow)

## Graph 1: Laufzeit-Kommunikation
```mermaid
flowchart LR
  UI[Dashboard UI] --> FN[Funktionsflow]
  FN --> LOUT[Link Out: to data_exchange]
  LOUT --> TX[Serial Out /dev/serial0]
  TX --> ARD[Arduino Serial1]
  ARD --> RX[Serial In /dev/serial0]
  RX --> JP[JSON Parse]
  JP --> ROUTE[Response Router]
  ROUTE --> LIN[Link In: from data_exchange]
  LIN --> FN
  FN --> UI
```

## Graph 2: Umsetzungsfahrplan
```mermaid
graph TD
  S1[1. Funktionskatalog festlegen] --> S2[2. Pro Funktion eigenen Flow-Tab anlegen]
  S2 --> S3[3. Link-Nodes zur Kommunikationsschicht setzen]
  S3 --> S4[4. Response-Routing in data_exchange ergaenzen]
  S4 --> S5[5. Komponentenregister in components.yaml pflegen]
  S5 --> S6[6. JSON-Checks und Deploy]
  S6 --> S7[7. End-to-End Test je Funktion]
```

## Template pro Funktion (Copy-Paste fuer neue Funktionen)
- Eingangs-Link: `link in` von `data_exchange_flow.json`
- Ausgangs-Link: `link out` nach `data_exchange_flow.json`
- Command-Builder: erzeugt `msg.payload` als Serial-Kommando (`READ`, `ACT,<pin>,<state>`, ...)
- Response-Filter: verarbeitet nur passende Antworten (`payload.type`, optional `payload.pin`)
- UI/Logik-Nodes: Anzeige/Steuerung fuer genau eine Funktion

## Reihenfolge fuer die naechsten Sprints
1. `fn_sensoren_flow.json` aus `READ`-Pfad herausloesen
2. `fn_aktoren_flow.json` aus `ACT`-Pfad herausloesen
3. Weitere Funktionen nach gleichem Muster (`fn_<name>_flow.json`)
4. `dashboard_flow.json` auf reine UI-Orchestrierung reduzieren

## Referenzen auf bestehende Komponenten
- `refer: button_update_sensors`
- `refer: serial_out_arduino`

## Definition of Done je Funktion
- Eigener Flow-Tab vorhanden
- Link zur Kommunikationsschicht vorhanden (hin und zurueck)
- End-to-End Test erfolgreich
- In `components.yaml` dokumentiert (inkl. `version`-Pflege)
- Deploy ueber `./nodered/flows/deploy_flows.sh` erfolgreich
