import tkinter as tk
from tkinter import scrolledtext
import serial
import threading

SERIAL_PORT = 'COM4'
BAUDRATE = 115200

ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=0.5)

def send_serial():
    message = entry.get()
    if message:
        ser.write((message + '\r\n').encode('utf-8'))
        entry.delete(0, tk.END)

def read_serial():
    buffer = ""
    while True:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            buffer += data
            while '\r' in buffer:
                line, buffer = buffer.split('\r', 1)
                #log_box.insert(tk.END, line + '\n')
                log_box.insert(tk.END, line)
                log_box.see(tk.END)

#
root = tk.Tk()
root.title("HamMessenger Serial GUI")

entry = tk.Entry(root, width=50)
entry.pack(padx=10, pady=5)

send_btn = tk.Button(root, text="Send", command=send_serial)
send_btn.pack(pady=5)

log_box = scrolledtext.ScrolledText(root, width=60, height=20)
log_box.pack(padx=10, pady=5)

# Background thread to read serial
threading.Thread(target=read_serial, daemon=True).start()

root.mainloop()