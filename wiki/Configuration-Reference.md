# Configuration Reference

This page lists every configurable setting in HamMessenger, including the serial command path, valid range or values, and a description of what the setting does.

Settings can be changed in three ways:

- **On-device:** Navigate to the SETTINGS screen with the CardKB keyboard.
- **Serial command:** `CMD:Settings:<path>:<value>` (see [Serial Interface](Serial-Interface))
- **Desktop GUI:** The settings panel in the companion application.

After making changes, always run `CMD:Settings:Save:` or click **Save Settings** in the GUI to write values to EEPROM. Without saving, settings revert to their previous values on the next power cycle.

---

## Callsign & Identification

| Setting | Serial Path | Type | Max Length | Description |
|---------|------------|------|-----------|-------------|
| My Callsign | `Callsign:MyCallsign` | String | 6 chars | Your amateur radio callsign |
| My SSID | `Callsign:MySSID` | Integer | 0–15 | SSID suffix for your station (e.g., `9` for mobile) |
| Destination Callsign | `Callsign:DestCallsign` | String | 6 chars | APRS destination field callsign |
| Destination SSID | `Callsign:DestSSID` | Integer | 0–15 | APRS destination field SSID |

**Example:**

```
CMD:Settings:Callsign:MyCallsign:W1AW
CMD:Settings:Callsign:MySSID:9
```

---

## Beacon

| Setting | Serial Path | Type | Range | Description |
|---------|------------|------|-------|-------------|
| Beacon Enabled | `Beacon:Enabled` | Boolean | `true` / `false` | Enable or disable automatic position beaconing |
| Beacon Distance | `Beacon:Distance` | Float | miles | Distance (miles) traveled since last beacon before auto-transmitting |
| Beacon Idle Time | `Beacon:IdleTime` | Integer | 0–4,294,967,295 ms | Time (ms) of no movement before transmitting an idle beacon |
| Beacon Comment | `Beacon:Comment` | String | ≤99 chars | Text appended to your position beacon packet |
| Raw Packet | `Beacon:RawPacket` | String | ≤99 chars | Optional raw APRS data string sent with beacon |

**Example:**

```
CMD:Settings:Beacon:Enabled:true
CMD:Settings:Beacon:Distance:0.5
CMD:Settings:Beacon:IdleTime:600000
CMD:Settings:Beacon:Comment:HamMessenger v1.2
```

---

## APRS Path

APRS path settings control how your packets are routed through the APRS network.

| Setting | Serial Path | Type | Max Length | Description |
|---------|------------|------|-----------|-------------|
| PATH1 Callsign | `Path:Path1Callsign` | String | 6 chars | First path element callsign (e.g., `WIDE1`) |
| PATH1 SSID | `Path:Path1SSID` | Integer | 0–15 | First path SSID (e.g., `1` for `WIDE1-1`) |
| PATH2 Callsign | `Path:Path2Callsign` | String | 6 chars | Second path element callsign (e.g., `WIDE2`) |
| PATH2 SSID | `Path:Path2SSID` | Integer | 0–15 | Second path SSID (e.g., `1` for `WIDE2-1`) |

The most common setting for general use is `WIDE1-1,WIDE2-1`, which routes your packet through one local and one wide-area digipeater.

**Example:**

```
CMD:Settings:Path:Path1Callsign:WIDE1
CMD:Settings:Path:Path1SSID:1
CMD:Settings:Path:Path2Callsign:WIDE2
CMD:Settings:Path:Path2SSID:1
```

---

## Messaging

| Setting | Serial Path | Type | Range | Description |
|---------|------------|------|-------|-------------|
| Automatic ACK | `Message:AutoACK` | Boolean | `true` / `false` | Automatically send an acknowledgment when a message is received |
| Retry Count | `Message:RetryCount` | Integer | 0–65,535 | Number of times to retry sending a message with no ACK |
| Retry Interval | `Message:RetryInterval` | Integer | 0–65,535 ms | Time (ms) between message retry attempts |
| Recipient Callsign | `Message:RecipCallsign` | String | 6 chars | Default recipient callsign for new messages |
| Recipient SSID | `Message:RecipSSID` | Integer | 0–15 | Default recipient SSID |
| Message Text | `Message:Text` | String | 67 chars | Pre-filled message text |

