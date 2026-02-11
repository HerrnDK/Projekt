#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CLI=""

resolve_cli() {
  if [[ -n "${ARDUINO_CLI:-}" && -x "${ARDUINO_CLI}" ]]; then
    CLI="${ARDUINO_CLI}"
    return 0
  fi

  if [[ -x "$REPO_ROOT/bin/arduino-cli" ]]; then
    CLI="$REPO_ROOT/bin/arduino-cli"
    return 0
  fi

  if command -v arduino-cli >/dev/null 2>&1; then
    CLI="$(command -v arduino-cli)"
    return 0
  fi

  if ! command -v curl >/dev/null 2>&1; then
    echo "FEHLER: arduino-cli fehlt und curl ist nicht installiert." >&2
    echo "Installiere curl oder setze ARDUINO_CLI auf ein vorhandenes Binary." >&2
    exit 1
  fi

  echo "arduino-cli nicht gefunden. Installiere lokal nach $REPO_ROOT/bin ..."
  (cd "$REPO_ROOT" && curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh)

  if [[ ! -x "$REPO_ROOT/bin/arduino-cli" ]]; then
    echo "FEHLER: lokale arduino-cli Installation fehlgeschlagen." >&2
    exit 1
  fi

  CLI="$REPO_ROOT/bin/arduino-cli"
}

resolve_cli
"$CLI" version >/dev/null 2>&1

if ! "$CLI" core list | grep -q "^arduino:avr"; then
  "$CLI" core update-index
  "$CLI" core install arduino:avr
fi

"$CLI" compile --fqbn arduino:avr:mega "$REPO_ROOT/arduino/mega"
