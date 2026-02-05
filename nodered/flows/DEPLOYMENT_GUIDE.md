# 🚀 Node-RED Flows Deployment Guide

## Übersicht

Es gibt zwei Methoden, um die Flows zur Raspberry Pi zu deployen:

### **Methode 1: Bash-Script auf der Raspberry Pi (EMPFOHLEN)**
- ✅ Einfachste, schnellste Methode
- ✅ Direkt auf der Pi, keine Netzwerk-Komplexität
- ✅ Vollständige Kontrolle und Transparenz

### **Methode 2: Python-Script vom Entwicklungs-PC**
- ⚠️ Benötigt direkte Netzwerk-Verbindung zur Pi
- ⚠️ Könnte durch Firewall blockiert werden

---


**Hinweis:** `Network.json` ist ein Flow-Fragment (ohne UI-Tab/Group-Definitionen) und muss **immer zusammen** mit `dashboard_flow.json` deployt werden.

## 🎯 Methode 1: Bash Deployment (Raspberry Pi)

### Schritt 1: Flows-Verzeichnis zur Pi kopieren (falls noch nicht da)

```bash
# Vom Develop-PC
scp -r nodered/flows/ pi@192.168.0.250:~/Projekt/nodered/

# oder: git clone/pull machen
```

### Schritt 2: SSH zur Raspberry Pi

```bash
ssh pi@192.168.0.250
```

### Schritt 3: Deployment-Script ausführen

```bash
cd ~/Projekt
bash deploy_flows.sh
```

### Beispiel-Output:

```
============================================================
  Node-RED Flow Deployment (Projekt Automatisierung)
============================================================

🔍 Verbindung zu Node-RED wird geprüft auf http://localhost:1880...
✅ Node-RED erreichbar

📦 Flow-Dateien werden geladen...
✅ dashboard_flow.json geladen
✅ Network.json geladen
✅ data_exchange_flow.json geladen

🔧 Flows werden zusammengefasst...
📤 Flows werden zu Node-RED gesendet...
   Endpoint: POST http://localhost:1880/flows

✅ ERFOLGREICH DEPLOYED!

============================================================
🎉 Flows sind jetzt in Node-RED aktiv!
============================================================

📊 Dashboard: http://localhost:1880/ui
📋 Editor:    http://localhost:1880/

💡 Nächste Schritte:
   1. Öffnen Sie http://localhost:1880 im Browser
   2. Sie sollten neue Tabs sehen...
   3. Top-rechts auf 'Deploy' klicken
   4. Arduino Mega an /dev/serial0 anschließen
```

---

## 🐍 Methode 2: Python Deployment (vom PC)

> ⚠️ Funktioniert nur, wenn die Raspberry Pi im Netzwerk erreichbar ist

### Schritt 1: Python-Dependencies installieren (einmalig)

```bash
pip install requests
```

### Schritt 2: Script ausführen

```bash
cd /workspaces/Projekt
python3 deploy_flows.py
```

oder mit custom URL:

```bash
NODE_RED_IP=192.168.0.250 python3 deploy_flows.py
```

---

## ✅ Nach dem Deployment

### 1. Node-RED Editor prüfen

Öffne http://192.168.0.250:1880 (oder http://localhost:1880 auf der Pi)

Du solltest **5 neue Tabs** im Dashboard sehen:
- **Welcome** - Netzwerk-Status & QR-Code
- **WiFi** - WLAN-Konfiguration
- **Projekt-info** - Sensoren & Steuerung
- **Projekt-Parametrierung** - Actuator-Buttons
- **Projekt-Datenlog** - Datenlogger (placeholder)

Zusätzlich siehst du im **Node-RED Editor** einen eigenen Flow-Tab:
- **Netzwerkverbindung** - WLAN-Verbindung & Checks

### 2. Deploy Button

Falls die Flows nicht automatisch active sind:
- Klick **oben rechts** auf den grünen "Deploy" Button

### 3. Dashboard Anleitung

Gehe zu: http://192.168.0.250:1880/ui

**Welcome Tab:**
- Zeigt Netzwerk-Status (WLAN verbunden / AP aktiv)
- QR-Code zum schnellen Zugriff

**WiFi Tab:**
- WLAN-Scan durchführen
- SSID + Passwort eingeben
- "Verbinden" klicken

**Projekt-info Tab:**
- "Sensoren aktualisieren" Klick → Arduino wird abgefragt
- Zeigt A0, A1, Uptime wenn Arduino verbunden

**Projekt-Parametrierung Tab:**
- Buttons für Pin 22 und Pin 23 (EIN/AUS)
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
# Flow-Dateien prüfen
jq . nodered/flows/dashboard_flow.json > /dev/null
jq . nodered/flows/data_exchange_flow.json > /dev/null
```

### Problem: "Authentifizierung erforderlich"

Node-RED hat `adminAuth` aktiviert. In diesem Fall:

1. Öffne `~/.node-red/settings.js`
2. Kommentiere die `adminAuth` Section aus oder
3. Verwende den Token/Passwort in der Config

---

## 📊 Responsive Design Features

Das Dashboard passt sich automatisch an:

- **📱 Smartphone (< 600px):** 60px große Buttons, vollständige Breite
- **🖥️ Desktop 800x480:** Optimierte Layouts, 44px Buttons
- **🔒 Touch-Geräte:** Mindestens 44x44 Pixel Touchflächen
- **🔄 Landscape/Portrait:** Automatische Anpassung

---

## 📝 Files Übersicht

| Datei | Beschreibung |
|-------|-------------|
| `deploy_flows.sh` | Bash-Script für Raspberry Pi (direkt dort ausführen) |
| `deploy_flows.py` | Python-Script für Remote-Deployment |
| `nodered/flows/dashboard_flow.json` | UI/UX + Sensoren/Aktoren (ohne Netzwerk-Logik) |
| `nodered/flows/Network.json` | WLAN-Verbindung, Status, QR-Codes |
| `nodered/flows/data_exchange_flow.json` | Serial-I/O Layer zur Arduino |

---

## 🎓 Nächste Schritte

1. ✅ Flows deployen (dieses Guide)
2. ⏳ Arduino an `/dev/serial0` anschließen
3. ⏳ Dashboard testen ("Sensoren aktualisieren" klicken)
4. ⏳ Automation Flows hinzufügen (pro Funktion ein Flow)
5. ⏳ Datenlogging implementieren

---

**Fragen?** → Siehe die Node-RED Logs:

```bash
# Live logs auf der Pi:
pm2 logs node-red
```