**Example:**

```
CMD:Settings:Message:AutoACK:true
CMD:Settings:Message:RetryCount:5
CMD:Settings:Message:RetryInterval:30000
```

---

## Modem / Transmit Timing

| Setting | Serial Path | Type | Range | Description |
|---------|------------|------|-------|-------------|
| Preamble | `Modem:Preamble` | Integer | 0–65,535 ms | Duration (ms) of audio preamble before packet (allows repeater squelch to open) |
| Tail | `Modem:Tail` | Integer | 0–65,535 ms | Duration (ms) of audio after packet ends |

Higher preamble values improve reliability through digipeaters at the cost of slightly longer transmissions. A value of 300–500ms is typical.

**Example:**

```
CMD:Settings:Modem:Preamble:350
CMD:Settings:Modem:Tail:50
```

---

## APRS Symbol

| Setting | Serial Path | Type | Description |
|---------|------------|------|-------------|
| Symbol | `Symbol:Symbol` | Char | APRS symbol character (see APRS symbol chart) |
| Symbol Table | `Symbol:Table` | Char | `/` for primary table, `\` for alternate table |

Common symbols:

| Symbol | Table | Description |
|--------|-------|-------------|
| `>` | `/` | Car (mobile) |
| `[` | `/` | Human / pedestrian |
| `-` | `/` | House |
| `b` | `/` | Bicycle |

**Example:**

```
CMD:Settings:Symbol:Symbol:>
CMD:Settings:Symbol:Table:/
```

---

## GPS

| Setting | Serial Path | Type | Range | Description |
|---------|------------|------|-------|-------------|
| Position Tolerance | `GPS:Tolerance` | Float | degrees | Minimum change in position (degrees) to register as movement |
| Destination Latitude | `GPS:DestLat` | Float | ±90.0 | Target latitude for distance-to-destination calculation |
| Destination Longitude | `GPS:DestLon` | Float | ±180.0 | Target longitude for distance-to-destination calculation |

**Example:**

```
CMD:Settings:GPS:Tolerance:0.0001
CMD:Settings:GPS:DestLat:41.8781
CMD:Settings:GPS:DestLon:-87.6298
```

---

## Display

| Setting | Serial Path | Type | Range | Description |
|---------|------------|------|-------|-------------|
| Brightness | `Display:Brightness` | Integer | 0–100 | OLED brightness percentage |
| Timeout | `Display:Timeout` | Integer | 0–4,294,967,295 ms | Inactivity time before dimming (0 = never dim) |
| Invert | `Display:Invert` | Boolean | `true` / `false` | Invert display colors |
| Show Position | `Display:ShowPosition` | Boolean | `true` / `false` | Show GPS coordinates on HOME screen |
| Scroll Messages | `Display:ScrollMessages` | Boolean | `true` / `false` | Auto-scroll long messages on display |
| Scroll Speed | `Display:ScrollSpeed` | Integer | 0–65,535 ms | Delay (ms) between scroll steps; lower = faster |

**Example:**

```
CMD:Settings:Display:Brightness:80
CMD:Settings:Display:Timeout:30000
CMD:Settings:Display:Invert:false
CMD:Settings:Display:ShowPosition:true
```

---

## Saving and Restoring Settings

### Save to EEPROM

```
CMD:Settings:Save:
```

### Print All Current Settings

```
CMD:Settings:Print:
```

### Verify a Setting After Change

Change the setting, then immediately print to confirm:

```
CMD:Settings:Beacon:Distance:1.0
CMD:Settings:Print:
```

---

*Previous: [Serial Interface](Serial-Interface) · Next: [Troubleshooting](Troubleshooting)*
