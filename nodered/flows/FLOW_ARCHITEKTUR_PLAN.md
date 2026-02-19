# Projekt-Flussdiagramme und Ablaufdarstellungen

## Ziel
- Komplettsicht auf Architektur und Laufzeit fuer Arduino, Raspberry Pi und Node-RED.
- Einheitliche Grundlage fuer weitere Funktions-Module (`fn_<name>_flow.json`).

## 1) Systemarchitektur (Komponentenblick)
```mermaid
flowchart TD
  S([Start]) --> Benutzer["Benutzer am Touch-Browser"]
  Benutzer --> Dashboard["Node-RED Dashboard (dashboard_flow.json)"]

  subgraph RPi["Raspberry Pi"]
    Dashboard --> NetFlow["Network.json"]
    Dashboard --> DataFlow["data_exchange_flow.json"]
    Dashboard --> StartupFlow["fn_startup_test_flow.json"]
    Dashboard --> ParamFlow["fn_parameters_flow.json"]
    Dashboard --> ProfileFlow["fn_profiles_flow.json"]
  end

  DataFlow --> SerialPort["UART /dev/serial0"]
  SerialPort --> Pegel["Pegelwandler 5V auf 3.3V"]
  Pegel --> Mega["Arduino Mega Serial1"]

  subgraph Arduino["Arduino-Sketch"]
    Mega --> MainCpp["mega.ino"]
    MainCpp --> DataCpp["daten.cpp"]
    MainCpp --> SensorsCpp["sensoren.cpp"]
    MainCpp --> ActCpp["aktoren.cpp"]
    DataCpp --> Shared["mega_gemeinsam.h und mega_gemeinsam.cpp"]
    SensorsCpp --> Shared
    ActCpp --> Shared
  end

  Shared --> E([Ende])
```

## 2) RFID-Funktionsablauf (Lesen, Anlernen, Loeschen)
```mermaid
flowchart TD
  S([Start]) --> T0["Trigger: Lesen, Profil 1, Profil 2 oder RFID_POLL"]
  T0 --> D0{"Profil-Button und Slot bereits belegt?"}

  D0 -- Ja --> L0["UID-Bindung loeschen"]
  L0 --> UI0["Profilstatus im Dashboard aktualisieren"]
  UI0 --> E([Ende])

  D0 -- Nein --> C0["Befehl RFID an data_exchange_flow senden"]
  C0 --> C1["Serial1 Anfrage RFID an Arduino"]
  C1 --> C2["RC522 lesen: UID, Status, Hardwarestatus"]
  C2 --> D1{"Hardwarestatus ok?"}

  D1 -- Nein --> F0["Status: RFID Modulstoerung"]
  F0 --> UI1["Dashboard: Profilstatus und Modulstatus aktualisieren"]
  UI1 --> E

  D1 -- Ja --> D2{"rfid_status ok und UID vorhanden?"}
  D2 -- Nein --> D3{"rfid_status no_card oder probe_error?"}
  D3 -- Ja --> W0["Wartezustand oder Bereitschaft anzeigen"]
  W0 --> UI1
  D3 -- Nein --> F1["RFID Fehlertext setzen"]
  F1 --> UI1

  D2 -- Ja --> D4{"Anlernen Profil 1 oder Profil 2 aktiv?"}
  D4 -- Ja --> S1["UID in entsprechenden Slot speichern"]
  S1 --> P1["Lernmodus beenden"]
  P1 --> UI1

  D4 -- Nein --> D5{"UID passt zu Profil 1 oder Profil 2?"}
  D5 -- Ja --> A0["Aktives Profil setzen"]
  A0 --> UI1
  D5 -- Nein --> U0["Unbekannten Chip melden"]
  U0 --> UI1
```

### 2.1 Flusslogik (Profilsteuerung)
```mermaid
flowchart TD
  S([Start]) --> B0["Zustand Bereit"]
  B0 --> D0{"Ereignis?"}
  D0 -- RFID_READ --> R0["Zustand Lesen aktiv"]
  D0 -- RFID_LEARN_P1 --> D1{"Slot 1 leer?"}
  D0 -- RFID_LEARN_P2 --> D2{"Slot 2 leer?"}

  D1 -- Ja --> L1["Zustand LearnP1"]
  D1 -- Nein --> X1["Slot 1 Bindung loeschen"]
  X1 --> B0

  D2 -- Ja --> L2["Zustand LearnP2"]
  D2 -- Nein --> X2["Slot 2 Bindung loeschen"]
  X2 --> B0

  R0 --> D3{"RFID Antwort ok/no_card/probe_error?"}
  D3 -- Ja --> B0
  D3 -- Nein --> B0

  L1 --> D4{"UID gelesen?"}
  D4 -- Ja --> S1["UID in Slot 1 speichern"] --> B0
  D4 -- Nein --> L1

  L2 --> D5{"UID gelesen?"}
  D5 -- Ja --> S2["UID in Slot 2 speichern"] --> B0
  D5 -- Nein --> L2
```

