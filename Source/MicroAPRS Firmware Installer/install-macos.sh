#!/bin/bash

# Path to the .hex file, relative to script location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HEX_FILE="$SCRIPT_DIR/microaprs-5v-ss-latest.hex"

# Use avrdude installed by Homebrew (auto-resolved from PATH)
AVRDUDE_BIN="avrdude"

# Use the default config that Homebrew avrdude knows about
# No need to specify -C unless you're using a custom config

# Set your serial port (adjust this!)
PORT="/dev/tty.usbserial-FT3VOZ4X"

# Flash it!
"$AVRDUDE_BIN" -c arduino -v -p ATMEGA328P -P "$PORT" -b 57600 -D -U flash:w:"$HEX_FILE":i
