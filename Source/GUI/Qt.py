import sys
import serial
import serial.tools.list_ports
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QTabWidget,
    QLabel, QComboBox, QPushButton, QPlainTextEdit, QLineEdit, QFileDialog,
    QMessageBox, QCheckBox, QBoxLayout
)
from PySide6.QtCore import Qt, QTimer, QThread, Signal, QUrl, Signal
from PySide6.QtWebEngineWidgets import QWebEngineView
from PySide6.QtGui import QTextCharFormat, QTextCursor, QColor, QFont
from datetime import datetime
import re
import base64
from queue import Queue
import platform
from aprspy import APRS

IS_DARK_MODE = True
IS_MAC = platform.system() == "Darwin"
IS_WINDOWS = platform.system() == "Windows"

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
                          ,"CMD:Settings:APRS:Beacon Enabled:<True/False>"
                          ,"CMD:Settings:APRS:Beacon Distance:<-3.4028235E+38 to 3.4028235E+38>"
                          ,"CMD:Settings:APRS:Beacon Idle Time:<0 to 4,294,967,295>"
                          ,"CMD:Settings:APRS:Raw Packet:<alphanumeric 99 char max>"
                          ,"CMD:Settings:APRS:Comment:<alphanumeric 99 char max>"
                          ,"CMD:Settings:APRS:Message Text:<alphanumeric 99 char max>"
                          ,"CMD:Settings:APRS:Recipient Callsign:<alphanumeric 6 char max>"
                          ,"CMD:Settings:APRS:Recipient SSID:<alphanumeric 2 char max>"
                          ,"CMD:Settings:APRS:My Callsign:<alphanumeric 6 char max>"
                          ,"CMD:Settings:APRS:My SSID:<alphanumeric 2 char max>"
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
                          ,"CMD:Settings:GPS:Pos Tolerance:<-3.4028235E+38 to 3.4028235E+38>"
                          ,"CMD:Settings:GPS:Dest Latitude:<-3.4028235E+38 to 3.4028235E+38>"
                          ,"CMD:Settings:GPS:Dest Longitude:<-3.4028235E+38 to 3.4028235E+38>"
                          ,"CMD:Settings:Display:Timeout:<0 to 4,294,967,295>"
                          ,"CMD:Settings:Display:Brightness:<0 to 100>"
                          ,"CMD:Settings:Display:Show Position:<True/False>"
                          ,"CMD:Settings:Display:Scroll Messages:<True/False>"
                          ,"CMD:Settings:Display:Scroll Speed:<0 to 65,535>"
                          ,"CMD:Settings:Display:Invert:<True/False>"]

if IS_MAC:
    import objc
    from Foundation import NSObject, NSDistributedNotificationCenter
    import AppKit

    class AppearanceObserver(NSObject):
        def getDarkMode(self):
            appearance = AppKit.NSApplication.sharedApplication().effectiveAppearance().name()
            is_dark = "Dark" in appearance
            return is_dark
        # the trailing underscore is needed to match the Objective-C selector name used by macOS notifications
        # in objective-c this would be - (void)darkModeChanged:(NSNotification *)notification;
        def darkModeChanged_(self, notification):
            global IS_DARK_MODE
            IS_DARK_MODE = self.getDarkMode()
            global window
            window.change_text_colors()

if IS_WINDOWS:
    import winreg
    import ctypes

    class WindowsDarkModeThread(QThread):
        darkModeChanged = Signal(bool)  # Signal to emit: True = dark, False = light

        def run(self):
            REG_PATH = r"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize"
            REG_NAME = "AppsUseLightTheme"
            REG_NOTIFY_CHANGE_LAST_SET = 0x00000004

            hKey = winreg.OpenKey(winreg.HKEY_CURRENT_USER, REG_PATH, 0, winreg.KEY_READ)
            current_value, _ = winreg.QueryValueEx(hKey, REG_NAME)
            self.darkModeChanged.emit(current_value == 0)  # Emit current state

            while True:
                ctypes.windll.advapi32.RegNotifyChangeKeyValue(
                    hKey.handle,
                    True,
                    REG_NOTIFY_CHANGE_LAST_SET,
                    None,
                    False
                )
                new_value, _ = winreg.QueryValueEx(hKey, REG_NAME)
                is_dark = new_value == 0
                if is_dark != (current_value == 0):
                    current_value = new_value
                    self.darkModeChanged.emit(is_dark)

        def on_dark_mode_change(self, is_dark):
            global IS_DARK_MODE
            IS_DARK_MODE = is_dark
            global window
            window.change_text_colors()

