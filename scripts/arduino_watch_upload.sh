#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SKETCH_DIR="${SKETCH_DIR:-$REPO_ROOT/arduino/mega}"
FQBN="${FQBN:-arduino:avr:mega}"
ARDUINO_PORT="${ARDUINO_PORT:-}"
DEBOUNCE_SECONDS="${DEBOUNCE_SECONDS:-2}"
POLL_SECONDS="${POLL_SECONDS:-2}"

FORCE_POLL=0
RUN_ON_START=1
RUN_ONCE=0
ARDUINO_CLI_BIN=""

log() {
  printf '[%s] %s\n' "$(date +'%H:%M:%S')" "$*"
}

usage() {
  cat <<'EOF'
Verwendung:
  ./scripts/arduino_watch_upload.sh [optionen]

Optionen:
  --port <device>         USB-Port setzen (z.B. /dev/ttyACM0)
  --fqbn <fqbn>           Board FQBN (Default: arduino:avr:mega)
  --debounce <sekunden>   Debounce fuer Event-Flut (Default: 2)
  --poll-seconds <sek>    Polling-Intervall im Fallback (Default: 2)
  --poll                  Polling erzwingen (ohne inotifywait)
  --no-initial            Keinen initialen Build/Upload beim Start
  --once                  Genau ein Build/Upload und dann Ende
  -h, --help              Hilfe anzeigen

Umgebungsvariablen:
  ARDUINO_PORT, FQBN, ARDUINO_CLI, SKETCH_DIR, DEBOUNCE_SECONDS, POLL_SECONDS
EOF
}

resolve_cli() {
  local cli_candidate
  cli_candidate="${ARDUINO_CLI:-$REPO_ROOT/bin/arduino-cli}"

  if [[ -x "$cli_candidate" ]]; then
    ARDUINO_CLI_BIN="$cli_candidate"
    return 0
  fi

  if command -v arduino-cli >/dev/null 2>&1; then
    ARDUINO_CLI_BIN="$(command -v arduino-cli)"
    return 0
  fi

  if ! command -v curl >/dev/null 2>&1; then
    log "FEHLER: arduino-cli nicht gefunden und curl ist nicht installiert."
    log "Installiere curl oder setze ARDUINO_CLI explizit."
    exit 1
  fi

  log "arduino-cli nicht gefunden. Installiere lokal nach $REPO_ROOT/bin ..."
  if ! (cd "$REPO_ROOT" && curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh); then
    log "FEHLER: arduino-cli Installation fehlgeschlagen."
    exit 1
  fi

  if [[ -x "$REPO_ROOT/bin/arduino-cli" ]]; then
    ARDUINO_CLI_BIN="$REPO_ROOT/bin/arduino-cli"
    return 0
  fi

  log "FEHLER: arduino-cli weiterhin nicht verfuegbar."
  exit 1
}

detect_port() {
  if [[ -n "$ARDUINO_PORT" ]]; then
    return 0
  fi

  local detected=""
  detected="$("$ARDUINO_CLI_BIN" board list 2>/dev/null | awk -v fqbn="$FQBN" '$0 ~ fqbn {print $1; exit}')"

  if [[ -z "$detected" ]]; then
    detected="$(ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | head -n 1 || true)"
  fi

  if [[ -z "$detected" ]]; then
    log "FEHLER: Kein Arduino-USB-Port erkannt."
    log "Setze ARDUINO_PORT explizit, z.B. ARDUINO_PORT=/dev/ttyACM0"
    "$ARDUINO_CLI_BIN" board list || true
    exit 1
  fi

  ARDUINO_PORT="$detected"
  log "Erkannter Port: $ARDUINO_PORT"
}

build_and_upload() {
  log "Kompiliere Sketch..."
  (cd "$REPO_ROOT" && ./scripts/arduino_build.sh)

  detect_port
  log "Upload auf $ARDUINO_PORT ..."
  "$ARDUINO_CLI_BIN" upload --fqbn "$FQBN" -p "$ARDUINO_PORT" "$SKETCH_DIR"
  log "Upload erfolgreich."
}

