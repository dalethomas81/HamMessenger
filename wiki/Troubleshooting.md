# Troubleshooting

This page covers the most common problems encountered with HamMessenger and how to resolve them.

---

## Display Issues

### OLED is blank after power-on

**Possible causes and fixes:**

- **Firmware not uploaded:** Verify the HamMessenger sketch was successfully uploaded to the Arduino Mega (see [Firmware Installation](Firmware-Installation)).
- **Loose I2C connection:** Check that the OLED's SDA and SCL lines are securely connected to the correct pins on the shield.
- **Wrong I2C address:** The SSD1106/SH1106 display may use address `0x3C` or `0x3D`. Open the Arduino IDE Serial Monitor and check for I2C address errors in startup output, or run an I2C scanner sketch to confirm the display's address and match it in the firmware.
- **Power issue:** Verify the battery module is charged and outputting 5V.

### Display shows garbled text or random pixels

- Re-flash the firmware. A partial upload can corrupt the sketch.
- Check for loose connections on the I2C lines.

### Display dims or turns off unexpectedly

- The **Display Timeout** setting may be active. Set it to `0` to disable auto-dim: `CMD:Settings:Display:Timeout:0`.

---

## GPS Issues

### GPS never acquires a fix

- **Indoor testing:** GPS requires a clear view of the sky. Move the device outdoors or near an unobstructed window.
- **Cold start:** After being off for an extended period, GPS can take 5–10 minutes to acquire a fix (Time to First Fix). Be patient.
- **Antenna:** The standard Neo-6M ceramic patch antenna has poor gain. If poor GPS performance persists, upgrade to a Neo-6M variant with a larger or external antenna.
- **Cable connection:** Verify the UART cable between the GPS module and the Arduino Mega's Serial1 port is secure.

### GPS coordinates are wildly wrong

- Verify the GPS module is receiving data at 9600 baud (the expected rate). A baud mismatch can cause garbled NMEA sentences and incorrect parsing.
- Ensure no other device is sharing the GPS serial port.

### Position drifts when stationary

- Increase the **Position Tolerance** setting to filter out GPS noise: `CMD:Settings:GPS:Tolerance:0.0001`.

---

## Radio and Packet Issues

### No packets received

- **Radio frequency:** Confirm the radio is tuned to 144.390 MHz (or your regional APRS frequency) with no CTCSS/DCS tones.
- **Audio connection:** Check that the radio's speaker output is connected to the modem's audio input.
- **Volume:** Adjust the radio volume (typically 75–80%). Too low or too high prevents correct decoding.
- **Modem firmware:** Verify MicroAPRS was successfully flashed to the Arduino Pro Mini (see [Firmware Installation](Firmware-Installation)).
- **Serial connection between Mega and Pro Mini:** Check continuity on the TX/RX lines between the two boards.

### Packets are received but not decoded correctly

- Check the audio level from the radio. Clipping (volume too high) causes decode failures.
- Verify MicroAPRS firmware version is current.
- Try `CMD:Modem:` pass-through commands to test the modem directly.

### No beacons transmitted / radio does not key up

- **PTT line:** Verify the PTT cable is connected from the shield to the radio's 2.5mm PTT jack.
- **Beacon Enabled setting:** Confirm beaconing is enabled: `CMD:Settings:Beacon:Enabled:true`.
- **GPS fix required:** The device will not transmit a position beacon without a valid GPS fix. Acquire a GPS fix first.
- **Ferrites:** RF from the antenna can interfere with the PTT control line. Ensure ferrite rings are installed on the PTT and audio cables.

### Device resets when transmitting

- This is almost always an RF interference issue. Add more ferrite rings to the audio and PTT cables, and increase the physical distance between the radio's antenna and the HamMessenger enclosure.
- Check power supply connections. A voltage dip during transmit (caused by the radio drawing current) can reset the Mega if the power supply is undersized.

### Beacons transmit but don't appear on aprs.fi

- You may be out of range of a digipeater or iGate. Check [aprs.fi](https://aprs.fi) for nearby APRS infrastructure.
- Verify your PATH settings include `WIDE1-1,WIDE2-1` for standard network routing.
- Your callsign and SSID must be correctly configured.

---

## Keyboard Issues

### CardKB not responding

- Check the I2C connection between the CardKB and the shield (SDA, SCL, VCC, GND).
- The CardKB uses I2C address `0x5F`. Verify no other device on the I2C bus conflicts.
- Try re-seating the keyboard connector.

---

## SD Card Issues

### SD card not recognized

- Ensure the card is formatted as **FAT32**.
- Try a different SD card. Not all cards are compatible with the Arduino SD library.
- Verify SPI connections (MOSI, MISO, SCK, CS) are correct and secure.

### Log files are not being created

- Confirm the SD card is inserted before powering on. The SD library initializes only at boot.
- Check available space on the card.

### Log file is very large

- Use `CMD:SD:Raw:Delete:` or `CMD:SD:Msg:Delete:` to clear the log files periodically.

---

## Desktop GUI Issues

### GUI does not detect the serial port

- Ensure HamMessenger is connected via USB before launching the application.
- On macOS, the device appears as `/dev/cu.usbmodem*`. On Windows, check Device Manager for the COM port number.
- Try a different USB cable or port.

### GUI connects but shows no data

- Verify baud rate is set to **115200**.
- Send `?` via the serial terminal to confirm the device responds.
- Check that the firmware is current.

### GUI crashes on launch (macOS)

- On first launch, you may need to right-click → Open to bypass macOS Gatekeeper.
- If running from source, ensure all dependencies from `requirements-macos.txt` are installed in your active Python environment.

---

## Firmware Issues

### Upload fails in Arduino IDE

- Select the correct board (**Arduino Mega or Mega 2560**) and processor (**ATmega2560**).
- Verify the correct COM/serial port is selected.
- Try pressing the Mega's reset button just as the upload begins.
- Check that no other application (serial monitor, GUI app) is holding the port open.

### MicroAPRS flash fails

- Verify the FTDI cable is wired correctly (TX↔RX crossed, matching voltage levels).
- Confirm the correct serial port is specified in the install script.
- Try lowering the baud rate in the AVRDude command (try `-b 19200`).

---

## General Tips

- **Serial monitor is your friend.** Connect at 115200 baud and type `?` to confirm the device is alive and responding.
- **Print settings after changes.** Use `CMD:Settings:Print:` to verify that settings were accepted.
- **Always save after configuring.** `CMD:Settings:Save:` commits changes to EEPROM.
- **Test outdoors.** GPS and RF behavior is best characterized in the field, not on a bench.

---

## Getting Help

If you encounter an issue not covered here:

- Search the [GitHub Issues](../../issues) page for similar reports.
- Open a new issue with details: device revision, firmware version, steps to reproduce, and serial output if applicable.

---

*Previous: [Configuration Reference](Configuration-Reference) · Back to [Home](Home)*
