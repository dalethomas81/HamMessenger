import tkinter as tk
from tkinter import scrolledtext, ttk, simpledialog, messagebox, filedialog
import serial
import serial.tools.list_ports
import threading
import time
import sys
import os
import json
from datetime import datetime
from tkinter.ttk import Style
import re
from aprspy import APRS
import base64

from tkinter import ttk as ttk_gui  # Avoid conflict with existing ttk
from tkintermapview import TkinterMapView

import platform

os_name = platform.system()

'''
if os_name == "Darwin":
    # macOS specific code
    print("Running on macOS")
elif os_name == "Windows":
    # Windows specific code
    print("Running on Windows")
elif os_name == "Linux":
    # Linux specific code
    print("Running on Linux")
'''

# Shared Queue for incoming complete messages
from queue import Queue
message_queue = Queue()

# python -m pip install pyserial
# python -m pip install tkintermapview
# python -m pip install bitstring
# python -m pip install aprspy


# ---------- Path Setup ----------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
config_file = os.path.join(BASE_DIR, "ham_gui_config.json")
log_dir = os.path.join(BASE_DIR, "logs")

def resource_path(relative_path):
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.dirname(os.path.abspath(__file__))

    return os.path.join(base_path, relative_path)

# ---------- Globals ----------
ser = None
connected = False
default_quick_commands = ["?"
                          ,"CMD:Settings:Print:"
                          ,"CMD:Settings:Save:"
                          ,"CMD:Beacon:"
                          ,"CMD:Message:<Recipient Callsign>:<Recipient SSID>:<Message>"
                          ,"CMD:SD:Raw:Print:"
                          ,"CMD:SD:Raw:Delete:"
                          ,"CMD:SD:Msg:Print:"
                          ,"CMD:SD:Msg:Delete:"
                          ,"CMD:Modem:<command https://github.com/markqvist/MicroAPRS#serial-commands>"
                          ,"CMD:Modem:!<data Send raw packet>"
                          ,"CMD:Modem:V<1/0 Silent Mode>"
                          ,"CMD:Modem:v<1/0 Verbose Mode>"
                          ,"CMD:Settings:APRS:Beacon Frequency:<0 to 4,294,967,295>"
                          ,"CMD:Settings:APRS:Raw Packet:<alphanumeric 99 char max>"
                          ,"CMD:Settings:APRS:Comment:<alphanumeric 99 char max>"
                          ,"CMD:Settings:APRS:Message:<alphanumeric 99 char max>"
                          ,"CMD:Settings:APRS:Recipient Callsign:<alphanumeric 6 char max>"
                          ,"CMD:Settings:APRS:Recipient SSID:<alphanumeric 2 char max>"
                          ,"CMD:Settings:APRS:My Callsign:<alphanumeric 6 char max>"
                          ,"CMD:Settings:APRS:Callsign SSID:<alphanumeric 2 char max>"
                          ,"CMD:Settings:APRS:Dest Callsign:<alphanumeric 6 char max>"
                          ,"CMD:Settings:APRS:Dest SSID:<alphanumeric 2 char max>"
                          ,"CMD:Settings:APRS:PATH1 Callsign:<alphanumeric 6 char max>"
                          ,"CMD:Settings:APRS:PATH1 SSID:<alphanumeric 2 char max>"
                          ,"CMD:Settings:APRS:PATH2 Callsign:<alphanumeric 6 char max>"
                          ,"CMD:Settings:APRS:PATH2 SSID:<alphanumeric 2 char max>"
                          ,"CMD:Settings:APRS:Symbol:<alphanumeric 1 char max>"
                          ,"CMD:Settings:APRS:Table:<alphanumeric 1 char max>"
                          ,"CMD:Settings:APRS:Automatic ACK:<True/False>"
                          ,"CMD:Settings:APRS:Preamble:<0 to 65,535>"
                          ,"CMD:Settings:APRS:Tail:<0 to 65,535>"
                          ,"CMD:Settings:APRS:Retry Count:<0 to 65,535>"
                          ,"CMD:Settings:APRS:Retry Interval:<0 to 65,535>"
                          ,"CMD:Settings:GPS:Update Freq:<0 to 4,294,967,295>"
                          ,"CMD:Settings:GPS:Pos Tolerance:<0-100%>"
                          ,"CMD:Settings:GPS:Dest Latitude:<-3.4028235E+38 to 3.4028235E+38>"
                          ,"CMD:Settings:GPS:Dest Longitude:<-3.4028235E+38 to 3.4028235E+38>"
                          ,"CMD:Settings:Display:Timeout:<0 to 4,294,967,295>"
                          ,"CMD:Settings:Display:Brightness:<0 to 100>"
                          ,"CMD:Settings:Display:Show Position:<True/False>"
                          ,"CMD:Settings:Display:Scroll Messages:<True/False>"
                          ,"CMD:Settings:Display:Scroll Speed:<0 to 65,535>"
                          ,"CMD:Settings:Display:Invert:<True/False>"]