is_watched_file() {
  case "$1" in
    *.ino|*.cpp|*.c|*.h|*.hpp) return 0 ;;
    *) return 1 ;;
  esac
}

files_fingerprint() {
  find "$SKETCH_DIR" -type f \
    \( -name '*.ino' -o -name '*.cpp' -o -name '*.c' -o -name '*.h' -o -name '*.hpp' \) \
    -printf '%p %T@\n' | sort | sha1sum | awk '{print $1}'
}

watch_with_polling() {
  local last_sig cur_sig
  last_sig="$(files_fingerprint)"
  log "Watch aktiv (Polling alle ${POLL_SECONDS}s)."

  while true; do
    sleep "$POLL_SECONDS"
    cur_sig="$(files_fingerprint)"
    if [[ "$cur_sig" != "$last_sig" ]]; then
      last_sig="$cur_sig"
      log "Aenderung erkannt (Polling)."
      if ! build_and_upload; then
        log "Build/Upload fehlgeschlagen. Warte auf naechste Aenderung."
      fi
    fi
  done
}

watch_with_inotify() {
  if ! command -v inotifywait >/dev/null 2>&1; then
    return 1
  fi

  local last_run=0
  log "Watch aktiv (inotifywait)."

  inotifywait -m -r -e close_write,create,move,delete --format '%w%f' "$SKETCH_DIR" | while read -r changed; do
    is_watched_file "$changed" || continue
    local now
    now="$(date +%s)"
    if (( now - last_run < DEBOUNCE_SECONDS )); then
      continue
    fi
    last_run=$now
    log "Aenderung erkannt: $changed"
    if ! build_and_upload; then
      log "Build/Upload fehlgeschlagen. Warte auf naechste Aenderung."
    fi
  done
}

parse_args() {
  require_value() {
    local option_name="$1"
    local option_value="${2:-}"
    if [[ -z "$option_value" || "$option_value" == --* ]]; then
      log "FEHLER: $option_name erwartet einen Wert."
      usage
      exit 1
    fi
  }

  while [[ $# -gt 0 ]]; do
    case "$1" in
      --port)
        require_value "$1" "${2:-}"
        ARDUINO_PORT="$2"
        shift 2
        ;;
      --fqbn)
        require_value "$1" "${2:-}"
        FQBN="$2"
        shift 2
        ;;
      --debounce)
        require_value "$1" "${2:-}"
        DEBOUNCE_SECONDS="$2"
        shift 2
        ;;
      --poll-seconds)
        require_value "$1" "${2:-}"
        POLL_SECONDS="$2"
        shift 2
        ;;
      --poll)
        FORCE_POLL=1
        shift
        ;;
      --no-initial)
        RUN_ON_START=0
        shift
        ;;
      --once)
        RUN_ONCE=1
        shift
        ;;
      -h|--help)
        usage
        exit 0
        ;;
      *)
        log "Unbekannte Option: $1"
        usage
        exit 1
        ;;
    esac
  done
}

watch_and_upload() {
  parse_args "$@"
  resolve_cli

  if [[ ! -d "$SKETCH_DIR" ]]; then
    log "FEHLER: Sketch-Verzeichnis nicht gefunden: $SKETCH_DIR"
    exit 1
  fi

  log "Sketch: $SKETCH_DIR"
  log "Board:  $FQBN"
  if [[ -n "$ARDUINO_PORT" ]]; then
    log "Port:   $ARDUINO_PORT (vorgegeben)"
  else
    log "Port:   auto"
  fi

  if (( RUN_ON_START == 1 )); then
    build_and_upload
  fi

  if (( RUN_ONCE == 1 )); then
    return 0
  fi

  if (( FORCE_POLL == 1 )); then
    watch_with_polling
    return 0
  fi

  if ! watch_with_inotify; then
    log "Hinweis: inotifywait nicht gefunden. Fallback auf Polling."
    watch_with_polling
  fi
}

watch_and_upload "$@"
