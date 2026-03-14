# Serial Interface

HamMessenger exposes a full command interface over USB serial at **115200 baud**. You can use any serial terminal (Arduino IDE Serial Monitor, PuTTY, `screen`, etc.) or the [Desktop GUI](Desktop-GUI) to send commands and receive output.

---

## Connection Settings

| Parameter | Value |
|-----------|-------|
| Baud rate | 115,200 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Line ending | Newline (`\n`) |

---

## Command Format

All commands follow this general pattern:

```
CMD:<category>:<sub-command>:<value>
```

Terminate each command with a newline. The device echoes the command and responds with a result or confirmation.

---

## Help

```
?
```

Prints a quick-reference list of command examples to the serial monitor.

---

## Settings Commands

### Print All Settings

```
CMD:Settings:Print:
```

Outputs all current settings and their values in a readable format. Useful for verifying your configuration.

### Save Settings to EEPROM

```
CMD:Settings:Save:
```

Writes all current settings to EEPROM so they persist across power cycles. **Always run this after making configuration changes you want to keep.**

### Change a Setting

Settings are addressed by their full path. The general form is:

```
CMD:Settings:<SettingPath>:<value>
```

See the [Configuration Reference](Configuration-Reference) page for the full list of setting paths and valid values.

**Examples:**

```
CMD:Settings:Callsign:MyCallsign:W1AW
CMD:Settings:Callsign:MySSID:9
CMD:Settings:Beacon:Enabled:true
CMD:Settings:Beacon:Distance:0.5
CMD:Settings:Display:Brightness:75
```

---

## Beacon Commands

### Immediate Beacon Transmit

```
CMD:Beacon:
```

Transmits a position beacon immediately using the current GPS fix and configured comment/symbol. Does not affect the automatic beacon timer.

---

## Messaging Commands

### Send a Message

```
CMD:Message:<RecipientCallsign>:<RecipientSSID>:<MessageText>
```

Sends an APRS text message to the specified station. The device will retry delivery according to the configured Retry Count and Retry Interval.

**Example:**

```
CMD:Message:W1AW:9:Hello from HamMessenger!
```

---

## SD Card Commands

### Print Message Log

```
CMD:SD:Msg:Print:
```

Outputs the contents of `msg.txt` (the received messages log) to the serial terminal.

### Delete Message Log

```
CMD:SD:Msg:Delete:
```

Deletes `msg.txt` from the SD card. This cannot be undone.

### Print Raw Packet Log

```
CMD:SD:Raw:Print:
```

Outputs the contents of `raw.txt` (the raw APRS packet log) to the serial terminal.

### Delete Raw Packet Log

```
CMD:SD:Raw:Delete:
```

Deletes `raw.txt` from the SD card. This cannot be undone.

---

## Modem (MicroAPRS) Pass-Through

```
CMD:Modem:<command>
```

Passes a raw command directly to the MicroAPRS modem on the Arduino Pro Mini. This is useful for diagnostics or sending raw APRS frames.

Refer to the [MicroAPRS documentation](https://github.com/markqvist/MicroAPRS) for supported modem commands.

---

## Example Session

The following shows a typical serial session after connecting:

```
> ?
[device prints command list]

> CMD:Settings:Print:
Callsign: W1AW
SSID: 9
Beacon Enabled: true
Beacon Distance: 0.50
...

> CMD:Settings:Callsign:MyCallsign:W1AW
OK

> CMD:Settings:Beacon:Enabled:true
OK

> CMD:Settings:Save:
Settings saved to EEPROM.

> CMD:Beacon:
Transmitting beacon...

> CMD:Message:KD9XYZ:0:Are you copying?
Message queued. Retry 1/5...
ACK received from KD9XYZ-0.
```

---

## Tips

- Always send `CMD:Settings:Save:` after a configuration session, or your changes will be lost on the next power cycle.
- The `CMD:Settings:Print:` command is helpful before and after configuration changes to verify values were accepted.
- If a command is not recognized, the device will respond with an error message indicating the unrecognized path.

---

*Previous: [Desktop GUI](Desktop-GUI) · Next: [Configuration Reference](Configuration-Reference)*