config = {
    "port": "",
    "baud": "115200",
    "quick_commands": default_quick_commands
}
history = []
history_index = -1
log_entries = []
auto_scroll_enabled = True
filter_mode = "All"
dark_mode = False

symbol_map = {
    # Primary Table ('/')
    "/!": "Emergency",
    "/\"": "Reserved (not used)",
    "/#": "Digipeater",
    "/$": "Phone",
    "/%": "DX Cluster",
    "/&": "HF Gateway",
    "/'": "Small Aircraft",
    "/(": "Mobile Satellite Station",
    "/)": "Wheelchair",
    "/*": "Snowmobile",
    "/+": "Red Cross",
    "/,": "Boy Scout",
    "/-": "House (QTH)",
    "/.": "X — Overlay position",
    "/0": "Circle (Node)",
    "/1": "ARC",
    "/2": "Bicycle",
    "/3": "Church",
    "/4": "Campground",
    "/5": "QTH with phone",
    "/6": "iGate",
    "/7": "Aircraft (large)",
    "/8": "Boat",
    "/9": "Motorcycle",
    "/:": "Fire Dept",
    "/;": "Police Station",
    "/<": "Truck",
    "/=": "RV",
    "/>": "Car",
    "/?": "Info Kiosk",
    "@/": "JOTA (Scouts)",
    "/A": "Ambulance",
    "/B": "BBS",
    "/C": "Computer",
    "/D": "Delivery Truck",
    "/E": "Eyeball (Meeting)",
    "/F": "Satellite",
    "/G": "GPS Receiver",
    "/H": "Hospital",
    "/I": "TNC",
    "/J": "Jeep",
    "/K": "School",
    "/L": "Laptop",
    "/M": "Mic/Echolink",
    "/N": "NTS Traffic",
    "/O": "Balloon",
    "/P": "Parking",
    "/Q": "ATV",
    "/R": "Repeater",
    "/S": "Ship",
    "/T": "Truck Stop",
    "/U": "Bus",
    "/V": "Van",
    "/W": "Water Station",
    "/X": "Helicopter",
    "/Y": "Yacht",
    "/Z": "Winlink",
    
    # Alternate Table ('\\')
    "\\!": "Police Car",
    "\\\"": "Reserved",
    "\\#": "Smoke Detector",
    "\\$": "Cash Register",
    "\\%": "Power Plant",
    "\\&": "Topo Map",
    "\\'": "Crash Site",
    "\\(": "Cloudy",
    "\\)": "Rain",
    "\\*": "Snow",
    "\\+": "Church (alt)",
    "\\,": "Girl Scout",
    "\\-": "QSL Card",
    "\\.": "Ambulance (alt)",
    "\\0": "Circle+overlay",
    "\\1": "Power Outage",
    "\\2": "Tornado",
    "\\3": "Flood",
    "\\4": "Solar Power",
    "\\5": "Tsunami",
    "\\6": "Civil Defense",
    "\\7": "Hazard",
    "\\8": "Radiation",
    "\\9": "Biohazard",
    "\\:": "Fog",
    "\\;": "Snowstorm",
    "\\<": "Hurricane",
    "\\=": "Volcano",
    "\\>": "Lightning",
    "\\?": "Dust",
    "\\A": "Box/Package",
    "\\B": "Blowing Snow",
    "\\C": "Coastal Flood",
    "\\D": "Drizzle",
    "\\E": "Freezing Rain",
    "\\F": "Funnel Cloud",
    "\\G": "Gale",
    "\\H": "Hail",
    "\\I": "Icy Roads",
    "\\J": "Jackknife",
    "\\K": "Blizzard",
    "\\L": "Low Visibility",
    "\\M": "Moon",
    "\\N": "News Station",
    "\\O": "Balloon (alt)",
    "\\P": "Pick-up Truck",
    "\\Q": "Quake",
    "\\R": "Rocket",
    "\\S": "Sleet",
    "\\T": "Thunderstorm",
    "\\U": "Sun",
    "\\V": "VHF Station",
    "\\W": "Flooding",
    "\\X": "X-ray",
    "\\Y": "Yagi Antenna",
    "\\Z": "Zombie (!)"
}
emoji_map = {
    # Primary Symbol Table ('/')
    "/!": "🚨",  # Emergency
    "/#": "📶",  # Digipeater
    "/$": "📞",  # Phone
    "/%": "🗼",  # DX Cluster
    "/&": "🌐",  # HF Gateway
    "/'": "🛩️",  # Small aircraft
    "/(": "🛰️",  # Mobile satellite
    "/)": "♿",  # Wheelchair
    "/*": "🏂",  # Snowmobile
    "/+": "➕",  # Red Cross
    "/,": "🏕️",  # Boy Scout
    "/-": "🏠",  # Home / QTH
    "/.": "❌",  # Overlay position
    "/0": "⭕",  # Circle (Node)
    "/1": "🏢",  # Icon (generic)
    "/2": "🚴",  # Bicycle
    "/3": "⛪",  # Church
    "/4": "⛺",  # Campground
    "/5": "📱",  # QTH with phone
    "/6": "📡",  # iGate
    "/7": "✈️",  # Large aircraft
    "/8": "⛵",  # Boat
    "/9": "🏍️",  # Motorcycle
    "/:": "🚒",  # Fire Dept
    "/;": "🚓",  # Police Station
    "/<": "🚚",  # Truck
    "/=": "🚐",  # RV
    "/>": "🚗",  # Car
    "/?": "ℹ️",  # Info Kiosk
    "/A": "🚑",  # Ambulance
    "/B": "📦",  # BBS
    "/C": "🖥️",  # Computer
    "/D": "📦",  # Delivery Truck
    "/E": "👀",  # Eyeball / Event
    "/F": "🛰️",  # Satellite
    "/G": "📍",  # GPS Receiver
    "/H": "🏥",  # Hospital
    "/I": "📡",  # TNC
    "/J": "🚙",  # Jeep
    "/K": "🏫",  # School
    "/L": "💻",  # Laptop
    "/M": "🎙️",  # Mic / EchoLink
    "/N": "📨",  # NTS Traffic
    "/O": "🎈",  # Balloon
    "/P": "🅿️",  # Parking
    "/Q": "📺",  # ATV (TV)
    "/R": "📡",  # Repeater
    "/S": "🚢",  # Ship
    "/T": "⛽",  # Truck Stop
    "/U": "🚌",  # Bus
    "/V": "🚐",  # Van
    "/W": "🚰",  # Water Station
    "/X": "🚁",  # Helicopter
    "/Y": "🛥️",  # Yacht
    "/Z": "📬",  # Winlink

    # 
    "/[": "🚶",  # Walking person
    "/]": "🏃",  # Running person
    "/{": "🎒",  # Hiking
    "/}": "🎿",  # Skier
    "/|": "🗼",  # Tower
    "/\\": "💻",  # PC station
    "/^": "📻",  # Ham shack
    "/_": "🛶",  # Watercraft
    "/`": "🌦️",  # Alt weather
    "/~": "🏠",  # House alt
    "/b": "🚴",  # Bicycle

    # Alternate Symbol Table ('\\')
    "\\!": "🚓",  # Police car
    "\\#": "🔥",  # Smoke detector
    "\\$": "💵",  # Cash register
    "\\%": "🏭",  # Power Plant
    "\\&": "🗺️",  # Topo Map
    "\\'": "💥",  # Crash Site
    "\\(": "☁️",  # Cloudy
    "\\)": "🌧️",  # Rain
    "\\*": "❄️",  # Snow
    "\\+": "✝️",  # Church (alt)
    "\\,": "👧",  # Girl Scout
    "\\-": "📮",  # QSL Card
    "\\0": "⭘",  # Overlay circle
    "\\1": "💡",  # Power outage
    "\\2": "🌪️",  # Tornado
    "\\3": "🌊",  # Flood
    "\\4": "🔋",  # Solar Power
    "\\5": "🌊",  # Tsunami
    "\\6": "🏛️",  # Civil Defense
    "\\7": "☢️",  # Hazard
    "\\8": "☢️",  # Radiation
    "\\9": "☣️",  # Biohazard
    "\\:": "🌫️",  # Fog
    "\\;": "🌨️",  # Snowstorm
    "\\<": "🌀",  # Hurricane
    "\\=": "🌋",  # Volcano
    "\\>": "🌩️",  # Lightning
    "\\?": "💨",  # Dust
    "\\A": "📦",  # Box/Package
    "\\B": "🌬️",  # Blowing snow
    "\\C": "🌊",  # Coastal Flood
    "\\D": "🌦️",  # Drizzle
    "\\E": "🌧️",  # Freezing Rain
    "\\F": "🌪️",  # Funnel Cloud
    "\\G": "🌬️",  # Gale
    "\\H": "🌨️",  # Hail
    "\\I": "🧊",  # Icy Roads
    "\\J": "🚛",  # Jackknife
    "\\K": "🌨️",  # Blizzard
    "\\L": "🌁",  # Low Visibility
    "\\M": "🌕",  # Moon
    "\\N": "📰",  # News Station
    "\\O": "🎈",  # Balloon (alt)
    "\\P": "🛻",  # Pickup Truck
    "\\Q": "🌎",  # Earthquake
    "\\R": "🚀",  # Rocket
    "\\S": "🌨️",  # Sleet
    "\\T": "⛈️",  # Thunderstorm
    "\\U": "☀️",  # Sun
    "\\V": "📡",  # VHF Station
    "\\W": "🌊",  # Flooding
    "\\X": "☢️",  # X-ray (symbol)
    "\\Y": "📡",  # Yagi antenna
    "\\Z": "🧟",  # Zombie

    # General fallbacks
    "/#": "📶",  # Digipeater
    "/G": "📍",  # GPS Receiver
    "/I": "📡",  # TNC
    "/?": "ℹ️",  # Info Kiosk

    # Catch-all fallback
    "default": "📍",
}