## 3) Sensorablauf HC-SR04, Tropfensensor, Truebungssensor
```mermaid
flowchart TD
  S([Start]) --> D0{"Trigger?"}
  D0 -- Manuell --> T1["Button Sensoren aktualisieren"]
  D0 -- Zyklisch --> T2["Starttest-Intervall"]
  T1 --> C0["READ an data_exchange_flow senden"]
  T2 --> C0

  C0 --> C1["Serial1 READ an Arduino"]
  C1 --> C2["Sensors_readSnapshot ausfuehren"]
  C2 --> C3["JSON Sensorpayload empfangen"]
  C3 --> C4["Offsets anwenden im fn_parameters_flow"]
  C4 --> UI0["Dashboard-Werte und Status aktualisieren"]
  C4 --> ST0["Payload an fn_startup_test_flow"]
  ST0 --> D1{"Alle Sensorstatus gueltig?"}
  D1 -- Ja --> A0["Anlagenstatus: bereit"]
  D1 -- Nein --> A1["Anlagenstatus: stoerung"]
  A0 --> E([Ende])
  A1 --> E
```

### 3.1 Flusslogik (Anlagenstatus)
```mermaid
flowchart TD
  S([Start]) --> I0["Initial: Anlage stoerung"]
  I0 --> D0{"hcsr04_status ok?"}
  D0 -- Nein --> F0["Stoerung bleibt aktiv"]
  D0 -- Ja --> D1{"droplet_status ok?"}
  D1 -- Nein --> F0
  D1 -- Ja --> D2{"turbidity_status ok?"}
  D2 -- Nein --> F0
  D2 -- Ja --> D3{"uptime gueltig?"}
  D3 -- Nein --> F0
  D3 -- Ja --> OK0["Anlage bereit"]
  F0 --> E([Ende])
  OK0 --> E
```

## 4) data_exchange_flow (Serial-Gateway)
```mermaid
flowchart TD
  S([Start]) --> IN0["Link-In von Dashboard oder Funktionsflow"]
  IN0 --> TX0["Serial out an Arduino mit Newline"]
  TX0 --> RX0["Serial in vom Arduino empfangen"]
  RX0 --> D0{"JSON parsebar?"}
  D0 -- Nein --> DBG0["Debug: Rohdaten und Parse-Fehler"]
  DBG0 --> E([Ende])
  D0 -- Ja --> P0["Payload uebernehmen"]
  P0 --> P1["uptime_hms berechnen"]
  P1 --> OUT0["Link-Out zu startup, parameter, profile"]
  OUT0 --> DBG1["Debug: geparste Daten"]
  DBG1 --> E
```

### 4.1 Technischer Datenpfad (data_exchange_flow)
```mermaid
flowchart TD
  S([Start]) --> A0["Befehl READ, RFID oder ACT empfangen"]
  A0 --> A1["Befehl seriell senden"]
  A1 --> A2["JSON Antwortzeile empfangen"]
  A2 --> A3["JSON verarbeiten"]
  A3 --> A4["Uptime formatieren"]
  A4 --> A5["An Funktionsflows verteilen"]
  A5 --> E([Ende])
```

## 5) fn_startup_test_flow (Starttest-Validierung)
```mermaid
flowchart TD
  S([Start]) --> I0["Boot: INIT_STARTUP_STATUS setzen"]
  I0 --> I1["Standardstatus: Anlage stoerung"]
  I1 --> L0["Alle 2 Sekunden READ ausloesen"]
  L0 --> C0["Sensorpayload ueber data_exchange_flow empfangen"]
  C0 --> D0{"payload.type gleich sensor?"}
  D0 -- Nein --> L0
  D0 -- Ja --> V0["Uptime und Sensorstatus validieren"]
  V0 --> D1{"Alle Pruefungen ok?"}
  D1 -- Ja --> OK0["Dashboard: Anlage bereit"]
  D1 -- Nein --> ER0["Dashboard: Anlage stoerung"]
  OK0 --> L0
  ER0 --> L0
```

