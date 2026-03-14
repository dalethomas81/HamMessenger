# Operating the Device

This page covers day-to-day use of HamMessenger: navigating the on-device menus, sending and receiving messages, managing beacons, and using the SD card log.

---

## Powering On

Press the power button on the UPS battery module. The OLED display will illuminate and show the **HOME** screen after a brief startup. GPS acquisition begins automatically; a fix may take 1–5 minutes, especially in a new location or after the device has been off for an extended period.

---

## Keyboard Overview

HamMessenger uses the **M5Stack CardKB** keyboard for all input. Key functions:

| Key | Function |
|-----|----------|
| Arrow keys | Navigate menus |
| Enter | Select / confirm |
| Backspace / Delete | Delete characters |
| Fn + key | Access alternate characters (see CardKB documentation) |
| Alphanumeric keys | Type callsigns, messages, and values |

---

## Screen Navigation

The device has several screens. Navigate between them using the arrow keys and Enter.

### HOME Screen

The default screen shown on power-on. Displays:

- Current GPS coordinates (latitude/longitude)
- GPS fix status
- Last beacon timestamp
- Unread message count

From HOME, press **Enter** or use arrow keys to navigate to sub-screens.

### MESSAGES Screen

Lists received messages in reverse-chronological order. Each entry shows:

- Sender callsign and SSID
- Message text (scrolling if longer than the display width)
- Received timestamp

Navigate with arrow keys. Press **Enter** on a message to view the full text.

### NEW MESSAGE Screen

Compose and send an APRS text message:

1. Navigate to **Messages → New Message** from the HOME screen.
2. Enter the **recipient callsign** (up to 6 characters).
3. Enter the **recipient SSID** (0–15).
4. Type your **message text** (up to 67 characters, per APRS specification).
5. Press **Enter** to transmit.

The device will automatically retry the message at the configured retry interval until an acknowledgment (ACK) is received or the retry count is exhausted.

### BEACON Screen

Displays beacon status:

- Beacon enabled / disabled
- Distance since last beacon (miles)
- Beacon distance threshold
- Idle time countdown

Press **Enter** to immediately transmit a beacon.

### SETTINGS Screen

Browse and edit device settings. Navigate to a setting with arrow keys and press **Enter** to edit. Use the keyboard to enter new values and press **Enter** to confirm.

> **Tip:** Settings changed on-device take effect immediately but are stored in EEPROM only when you save. Use `CMD:Settings:Save:` via serial or the GUI's save button to persist changes across power cycles.

---

## Sending Messages

1. From HOME, navigate to **Messages → New Message**.
2. Enter the recipient's callsign (e.g., `W1AW`) and SSID (e.g., `9`).
3. Type the message (up to 67 characters).
4. Press **Enter** to send.

The device transmits the message and waits for an acknowledgment. If no ACK is received within the configured **Retry Interval**, it retransmits, up to the **Retry Count** limit. The OLED displays the current retry status.

---

## Receiving Messages

HamMessenger continuously listens for incoming APRS packets. When a message addressed to your callsign is received:

- The OLED display shows an alert.
- If **Automatic ACK** is enabled, the device replies with an acknowledgment automatically.
- The message is stored in the MESSAGES screen and logged to SD (if a card is present).

---

## Beaconing

### Automatic (Distance-Triggered) Beacon

When **Beacon Enabled** is `true`, the device monitors your GPS position. When you have traveled more than the **Beacon Distance** threshold since the last beacon, it transmits your position automatically.

The beacon packet includes:

- Your callsign and SSID
- Current latitude and longitude
- Speed and course (if GPS has a fix)
- Configurable comment text
- Configurable APRS symbol

### Idle Beacon

If you remain stationary for longer than the **Beacon Idle Time**, the device transmits a beacon regardless of distance traveled. This ensures your station remains visible on the APRS network even when parked.

### Manual Beacon

Transmit a beacon immediately at any time:

- On-device: Navigate to the BEACON screen and press **Enter**.
- Via serial: Send `CMD:Beacon:`.
- Via desktop GUI: Click the **Beacon** button.

---

## SD Card Logging

Insert a FAT32-formatted microSD card before powering on. The device creates and appends to two files:

| File | Contents |
|------|----------|
| `msg.txt` | Received APRS messages (decoded) |
| `raw.txt` | All raw APRS packets received |

Log files can be read or deleted via serial commands (see [Serial Interface](Serial-Interface)):

```
CMD:SD:Msg:Print:      — Print message log to serial
CMD:SD:Msg:Delete:     — Delete message log
CMD:SD:Raw:Print:      — Print raw packet log to serial
CMD:SD:Raw:Delete:     — Delete raw packet log
```

> **Note:** If the SD card fills up, the device will not gracefully truncate old entries in this version. Clear the logs periodically.

---

## Display Settings

| Setting | Description |
|---------|-------------|
| Brightness | Adjust OLED brightness (0–100%) |
| Timeout | Time before the display dims (ms); 0 = always on |
| Invert | Invert display colors |
| Show Position | Show GPS coordinates on HOME screen |
| Scroll Messages | Automatically scroll long messages |
| Scroll Speed | Delay between scroll steps (ms) |

---

## Monitoring on APRS.fi

Once your beacon has been transmitted and heard by a digipeater or iGate, your station will appear on [aprs.fi](https://aprs.fi). Search for your callsign to view your position on the map, track your path history, and see your beacon comment.

---

## Screen Navigation Reference

For a complete guide to every menu and screen layout, see:

```
Documentation/Operating Instructions/Screen Navigation.md
```

---

*Previous: [Radio Setup](Radio-Setup) · Next: [Desktop GUI](Desktop-GUI)*