# ---------- Config Functions ----------
def load_config():
    global config
    if os.path.exists(config_file):
        with open(config_file, "r") as f:
            config.update(json.load(f))

def save_config():
    with open(config_file, "w") as f:
        json.dump(config, f, indent=2)

# ---------- Serial Functions ----------
def list_serial_ports():
    return [port.device for port in serial.tools.list_ports.comports()]

def refresh_ports():
    ports = list_serial_ports()
    port_menu['values'] = ports
    if ports:
        if config.get("port") in ports:
            port_var.set(config["port"])
        else:
            port_var.set(ports[0])
    else:
        port_var.set("")

def connect_serial():
    global ser, connected
    port = port_var.get()
    baud = int(baud_var.get())
    try:
        ser = serial.Serial(port, baud, timeout=0.5)
        connected = True
        config["port"] = port
        config["baud"] = str(baud)
        save_config()
        status_label.config(text=f"Connected to {port} @ {baud} baud", fg="green")
        threading.Thread(target=read_serial, daemon=True).start()
        threading.Thread(target=process_messages, daemon=True).start()
        threading.Thread(target=monitor_connection, daemon=True).start()
    except Exception as e:
        status_label.config(text=f"Connection failed: {e}", fg="red")
        connected = False

def monitor_connection():
    global connected
    while True:
        if ser and not ser.is_open:
            connected = False
            status_label.config(text="Disconnected", fg="red")
            try_reconnect()
            break
        time.sleep(2)

