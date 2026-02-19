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
    Dashboard --> ParamFlow[fn_parameters_flow.json\nParameter + Offsets + Relais]
    Dashboard --> ProfileFlow[fn_profiles_flow.json\nRFID Profile]
  end

  DataFlow --> SerialPort[/dev/serial0 UART]
  SerialPort --> LevelShift[Pegelwandler 5V -> 3.3V]
  LevelShift --> Mega[Arduino Mega 2560 Serial1\nTX1=18 RX1=19]

  subgraph Arduino[Arduino Sketch arduino/mega]
    Mega --> MainCpp[mega.ino\nsetup() / loop()]
    MainCpp --> DataCpp[data.cpp\nREAD / ACT / RFID Protokoll]
    MainCpp --> SensorsCpp[sensors.cpp\nHC-SR04 + Tropfensensor + Truebungssensor + RC522]
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

## 3) UML HC-SR04 + Tropfensensor + Truebungssensor-Funktionsablauf (READ, Offsets, Status)
```mermaid
sequenceDiagram
  participant U as User
  participant D as Dashboard(Projekt-info)
  participant ST as fn_startup_test_flow
  participant X as data_exchange_flow
  participant A as Arduino data.cpp
  participant S as sensors.cpp HC-SR04 + Tropfensensor + Truebungssensor
  participant P as fn_parameters_flow

  alt Manuell
    U->>D: Klick "Sensoren aktualisieren"
    D->>X: payload READ
  else Startup/zyklisch
    ST->>X: payload READ
  end

  X->>A: Serial1 "READ"
  A->>S: Sensors_readSnapshot()
  S-->>A: hcsr04_distance_cm/status + droplet_raw/status + turbidity_raw/status + uptime_ms
  A-->>X: JSON {type:"sensor", ...}
  X->>P: msg.payload (sensor)
  P->>P: Offsets anwenden (hcsr04_offset_cm + droplet_offset_raw + turbidity_offset_raw)
  P-->>D: hcsr04_distance_display_cm + droplet_display_raw + turbidity_display_raw + Status + uptime_hms
  P-->>ST: Sensorpayload fuer Startup-Validierung
  ST-->>D: Anlagenstatus (bereit/stoerung)
```

### 3.1 UML Zustandsmodell (HC-SR04 + Tropfensensor + Truebungssensor/Anlagenstatus)
```mermaid
stateDiagram-v2
  [*] --> Startup
  Startup --> Stoerung: Default beim Boot

  Stoerung --> Bereit: hcsr04_status == ok && droplet_status == ok && turbidity_status == ok && uptime gueltig
  Stoerung --> Stoerung: hcsr04_status != ok || droplet_status != ok || turbidity_status != ok

  Bereit --> Bereit: hcsr04_status == ok && droplet_status == ok && turbidity_status == ok
  Bereit --> Stoerung: HC-SR04/Tropfensensor/Truebungssensor fehlerhaft/ungueltig
```

## 4) UML data_exchange_flow (Serial Gateway)
```mermaid
sequenceDiagram
  participant Src as Dashboard/fn_flows
  participant X as data_exchange_flow
  participant A as Arduino Serial1
  participant Fn as fn_startup/fn_parameters/fn_profiles

  Src->>X: Link-In (READ/RFID/ACT,...)
  X->>A: serial out "/dev/serial0" + "\\n"
  A-->>X: serial in JSON line
  X->>X: Parse JSON
  X->>X: Format uptime_hms
  X-->>Fn: Link-Out (msg.payload mit type)
```

### 4.1 UML Aktivitaet (data_exchange_flow)
```mermaid
flowchart LR
  A[Link-In von Dashboard/Funktionen] --> B[serial out -> Arduino]
  B --> C[serial in vom Arduino]
  C --> D[JSON Parse]
  D --> E[Function: uptime_hms berechnen]
  E --> F[Link-Out zu fn_startup/fn_parameters/fn_profiles]
  C --> G[Debug Raw]
  D --> H[Debug Parsed]
```

