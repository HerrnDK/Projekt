/*
  mega.ino
  Nutzen:
  - Einstiegspunkt des Arduino-Sketches.
  Funktion:
  - Initialisiert beim Start alle Module (Aktoren, Sensoren, Datenkommunikation)
    und fuehrt im Hauptzyklus die serielle Verarbeitung aus.
*/

#include "mega_gemeinsam.h"

/*
  Zweck:
  - Einmalige Initialisierung aller Module beim Boot.
  Verhalten:
  - Startet Debug- und Daten-UART, initialisiert Aktoren, Sensoren und Datenlogik.
  Rueckgabe:
  - Keine.
*/
void setup() {
  PORT_DEBUG.begin(BAUDRATE_DEBUG);  // USB-Debugschnittstelle
  PORT_DATEN.begin(BAUDRATE_DATEN);  // UART zu Node-RED (Serial1)

  Aktoren_starten();
  Schrittmotor_starten();
  Sensoren_starten();
  Daten_starten();

  PORT_DEBUG.println("Mega Grundgeruest bereit.");
}

/*
  Zweck:
  - Hauptzyklus fuer die Laufzeitverarbeitung.
  Verhalten:
  - Bearbeitet fortlaufend eingehende Serial1-Kommandos.
  Rueckgabe:
  - Keine.
*/
void loop() {
  // Serielle Kommunikation (Node-RED)
  Daten_tick();
  Schrittmotor_tick();

  // Platz fuer weitere zyklische Tasks
}
