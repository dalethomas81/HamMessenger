# Desktop GUI

The HamMessenger desktop application provides a graphical interface for monitoring APRS activity, sending messages, adjusting settings, and viewing your position on a map — all over a USB serial connection to the device.

The GUI is built with Python and PySide6 (Qt6) and runs on Windows and macOS.

---

## Installation

### Option A — Pre-Built Installer (Recommended)

Download the latest installer from the [Releases](../../releases) page:

| Platform | File |
|----------|------|
| Windows | `HamMessenger-windows.exe` |
| macOS | `HamMessenger-macos.dmg` |

**Windows:** Run the `.exe` installer and follow the prompts.

**macOS:** Open the `.dmg`, drag HamMessenger to your Applications folder, and launch it. On first launch you may need to right-click → Open to bypass Gatekeeper.

---

### Option B — Run from Source

#### Requirements

- Python 3.11 or later
- pip

#### Steps

1. Clone or download the repository.

2. Navigate to the GUI source directory:

   ```bash
   cd Source/GUI
   ```

3. Create a virtual environment (recommended):

   ```bash
   python3 -m venv venv
   source venv/bin/activate        # macOS/Linux
   venv\Scripts\activate.bat       # Windows
   ```

4. Install dependencies:

   **macOS:**
   ```bash
   pip install -r requirements-macos.txt
   ```

   **Windows:**
   ```bash
   pip install -r requirements-windows.txt
   ```

5. Launch the application:

   ```bash
   python3 Qt.py
   ```

---

## Connecting to the Device

1. Connect HamMessenger to your computer via USB.
2. Launch the desktop application.
3. In the **Port** dropdown, select the serial port for your device:
   - macOS: `/dev/cu.usbmodem101` (or similar)
   - Windows: `COM3` (or similar — check Device Manager)
4. Ensure the **Baud Rate** is set to `115200`.
5. Click **Connect**.

The status indicator will turn green when the connection is established. Incoming packets and device messages will immediately begin appearing in the **Log** tab.

---

## Interface Overview

The application window has two main tabs and a command panel.

### Log Tab

The Log tab displays a live feed of events from the device:

- Received APRS messages (decoded)
- Raw packet data
- Beacon transmissions
- System events (connection, errors, settings changes)

Each log entry is timestamped. The log auto-scrolls to the latest entry; you can scroll up to review history.

### Map Tab

The Map tab shows a web-based APRS map (powered by [aprs.fi](https://aprs.fi)) centered on your current position. As beacons are transmitted, the map updates to reflect your latest reported location.

> An internet connection is required for the map to load.

### Command Panel

The command panel (side or bottom of the window, depending on platform) provides quick-access buttons and input fields for common actions:

| Control | Function |
|---------|----------|
| **Beacon** button | Immediately transmit a position beacon |
| **Send Message** | Address and send an APRS text message |
| **Settings** | View and edit device configuration |
| **Save Settings** | Write current settings to EEPROM |
| **Print Settings** | Display all current settings in the log |

---

## Sending a Message via GUI

1. In the command panel, locate the **Send Message** section.
2. Enter the recipient callsign and SSID.
3. Type the message text (up to 67 characters).
4. Click **Send** or press Enter.

The device will acknowledge receipt in the Log tab and display retry attempts if no ACK is received from the recipient.

---

## Adjusting Settings via GUI

The GUI exposes all device settings in a form or table. Changes are sent to the device immediately via serial commands. To make changes persistent across power cycles, click **Save Settings** — this writes all settings to the device's EEPROM.

See [Configuration Reference](Configuration-Reference) for the full list of settings and their valid ranges.

---

## Configuration File

The GUI stores its own preferences (last-used port, baud rate, window layout) in:

```
Source/GUI/ham_gui_config.json
```

You can edit this file manually if needed, or it will be updated automatically by the application.

---

## Dark Mode

The application respects your operating system's dark/light mode preference and adjusts the theme automatically.

---

## Building a Standalone Executable

To build a self-contained executable from source (for distribution without requiring Python):

**Windows:**

```bash
pip install pyinstaller
pyinstaller --noconfirm --windowed --name HamMessenger --onefile Qt.py
```

**macOS:**

```bash
pip install pyinstaller create-dmg
python -m PyInstaller --windowed --name HamMessenger --onefile Qt.py
create-dmg --volname "HamMessenger" --window-size 600 400 \
  HamMessenger.dmg dist/HamMessenger.app
```

The `HamMessenger.spec` file in `Source/GUI/` contains saved PyInstaller settings for reproducible builds. GitHub Actions uses this automatically on every release.

---

*Previous: [Operating the Device](Operating-the-Device) · Next: [Serial Interface](Serial-Interface)*