class SerialThread(QThread):
    message_received = Signal(str)

    def __init__(self, port, baudrate):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.running = False
        self.serial_connection = None

    def run(self):
        try:
            self.serial_connection = serial.Serial(self.port, self.baudrate, timeout=1)
            self.running = True
            while self.running:
                if self.serial_connection.in_waiting:
                    line = self.serial_connection.readline().decode('utf-8', errors='replace').strip()
                    if line:
                        self.message_received.emit(line)
        except serial.SerialException as e:
            self.message_received.emit(f"Serial error: {e}")

    def stop(self):
        self.running = False
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()

    def write(self, data):
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.write(data.encode('utf-8'))

class HamMessengerGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.my_callsign = "NOCALL-0"
        
        self.mono_font = QFont("Courier New")
        self.mono_font.setStyleHint(QFont.Monospace)
        self.mono_font.setFixedPitch(True)

        self.setWindowTitle("HamMessenger Serial GUI")
        self.resize(950, 620)
        self.serial_thread = None
        self.aprs_parser = APRS()

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.main_layout = QVBoxLayout(self.central_widget)

        self.auto_scroll_enabled = True
        self.message_to = "NOCALL-0"
        self.messsage_msg = "Hello, HamMessenger."

        self.create_control_panel()
        self.create_command_panel()
        self.create_tabs()
        self.populate_serial_ports()

        self.marker_registry = {}
        self.log_entries = []
        self.msg_entries = []

        self.tag_colors_dark_mode = {
                                "Sent": "#4DA6FF",      # softer sky blue
                                "Received": "#90EE90",  # light green
                                "Modem": "#FFB347",     # warm amber (less harsh than bright orange)
                                "SD": "#FF6B6B",        # soft coral red
                                "Message": "#DDA0DD",   # muted light purple
                            }

        self.tag_colors_light_mode = {
                            "Sent": "#005EA2",      # deep azure blue
                            "Received": "#2E8B57",  # medium sea green
                            "Modem": "#D99000",     # warm mustard gold
                            "SD": "#C14444",        # muted brick red
                            "Message": "#A64CA6",   # rich orchid purple
                            }

    def toggle_auto_scroll(self, state):
        #print(f"[Toggle Handler] State: {state} ({type(state)}), Qt.Checked: {Qt.Checked}")
        self.auto_scroll_enabled = state == Qt.CheckState.Checked.value
        #print(f"[Toggle Handler] Enabled: {self.auto_scroll_enabled}")
    
    def add_movable_widget(self,layout:QBoxLayout, widget:QWidget, padding: list[int], 
                        AlignmentX:Qt.AlignmentFlag=Qt.AlignmentFlag.AlignCenter, 
                        AlignmentY:Qt.AlignmentFlag=Qt.AlignmentFlag.AlignCenter):
        
        if not isinstance(padding, list):
            raise TypeError("Input must be a list.")

        LayoutV = QVBoxLayout()
        LayoutV.addSpacing(padding[2])
        LayoutV.addWidget(widget, alignment=AlignmentX | AlignmentY)
        LayoutV.addSpacing(padding[3])

        LayoutH = QHBoxLayout()
        LayoutH.addSpacing(padding[0])
        LayoutH.addLayout(LayoutV)
        LayoutH.addSpacing(padding[1])

        layout.addLayout(LayoutH)

        return LayoutH
    
    def create_control_panel(self):
        # create a horizontal layout that will hold all items in a single row for connecting to HamMessenger
        control_row = QHBoxLayout()

        # create layout for port label, port combobox, and refresh button
        port_combo_qbox = QHBoxLayout()
        # create and add port label
        label = QLabel("Port:")
        self.add_movable_widget(port_combo_qbox, label, [50,0,0,0], Qt.AlignmentFlag.AlignRight)
        # create and add port combobox
        self.port_combo = QComboBox()
        self.add_movable_widget(port_combo_qbox, self.port_combo, [0,0,0,0], Qt.AlignmentFlag.AlignLeft)
        # create and add refresh button
        button = QPushButton("Refresh")
        button.clicked.connect(self.populate_serial_ports)
        self.add_movable_widget(port_combo_qbox, button, [0,0,0,2], Qt.AlignmentFlag.AlignLeft)
        # set spacing between all widgets in port layout
        port_combo_qbox.setSpacing(0)
        # add the port layout to the control row
        control_row.addLayout(port_combo_qbox)

        # add some spacing between the port horizontal layout and the baud horizontal layout
        #control_row.addSpacing(20)

        ###
        # create a horizontal layout that will hold the baud label and baud combobox
        baud_combo_qbox = QHBoxLayout()
        # create the baud label and add it to the port horizontal layout
        baud_label = QLabel("Baud:")
        self.add_movable_widget(baud_combo_qbox, baud_label, [8,0,0,0], Qt.AlignmentFlag.AlignRight)
        # create the combobox
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["9600", "19200", "38400", "57600", "115200"])
        self.baud_combo.setCurrentText("9600")
        self.add_movable_widget(baud_combo_qbox, self.baud_combo, [0,0,0,0], Qt.AlignmentFlag.AlignLeft)
        # set the spacing of all items in the port horizontal layout to 0 so they are all clos
        baud_combo_qbox.setSpacing(0)
        # finally, add the port horizontal layout to the control row horizontal layout
        control_row.addLayout(baud_combo_qbox)

        # add some spacing between the baud horizontal layout and the connect horizontal layout
        #control_row.addSpacing(20)

        ###
        # create a horizontal layout that will hold the connect button and connect label
        conn_combo_qbox = QHBoxLayout()
        # create the connect button and add it to the connection horizontal layout
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.toggle_connection)
        self.add_movable_widget(conn_combo_qbox, self.connect_button, [0,0,0,2.5], Qt.AlignmentFlag.AlignRight)
        # create the baud label and add it to the port horizontal layout
        self.status_label = QLabel("Not connected")
        self.add_movable_widget(conn_combo_qbox, self.status_label, [10,0,0,0], Qt.AlignmentFlag.AlignLeft)
        # set the spacing of all items in the port horizontal layout to 0 so they are all close
        conn_combo_qbox.setSpacing(0)
        # finally, add the port horizontal layout to the control row horizontal layout
        control_row.addLayout(conn_combo_qbox)

        #
        control_row.setSpacing(0)

        #
        self.main_layout.addLayout(control_row)

    def create_command_panel(self):
        cmd_row = QHBoxLayout()

        ###
        # create a horizontal layout that will hold the command label and command combobox
        cmd_combo_qbox = QHBoxLayout()
        # create the command label and add it to the command horizontal layout
        cmd_label = QLabel("Command:")
        self.add_movable_widget(cmd_combo_qbox, cmd_label, [20,0,0,0], Qt.AlignmentFlag.AlignRight)
        # create the command combobox and add it to the command horizontal layout
        self.command_input = QComboBox()
        self.command_input.setEditable(True)
        self.command_input.addItems(default_quick_commands)
        self.command_input.setCurrentIndex(-1)
        self.command_input.lineEdit().returnPressed.connect(self.send_serial_command)
        self.command_input.setFont(self.mono_font)
        self.command_input.setMinimumWidth(800)
        self.add_movable_widget(cmd_combo_qbox, self.command_input, [0,0,0,0], Qt.AlignmentFlag.AlignLeft)
        # set the spacing of all items in the command horizontal layout to 0 so they are all close
        cmd_combo_qbox.setSpacing(0)
        # finally, add the command horizontal layout to the command row horizontal layout
        cmd_row.addLayout(cmd_combo_qbox)

        # add some spacing between the command layout and the send button
        #cmd_row.addSpacing(20)

        ###
        # create a send button and add it directly to the command row horizontal layout
        self.send_button = QPushButton("Send")
        self.send_button.clicked.connect(self.send_serial_command)
        #self.send_button.setMaximumWidth(100)
        self.add_movable_widget(cmd_row, self.send_button, [8,0,0,0], Qt.AlignmentFlag.AlignLeft)

        #
        cmd_row.setSpacing(0)

        #
        self.main_layout.addLayout(cmd_row)

    def create_tabs(self):
        self.tabs = QTabWidget()

        self.log_tab = QWidget()
        self.log_layout = QVBoxLayout(self.log_tab)
        self.log_output = QPlainTextEdit()
        self.log_output.setReadOnly(True)
        self.log_output.setFont(self.mono_font)
        self.log_layout.addWidget(self.log_output)
        self.tabs.addTab(self.log_tab, "Log")

        self.map_tab = QWidget()
        self.map_layout = QVBoxLayout(self.map_tab)
        self.map_view = QWebEngineView()
        self.leaflet_script = """
          var map = L.map('map').setView([37.7749, -122.4194], 10);
          L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            maxZoom: 19,
            attribution: '&copy; OpenStreetMap contributors'
          }).addTo(map);
          var markers = [];
          function addMarker(lat, lon, label) {
            var marker = L.marker([lat, lon]).addTo(map).bindPopup(label);
            markers.push(marker);
          }
        """
        leaflet_html = f"""
        <!DOCTYPE html>
        <html>
        <head>
          <meta charset='utf-8' />
          <title>Leaflet Map</title>
          <meta name='viewport' content='width=device-width, initial-scale=1.0' />
          <link rel='stylesheet' href='https://unpkg.com/leaflet@1.9.4/dist/leaflet.css' />
          <script src='https://unpkg.com/leaflet@1.9.4/dist/leaflet.js'></script>
          <style>html, body, #map {{ height: 100%; margin: 0; }}</style>
        </head>
        <body>
        <div id='map'></div>
        <script>{self.leaflet_script}</script>
        </body>
        </html>
        """
        self.map_view.setHtml(leaflet_html)
        self.map_layout.addWidget(self.map_view)
        self.tabs.addTab(self.map_tab, "Map")

        self.msg_tab = QWidget()
        self.msg_layout = QVBoxLayout(self.msg_tab)
        self.msg_output = QPlainTextEdit()
        self.msg_output.setReadOnly(True)
        self.msg_output.setFont(self.mono_font)
        self.msg_layout.addWidget(self.msg_output)
        self.tabs.addTab(self.msg_tab, "Messages")

        self.main_layout.addWidget(self.tabs)

        self.auto_scroll_checkbox = QCheckBox("Auto-Scroll")
        self.auto_scroll_checkbox.setChecked(True)
        self.auto_scroll_checkbox.stateChanged.connect(self.toggle_auto_scroll)
        log_center_layout = QVBoxLayout()
        log_center_layout.setAlignment(Qt.AlignCenter)
        log_center_layout.addWidget(self.auto_scroll_checkbox)
        self.log_layout.addLayout(log_center_layout)

        # Filter dropdown
        self.filter_box = QComboBox()
        self.filter_box.addItems(["All", "Sent", "Received", "Modem", "SD"])
        self.filter_box.setFixedWidth(150)
        self.filter_box.currentTextChanged.connect(self.filter_log_entries)

        # Center the dropdown
        filter_row = QHBoxLayout()
        filter_row.addStretch()
        filter_row.addWidget(self.filter_box)
        filter_row.addStretch()
        self.log_layout.addLayout(filter_row)

        #
        message_send_qbox = QHBoxLayout()
        message_send_qbox.addStretch()
        #
        label = QLabel("To:")
        self.add_movable_widget(message_send_qbox, label, [0,5,0,0], Qt.AlignmentFlag.AlignRight, Qt.AlignmentFlag.AlignTop)
        #
        self.message_to = QLineEdit()
        self.message_to.setMaximumWidth(80)
        self.message_to.setFont(self.mono_font)
        self.message_to.setMaxLength(9) # 2x3 callsing with a dash and 2 digit ssid is 9 chars
        self.add_movable_widget(message_send_qbox, self.message_to, [0,30,0,0], Qt.AlignmentFlag.AlignLeft)
        #
        label = QLabel("Message:")
        self.add_movable_widget(message_send_qbox, label, [0,5,0,0], Qt.AlignmentFlag.AlignRight, Qt.AlignmentFlag.AlignTop)
        #
        self.message_msg = QLineEdit()
        self.message_msg.setMinimumWidth(400)
        self.message_msg.setFont(self.mono_font)
        self.message_to.setMaxLength(99) # max message length for HamMessenger is 99 chars for now
        self.add_movable_widget(message_send_qbox, self.message_msg, [0,30,0,0], Qt.AlignmentFlag.AlignLeft)
        #
        button = QPushButton("Send")
        button.clicked.connect(self.send_message)
        self.add_movable_widget(message_send_qbox, button, [0,0,0,3], Qt.AlignmentFlag.AlignLeft)
        #
        message_send_qbox.setSpacing(0)
        message_send_qbox.addStretch()
        #
        self.msg_layout.addLayout(message_send_qbox)

    def filter_log_entries(self, selected_tag):
        self.log_output.clear()
        for entry in self.log_entries:
            if selected_tag == "All" or entry["tag"] == selected_tag:
                self.render_log_entry(entry)

    def change_text_colors(self):
        self.log_output.clear()
        for entry in self.log_entries:
            self.render_log_entry(entry)

        self.msg_output.clear()
        for entry in self.msg_entries:
            self.render_msg_entry(entry)

    def populate_serial_ports(self):
        current_selection = self.port_combo.currentData()
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            desc = f"{port.device} — {port.description}"
            self.port_combo.addItem(desc, port.device)

        # Try to restore previous selection
        for i in range(self.port_combo.count()):
            if self.port_combo.itemData(i) == current_selection:
                self.port_combo.setCurrentIndex(i)
                break

    def toggle_connection(self):
        if self.serial_thread and self.serial_thread.isRunning():
            self.serial_thread.stop()
            self.serial_thread.quit()
            self.serial_thread.wait()
            self.serial_thread = None
            self.connect_button.setText("Connect")
            self.status_label.setText("Disconnected")
        else:
            port = self.port_combo.currentData()
            baudrate = int(self.baud_combo.currentText())
            if not port:
                QMessageBox.warning(self, "Error", "No port selected")
                return
            self.serial_thread = SerialThread(port, baudrate)
            self.serial_thread.message_received.connect(self.handle_serial_data)
            self.serial_thread.start()
            self.connect_button.setText("Disconnect")
            self.status_label.setText(f"Connected to {port}")

    def parse_callsign_ssid(self, s):
        # Normalize dash types: convert en dash (–) to hyphen (-)
        s = s.replace("–", "-")
        
        # Match callsign and SSID
        match = re.match(r"^([A-Z0-9]+)-(\d+)$", s, re.IGNORECASE)
        if match:
            callsign, ssid = match.groups()
            return callsign.upper(), ssid
        else:
            return s.upper(), ""  # If no SSID, return full string as callsign

    def send_message(self):
        to = self.message_to
        msg = self.message_msg
        if not to or not msg:
            return
        if self.serial_thread and self.serial_thread.isRunning():
            # CMD:Message:<Recipient Callsign>:<Recipient SSID>:<Message>
            callsign, ssid = self.parse_callsign_ssid(to.text())
            text = f"CMD:Message:{callsign}:{ssid}:{msg.text()}"

            self.serial_thread.write(text + "\n")

            timestamp = datetime.now().strftime("[%H:%M:%S]")
            msg_entry = {
                "text": f"{timestamp} TX: From: {self.my_callsign} To: {to.text()} Message: {msg.text()}\n",
                "tag": "Sent"
            }
            self.msg_entries.append(msg_entry)
            self.render_msg_entry(msg_entry)

            self.message_msg.clear()
            self.message_msg.setFocus()

        else:
            QMessageBox.warning(self, "Not Connected", "No serial connection is active.")

    def send_serial_command(self):
        text = self.command_input.currentText().strip()
        if not text:
            return
        if self.serial_thread and self.serial_thread.isRunning():
            self.serial_thread.write(text + "\n")
            #self.command_input.clear()
            self.command_input.setCurrentIndex(-1)
            self.command_input.setEditText("")
            self.command_input.setFocus()

            timestamp = datetime.now().strftime("[%H:%M:%S]")
            log_entry = {
                "text": f"{timestamp} TX: {text}\n",
                "tag": "Sent"
            }
            self.log_entries.append(log_entry)
            self.render_log_entry(log_entry)
        else:
            QMessageBox.warning(self, "Not Connected", "No serial connection is active.")

    def parse_raw_modem_fields(self, line: str) -> dict:
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
    
    def decode_aprs(self, line):
        fields = self.parse_raw_modem_fields(line)
        if fields:
            # KN6ARG-9>SWQTWR,WIDE1-1:`2Z5lr|j/`"7I}146.520MHz_1
            message = fields['SRC'] + '>' + fields['DST'] + ',' + fields['PATH'] + ':' + fields['DATA']
            try:
                packet = self.aprs_parser.parse(message)
                return packet
            except:
                pass

        return None

    def handle_js_result(self, result):
        if result is None:
            print("[JS Error] JavaScript execution failed or returned None.")
        elif isinstance(result, str) and result.strip() == "":
            print("[JS Error] JavaScript returned an empty string.")
        #else:
            #print("[JS Result]:", result)

    def handle_serial_data(self, line):
        timestamp = datetime.now().strftime("[%H:%M:%S]")

        # "Modem Raw:Callsign: KN4UAH-0"
        match = re.search(r"Modem Raw:Callsign:\s*(\S+)", line, re.IGNORECASE)
        if match:
            self.my_callsign = match.group(1)

        if re.search(r"^SD Raw:", line, re.IGNORECASE):
            tag = "SD"
        elif re.search(r"^Modem Raw:", line, re.IGNORECASE):
            tag = "Modem"
        else:
            tag = "Received"

        if re.search(r"^Modem Raw:SRC:", line, re.IGNORECASE) \
                    or re.search(r"^SD Raw:", line, re.IGNORECASE):
            
            # Check for SD Raw prefix and decode base64 if present
            if re.search(r"^SD Raw:", line, re.IGNORECASE):
                try:
                    b64 = line[7:].strip()
                    data = base64.b64decode(b64)
                    if len(data) != 173:
                        #self.log_output.appendPlainText(f"{timestamp} Unexpected packet size: {len(data)}")
                        return
                    src = data[0:15].decode('ascii', errors='ignore').strip('\x00')
                    dst = data[15:30].decode('ascii', errors='ignore').strip('\x00')
                    path = data[30:40].decode('ascii', errors='ignore').strip('\x00')
                    msg = data[40:165].decode('ascii', errors='ignore').strip('\x00')
                    line = f"SD Raw:SRC:{src} DST:{dst} PATH:{path} DATA:{msg}"
                except Exception as e:
                    #self.log_output.appendPlainText(f"{timestamp} Base64 decode error: {e}")
                    pass

            #self.log_output.appendPlainText(f"{timestamp} {line}")

            # Attempt to parse APRS
            try:
                packet = self.decode_aprs(line)
                if packet:
                    if packet.data_type_id == ">": # status
                        is_status = True
                    if packet.data_type_id == "!": # Position
                        is_position = True
                    if packet.data_type_id == "`": # MIC-E
                        is_mice = True
                    if packet.data_type_id == ":": # is a message
                        # :NOCALL-3 :Hi!{006
                        msg_entry = {
                            "text": f"{timestamp} RX: From: {packet.source} To: {packet.addressee} Message: {packet.message}\n",
                            "tag": "Received" if packet.addressee == self.my_callsign else "Message"
                        }
                        self.msg_entries.append(msg_entry)
                        self.render_msg_entry(msg_entry)
                    #self.msg_output.appendPlainText(f"{timestamp} APRS: {packet}")
                    log_entry = {
                        "text": f"{timestamp} RX: {packet}\n",
                        "tag": tag
                    }
                    self.log_entries.append(log_entry)
                    self.render_log_entry(log_entry)

                    lat = getattr(packet, "latitude", None)
                    lon = getattr(packet, "longitude", None)
                    src = getattr(packet, "source", "")
                    info = getattr(packet, "info", "")

                    #symbol_description = symbol_map.get(f'{packet.symbol_table}{packet.symbol_id}', 'Unknown')
                    symbol_code = f"{packet.symbol_table}{packet.symbol_id}"
                    emoji = emoji_map.get(symbol_code, "📍")  # default pin if unknown  

                    if lat and lon:
                        key = src.strip().upper()
                        marker_id = f"m_{key.replace('-', '_')}"

                        # Remove existing marker if present
                        if key in self.marker_registry:
                            prev_marker = self.marker_registry[key]
                            js_remove = f"""
                                        try {{
                                            map.removeLayer({prev_marker});
                                            'success';
                                        }} catch (err) {{
                                            console.error('[Remove Marker Error]', err);
                                            '[Remove Marker Error] ' + err.toString();
                                        }}
                                        """
                            self.map_view.page().runJavaScript(js_remove, self.handle_js_result)


                        # Add new marker with emoji (without recentering the map)
                        #popup_text = f"{emoji} {src}"
                        #info.replace("`", "\\`").replace("$", "\\$")
                        info.replace("\\", "\\\\").replace("'", "\\'").replace("\"", "\\\"").replace("\n", "\\n").replace("\r", "")
                        popup_text = f"{emoji} {src}<br>{info}"
                        #popup_text = f"{emoji} {src}" + (f"<br>{html.escape(info)}" if info else "")
                        js_add = f"""
                                    try {{
                                        var zoom = map.getZoom();
                                        var scale = 1 + (zoom - 10) * 0.25;
                                        var {marker_id} = L.marker([{lat}, {lon}], {{
                                            icon: L.divIcon({{
                                                className: 'emoji-icon',
                                                html: '{emoji}',
                                                iconSize: null
                                            }})
                                        }}).addTo(map).bindPopup(`{popup_text}`);
                                        document.querySelectorAll('.emoji-icon').forEach(function(el) {{
                                            el.style.fontSize = (24 * scale) + 'px';
                                        }});
                                        'success'
                                    }} catch (err) {{
                                        'JS Error: ' + err.message
                                    }}
                                    """
                        #self.map_view.page().runJavaScript(js_add)
                        self.map_view.page().runJavaScript(js_add, self.handle_js_result)


                        # Add zoom scaling logic
                        js_scale = """
                                    try {
                                        map.off('zoomend');
                                        map.on('zoomend', function() {
                                            var zoom = map.getZoom();
                                            var scale = 1 + (zoom - 10) * 0.25;
                                            var icons = document.getElementsByClassName('emoji-icon');
                                            for (var i = 0; i < icons.length; i++) {
                                                icons[i].style.fontSize = (24 * scale) + 'px';
                                            }
                                        });
                                        'success';
                                    } catch (err) {
                                        console.error('[Zoom Scale Error]', err);
                                        '[Zoom Scale Error] ' + err.toString();
                                    }
                                    """

                        #self.map_view.page().runJavaScript(js_scale)
                        self.map_view.page().runJavaScript(js_scale, self.handle_js_result)

                        # Optionally open popup
                        #js_popup = f"{marker_id}.openPopup();"
                        #self.map_view.page().runJavaScript(js_popup)

                        # Store reference
                        self.marker_registry[key] = marker_id

            except Exception as e:
                #self.msg_output.appendPlainText(f"{timestamp} Failed to parse APRS: {e}")
                pass
        
        log_entry = {
            "text": f"{timestamp} RX: {line}\n",
            "tag": tag
        }
        self.log_entries.append(log_entry)
        self.render_log_entry(log_entry)

    def render_msg_entry(self, entry):
        from PySide6.QtGui import QTextCharFormat, QTextCursor, QColor

        text = entry["text"]
        tag = entry.get("tag", "Message")
        global IS_DARK_MODE
        if IS_DARK_MODE:
            color = self.tag_colors_dark_mode.get(tag, "white")
        else:
            color = self.tag_colors_light_mode.get(tag, "black")

        fmt = QTextCharFormat()
        fmt.setForeground(QColor(color))

        cursor = self.msg_output.textCursor()
        cursor.movePosition(QTextCursor.End)
        cursor.insertText(text, fmt)

        self.msg_output.setTextCursor(cursor)
        self.msg_output.ensureCursorVisible()

    def render_log_entry(self, entry):
        from PySide6.QtGui import QTextCharFormat, QTextCursor, QColor

        # Respect current filter
        selected_tag = self.filter_box.currentText()
        if selected_tag != "All" and entry["tag"] != selected_tag:
            return  # Don't show it

        text = entry["text"]
        tag = entry.get("tag", "Received")
        global IS_DARK_MODE
        if IS_DARK_MODE:
            color = self.tag_colors_dark_mode.get(tag, "white")
        else:
            color = self.tag_colors_light_mode.get(tag, "black")

        fmt = QTextCharFormat()
        fmt.setForeground(QColor(color))

        cursor = self.log_output.textCursor()
        cursor.movePosition(QTextCursor.End)
        cursor.insertText(text, fmt)

        #print(f"[Render] Auto-scroll is: {self.auto_scroll_enabled}")
        if self.auto_scroll_enabled:
            self.log_output.setTextCursor(cursor)
            self.log_output.ensureCursorVisible()


