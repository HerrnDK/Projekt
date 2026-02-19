# Hardware-Uebersicht

- Arduino Mega 2560 R3
- Raspberry Pi (Node-RED, UART `/dev/serial0`)
- Pegelwandler 5V->3.3V (Pflicht fuer Mega TX1 -> Raspberry Pi RXD)
- 1x 4-Kanal Relaismodul 5V (IN1..IN4 an D22..D25, 10A @ 250VAC / 30VDC)
- Relais 1: 5V Wasserpumpe
- Relais 2-4: Reserve
- 1x HC-SR04 Ultraschallsensor (TRIG D26, ECHO D27, 5V)
- 1x Funduino Tropfensensor (A0, +5V, GND, analoger Rohwert 0..1023)
- 1x Wassertruebungssensor (A1, +5V, GND, analoger Rohwert 0..1023)
- 1x Ocean-TDS-Meter-Sensor (A2, +5V, GND, analoger Rohwert 0..1023)
- 1x RFID-RC522 Modul (SPI: SS D53, SCK D52, MOSI D51, MISO D50, RST D49, 3.3V)