def try_reconnect():
    for attempt in range(1, 6):
        status_label.config(text=f"Reconnecting... (attempt {attempt})", fg="orange")
        time.sleep(2)
        try:
            connect_serial()
            if connected:
                return
        except:
            continue
    status_label.config(text="Reconnection failed", fg="red")

# ---------- Logging Functions ----------
def get_log_filename():
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)
    return os.path.join(log_dir, datetime.now().strftime("%Y-%m-%d") + ".txt")

def log_to_file(text):
    with open(get_log_filename(), "a", encoding="utf-8") as f:
        f.write(text)

def open_log_file():
    file_path = filedialog.askopenfilename(initialdir=log_dir, title="Open Log File",
                                           filetypes=[("Text files", "*.txt")])
    if file_path:
        with open(file_path, "r", encoding="utf-8") as f:
            log_box.config(state=tk.NORMAL)
            log_box.delete(1.0, tk.END)
            log_box.insert(tk.END, f.read())
            log_box.config(state=tk.DISABLED)
            log_box.see(tk.END)

def clear_log():
    global log_entries
    log_entries = []
    update_log_display()

def update_log_display():
    global log_entries
    max_entries = 1000
    if len(log_entries) > max_entries:
        log_entries = log_entries[-max_entries:]
    log_box.config(state=tk.NORMAL)
    log_box.delete(1.0, tk.END)
    for entry in log_entries:
        if filter_mode == "All" or entry["tag"] == filter_mode:
            log_box.insert(tk.END, entry["text"], entry["tag"])
    if auto_scroll_enabled:
        log_box.see(tk.END)
    log_box.config(state=tk.DISABLED)

