# HamMessenger Wiki

Welcome to the **HamMessenger** project wiki. HamMessenger is a portable, battery-powered amateur radio text messaging device that lets licensed ham radio operators send and receive text messages and position updates using the **APRS** (Automatic Packet Reporting System) protocol.

> **License required:** Operating HamMessenger on radio frequencies requires a valid amateur radio license in your jurisdiction.

---

## What is HamMessenger?

HamMessenger combines an Arduino microcontroller, a MicroAPRS modem, a small OLED display, a compact keyboard, and a GPS receiver into a self-contained handheld device. It connects to an inexpensive ham radio transceiver (such as the Baofeng UV-5R5) to transmit and receive APRS packets over the air. A companion desktop application for Windows and macOS lets you monitor activity, send messages, and adjust settings from your computer.

**Core capabilities:**

- Send and receive APRS text messages with automatic acknowledgment and retry
- Broadcast your GPS position as an APRS beacon (distance-triggered or timed)
- View incoming messages and beacon status on the built-in OLED display
- Log all messages and raw packets to a microSD card
- Control and configure the device via USB serial or the desktop GUI

---

## Wiki Pages

| Page | Description |
|------|-------------|
| [Getting Started](Getting-Started) | Requirements overview and first steps |
| [Hardware](Hardware) | Components, bill of materials, PCB, and enclosure |
| [Building the Device](Building-the-Device) | Step-by-step assembly guide |
| [Firmware Installation](Firmware-Installation) | Flash the Arduino and MicroAPRS modem firmware |
| [Radio Setup](Radio-Setup) | Configure your radio for APRS operation |
| [Operating the Device](Operating-the-Device) | Daily use: navigation, messaging, beaconing |
| [Desktop GUI](Desktop-GUI) | Install and use the companion desktop application |
| [Serial Interface](Serial-Interface) | Full serial command reference |
| [Configuration Reference](Configuration-Reference) | All settings explained with valid ranges |
| [Troubleshooting](Troubleshooting) | Common problems and solutions |

---

## Project Status

HamMessenger is in active **beta** development. The hardware design is at revision **v1.2** and is functional. Releases (pre-built firmware and GUI installers) are published automatically via GitHub Actions on the [Releases](../../releases) page.

---

## License

HamMessenger is released under the **GNU General Public License v3 (GPLv3)**. You are free to use, modify, and distribute this project provided derivative works are shared under the same license.

---

## External Resources

- [APRS.fi](https://aprs.fi) — Live APRS tracking map
- [MicroAPRS](https://github.com/markqvist/MicroAPRS) — The modem firmware this project depends on
- [Baofeng UV-5R5](https://baofengtech.com) — Recommended radio transceiver
- [CHIRP](https://chirp.app) — Free radio programming software
