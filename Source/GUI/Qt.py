import sys
import serial
import serial.tools.list_ports
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QTabWidget,
    QLabel, QComboBox, QPushButton, QPlainTextEdit, QLineEdit, QFileDialog,
    QMessageBox
)
from PySide6.QtCore import Qt, QTimer, QThread, Signal, QUrl
from PySide6.QtWebEngineWidgets import QWebEngineView
from datetime import datetime
import json
import os
import platform
import re
import base64
from queue import Queue

# Placeholder for APRS functionality
from aprspy import APRS

# Global message queue
message_queue = Queue()

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
        self.setWindowTitle("HamMessenger Serial GUI")
        self.resize(950, 620)
        self.serial_thread = None
        self.aprs_parser = APRS()

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.main_layout = QVBoxLayout(self.central_widget)

        self.create_control_panel()
        self.create_command_panel()
        self.create_tabs()
        self.populate_serial_ports()

    def create_control_panel(self):
        control_row = QHBoxLayout()

        control_row.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        control_row.addWidget(self.port_combo)

        refresh_button = QPushButton("Refresh")
        refresh_button.clicked.connect(self.populate_serial_ports)
        control_row.addWidget(refresh_button)

        control_row.addWidget(QLabel("Baud:"))
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["9600", "19200", "38400", "57600", "115200"])
        self.baud_combo.setCurrentText("115200")
        control_row.addWidget(self.baud_combo)

        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.toggle_connection)
        control_row.addWidget(self.connect_button)

        self.status_label = QLabel("Not connected")
        control_row.addWidget(self.status_label)

        self.main_layout.addLayout(control_row)

    def create_command_panel(self):
        cmd_row = QHBoxLayout()

        cmd_row.addWidget(QLabel("Command:"))
        self.command_input = QLineEdit()
        self.command_input.returnPressed.connect(self.send_serial_command)
        cmd_row.addWidget(self.command_input)

        self.send_button = QPushButton("Send")
        self.send_button.clicked.connect(self.send_serial_command)
        cmd_row.addWidget(self.send_button)

        self.main_layout.addLayout(cmd_row)

    def create_tabs(self):
        self.tabs = QTabWidget()

        self.log_tab = QWidget()
        self.log_layout = QVBoxLayout(self.log_tab)
        self.log_output = QPlainTextEdit()
        self.log_output.setReadOnly(True)
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
        self.msg_layout.addWidget(self.msg_output)
        self.tabs.addTab(self.msg_tab, "Messages")

        self.main_layout.addWidget(self.tabs)

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

    def send_serial_command(self):
        text = self.command_input.text().strip()
        if not text:
            return
        if self.serial_thread and self.serial_thread.isRunning():
            self.serial_thread.write(text + "\n")
            self.command_input.clear()
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

    def handle_serial_data(self, line):
        timestamp = datetime.now().strftime("[%H:%M:%S]")

        # Check for SD Raw prefix and decode base64 if present
        if re.search(r"^SD Raw:", line, re.IGNORECASE):
            try:
                b64 = line[7:].strip()
                data = base64.b64decode(b64)
                if len(data) != 173:
                    self.msg_output.appendPlainText(f"{timestamp} Unexpected packet size: {len(data)}")
                    return
                src = data[0:15].decode('ascii', errors='ignore').strip('\x00')
                dst = data[15:30].decode('ascii', errors='ignore').strip('\x00')
                path = data[30:40].decode('ascii', errors='ignore').strip('\x00')
                msg = data[40:165].decode('ascii', errors='ignore').strip('\x00')
                line = f"SD Raw:SRC:{src} DST:{dst} PATH:{path} DATA:{msg}"
            except Exception as e:
                self.msg_output.appendPlainText(f"{timestamp} Base64 decode error: {e}")

        self.log_output.appendPlainText(f"{timestamp} {line}")

        # Attempt to parse APRS
        try:
            packet = self.decode_aprs(line)
            if packet:
                message_queue.put(packet)
                self.msg_output.appendPlainText(f"{timestamp} APRS: {packet}")

                lat = getattr(packet, "latitude", None)
                lon = getattr(packet, "longitude", None)
                src = getattr(packet, "source", "")
                if lat and lon:
                    js = f"addMarker({lat}, {lon}, '{src}');"
                    self.map_view.page().runJavaScript(js)
        except Exception as e:
            self.msg_output.appendPlainText(f"{timestamp} Failed to parse APRS: {e}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = HamMessengerGUI()
    window.show()
    sys.exit(app.exec())