def append_to_log(entry):
    if filter_mode == "All" or entry["tag"] == filter_mode:
        log_box.config(state=tk.NORMAL)
        log_box.insert(tk.END, entry["text"], entry["tag"])
        if auto_scroll_enabled:
            log_box.see(tk.END)
        log_box.config(state=tk.DISABLED)

#
def parse_aprs_position(data_str):
    """
    Extract latitude and longitude from an APRS packet DATA field.
    Supports uncompressed and timestamped formats.
    Handles cases with malformed extra characters like 'NS' or 'EW'.
    """
    # Pattern A — Uncompressed position
    match = re.search(
        r'([0-9]{2})([0-9]{2}\.[0-9]+)([NS])[^0-9]*([0-9]{3})([0-9]{2}\.[0-9]+)([EW])',
        data_str
    )
    if match:
        lat = int(match.group(1)) + float(match.group(2)) / 60.0
        if match.group(3) == 'S': lat *= -1
        lon = int(match.group(4)) + float(match.group(5)) / 60.0
        if match.group(6) == 'W': lon *= -1
        return lat, lon

    # Pattern B — Timestamped uncompressed position (e.g. @060021z3633.21N12155.60W)
    match = re.search(
        r'(?:@\d{6}z)?([0-9]{2})([0-9]{2}\.[0-9]+)([NS])[^0-9]*([0-9]{3})([0-9]{2}\.[0-9]+)([EW])',
        data_str
    )
    if match:
        lat = int(match.group(1)) + float(match.group(2)) / 60.0
        if match.group(3) == 'S': lat *= -1
        lon = int(match.group(4)) + float(match.group(5)) / 60.0
        if match.group(6) == 'W': lon *= -1
        return lat, lon

    return None