if __name__ == "__main__":

    # set up Qt application
    app = QApplication(sys.argv)
    window = HamMessengerGUI()

    if IS_MAC:  # macOS
        try:
            # set up dark mode monitoring
            AppKit.NSApplication.sharedApplication()
            observer = AppearanceObserver.alloc().init()
            center = NSDistributedNotificationCenter.defaultCenter()
            # Listen for macOS dark mode change notification
            center.addObserver_selector_name_object_(
                observer,
                objc.selector(observer.darkModeChanged_, signature=b'v@:@'),
                "AppleInterfaceThemeChangedNotification",
                None
            )
            # get current dark mode
            IS_DARK_MODE = observer.getDarkMode()

        except Exception:
            pass

    elif IS_WINDOWS:
        try:
            #watch_windows_dark_mode(on_dark_mode_change)
            win_dark_mode_thread = WindowsDarkModeThread()
            win_dark_mode_thread.darkModeChanged.connect(lambda is_dark: win_dark_mode_thread.on_dark_mode_change(is_dark))
            win_dark_mode_thread.start()
            pass
        except Exception:
            pass

    else:
        #raise NotImplementedError("Dark mode detection not supported on this OS.")
        pass

    # open app
    window.show()
    # exit cleanly
    sys.exit(app.exec())
