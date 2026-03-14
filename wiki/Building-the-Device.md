# Building the Device

This page walks through assembling HamMessenger from bare components. Complete the [Hardware](Hardware) page first to ensure you have everything needed before starting.

---

## Overview

Assembly has five main phases:

1. Fabricate and populate the PCB shield
2. Flash firmware onto both microcontrollers (see [Firmware Installation](Firmware-Installation))
3. Print and prepare the enclosure
4. Install components into the enclosure
5. Connect and test all cables

---

## Phase 1 — PCB Shield Assembly

### 1.1 Order the PCB

Upload `Source/CAD/Fusion-360/Builds/Gerber Shield v12_2025-05-17.zip` to your PCB fab of choice (JLCPCB, PCBWay, etc.). Standard options work fine:

- Layers: 2
- Thickness: 1.6 mm
- Surface finish: HASL (lead-free preferred)
- Color: your choice

### 1.2 Source Components

Refer to `Documentation/Parts/BOMs/shield.md` for the complete component list with reference designators and values. Source components before the PCBs arrive to avoid delays.

### 1.3 Solder SMD Components First

Solder any surface-mount components before through-hole parts:

1. Apply solder paste or flux to SMD pads.
2. Place components using tweezers.
3. Reflow with a hot air station or carefully with a fine-tip iron.
4. Inspect joints under magnification.

### 1.4 Solder Through-Hole Components

Install through-hole parts in order from lowest to tallest profile:

1. Resistors and diodes
2. Capacitors
3. Connectors and headers
4. The Arduino Pro Mini (modem) — solder directly to the shield or use pin headers for removability

> **Tip:** Use low-profile pin sockets for the Arduino Pro Mini so it can be removed for re-flashing without desoldering.

### 1.5 Inspect the Shield

Before installing into the enclosure, inspect all joints with a magnifier. Use a multimeter to verify:

- No shorts between power rails (GND and VCC)
- Continuity on signal lines to headers

---

## Phase 2 — Flash Firmware

Flash both microcontrollers **before** installing them in the enclosure — it is much easier to access USB and FTDI connections with the boards on your bench.

See the full instructions on the [Firmware Installation](Firmware-Installation) page:

- Flash MicroAPRS to the **Arduino Pro Mini** (modem) using an FTDI cable
- Flash the HamMessenger sketch to the **Arduino Mega 2560** via USB

---

## Phase 3 — Enclosure Preparation

### 3.1 Print the Enclosure Parts

Print the STL files from `Source/CAD/Fusion-360/Builds/`:

- `UniBody.stl` — main body
- `UniBody-Keyboard.stl` — keyboard mount
- `Front Cover.stl`
- `Back Cover.stl`

Allow parts to cool completely before handling. Remove support material carefully with flush cutters and a craft knife.

### 3.2 Clean Up Print Surfaces

Sand mating surfaces lightly (400-grit) for a better fit. Test-fit all parts dry before adding any electronics.

---

## Phase 4 — Component Installation

### 4.1 Install the Battery Module

1. Place the UPS battery power module in its bay in the main body.
2. Insert the two 18650 cells with correct polarity.
3. Route power cables to the shield connector.

### 4.2 Install the Arduino Mega + Shield

1. Mount the Arduino Mega into the main body.
2. Verify that the USB port aligns with its cutout.
3. Press or screw the board into place (use M3 standoffs if holes are provided).

### 4.3 Install the OLED Display

1. Route the I2C cable from the OLED connector on the shield.
2. Seat the OLED into the front cover's display window.
3. Secure with a small dab of adhesive or the retention clip if designed into the enclosure.

### 4.4 Install the CardKB Keyboard

1. Slide the M5Stack CardKB into the keyboard mount.
2. Connect the I2C cable to the shield.
3. Ensure the keyboard is snug and won't slide during use.

### 4.5 Install the GPS Module

1. Place the Neo-6M GPS module in its bay.
2. Ensure the antenna (ceramic patch or external) faces upward and is not obstructed.
3. Connect the UART cable to the GPS header on the shield.

### 4.6 Install the MicroSD Breakout (if separate)

If your shield does not include an integrated SD slot, install the breakout board and connect via the SPI header.

---

## Phase 5 — Cabling and RF Mitigation

### 5.1 Radio Connection Cable

The radio connects via a **Kenwood 2-pin connector** (2.5mm PTT + 3.5mm audio). Build or source a cable that splits into:

- **PTT line** → shield PTT pin
- **Audio out** (from radio speaker jack) → modem audio input
- **Audio in** (to radio mic jack) → modem audio output

### 5.2 Ferrite Rings

Wrap ferrite rings (snap-on or through-hole) on:

- The PTT control cable
- The audio cables to/from the radio

Ferrites suppress RF interference that can cause false packet decodes or reset the microcontrollers. Run the cable through the ferrite ring 2–3 times for best effect.

> **Important:** RF interference is a common cause of erratic behavior. Do not skip the ferrites.

### 5.3 Final Assembly

1. Close the enclosure, feeding cables through their respective cutouts.
2. Secure the front and back covers.
3. Attach the antenna to your radio transceiver.

---

## Initial Power-On Test

1. Connect the radio (powered off initially).
2. Power on the HamMessenger device.
3. Verify the OLED display lights up and shows the HOME screen.
4. Confirm the GPS module acquires satellites (may take a few minutes outdoors).
5. Connect the radio cable and power on the radio.
6. Send a test beacon and verify it appears on [aprs.fi](https://aprs.fi).

---

## Build Reference Images

Photos from the build process are in `Media/Build Instructions/` and `Media/Install/`. Board photos are in `Media/Board-v1/` and `Media/Board-v1.1/`.

---

*Previous: [Hardware](Hardware) · Next: [Firmware Installation](Firmware-Installation)*
