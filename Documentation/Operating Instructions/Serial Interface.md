
# Serial Interface #

HamMessenger can be operated from a serial interface by connecting the USB port to a personal computer and using a terminal application like [PuTTY](https://www.putty.org), [RealTerm](https://sourceforge.net/projects/realterm/), or the built in serial monitor of the [Arduino IDE](https://www.arduino.cc/en/Tutorial/getting-started-with-ide-v2/ide-v2-serial-monitor).  

In the 2025 release of HamMessenger, there is now a dedicated GUI app that will run in Windows or MacOS (Python) and allows full control of HamMessenger. There is even a nice map that will show the realtime location of stations.  

## Commands ##

Here are a list of commands supported by HamMessenger.

Command | Description | Example
--- | --- | :---
__?__ | Displays serial command examples
__CMD:Settings:Print:__ | Prints the current settings
__CMD:Settings:Save:__ | Saves settings to EEPROM
__CMD:Beacon:__ | Immediately transmits a beacon
__CMD:Message:<Recipient Callsign>:<Recipient SSID>:<Message>"__ | Sends a message to a recipient
__CMD:SD:Raw:Print:__ | Prints all raw data saved in raw.txt on the SD card
__CMD:SD:Raw:Delete:__ | Deletes all raw data saved in raw.txt on the SD card
__CMD:SD:Msg:Print:__ | Prints all messages saved in msg.txt on the SD card
__CMD:SD:Msg:Delete:__ | Delete all messages saved in msg.txt on the SD card
__CMD:Modem:<command>__ | Writes commands directly to the modem. See https://github.com/markqvist/MicroAPRS#serial-commands | CMD:Modem:cNOCALL
&nbsp;
__CMD:Settings:APRS:Beacon Enabled:<True/False>__ | When true, a beacon is automatically transmitted | True (1)
__CMD:Settings:APRS:Beacon Distance:<-3.4028235E+38 to 3.4028235E+38>__ | Distance that a beacon is transmitted in miles | 0.5
__CMD:Settings:APRS:Beacon Idle Time:<0 to 4,294,967,295>__ | Time that the station can be idle in a location before a beacon is transmitted in milliseconds | 60000
__CMD:Settings:APRS:Raw Packet:<alphanumeric 99 char max>__ | !:ZZ5ABC-1 :Hi there!{01 | See https://aprs.fi/?c=raw&call= (login required)
__CMD:Settings:APRS:Comment:<alphanumeric 99 char max>__ | APRS comment | https://github.com/dalethomas81/HamMessenger
__CMD:Settings:APRS:Message:<alphanumeric 99 char max>__ | APRS message to recipient | Hi!
__CMD:Settings:APRS:Recipient Callsign:<alphanumeric 6 char max>__ | Callsign of the recipient | NOCALL
__CMD:Settings:APRS:Recipient SSID:<alphanumeric 1 char max>__ | SSID of recipient i.e. the number 3 in 'NOCALL-3' | 3
__CMD:Settings:APRS:My Callsign:<alphanumeric 6 char max>__ | Callsign of the sender (me) | NOCALL
__CMD:Settings:APRS:Callsign SSID:<alphanumeric 1 char max>__ | SSID of the sender i.e. the number 4 in 'NOCALL-4' | 4
__CMD:Settings:APRS:Dest Callsign:<alphanumeric 6 char max>__ | Callsign of destination when sending | APRS
__CMD:Settings:APRS:Dest SSID:<alphanumeric 1 char max>__ | SSID if the destination station | 0
__CMD:Settings:APRS:PATH1 Callsign:<alphanumeric 6 char max>__ | http://wa8lmf.net/DigiPaths/ | WIDE1
__CMD:Settings:APRS:PATH1 SSID:<alphanumeric 1 char max>__ | http://wa8lmf.net/DigiPaths/ | 1
__CMD:Settings:APRS:PATH2 Callsign:<alphanumeric 6 char max>__ | http://wa8lmf.net/DigiPaths/ | WIDE2
__CMD:Settings:APRS:PATH2 SSID:<alphanumeric 1 char max>__ | http://wa8lmf.net/DigiPaths/ | 2
__CMD:Settings:APRS:Symbol:<alphanumeric 1 char max>__ | http://www.aprs.net/vm/DOS/SYMBOLS.HTM | n (node) 
__CMD:Settings:APRS:Table:<alphanumeric 1 char max>__ | 's' or 'a' (standard or alternate) | s
__CMD:Settings:APRS:Automatic ACK:<True/False>__ | Dictates if the modem will automatically ack messages | True (1)
__CMD:Settings:APRS:Preamble:<0 to 65,535>__ | Number of milliseconds of grey noise before APRS message |350
__CMD:Settings:APRS:Tail:<0 to 65,535>__ | Number of milliseconds of grey noise after APRS message | 80
__CMD:Settings:APRS:Retry Count:<0 to 65,535>__ | How many times HamMessenger will retry a message send | 2
__CMD:Settings:APRS:Retry Interval:<0 to 65,535>__ | Number of milliseconds in between message retries| 10000
&nbsp;
__CMD:Settings:GPS:Pos Tolerance:<-3.4028235E+38 to 3.4028235E+38>__ | GPS position must change this much in degrees to register as a change | 0.1
__CMD:Settings:GPS:Dest Latitude:<-3.4028235E+38 to 3.4028235E+38>__ | GPS latitude coordinates of destination | 51.508131
__CMD:Settings:GPS:Dest Longitude:<-3.4028235E+38 to 3.4028235E+38>__ | GPS longitude coordinates of destination | -0.128002
&nbsp;
__CMD:Settings:Display:Timeout:<0 to 4,294,967,295>__ | Amount of time before displays timeout and go to sleep | 120000
__CMD:Settings:Display:Brightness:<0 to 100>__ | Brightness of Oled (0 is as dim but not off) | 100%
__CMD:Settings:Display:Show Position:<True/False>__ | Lat and Long position is shown on screen | True (1)
__CMD:Settings:Display:Scroll Messages:<True/False>__ | Messages are scrolled on Message display | True (1)
__CMD:Settings:Display:Scroll Speed:<0 to 65,535>__ | Speed in milliseconds that each pixel of messages is scrolled | 4
__CMD:Settings:Display:Invert:<True/False>__ | The colors of the display will be inverted | False
