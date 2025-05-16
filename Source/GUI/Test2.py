import tkinter as tk

def on_button_click():
    name = entry.get()
    if name.strip() == "":
        label.config(text="Hello, World!")
    else:
        label.config(text=f"Hello, {name}!")

# Create the main window
root = tk.Tk()
root.title("Hello World GUI with Input")
root.geometry("300x200")

# Create a label
label = tk.Label(root, text="Hello, World!", font=("Arial", 18))
label.pack(pady=10)

# Create an input field (Entry)
entry = tk.Entry(root, font=("Arial", 14))
entry.pack(pady=10)

# Create a button
button = tk.Button(root, text="Greet Me!", command=on_button_click)
button.pack(pady=10)

# Start the GUI event loop
root.mainloop()








'''


import sys
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout,
    QPushButton, QTextEdit, QTabWidget, QHBoxLayout
)

class HamMessengerGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("HamMessenger Serial GUI")
        self.setGeometry(100, 100, 950, 620)

        tabs = QTabWidget()
        tabs.addTab(self.create_log_tab(), "Log")
        tabs.addTab(self.create_map_tab(), "Map")

        self.setCentralWidget(tabs)

    def create_log_tab(self):
        log_tab = QWidget()
        layout = QVBoxLayout()

        button_row = QHBoxLayout()
        for name in ["Refresh", "Connect", "Open Log", "Clear Log", "Send"]:
            btn = QPushButton(name)
            btn.clicked.connect(lambda _, b=name: self.button_clicked(b))
            button_row.addWidget(btn)

        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)

        layout.addLayout(button_row)
        layout.addWidget(self.log_text)
        log_tab.setLayout(layout)
        return log_tab

    def create_map_tab(self):
        map_tab = QWidget()
        layout = QVBoxLayout()
        layout.addWidget(QTextEdit("Map tab placeholder"))
        map_tab.setLayout(layout)
        return map_tab

    def button_clicked(self, name):
        self.log_text.append(f"{name} button clicked.")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = HamMessengerGUI()
    window.show()
    sys.exit(app.exec_())




'''