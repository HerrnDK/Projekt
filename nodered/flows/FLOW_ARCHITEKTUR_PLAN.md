# Projekt UML und Ablaufdiagramme

## Ziel
- Komplettsicht auf Architektur und Laufzeit fuer Arduino, Raspberry Pi und Node-RED.
- Einheitliche Grundlage fuer weitere Funktions-Module (`funktion_<name>` / `fn_<name>_flow.json`).

## 1) Systemarchitektur (Komponentenblick)
```mermaid
flowchart TB
  User[User] --> Browser[Touch Browser]
  Browser --> Dashboard[Node-RED Dashboard\nnodered/flows/dashboard_flow.json]

  subgraph RPi[Raspberry Pi]
    Dashboard --> DataFlow[data_exchange_flow.json\nSerial In/Out + JSON Parse]
    Dashboard --> NetFlow[Network.json\nWLAN / Status / QR]
    Dashboard --> FnFlow[fn_startup_test_flow.json\n+ weitere fn_*_flow.json]
  end

  DataFlow --> SerialPort[/dev/serial0 UART]
  SerialPort --> LevelShift[Pegelwandler 5V -> 3.3V]
  LevelShift --> Mega[Arduino Mega 2560 Serial1\nTX1=18 RX1=19]

  subgraph Arduino[Arduino Sketch arduino/mega]
    Mega --> MainCpp[main.cpp\nsetup() / loop()]
    MainCpp --> DataCpp[data.cpp\nREAD / ACT Protokoll]
    MainCpp --> SensorsCpp[funktion_sensors.cpp]
    MainCpp --> ActCpp[funktion_actuators.cpp]
    DataCpp --> Shared[mega_shared.h/.cpp]
    SensorsCpp --> Shared
    ActCpp --> Shared
  end

  SensorsCpp --> Inputs[Sensoren A0/A1 + spaeter weitere]
  ActCpp --> Outputs[Aktoren Pins 22-25 + spaeter weitere]
```

## 2) UML Klassendiagramm (Arduino-Software)
```mermaid
classDiagram
  class SensorSnapshot {
    +int a0
    +int a1
    +unsigned long uptime_ms
  }

  class MainModule {
    +setup()
    +loop()
  }

  class DataModule {
    +Data_begin()
    +Data_tick()
    +Data_sendSensorSnapshot()
  }

  class SensorsModule {
    +Sensors_begin()
    +Sensors_readSnapshot(SensorSnapshot& out)
  }

  class ActuatorsModule {
    +Actuators_begin()
    +Actuators_set(uint8_t pin, bool state) bool
  }

  class SharedConfig {
    +DEBUG_BAUD
    +DATA_BAUD
    +ACTUATOR_PINS[]
    +ACTUATOR_COUNT
    +DEBUG_PORT
    +DATA_PORT
  }

  MainModule --> DataModule : ruft auf
  MainModule --> SensorsModule : initialisiert
  MainModule --> ActuatorsModule : initialisiert
  DataModule --> SensorsModule : liest Snapshot
  DataModule --> ActuatorsModule : setzt Pins
  DataModule --> SensorSnapshot : serialisiert JSON
  DataModule --> SharedConfig
  SensorsModule --> SharedConfig
  ActuatorsModule --> SharedConfig
```

## 3) Sequenzdiagramm (READ und ACT)
```mermaid
sequenceDiagram
  participant U as User
  participant D as Dashboard
  participant X as data_exchange_flow
  participant A as Arduino data.cpp
  participant S as Sensors
  participant C as Actuators

  U->>D: Klick "Sensoren aktualisieren"
  D->>X: payload "READ"
  X->>A: Serial1: READ
  A->>S: Sensors_readSnapshot()
  S-->>A: SensorSnapshot
  A-->>X: {"type":"sensor",...}
  X-->>D: msg.payload (JSON)
  D-->>U: Anzeige A0/A1/Uptime

  U->>D: Klick "Pin 22 EIN"
  D->>X: payload "ACT,22,1"
  X->>A: Serial1: ACT,22,1
  A->>C: Actuators_set(22,true)
  C-->>A: ok=true
  A-->>X: {"type":"act","ok":1,...}
  X-->>D: msg.payload (ACK)
  D-->>U: Rueckmeldung/Status
```

## 4) Ablaufdiagramm (Entwicklung bis Betrieb)
```mermaid
flowchart TD
  Start[Code Aenderung im Repo] --> Build[./scripts/arduino_build.sh]
  Build -->|ok| Upload[Arduino IDE Upload oder arduino-cli upload]
  Build -->|fehler| Fix[Code korrigieren]
  Fix --> Build

  Upload --> FlowEdit[Node-RED Flows anpassen]
  FlowEdit --> Pull[git pull auf Raspberry Pi]
  Pull --> Deploy[./nodered/flows/deploy_flows.sh]
  Deploy --> E2E[End-to-End Test READ/ACT]
  E2E -->|ok| Run[Betrieb]
  E2E -->|fehler| Fix
```

## Struktur je Funktion (Standardmuster)
- Arduino: `funktion_<name>.cpp` + optional `funktion_<name>.ino` Wrapper.
- Node-RED: `fn_<name>_flow.json` mit Link In/Out zur `data_exchange_flow.json`.
- Protokoll: Kommandos nach Serial1, Antworten immer als JSON mit `type`.

## Definition of Done je neue Funktion
- Arduino-Modul implementiert und Build erfolgreich.
- Node-RED Funktions-Flow angelegt und ueber Link-Nodes angebunden.
- Dashboard-Interaktion vorhanden (Button/Anzeige).
- End-to-End Test (Kommando -> Hardware -> JSON -> UI) erfolgreich.
