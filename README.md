# Arduino Mega 2560 + Node-RED (Raspberry Pi)

This repository contains a starter Arduino sketch and a Node-RED flow for
serial communication between an Arduino Mega 2560 R3 and a Raspberry Pi 4
running Node-RED.

## Wiring (Serial1)
- **Arduino Mega 2560** Serial1:
  - TX1 (pin 18) -> Raspberry Pi RX
  - RX1 (pin 19) -> Raspberry Pi TX
  - GND -> GND

> The USB port (Serial) stays free for programming and debugging.

## Arduino Sketch
- Location: `arduino/mega_serial_control/mega_serial_control.ino`
- Protocol (newline terminated):
  - `READ` -> Arduino returns JSON sensor snapshot
  - `ACT,<pin>,<state>` -> Control actuator pins (example: `ACT,22,1`)

## Node-RED Flow
- Location: `nodered/flows/mega_control_flow.json`
- Imports a simple dashboard with:
  - **Read Sensors** button
  - **A0, A1, Uptime** text fields
  - **Actuator 22 ON/OFF** buttons

## Next Steps (when sensors/actuators are known)
- Replace the placeholder analog reads (A0, A1) with your real sensors.
- Extend the `ACTUATOR_PINS` list and add UI buttons as needed.
- Adjust baud rate if required by your UART settings.