## 6) fn_parameters_flow (Offsets + Relais + Anzeigewerte)
```mermaid
flowchart TD
  S([Start]) --> I0["INIT_OFFSET verarbeiten"]
  I0 --> I1["Standard-Offsets setzen und begrenzen"]
  I1 --> UI0["Parameterstatus an Dashboard senden"]
  UI0 --> W0["Warten auf Eingangsereignis"]

  W0 --> D0{"Eingangstyp?"}
  D0 -- Slider --> P0["Offset nach topic speichern"]
  P0 --> P1["Werte begrenzen und global speichern"]
  P1 --> UI1["Parameterstatus aktualisieren"]
  UI1 --> W0

  D0 -- Relais-Button --> R0["ACT pin state erzeugen"]
  R0 --> R1["An data_exchange_flow senden"]
  R1 --> R2["act ACK empfangen"]
  R2 --> D1{"ACK ok?"}
  D1 -- Ja --> R3["Relaisstatus aktualisieren"]
  D1 -- Nein --> R4["Relaisstatus unveraendert lassen"]
  R3 --> UI2["ON/OFF an Dashboard senden"]
  R4 --> UI2
  UI2 --> W0

  D0 -- Sensorpayload --> S0["Offsets auf Distanz und Rohwerte anwenden"]
  S0 --> UI3["Sensoranzeige mit korrigierten Werten senden"]
  UI3 --> W0
```

### 6.1 Flusslogik (Offset)
```mermaid
flowchart TD
  S([Start]) --> L0["Aktuelle Offsets laden"]
  L0 --> C0["Grenzwerte anwenden"]
  C0 --> D0{"Welcher topic-Wert kommt?"}
  D0 -- hcsr04_offset_cm --> U0["HC-SR04 Offset aktualisieren"]
  D0 -- droplet_offset_raw --> U1["Tropfensensor Offset aktualisieren"]
  D0 -- turbidity_offset_raw --> U2["Truebungssensor Offset aktualisieren"]
  U0 --> S0["Offsets speichern"]
  U1 --> S0
  U2 --> S0
  S0 --> P0["Parameterstatus veroeffentlichen"]
  P0 --> E([Ende])
```

### 6.2 Flusslogik (Relais 1 bis 4)
```mermaid
flowchart TD
  S([Start]) --> I0["Relaiszustand initialisieren"]
  I0 --> W0["Warten auf relay_toggle"]
  W0 --> T0["Gewuenschten naechsten Zustand berechnen"]
  T0 --> C0["ACT pin state senden"]
  C0 --> A0["ACK empfangen"]
  A0 --> D0{"ACK ok?"}
  D0 -- Ja --> U0["Relaiszustand uebernehmen"]
  D0 -- Nein --> U1["Relaiszustand nicht aendern"]
  U0 --> P0["ON/OFF an Dashboard senden"]
  U1 --> P0
  P0 --> W0
```

## 7) Network.json (WLAN + QR + UI)
```mermaid
flowchart TD
  S([Start]) --> L0["Alle 1 Sekunde Netzwerkabfrage"]
  L0 --> N0["nmcli device status lesen"]
  N0 --> N1["WLAN/AP Status auswerten"]
  N1 --> N2["WLAN und LAN IP ermitteln"]
  N2 --> N3["QR Ziele festlegen"]
  N3 --> N4["QR Codes ueber qrencode erzeugen"]
  N4 --> UI0["Dashboard: WLAN-Status und URLs aktualisieren"]
  UI0 --> UI1["Dashboard: Willkommens-QR Karten aktualisieren"]
  UI1 --> L0
```

### 7.1 WLAN-Verbindungsablauf
```mermaid
flowchart TD
  S([Start]) --> I0["SSID und Passwort erfassen"]
  I0 --> D0{"SSID und Passwort gueltig?"}
  D0 -- Nein --> F0["Toast: Eingabe unvollstaendig"]
  F0 --> E([Ende])

  D0 -- Ja --> C0["AP stoppen: nmcli con down projekt-ap"]
  C0 --> C1["Verzoegerung"]
  C1 --> C2["Alte Verbindung loeschen"]
  C2 --> C3["Verzoegerung"]
  C3 --> C4["WLAN verbinden mit nmcli"]
  C4 --> D1{"Verbindung erfolgreich?"}
  D1 -- Ja --> OK0["Toast OK und Navigation zu Projekt-info"]
  D1 -- Nein --> ER0["Toast Fehler"]
  OK0 --> E
  ER0 --> E
```

## Struktur je Funktion (Standardmuster)
- Arduino: Erweiterung bestehender Module oder neues `funktion_<name>.cpp`.
- Node-RED: `fn_<name>_flow.json` mit Link In/Out zur `data_exchange_flow.json`.
- Protokoll: Kommandos nach Serial1, Antworten immer als JSON mit `type`.

## Abnahmekriterien je neue Funktion
- Arduino-Modul implementiert und Build erfolgreich.
- Node-RED-Funktionsflow angelegt und ueber Link-Nodes angebunden.
- Dashboard-Interaktion vorhanden (Buttons/Anzeigen).
- End-to-End-Test (Kommando -> Hardware -> JSON -> UI) erfolgreich.
