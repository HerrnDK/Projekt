#!/usr/bin/env bash
set -euo pipefail

CLI="./bin/arduino-cli"
if [ ! -x "$CLI" ]; then
  CLI="arduino-cli"
fi

"$CLI" version >/dev/null 2>&1

if ! "$CLI" core list | grep -q "^arduino:avr"; then
  "$CLI" core update-index
  "$CLI" core install arduino:avr
fi

"$CLI" compile --fqbn arduino:avr:mega arduino/mega
