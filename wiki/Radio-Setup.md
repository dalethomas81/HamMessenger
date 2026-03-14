# Radio Setup

HamMessenger interfaces with any FM transceiver that has a **Kenwood-style 2-pin connector** (2.5mm PTT jack + 3.5mm speaker/mic jack). The Baofeng UV-5R5 is the primary tested and recommended radio, but other compatible models work as well.

---

## Recommended Radio

**Baofeng UV-5R5** (or BF-F8HP, UV-5X3)

These radios are inexpensive, widely available, and well-supported. They use the standard Kenwood 2-pin connector and support the narrow bandwidth required for APRS.

Other radios with a Kenwood-compatible connector should also work. Radios with different connector standards require an appropriate adapter cable.

---

## APRS Frequency

In North America, the standard 2-meter APRS frequency is:

| Parameter | Value |
|-----------|-------|
| Frequency | **144.390 MHz** |
| Bandwidth | Narrow (NFM / WN) |
| CTCSS/DCS tone | **None** |
| Volume | ~80% |

> **Note:** Do not enable any CTCSS, DCS, or squelch tones. APRS packets must be transmitted and received without tone encoding.

International APRS frequencies differ by region. Common alternatives:

| Region | Frequency |
|--------|-----------|
| North America | 144.390 MHz |
| Europe | 144.800 MHz |
| Australia | 145.175 MHz |
| Japan | 144.640 MHz |

---

## Programming the Radio

### Option A — CHIRP (Recommended)

[CHIRP](https://chirp.app) is free, open-source radio programming software that supports most Baofeng models.

1. Download and install CHIRP.
2. Connect your radio to your computer via the programming cable (Kenwood connector to USB).
3. In CHIRP: **Radio → Download From Radio** to read the current channel list.
4. Add or edit a channel with the APRS frequency settings from the table above.
5. Set bandwidth to **Narrow**.
6. Leave all tone fields blank (no CTCSS/DCS).
7. **Radio → Upload To Radio** to write the changes.

### Option B — Manual Programming via Radio Keypad

On the Baofeng UV-5R5 (and similar models):

1. Press **VFO/MR** to enter VFO (frequency) mode.
2. Enter `144.390` using the keypad.
3. Press **MENU**, scroll to **WN** (Bandwidth), and set to **Narrow (WN)**.
4. Press **MENU** again, scroll to **T-CTCS** (transmit tone) and set to **OFF**.
5. Press **MENU** again, scroll to **R-CTCS** (receive tone) and set to **OFF**.
6. To save as a channel: press **MENU**, find **MEM-CH**, enter a channel number, press **MENU** to confirm.

Consult your radio's manual for exact menu navigation, as it varies slightly between Baofeng models.

---

## Connecting the Radio to HamMessenger

The radio connects to the HamMessenger shield via a split cable:

```
Radio (Kenwood 2-pin)
  ├── 2.5mm jack (PTT) ────────► PTT pin on shield
  └── 3.5mm jack (Speaker/Mic)
        ├── Speaker (audio out) ► Modem audio input
        └── Mic (audio in) ─────► Modem audio output
```

### Audio Levels

The modem expects audio at a level compatible with the radio's speaker output and mic input. If audio levels are too high or low, packet decode rates will suffer.

- **Too loud (clipping):** Reduce radio volume.
- **Too quiet:** Increase radio volume.
- A volume setting of around **75–80%** on the radio is a good starting point.

### PTT Behavior

When HamMessenger transmits a packet, it asserts the PTT line, which keys the radio's transmitter. The preamble (configurable, default ~300ms) plays before the packet to allow the repeater or digipeater to open its squelch.

---

## Verifying Radio Operation

### Receive Test

1. Power on HamMessenger with the radio connected and tuned to 144.390 MHz.
2. In an area with APRS activity, the device should receive and display incoming packets within a few minutes.
3. You can also monitor [aprs.fi](https://aprs.fi) to see nearby APRS stations that should be audible.

### Transmit Test

1. Configure your callsign and SSID (see [Configuration Reference](Configuration-Reference)).
2. Send a test beacon via the serial command `CMD:Beacon:` or the **Beacon** button in the desktop GUI.
3. Check [aprs.fi](https://aprs.fi) for your callsign — it should appear within a minute if digipeaters are in range.

> **Reminder:** Transmitting requires a valid amateur radio license.

---

## RF Interference Notes

Transmitting RF in close proximity to the microcontrollers can cause resets or spurious behavior. To mitigate this:

- Install **ferrite rings** on the PTT cable and audio cables (see [Building the Device](Building-the-Device)).
- Keep the radio antenna away from the HamMessenger enclosure where possible.
- If resets occur during transmit, add additional ferrites or increase the physical separation between the antenna and the device.

---

*Previous: [Firmware Installation](Firmware-Installation) · Next: [Operating the Device](Operating-the-Device)*
