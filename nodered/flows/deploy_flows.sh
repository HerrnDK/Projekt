#!/bin/bash
# Deploy Node-RED flows on Raspberry Pi
# Usage:
#   cd /path/to/repo && bash nodered/flows/deploy_flows.sh
#   NODE_RED_URL=http://192.168.0.250:1880 bash nodered/flows/deploy_flows.sh
#   bash nodered/flows/deploy_flows.sh http://192.168.0.250:1880

set -e

NODE_RED_URL="${NODE_RED_URL:-${1:-http://192.168.0.250:1880}}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FLOWS_DIR="$SCRIPT_DIR"

FLOW_FILES=(
  "dashboard_flow.json"
  "Network.json"
  "data_exchange_flow.json"
  "fn_startup_test_flow.json"
  "fn_parameters_flow.json"
  "fn_profiles_flow.json"
)

echo "============================================================"
echo "  Node-RED Flow Deployment (Projekt Automatisierung)"
echo "============================================================"
echo ""

# 1) Connectivity check
echo "Pruefe Node-RED Verbindung: $NODE_RED_URL"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" --connect-timeout 3 "$NODE_RED_URL/flows" || true)
if [[ "$HTTP_CODE" != "200" && "$HTTP_CODE" != "401" && "$HTTP_CODE" != "403" ]]; then
  echo "FEHLER: Node-RED nicht erreichbar auf $NODE_RED_URL (HTTP $HTTP_CODE)"
  echo "- Ist Node-RED gestartet? (ps aux | grep node-red)"
  echo "- Falls noetig: pm2 start node-red"
  echo "- Oder manuell: cd ~/.node-red && node-red"
  exit 1
fi

echo "Node-RED erreichbar"
echo ""

# 2) Validate required files
echo "Pruefe Flow-Dateien..."
for flow_file in "${FLOW_FILES[@]}"; do
  if [[ ! -f "$FLOWS_DIR/$flow_file" ]]; then
    echo "FEHLER: $flow_file nicht gefunden in $FLOWS_DIR"
    exit 1
  fi
  echo "OK: $flow_file"
done
echo ""

# 3) Combine flow files
echo "Kombiniere Flows..."
if command -v jq >/dev/null 2>&1; then
  COMBINED=$(jq -s 'add' \
    "$FLOWS_DIR/dashboard_flow.json" \
    "$FLOWS_DIR/Network.json" \
    "$FLOWS_DIR/data_exchange_flow.json" \
    "$FLOWS_DIR/fn_startup_test_flow.json" \
    "$FLOWS_DIR/fn_parameters_flow.json" \
    "$FLOWS_DIR/fn_profiles_flow.json")
else
  COMBINED=$(FLOWS_DIR="$FLOWS_DIR" python3 << 'PYTHON_EOF'
import json
import os
import sys
from pathlib import Path

try:
    p = Path(os.environ['FLOWS_DIR'])
    with open(p / 'dashboard_flow.json', encoding='utf-8') as f:
        dashboard = json.load(f)
    with open(p / 'Network.json', encoding='utf-8') as f:
        network = json.load(f)
    with open(p / 'data_exchange_flow.json', encoding='utf-8') as f:
        data_exchange = json.load(f)
    with open(p / 'fn_startup_test_flow.json', encoding='utf-8') as f:
        startup_test = json.load(f)
    with open(p / 'fn_parameters_flow.json', encoding='utf-8') as f:
        parameters = json.load(f)
    with open(p / 'fn_profiles_flow.json', encoding='utf-8') as f:
        profiles = json.load(f)

    combined = dashboard + network + data_exchange + startup_test + parameters + profiles
    print(json.dumps(combined, ensure_ascii=False))
except Exception as e:
    print(f"Error: {e}", file=sys.stderr)
    sys.exit(1)
PYTHON_EOF
)
fi

# 4) Build payload with optional rev
CURRENT=$(curl -s "$NODE_RED_URL/flows")
USE_REV=0
CURRENT_REV=""

if command -v jq >/dev/null 2>&1; then
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

# 5) Deploy
HTTP_POST_CODE=$(curl -s -o /dev/null -w "%{http_code}" \
  -X POST \
  -H "Content-Type: application/json" \
  --data-binary "$PAYLOAD" \
  "$NODE_RED_URL/flows")

if [[ "$HTTP_POST_CODE" == "204" || "$HTTP_POST_CODE" == "200" ]]; then
  echo ""
  echo "ERFOLGREICH DEPLOYED"
  echo "Dashboard: $NODE_RED_URL/ui"
  echo "Editor:    $NODE_RED_URL/"
  echo ""
  echo "Tabs: Welcome, Profile, WiFi, Projekt-info, Projekt-Parametrierung"
  exit 0
else
  echo ""
  echo "DEPLOYMENT FEHLGESCHLAGEN (HTTP $HTTP_POST_CODE)"
  echo "Moegliche Ursachen:"
  echo "- Node-RED laeuft nicht"
  echo "- adminAuth ist aktiviert"
  echo "- URL/Netzwerkproblem"
  exit 1
fi