def parse_raw_modem_fields(line: str) -> dict:
    pattern = (
        r'(?:Modem Raw:|SD Raw:)\s*'                   # allow “Modem Raw:” or “SD Raw:”
        r'SRC:\s*\[(?P<SRC>[^\]]+)\]\s*'               # SRC:[…]
        r'DST:\s*\[(?P<DST>[^\]]+)\]\s*'               # DST:[…]
        r'PATH:\s*(?P<PATH>(?:\[[^\]]+\]\s*)+)\s*'     # one or more PATH:[…] groups
        r'DATA:\s*(?P<DATA>.*)'                        # everything after DATA:
    )
    m = re.match(pattern, line, re.IGNORECASE)
    if not m:
        return {}
    fields = m.groupdict()
    fields['PATH'] = ",".join(re.findall(r'\[([^\]]+)\]', fields['PATH']))
    
    return fields

def decode_aprs(line):
    fields = parse_raw_modem_fields(line)
    if fields:
        # KN6ARG-9>SWQTWR,WIDE1-1:`2Z5lr|j/`"7I}146.520MHz_1
        message = fields['SRC'] + '>' + fields['DST'] + ',' + fields['PATH'] + ':' + fields['DATA']
        try:
            packet = APRS.parse(message)
            return packet
        except:
            pass

    return None

# ---------- Serial I/O ----------
def send_serial(message=None):
    global history, history_index
    if not connected:
        return
    if message:
        timestamp = time.strftime("%H:%M:%S")
        line = f"[{timestamp}] → {message}\n"
        log_entries.append({"text": line, "type": "Sent", "tag": "Sent"})
        log_to_file(line)
        ser.write((message + '\n').encode('utf-8'))
        #update_log_display()
        append_to_log(log_entries[-1])

def read_serial():
    buffer = ""
    while connected:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            buffer += data

            while '\r' in buffer:
                line, buffer = buffer.split('\r', 1)
                message_queue.put(line.strip())  # Push line to queue

        # adding just 1ms here drastically reduces CPU usage
        time.sleep(0.001) # milliseconds

def process_messages():
    while True:
        line = message_queue.get()
        if line is None:
            break  # Exit signal

        handle_line(line)
        message_queue.task_done()

        #
        time.sleep(0.001) # milliseconds

