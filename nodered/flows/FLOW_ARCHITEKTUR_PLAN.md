# Projekt UML und Ablaufdiagramme

## Ziel
- Komplettsicht auf Architektur und Laufzeit fuer Arduino, Raspberry Pi und Node-RED.
- Einheitliche Grundlage fuer weitere Funktions-Module (`fn_<name>_flow.json`).

## 1) Systemarchitektur (Komponentenblick)
```mermaid
flowchart TB
  User[User] --> Browser[Touch Browser]
  Browser --> Dashboard[Node-RED Dashboard\nnodered/flows/dashboard_flow.json]

  subgraph RPi[Raspberry Pi]
    Dashboard --> DataFlow[data_exchange_flow.json\nSerial In/Out + JSON Parse]
    Dashboard --> NetFlow[Network.json\nWLAN / Status / QR]
    Dashboard --> StartupFlow[fn_startup_test_flow.json\nStartup-Status]
    Dashboard --> ParamFlow[fn_parameters_flow.json\nParameter + Offset]
    Dashboard --> ProfileFlow[fn_profiles_flow.json\nRFID Profile]
  end

  DataFlow --> SerialPort[/dev/serial0 UART]
  SerialPort --> LevelShift[Pegelwandler 5V -> 3.3V]
  LevelShift --> Mega[Arduino Mega 2560 Serial1\nTX1=18 RX1=19]

  subgraph Arduino[Arduino Sketch arduino/mega]
    Mega --> MainCpp[mega.ino\nsetup() / loop()]
    MainCpp --> DataCpp[data.cpp\nREAD / ACT / RFID Protokoll]
    MainCpp --> SensorsCpp[sensors.cpp\nHC-SR04 + RC522]
    MainCpp --> ActCpp[actuators.cpp]
    DataCpp --> Shared[mega_shared.h/.cpp]
    SensorsCpp --> Shared
    ActCpp --> Shared
  end
```

## 2) UML RFID-Funktionsablauf (Lesen, Anlernen, Loeschen)
```mermaid
sequenceDiagram
  participant U as User
  participant D as Dashboard(Profile)
  participant P as fn_profiles_flow
  participant X as data_exchange_flow
  participant A as Arduino data.cpp
  participant S as sensors.cpp + RC522

  Note over P: Polling alle 2s: RFID_POLL -> RFID

  alt Lesen
    U->>D: Klick "Lesen"
    D->>P: payload RFID_READ
  else Profil 1
    U->>D: Klick "Profil 1 anlernen/loeschen"
    D->>P: payload RFID_LEARN_P1
  else Profil 2
    U->>D: Klick "Profil 2 anlernen/loeschen"
    D->>P: payload RFID_LEARN_P2
  end

  alt Profil bereits belegt und Learn-Button geklickt
    P-->>D: Bindung geloescht (ohne Hardwarezugriff)
  else Lesen/Anlernen aktiv
    P->>X: payload RFID
    X->>A: Serial1 "RFID"
    A->>S: Sensors_readRfid()
    S-->>A: uid/status/hw_status/probe_status/version
    A-->>X: JSON {type:"rfid", ...}
    X-->>P: msg.payload

    alt rfid_status == ok und UID bekannt
      P-->>D: Aktives Profil 1/2 setzen
    else rfid_status == ok und Anlernen aktiv
      P-->>D: UID Slot 1/2 speichern, Learn-Modus beenden
    else rfid_status == probe_error und kein Learn-Modus
      P-->>D: Status bleibt "Profile bereit"
    else sonstiger Fehler
      P-->>D: Fehlermeldung anzeigen
    end
  end
```

### 2.1 UML Zustandsmodell (Profile-Controller)
```mermaid
stateDiagram-v2
  [*] --> Bereit

  Bereit --> LesenAktiv: RFID_READ
  Bereit --> LearnP1: RFID_LEARN_P1 (Slot1 leer)
  Bereit --> LearnP2: RFID_LEARN_P2 (Slot2 leer)
  Bereit --> Bereit: RFID_LEARN_P1 (Slot1 belegt -> loeschen)
  Bereit --> Bereit: RFID_LEARN_P2 (Slot2 belegt -> loeschen)

  LesenAktiv --> Bereit: RFID ok/no_card/probe_error
  LearnP1 --> Bereit: RFID ok (UID -> Slot1)
  LearnP2 --> Bereit: RFID ok (UID -> Slot2)
  LearnP1 --> LearnP1: no_card/probe_error
  LearnP2 --> LearnP2: no_card/probe_error
```

## Struktur je Funktion (Standardmuster)
- Arduino: Erweiterung bestehender Module oder neues `funktion_<name>.cpp`.
- Node-RED: `fn_<name>_flow.json` mit Link In/Out zur `data_exchange_flow.json`.
- Protokoll: Kommandos nach Serial1, Antworten immer als JSON mit `type`.

## Definition of Done je neue Funktion
- Arduino-Modul implementiert und Build erfolgreich.
- Node-RED Funktions-Flow angelegt und ueber Link-Nodes angebunden.
- Dashboard-Interaktion vorhanden (Buttons/Anzeigen).
- End-to-End Test (Kommando -> Hardware -> JSON -> UI) erfolgreich.
