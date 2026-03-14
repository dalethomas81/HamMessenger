# Firmware Installation

HamMessenger uses two separate microcontrollers, each requiring its own firmware:

| Microcontroller | Firmware | Method |
|----------------|----------|--------|
| Arduino Mega 2560 | HamMessenger sketch | Arduino IDE or USB |
| Arduino Pro Mini (modem) | MicroAPRS | FTDI cable + AVRDude |

Flash the modem first (it requires a separate FTDI cable), then flash the Mega via USB.

---

## Part 1 — Flash MicroAPRS to the Arduino Pro Mini

The MicroAPRS firmware turns the Arduino Pro Mini into an AFSK (Audio Frequency Shift Keying) modem that handles APRS packet encoding and decoding.

### Requirements

- FTDI USB-to-serial cable (5V — matches the Pro Mini's 5V logic level)
- AVRDude (command-line flashing tool)
- Pre-compiled MicroAPRS `.hex` file (included in the repo)

### Step 1 — Install AVRDude

**macOS (using Homebrew):**

```bash
brew install avrdude
```

**Windows:**

Download and install AVRDude from the WinAVR package or a standalone build. Extract to a directory and add it to your PATH, or use the full path when running commands.

### Step 2 — Connect the FTDI Cable

Connect the FTDI cable to the Arduino Pro Mini's programming header:

| FTDI Pin | Pro Mini Pin |
|----------|-------------|
| GND | GND |
| VCC (5V) | VCC |
| TXD | RXD |
| RXD | TXD |
| DTR | DTR (or RST via 0.1µF cap) |

### Step 3 — Identify the Serial Port

**macOS:**

```bash
ls /dev/cu.*
```

Look for a device like `/dev/cu.usbserial-XXXX`.

**Windows:**

Open Device Manager and look under "Ports (COM & LPT)" for the FTDI device (e.g., `COM3`).

### Step 4 — Run the Install Script

Navigate to `Source/MicroAPRS Firmware Installer/` in the repository.

**macOS:**

```bash
chmod +x install-macos.sh
./install-macos.sh
```

**Windows:**

```
install-windows.bat
```

If the script does not detect your port automatically, open it in a text editor and set the `PORT` variable to your serial port.

Alternatively, run AVRDude manually:

```bash
avrdude -c arduino -p m328p -P /dev/cu.usbserial-XXXX -b 57600 \
  -U flash:w:MicroAPRS-LATEST.hex:i
```

Replace `/dev/cu.usbserial-XXXX` with your port and `MicroAPRS-LATEST.hex` with the appropriate hex file from the folder.

### Step 5 — Verify

AVRDude will print output similar to:

```
avrdude: verifying ...
avrdude: 12288 bytes of flash verified
avrdude done.  Thank you.
```

If you see errors, check your cable connections and port selection.

> **Detailed instructions with screenshots** are also available in:
> `Source/MicroAPRS Firmware Installer/readme.md` and `Media/avrdude/`

---

## Part 2 — Flash HamMessenger Firmware to the Arduino Mega

### Option A — Arduino IDE (recommended for most users)

#### Step 1 — Install the Arduino IDE

Download the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software). Version 2.x is recommended.

#### Step 2 — Install Required Libraries

In the Arduino IDE, open **Sketch → Include Library → Manage Libraries** and install:

| Library | Notes |
|---------|-------|
| Adafruit GFX Library | Graphics core |

The following libraries are **custom-modified** and are included in the repository. Copy them into your Arduino `libraries` folder:

| Library | Source location |
|---------|-----------------|
| Adafruit_SH1106 | Included in repo |
| TinyGPSPlus (custom) | Included in repo |
| Base64 | Included in repo |

> The custom libraries differ from the stock versions. Do **not** install them via the Library Manager — use the versions from the repository.

#### Step 3 — Open the Sketch

Open `Source/HamMessenger/HamMessenger.ino` in the Arduino IDE.

#### Step 4 — Select Board and Port

1. **Tools → Board → Arduino AVR Boards → Arduino Mega or Mega 2560**
2. **Tools → Processor → ATmega2560**
3. **Tools → Port** — select the port for your Arduino Mega (e.g., `/dev/cu.usbmodem101` on macOS or `COM4` on Windows)

#### Step 5 — Upload

Click the **Upload** button (right-arrow icon) or press `Ctrl+U` (`Cmd+U` on macOS). The IDE will compile and upload the sketch. Upload progress appears in the output pane.

---

### Option B — Arduino CLI (for advanced users / CI)

Install the Arduino CLI from [arduino.github.io/arduino-cli](https://arduino.github.io/arduino-cli/).

```bash
# Install the AVR core
arduino-cli core install arduino:avr

# Compile
arduino-cli compile \
  --fqbn arduino:avr:mega \
  --libraries path/to/custom-libraries \
  Source/HamMessenger/HamMessenger.ino

# Upload
arduino-cli upload \
  --fqbn arduino:avr:mega \
  --port /dev/cu.usbmodem101 \
  Source/HamMessenger/HamMessenger.ino
```

---

### Option C — Pre-Built Hex File

If you want to flash without the Arduino IDE, download the pre-compiled `HamMessenger.hex` from the [Releases](../../releases) page and flash with AVRDude:

```bash
avrdude -c wiring -p m2560 -P /dev/cu.usbmodem101 -b 115200 -D \
  -U flash:w:HamMessenger.hex:i
```

---

## Verifying the Installation

After flashing the Mega:

1. The OLED display should show the HamMessenger **HOME** screen within a few seconds of power-on.
2. Open a serial monitor (115200 baud) and type `?` — the device should respond with a list of available commands.
3. If the display is blank or the serial port is unresponsive, double-check the library installation and re-upload.

---

## First-Time Configuration

After a fresh firmware flash, the EEPROM is empty. The device will start with default values. You should configure at minimum:

- Your callsign and SSID
- Beacon settings

See [Configuration Reference](Configuration-Reference) for all available settings and how to change them.

---

*Previous: [Building the Device](Building-the-Device) · Next: [Radio Setup](Radio-Setup)*