markers_by_source = {}
def handle_line(line):
    try:
        #
        timestamp = time.strftime("%H:%M:%S")

        #
        if re.search(r"^Modem Raw:SRC:", line, re.IGNORECASE) \
            or re.search(r"^SD Raw:", line, re.IGNORECASE):

            if re.search(r"^SD Raw:", line, re.IGNORECASE):
                b64 = line[7:].strip() # remove 'SD Raw:'
                try:
                    data = base64.b64decode(b64)
                    if len(data) != 173:
                        print(f"Unexpected packet size: {len(data)}")
                        return
                    src  = data[0:15].decode('ascii', errors='ignore').strip('\x00')
                    dst  = data[15:30].decode('ascii', errors='ignore').strip('\x00')
                    path = data[30:40].decode('ascii', errors='ignore').strip('\x00')
                    msg  = data[40:165].decode('ascii', errors='ignore').strip('\x00')
                    line = f"SD Raw:SRC:{src} DST:{dst} PATH:{path} DATA:{msg}"
                    
                except:
                    pass

            packet = decode_aprs(line)
            if packet is not None:
                try:
                    # search markers for duplicate and delete
                    global markers_by_source
                    source_key = packet.source.strip().upper() # normalize key
                    if source_key in markers_by_source:
                        try:
                            old_marker = markers_by_source[source_key]
                            map_widget.delete(old_marker)
                        except Exception as e:
                            print(f"Error: {e}")
                            pass

                    # add marker to map
                    global symbol_map
                    symbol_description = symbol_map.get(f'{packet.symbol_table}{packet.symbol_id}', 'Unknown')
                    symbol_code = f"{packet.symbol_table}{packet.symbol_id}"
                    emoji = emoji_map.get(symbol_code, "📍")  # default pin if unknown  
                    marker = map_widget.set_marker(
                        packet.latitude, packet.longitude,
                        text = f"{emoji} {packet.source}\n{symbol_description}",
                        command=lambda m, raw='[' + timestamp + '] ' + line: show_raw_data(raw, m)
                    )
                    # optional: pan to marker
                    #map_widget.set_position(packet.latitude, packet.longitude)

                    # keep marker for reference
                    markers_by_source[source_key] = marker

                    # marker examples
                    #marker.delete()  # removes it from the map
                    #marker.set_position(new_lat, new_lon) # update position
                    #marker.set_text("new label") # update label
                    #marker.raw_data = full_raw_line # stash data for later

                except Exception as e:
                    print(f"Error: {e}")
                    pass

        #
        if re.search(r"^SD Raw:", line, re.IGNORECASE):
            tag = "SD"
        elif re.search(r"^Modem Raw:", line, re.IGNORECASE):
            tag = "Modem"
        else:
            tag = "Received"

        #
        log_entry = f"[{timestamp}] ← {line}\n"
        log_entries.append({"text": log_entry, "type": "Received", "tag": tag})
        log_to_file(log_entry)
        append_to_log(log_entries[-1])

    except Exception as e:
        print(f"Error: {e}")
        pass

# ---------- Filtering ----------
def toggle_autoscroll():
    global auto_scroll_enabled
    auto_scroll_enabled = not auto_scroll_enabled
    auto_scroll_btn.config(text=f"Auto-Scroll: {'On' if auto_scroll_enabled else 'Off'}")

def change_filter_mode(*args):
    global filter_mode
    filter_mode = filter_var.get()
    update_log_display()

# ---------- GUI ----------
root = tk.Tk()
#root.iconbitmap("ham_messenger_icon.ico")
root.title("HamMessenger Serial GUI")
root.geometry("950x620")
root.rowconfigure(5, weight=1)
root.columnconfigure(0, weight=1)

root.rowconfigure(6, weight=8)
tabs = ttk_gui.Notebook(root)
tabs.grid(row=6, column=0, sticky="nsew", padx=10, pady=(0, 10))

log_tab = tk.Frame(tabs)
map_tab = tk.Frame(tabs)
log_tab.rowconfigure(0, weight=1)
log_tab.columnconfigure(0, weight=1)

tabs.add(log_tab, text="Log")
tabs.add(map_tab, text="Map")

load_config()

# Control Bar
control_frame = tk.Frame(root)
control_frame.grid(row=0, column=0, sticky="ew", padx=10, pady=5)

tk.Label(control_frame, text="Port:").grid(row=0, column=0)
port_var = tk.StringVar()
port_menu = ttk.Combobox(control_frame, textvariable=port_var, width=12)
port_menu.grid(row=0, column=1, sticky="w")

refresh_btn = tk.Button(control_frame, text="Refresh", command=refresh_ports)
refresh_btn.grid(row=0, column=2, padx=(5, 10))

tk.Label(control_frame, text="Baud:").grid(row=0, column=3)
baud_var = tk.StringVar(value=config.get("baud", "115200"))
baud_menu = ttk.Combobox(control_frame, textvariable=baud_var,
                         values=["9600", "19200", "38400", "57600", "115200"], width=10)
baud_menu.grid(row=0, column=4)

connect_btn = tk.Button(control_frame, text="Connect", command=connect_serial)
connect_btn.grid(row=0, column=5, padx=(10, 0))

