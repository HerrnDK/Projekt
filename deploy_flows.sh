#!/bin/bash
# Deploy Node-RED Flows via curl on Raspberry Pi
# Verwendung:
#   cd /pfad/zu/Projekt && bash deploy_flows.sh
#   NODE_RED_URL=http://192.168.0.250:1880 bash deploy_flows.sh
#   bash deploy_flows.sh http://192.168.0.250:1880

set -e

NODE_RED_URL="${NODE_RED_URL:-${1:-http://192.168.0.250:1880}}"
FLOWS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/nodered/flows"

echo "============================================================"
echo "  Node-RED Flow Deployment (Projekt Automatisierung)"
echo "============================================================"
echo ""

# 1. Verbindung testen (Admin API)
echo "🔍 Verbindung zu Node-RED wird geprüft auf $NODE_RED_URL..."
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" --connect-timeout 3 "$NODE_RED_URL/flows" || true)
if [[ "$HTTP_CODE" != "200" && "$HTTP_CODE" != "401" && "$HTTP_CODE" != "403" ]]; then
    echo "❌ FEHLER: Node-RED nicht erreichbar auf $NODE_RED_URL (HTTP $HTTP_CODE)"
    echo ""
    echo "   Hilfe:"
    echo "   - Ist Node-RED gestartet? (ps aux | grep node-red)"
    echo "   - Falls nötig: pm2 start node-red"
    echo "   - Oder manuell: cd ~/.node-red && node-red"
    echo ""
    exit 1
fi

echo "✅ Node-RED erreichbar"
echo ""

# 2. Flows laden
echo "📦 Flow-Dateien werden geladen..."

if [[ ! -f "$FLOWS_DIR/dashboard_flow.json" ]]; then
    echo "❌ FEHLER: dashboard_flow.json nicht gefunden in $FLOWS_DIR"
    exit 1
fi

if [[ ! -f "$FLOWS_DIR/data_exchange_flow.json" ]]; then
    echo "❌ FEHLER: data_exchange_flow.json nicht gefunden in $FLOWS_DIR"
    exit 1
fi

echo "✅ dashboard_flow.json geladen"
echo "✅ data_exchange_flow.json geladen"
echo ""

# 3. Flows zusammenfassen mit jq oder Python
echo "🔧 Flows werden zusammengefasst..."

if command -v jq &> /dev/null; then
    # Verwende jq
    COMBINED=$(jq -s 'add' "$FLOWS_DIR/dashboard_flow.json" "$FLOWS_DIR/data_exchange_flow.json")
else
    # Fallback: Python
    COMBINED=$(python3 << 'PYTHON_EOF'
import json
import sys
from pathlib import Path

try:
    p = Path('nodered/flows')
    with open(p / 'dashboard_flow.json') as f:
        dashboard = json.load(f)
    with open(p / 'data_exchange_flow.json') as f:
        data_exchange = json.load(f)
    combined = dashboard + data_exchange
    print(json.dumps(combined, ensure_ascii=False))
except Exception as e:
    print(f"Error: {e}", file=sys.stderr)
    sys.exit(1)
PYTHON_EOF
)
fi

# 4. Zu Node-RED deployen
echo "📤 Flows werden zu Node-RED gesendet..."
echo "   Endpoint: POST $NODE_RED_URL/flows"
echo ""

RESPONSE=$(curl -s -X POST \
  -H "Content-Type: application/json" \
  -d "{\"flows\":$COMBINED,\"rev\":1}" \
  "$NODE_RED_URL/flows")

# 5. Prüfe auf Erfolg
if echo "$RESPONSE" | grep -q '"rev"'; then
    echo "✅ ERFOLGREICH DEPLOYED!"
    echo ""
    echo "============================================================"
    echo "🎉 Flows sind jetzt in Node-RED aktiv!"
    echo "============================================================"
    echo ""
    echo "📊 Dashboard: $NODE_RED_URL/ui"
    echo "📋 Editor:    $NODE_RED_URL/"
    echo ""
    echo "💡 Nächste Schritte:"
    echo "   1. Öffnen Sie $NODE_RED_URL im Browser"
    echo "   2. Sie sollten neue Tabs sehen:"
    echo "      - Welcome (Netzwerk-Status & QR-Code)"
    echo "      - WiFi (WLAN-Einrichtung)"
    echo "      - Projekt-info (Sensoren & Steuerung)"
    echo "      - Projekt-Parametrierung (Actuator-Buttons)"
    echo "   3. Top-rechts auf 'Deploy' klicken (falls gefordert)"
    echo "   4. Arduino Mega an /dev/serial0 anschließen"
    echo "   5. 'Sensoren aktualisieren' klicken"
    echo ""
    exit 0
else
    echo "❌ DEPLOYMENT FEHLGESCHLAGEN!"
    echo ""
    echo "Server Response:"
    echo "$RESPONSE"
    echo ""
    echo "Mögliche Ursachen:"
    echo "- Node-RED läuft nicht (pm2 status node-red)"
    echo "- Authentifizierung ist aktiviert (adminAuth in ~/.node-red/settings.js)"
    echo "- Network-Fehler oder falsche URL"
    echo ""
    exit 1
fi
