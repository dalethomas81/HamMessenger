# Hardware

This page describes all hardware components, the PCB shield design, and the 3D-printed enclosure.

---

## System Architecture

HamMessenger is built around two microcontrollers that communicate via serial:

```
┌────────────────────────────────────────────────────┐
│               Arduino Mega 2560                     │
│  (HamMessenger firmware — UI, GPS, settings, SD)   │
│                                                     │
│  Serial3 ──────────────────────────────────────┐   │
└────────────────────────────────────────────────┼───┘
                                                 │
                                    ┌────────────▼──────────┐
                                    │  Arduino Pro Mini 5V  │
                                    │  (MicroAPRS firmware) │
                                    └────────────┬──────────┘
                                                 │
                                         PTT / Audio
                                                 │
                                    ┌────────────▼──────────┐
                                    │  Ham Radio Transceiver│
                                    │  (e.g. Baofeng UV-5R5)│
                                    └───────────────────────┘
```

The **Arduino Mega** runs the main HamMessenger firmware and manages:
- The OLED display and CardKB keyboard
- GPS parsing and position tracking
- SD card logging
- USB serial communication with a host computer
- Settings storage in EEPROM

The **Arduino Pro Mini** runs the MicroAPRS modem firmware and handles:
- APRS packet encoding and decoding (audio tones)
- PTT (push-to-talk) control
- Passing decoded packets back to the Mega via serial

---

## Component List

### Main Controller

| Component | Specification | Notes |
|-----------|--------------|-------|
| Arduino Mega 2560 | ATmega2560, 5V | Main processor |

### Modem

| Component | Specification | Notes |
|-----------|--------------|-------|
| Arduino Pro Mini | ATmega328P, **5V** version | Must be 5V, not 3.3V |

### Display

| Component | Specification | Notes |
|-----------|--------------|-------|
| OLED Display | SSD1106, 0.96" or 1.3", I2C | SH1106 controller |

### Keyboard

| Component | Specification | Notes |
|-----------|--------------|-------|
| M5Stack CardKB | I2C, QWERTY layout | Compact card-style keyboard |

### GPS

| Component | Specification | Notes |
|-----------|--------------|-------|
| GPS Module | u-blox Neo-6M | 9600 baud UART |

> **Note on GPS antennas:** The standard Neo-6M module shipped with a small ceramic patch antenna has poor sensitivity. The revised design (v1.1+) uses a Neo-6M variant with a larger external antenna for significantly better satellite acquisition. If you experience poor GPS lock, consider swapping to the larger antenna model.

### Power

| Component | Specification | Notes |
|-----------|--------------|-------|
| 18650 Lithium-Ion Cells | Panasonic NCR18650B, 2× | High-capacity cells recommended |
| UPS Battery Module | 5V output, 18650 holder + protection | Powers the Mega and peripherals |

### Radio Interface

| Component | Specification | Notes |
|-----------|--------------|-------|
| Kenwood 2-pin connector | 2.5mm + 3.5mm | PTT and audio I/O to radio |
| Ferrite rings | Snap-on, appropriate for cable gauge | RF noise suppression on PTT and audio cables |

### Storage

| Component | Specification | Notes |
|-----------|--------------|-------|
| MicroSD card | FAT32 formatted, any capacity | For message and raw packet logging |
| MicroSD breakout board | SPI interface | If not integrated on shield |

---

## PCB Shield (v1.2)

The HamMessenger shield is a custom PCB that mounts onto the Arduino Mega and provides breakout connections for all peripherals. It simplifies wiring and reduces the chance of assembly errors.

**Gerber files** for PCB fabrication are located at:

```
Source/CAD/Fusion-360/Builds/Gerber Shield v12_2025-05-17.zip
```

Upload this file directly to a PCB fabrication service such as [JLCPCB](https://jlcpcb.com) or [PCBWay](https://pcbway.com). Standard 2-layer, 1.6mm FR4, HASL finish is sufficient.

The shield design files (editable) are in Autodesk Fusion 360 format:

```
Source/CAD/Fusion-360/Shield-Electronic-Design.f3z
```

Pick-and-place CSV files for SMD assembly are in:

```
Source/CAD/Fusion-360/CAMOutputs/Assembly/
```

The full bill of materials for the shield is in:

```
Documentation/Parts/BOMs/shield.md
```

---

## 3D-Printed Enclosure

HamMessenger uses a custom unibody enclosure designed in Autodesk Fusion 360. STL files for 3D printing are provided.

### Print Files

| File | Description |
|------|-------------|
| `Source/CAD/Fusion-360/Builds/UniBody.stl` | Main enclosure body |
| `Source/CAD/Fusion-360/Builds/UniBody-Keyboard.stl` | Keyboard mounting plate |
| `Source/CAD/Fusion-360/Builds/Front Cover.stl` | Front face plate |
| `Source/CAD/Fusion-360/Builds/Back Cover.stl` | Rear cover |

### Print Settings (recommended)

| Setting | Value |
|---------|-------|
| Material | PLA or PETG |
| Layer height | 0.2 mm |
| Infill | 20–30% |
| Supports | As needed for overhangs |
| Perimeters | 3+ for rigidity |

The editable Fusion 360 source file is `Source/CAD/Fusion-360/Enclosure.f3d`.

---

## External Peripherals BOM

A list of cables, connectors, ferrite rings, and other external parts is in:

```
Documentation/Parts/BOMs/peripherals.md
```

Vendor suggestions are in:

```
Documentation/Parts/Vendors/
```

---

## Board Revisions

| Revision | Status | Notes |
|----------|--------|-------|
| v1.0 | Deprecated | Initial prototype |
| v1.1 | Deprecated | Improved GPS antenna; minor fixes |
| v1.2 | **Current** | Latest stable design; use this version |

---

*Previous: [Getting Started](Getting-Started) · Next: [Building the Device](Building-the-Device)*