status_label = tk.Label(control_frame, text="Not connected", fg="red")
status_label.grid(row=0, column=6, padx=(10, 0))

# Quick Commands
quick_frame = tk.Frame(root)
quick_frame.grid(row=1, column=0, sticky="ew", padx=10, pady=(0, 5))

tk.Label(quick_frame, text="Commands:").pack(side=tk.LEFT)
quick_cmd_var = tk.StringVar()
quick_menu = ttk.Combobox(quick_frame, textvariable=quick_cmd_var,
                          values=config["quick_commands"], width=65)
quick_menu.pack(side=tk.LEFT, padx=(5, 0))
quick_menu.bind("<Return>", lambda event: send_serial(quick_cmd_var.get()))
send_quick_btn = tk.Button(quick_frame, text="Send", command=lambda: send_serial(quick_cmd_var.get()))
send_quick_btn.pack(side=tk.LEFT, padx=(5, 0))

def add_quick_command():
    new_cmd = simpledialog.askstring("New Quick Command", "Enter the command to add:")
    if new_cmd:
        config["quick_commands"].append(new_cmd)
        save_config()
        quick_menu['values'] = config["quick_commands"]
        quick_cmd_var.set(new_cmd)

add_quick_btn = tk.Button(quick_frame, text="Add", command=add_quick_command)
add_quick_btn.pack(side=tk.LEFT, padx=(5, 0))

# Log Controls
log_control_frame = tk.Frame(root)
log_control_frame.grid(row=3, column=0, sticky="ew", padx=10)

auto_scroll_btn = tk.Button(log_control_frame, text="Auto-Scroll: On", command=toggle_autoscroll)
auto_scroll_btn.pack(side=tk.LEFT)

tk.Label(log_control_frame, text="  Filter:").pack(side=tk.LEFT)
filter_var = tk.StringVar(value="All")
filter_menu = ttk.Combobox(log_control_frame, textvariable=filter_var,
                           values=["All", "Sent", "Received", "Modem", "SD"], width=10)
filter_menu.pack(side=tk.LEFT)
filter_menu.bind("<<ComboboxSelected>>", change_filter_mode)

clear_log_btn = tk.Button(log_control_frame, text="Clear Log", command=clear_log)
clear_log_btn.pack(side=tk.LEFT, padx=(10, 0))

open_log_btn = tk.Button(log_control_frame, text="Open Log", command=open_log_file)
open_log_btn.pack(side=tk.LEFT, padx=(10, 0))

# Log Viewer
log_box = scrolledtext.ScrolledText(log_tab, wrap=tk.WORD, state=tk.DISABLED)
log_box.grid(row=0, column=0, sticky="nsew", padx=10, pady=10, in_=log_tab)
log_box.tag_config("Sent", foreground="blue")
log_box.tag_config("Received", foreground="green")
log_box.tag_config("Modem", foreground="orange")
log_box.tag_config("SD", foreground="red")

# Map Viewer
map_widget = TkinterMapView(map_tab, width=900, height=500)
map_widget.pack(fill="both", expand=True)
map_widget.set_position(37.7749, -122.4194)  # Default to SF
map_widget.set_zoom(5)

def show_raw_data(raw: str, marker):
    """
    Pop up a little frameless window next to the given marker
    showing the full raw‐modem line.
    """

    # Get screen coordinates
    screen_x = root.winfo_pointerx()
    screen_y = root.winfo_pointery()    

    # Create popup
    popup = tk.Toplevel(root)
    popup.wm_overrideredirect(True)
    popup.attributes("-topmost", True)
    popup.geometry(f"+{screen_x + 10}+{screen_y + 10}")

    label = tk.Label(
        popup,
        text=raw,
        justify="left",
        background="#ffffe0",
        relief="solid",
        borderwidth=1
    )
    label.pack(padx=4, pady=2)
    popup.after(3000, popup.destroy)

# Startup
refresh_ports()
if config.get("port"):
    port_var.set(config["port"])

root.mainloop()