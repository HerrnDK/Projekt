# 🚀 Node-RED Flows Deployment Guide

## Übersicht

Die Flows werden **direkt auf der Raspberry Pi** aktualisiert:
1. `git pull` im Repo auf der Pi
2. `./nodered/flows/deploy_flows.sh`

**Hinweis:** `Network.json` ist ein separater Flow-Tab und muss immer zusammen mit
`dashboard_flow.json` und `data_exchange_flow.json` deployt werden (das Script erledigt das).

---

## 🎯 Deployment (Raspberry Pi)

### Schritt 1: SSH zur Raspberry Pi

```bash
ssh pi@192.168.0.250
```

### Schritt 2: Repository aktualisieren

```bash
cd ~/Projekt

git pull
```

### Schritt 3: Deployment-Script ausführen

```bash
./nodered/flows/deploy_flows.sh
```

### Beispiel-Output:

```
============================================================
  Node-RED Flow Deployment (Projekt Automatisierung)
============================================================

🔍 Verbindung zu Node-RED wird geprüft auf http://192.168.0.250:1880...
✅ Node-RED erreichbar

📦 Flow-Dateien werden geladen...
✅ dashboard_flow.json geladen
✅ Network.json geladen
✅ data_exchange_flow.json geladen

🔧 Flows werden zusammengefasst...
📤 Flows werden zu Node-RED gesendet...
   Endpoint: POST http://192.168.0.250:1880/flows

✅ ERFOLGREICH DEPLOYED!

============================================================
🎉 Flows sind jetzt in Node-RED aktiv!
============================================================

📊 Dashboard: http://192.168.0.250:1880/ui
📋 Editor:    http://192.168.0.250:1880/
```

---

## ✅ Nach dem Deployment

### 1. Node-RED Editor prüfen

Öffne http://192.168.0.250:1880

Du solltest Tabs sehen:
- **Welcome** - Netzwerk-Status & QR-Code
- **WiFi** - WLAN-Konfiguration
- **Projekt-info** - Sensoren & Steuerung
- **Projekt-Parametrierung** - Actuator-Buttons

Zusätzlich gibt es im **Node-RED Editor** einen eigenen Flow-Tab:
- **Netzwerkverbindung** - WLAN-Verbindung & Checks

### 2. Deploy Button

Falls die Flows nicht automatisch aktiv sind:
- Klick oben rechts auf den grünen "Deploy" Button

### 3. Dashboard Anleitung

Gehe zu: http://192.168.0.250:1880/ui

**Welcome Tab:**
- Zeigt Netzwerk-Status (WLAN verbunden / AP aktiv)
- QR-Code zum schnellen Zugriff

**WiFi Tab:**
- WLAN-Scan durchfuehren
- SSID + Passwort eingeben
- "Verbinden" klicken

**Projekt-info Tab:**
- "Sensoren aktualisieren" Klick → Arduino wird abgefragt
- Zeigt A0, A1, Uptime wenn Arduino verbunden

**Projekt-Parametrierung Tab:**
- Buttons für Pin 22 und Pin 23 (EIN/AUS)
- Reagiert auf Arduino-Befehle

---

## 🔧 Fehlerbehandlung

### Problem: "Node-RED nicht erreichbar"

```bash
# Auf der Pi prüfen:
ps aux | grep node-red

# Falls nicht laufen:
pm2 start node-red
# oder
cd ~/.node-red && node-red
```

### Problem: "Deployment fehlgeschlagen - rev error"

Dies deutet meist auf einen JSON-Parse-Fehler hin:

```bash
jq . nodered/flows/dashboard_flow.json > /dev/null
jq . nodered/flows/data_exchange_flow.json > /dev/null
```

### Problem: "Authentifizierung erforderlich"

Node-RED hat `adminAuth` aktiviert. In diesem Fall:

1. Öffne `~/.node-red/settings.js`
2. Kommentiere die `adminAuth` Section aus oder
3. Verwende den Token/Passwort in der Config

---

## 📝 Files Übersicht

| Datei | Beschreibung |
|-------|-------------|
| `deploy_flows.sh` | Bash-Script fuer Raspberry Pi (direkt dort ausfuehren) |
| `nodered/flows/dashboard_flow.json` | UI/UX + Sensoren/Aktoren (ohne Netzwerk-Logik) |
| `nodered/flows/Network.json` | WLAN-Verbindung, Status, QR-Codes |
| `nodered/flows/data_exchange_flow.json` | Serial-I/O Layer zur Arduino |
