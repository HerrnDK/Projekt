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

echo "📤 Flows werden zu Node-RED gesendet..."
echo "   Endpoint: POST $NODE_RED_URL/flows"
echo ""

# Hole aktuelle Flows, um Format/Rev zu erkennen
CURRENT=$(curl -s "$NODE_RED_URL/flows")
USE_REV=0
CURRENT_REV=""

if command -v jq &> /dev/null; then
    if echo "$CURRENT" | jq -e 'type=="object" and has("rev") and has("flows")' >/dev/null 2>&1; then
        USE_REV=1
        CURRENT_REV=$(echo "$CURRENT" | jq -r '.rev')
    fi
else
    if echo "$CURRENT" | grep -q '"rev"' && echo "$CURRENT" | grep -q '"flows"'; then
        USE_REV=1
        CURRENT_REV=$(echo "$CURRENT" | sed -n 's/.*"rev"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p')
    fi
fi

if [[ "$USE_REV" -eq 1 && -n "$CURRENT_REV" ]]; then
    PAYLOAD="{\"flows\":$COMBINED,\"rev\":\"$CURRENT_REV\"}"
else
    PAYLOAD="$COMBINED"
fi

# 5. Prüfe auf Erfolg (HTTP-Code)
HTTP_POST_CODE=$(curl -s -o /dev/null -w "%{http_code}" \
  -X POST \
  -H "Content-Type: application/json" \
  --data-binary "$PAYLOAD" \
  "$NODE_RED_URL/flows")

if [[ "$HTTP_POST_CODE" == "204" || "$HTTP_POST_CODE" == "200" ]]; then
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
