# Node-RED Leitfaden zur Flow-Bereitstellung

## Uebersicht

Die Flows werden direkt auf der Raspberry Pi aktualisiert.

Empfohlen:
1. `update` im Projektverzeichnis ausfuehren

Alternativ manuell:
1. `git pull` im Projekt auf der Pi
2. `./scripts/arduino_watch_upload.sh --once`
3. `./nodered/flows/deploy_flows.sh`

Hinweis: `Network.json` ist ein separater Flow-Tab und wird immer zusammen mit
`dashboard_flow.json`, `data_exchange_flow.json`, `fn_startup_test_flow.json`,
`fn_parameters_flow.json` und `fn_profiles_flow.json` deployt.

Hardware-Hinweis (UART-Pegel):
- Mega TX1 (5V) -> Pegelwandler 5V->3.3V -> Raspberry Pi RXD (Pflicht)
- Raspberry Pi TXD (3.3V) -> Mega RX1 (direkt moeglich)
- GND zwischen Raspberry Pi und Mega gemeinsam verbinden

## Bereitstellung (Raspberry Pi)

### Schritt 1: SSH zur Raspberry Pi

```bash
ssh pi@192.168.0.250
```

### Schritt 2: Projekt aktualisieren

```bash
cd ~/.node-red
# alternativ: cd /pfad/zum/repo

update
```

### Schritt 3: Manuelle Bereitstellung (optional)

```bash
git pull
./scripts/arduino_watch_upload.sh --once
./nodered/flows/deploy_flows.sh
```

## Nach der Bereitstellung

### 1. Node-RED Editor pruefen

Oeffne `http://192.168.0.250:1880`

Du solltest u. a. Tabs sehen:
- Willkommen
- Profile
- WLAN
- Projekt-info
- Projekt-Parametrierung
- Funktion - Starttest
- Funktion - Parameter
- Funktion - Profile

### 2. Dashboard pruefen

Gehe zu: `http://192.168.0.250:1880/ui`

Projekt-Parametrierung:
- 4 Relais-Buttons (`Relais 1 (Pumpe)`, `Relais 2-4 (Reserve)`) zum Umschalten ON/OFF
- Offset-Slider fuer `HC-SR04`, `Tropfensensor` und `Truebungssensor`

Projekt-info / Status / Sensoren:
- Relais 1..4 Statusanzeige als `ON` / `OFF`
- Sensoranzeigen fuer HC-SR04, Tropfensensor und Truebungssensor inkl. Status
- RFID RC522 Statusanzeige

Dashboard-Tab `Profile`:
- Button `Lesen`
- Button `Profil 1 anlernen/loeschen`
- Button `Profil 2 anlernen/loeschen`
- Es gibt genau 2 UID-Slots: Slot 1 -> `Profil 1`, Slot 2 -> `Profil 2`
- Wenn Slot 1/2 bereits belegt ist, loescht der jeweilige Profil-Button die Bindung
- Anzeigen fuer UID, Lernstatus und aktives Profil

## Fehlerbehandlung

### Problem: "Node-RED nicht erreichbar"

```bash
ps aux | grep node-red
pm2 start node-red
```

### Problem: "Bereitstellung fehlgeschlagen - rev error"

```bash
jq . nodered/flows/dashboard_flow.json > /dev/null
jq . nodered/flows/Network.json > /dev/null
jq . nodered/flows/data_exchange_flow.json > /dev/null
jq . nodered/flows/fn_startup_test_flow.json > /dev/null
jq . nodered/flows/fn_parameters_flow.json > /dev/null
jq . nodered/flows/fn_profiles_flow.json > /dev/null
```

## Datei-Uebersicht

| Datei | Beschreibung |
|-------|-------------|
| `nodered/flows/deploy_flows.sh` | Bash-Skript fuer Raspberry Pi |
| `nodered/flows/dashboard_flow.json` | UI + Dashboard-Tabs |
| `nodered/flows/Network.json` | WLAN-Verbindung, Status, QR-Codes |
| `nodered/flows/data_exchange_flow.json` | Datenaustausch mit Arduino (Serial) |
| `nodered/flows/fn_startup_test_flow.json` | Starttest und Anlagenstatus |
| `nodered/flows/fn_parameters_flow.json` | Parameterverwaltung HC-SR04 + Tropfensensor + Truebungssensor + Relaissteuerung |
| `nodered/flows/fn_profiles_flow.json` | RFID-Profile (Anlernen + Zuordnung) |
| `PROJEKT_ARCHITEKTUR_PLAN.md` | Ablaufplan und Zielarchitektur |
