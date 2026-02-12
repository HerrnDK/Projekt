# Node-RED Flows Deployment Guide

## Uebersicht

Die Flows werden direkt auf der Raspberry Pi aktualisiert:
1. `git pull` im Repo auf der Pi
2. `./nodered/flows/deploy_flows.sh`

Hinweis: `Network.json` ist ein separater Flow-Tab und wird immer zusammen mit
`dashboard_flow.json`, `data_exchange_flow.json`, `fn_startup_test_flow.json`,
`fn_parameters_flow.json` und `fn_profiles_flow.json` deployt.

Hardware-Hinweis (UART-Pegel):
- Mega TX1 (5V) -> Pegelwandler 5V->3.3V -> Raspberry Pi RXD (Pflicht)
- Raspberry Pi TXD (3.3V) -> Mega RX1 (direkt moeglich)
- GND zwischen Raspberry Pi und Mega gemeinsam verbinden

## Deployment (Raspberry Pi)

### Schritt 1: SSH zur Raspberry Pi

```bash
ssh pi@192.168.0.250
```

### Schritt 2: Repository aktualisieren

```bash
cd ~/.node-red
# alternativ: cd /pfad/zum/repo

git pull
```

### Schritt 3: Deployment-Script ausfuehren

```bash
./nodered/flows/deploy_flows.sh
```

## Nach dem Deployment

### 1. Node-RED Editor pruefen

Oeffne `http://192.168.0.250:1880`

Du solltest Tabs sehen:
- Welcome
- Profile
- WiFi
- Projekt-info
- Projekt-Parametrierung
- Funktion - Startup Test
- Funktion - Parameter
- Funktion - Profile

### 2. Dashboard pruefen

Gehe zu: `http://192.168.0.250:1880/ui`

Profile Tab:
- Button `Chip 1 anlernen`
- Button `Chip 2 anlernen`
- Je Chip Zuordnung zu `Profil 1` oder `Profil 2`
- Anzeigen fuer UID, Lernstatus und aktives Profil

## Fehlerbehandlung

### Problem: "Node-RED nicht erreichbar"

```bash
ps aux | grep node-red
pm2 start node-red
```

### Problem: "Deployment fehlgeschlagen - rev error"

```bash
jq . nodered/flows/dashboard_flow.json > /dev/null
jq . nodered/flows/Network.json > /dev/null
jq . nodered/flows/data_exchange_flow.json > /dev/null
jq . nodered/flows/fn_startup_test_flow.json > /dev/null
jq . nodered/flows/fn_parameters_flow.json > /dev/null
jq . nodered/flows/fn_profiles_flow.json > /dev/null
```

## Files Uebersicht

| Datei | Beschreibung |
|-------|-------------|
| `nodered/flows/deploy_flows.sh` | Bash-Script fuer Raspberry Pi |
| `nodered/flows/dashboard_flow.json` | UI + Dashboard Tabs |
| `nodered/flows/Network.json` | WLAN-Verbindung, Status, QR-Codes |
| `nodered/flows/data_exchange_flow.json` | Serial-I/O Layer zur Arduino |
| `nodered/flows/fn_startup_test_flow.json` | Startup-Test und Anlagenstatus |
| `nodered/flows/fn_parameters_flow.json` | Parameterverwaltung HC-SR04 |
| `nodered/flows/fn_profiles_flow.json` | RFID Profile (Anlernen + Zuordnung) |
| `nodered/flows/FLOW_ARCHITEKTUR_PLAN.md` | Ablaufplan und Zielarchitektur |