## 5) UML fn_startup_test_flow (Startup-Validierung)
```mermaid
sequenceDiagram
  participant Boot as Boot Inject
  participant ST as fn_startup_test_flow
  participant X as data_exchange_flow
  participant A as Arduino
  participant D as Dashboard Status

  Boot->>ST: INIT_STARTUP_STATUS
  ST-->>D: "Anlage stoerung" (Default)

  loop alle 2s (ab Boot +5s)
    ST->>X: payload READ
    X->>A: Serial1 READ
    A-->>X: {type:"sensor", ...}
    X-->>ST: sensor payload
    ST->>ST: switch: nur type=="sensor"
    ST->>ST: validate uptime + hcsr04_status/range + droplet_status/range + turbidity_status/range
    ST-->>D: "Anlage bereit" oder "Anlage stoerung"
  end
```

## 6) UML fn_parameters_flow (Offsets + Relais + Anzeigewerte)
```mermaid
sequenceDiagram
  participant Boot as Boot Inject
  participant UI as Dashboard Slider/Relais-Buttons
  participant PF as fn_parameters_flow
  participant X as data_exchange_flow
  participant D as Dashboard

  Boot->>PF: INIT_OFFSET
  PF->>PF: Default-Offsets clampen
  PF-->>D: parameter state (hcsr04_offset_cm + droplet_offset_raw + turbidity_offset_raw)

  UI->>PF: Slider-Wert
  PF->>PF: Offset speichern (abh. von msg.topic)
  PF-->>D: parameter state (hcsr04_offset_cm + droplet_offset_raw + turbidity_offset_raw)

  UI->>PF: Relais-Button (topic=relay_toggle)
  PF-->>X: ACT,<pin>,<state>
  X-->>PF: act ACK (ok/pin/state)
  PF-->>D: relay1..relay4 = ON/OFF

  X-->>PF: sensor payload
  PF->>PF: Offsets auf Distanz/Rohwert anwenden
  PF-->>D: hcsr04_distance_display_cm + droplet_display_raw + turbidity_display_raw + Sensorstatus + uptime_hms
```

### 6.1 UML Zustandsmodell (Offset)
```mermaid
stateDiagram-v2
  [*] --> OffsetInit
  OffsetInit --> OffsetReady: INIT_OFFSET
  OffsetReady --> OffsetReady: Slider update (HC-SR04 -5..+5 / Tropfen -300..+300 / Truebung -200..+200)
  OffsetReady --> OffsetReady: Sensorpayload -> display distance/raw
```

### 6.2 UML Zustandsmodell (Relais 1..4)
```mermaid
stateDiagram-v2
  [*] --> AlleOFF
  AlleOFF --> Gemischt: erster Toggle
  Gemischt --> Gemischt: weitere Toggles
  Gemischt --> AlleOFF: alle Relais auf OFF

  note right of Gemischt
    Jeder Button sendet relay_toggle.
    fn_parameters_flow erzeugt ACT,<pin>,<state>
    und aktualisiert Status erst nach ACT-ACK.
  end note
```

## 7) UML Network.json (WLAN + QR + UI)
```mermaid
sequenceDiagram
  participant Poll as 1s Poll
  participant Net as Network.json
  participant OS as nmcli/ip
  participant D as Dashboard WiFi/Welcome

  loop jede 1s
    Poll->>Net: trigger
    Net->>OS: nmcli device status
    OS-->>Net: wlan0 state/connection
    Net->>OS: nmcli wlan0 ip + ip end0
    OS-->>Net: WLAN/LAN IP
    Net->>Net: Status auswerten + QR targets
    Net->>OS: qrencode (AP/Login/Dashboard)
    OS-->>Net: SVG
    Net-->>D: WLAN Status + Debug/WiFi URL + Welcome QR Cards
  end
```

### 7.1 UML WLAN-Connect Ablauf
```mermaid
flowchart LR
  A[SSID Auswahl + Passwort] --> B[Connect Button]
  B --> C[Function: Connect Kommando bauen]
  C --> D[nmcli con down projekt-ap]
  D --> E[Delay]
  E --> F[nmcli connection delete <SSID>]
  F --> G[Delay]
  G --> H[nmcli dev wifi connect <SSID> password <PW>]
  H --> I[Toast OK + Navigate Projekt-info]
  H --> J[Toast Fehler]
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
