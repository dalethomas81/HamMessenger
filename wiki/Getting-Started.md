# Getting Started

This page gives you an overview of everything you need before building and operating HamMessenger. Follow the links at each step to the detailed pages for that phase.

---

## Prerequisites

### Amateur Radio License

HamMessenger transmits on amateur radio frequencies. You **must hold a valid amateur radio license** (Technician class or higher in the US) before operating the device on the air. Receiving-only use does not require a license, but you won't be able to transmit beacons or messages.

### Skills and Tools

| Skill / Tool | Required for |
|---|---|
| Basic soldering | Assembling the PCB and shield |
| Arduino IDE or Arduino CLI | Uploading firmware |
| Python 3.11+ | Running the GUI from source (optional if using pre-built installer) |
| 3D printer (or print service) | Printing the enclosure |
| FTDI USB-to-serial cable | Flashing the MicroAPRS modem |
| Multimeter | Verifying connections |

---

## High-Level Build Path

Here is the recommended sequence for building HamMessenger from scratch:

**1. Gather hardware** → [Hardware](Hardware)

Order the PCB, source all components, and print or order the enclosure.

**2. Assemble the device** → [Building the Device](Building-the-Device)

Solder the shield, assemble the enclosure, and wire all peripherals.

**3. Flash the firmware** → [Firmware Installation](Firmware-Installation)

Upload the HamMessenger sketch to the Arduino Mega and flash the MicroAPRS firmware to the Arduino Pro Mini modem.

**4. Set up your radio** → [Radio Setup](Radio-Setup)

Program your transceiver to the APRS frequency and connect it to the device.

**5. Configure the device** → [Configuration Reference](Configuration-Reference)

Enter your callsign, SSID, beacon settings, and other preferences.

**6. Start operating** → [Operating the Device](Operating-the-Device)

Send messages, track your position, and monitor the APRS network.

**7. (Optional) Install the desktop GUI** → [Desktop GUI](Desktop-GUI)

Connect HamMessenger to your computer to monitor activity and send commands from a graphical interface.

---

## Using Pre-Built Releases

If you already have the hardware assembled and just need the software, download the latest release from the [Releases](../../releases) page. Each release includes:

- `HamMessenger.hex` — Pre-compiled Arduino Mega firmware
- `HamMessenger-windows.exe` — Windows desktop GUI installer
- `HamMessenger-macos.dmg` — macOS desktop GUI installer

Pre-built modem firmware (MicroAPRS `.hex` files) is included in the repository under `Source/MicroAPRS Firmware Installer/`.

---

## Quick-Start Checklist

- [ ] Valid amateur radio license obtained
- [ ] PCB ordered or etched
- [ ] All components sourced (see [Hardware](Hardware))
- [ ] Enclosure printed (STL files in `Source/CAD/Fusion-360/Builds/`)
- [ ] Arduino Mega firmware flashed
- [ ] MicroAPRS modem firmware flashed
- [ ] Radio programmed to 144.390 MHz
- [ ] Callsign and SSID configured on device
- [ ] First beacon transmitted successfully

---

*Next: [Hardware](Hardware)*
