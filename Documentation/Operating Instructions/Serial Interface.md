
# Serial Interface #

HamMessenger can be operated from a serial interface by connecting the USB port to a personal computer and using a terminal application like [PuTTY](https://www.putty.org), [RealTerm](https://sourceforge.net/projects/realterm/), or the built in serial monitor of the [Arduino IDE](https://www.arduino.cc/en/Tutorial/getting-started-with-ide-v2/ide-v2-serial-monitor).

## Commands ##

Here are a list of commands supported by HamMessenger.

Command | Description | Example
--- | --- | :---
__?__ | Displays serial command examples
__CMD:Settings:Print:__ | Prints the current settings
__CMD:Settings:Save:__ | Saves settings to EEPROM
__CMD:SD:Raw:Print:__ | Prints all raw data saved in raw.txt on the SD card
__CMD:SD:Raw:Delete:__ | Deletes all raw data saved in raw.txt on the SD card
__CMD:SD:Msg:Print:__ | Prints all messages saved in msg.txt on the SD card
__CMD:SD:Msg:Delete:__ | Delete all messages saved in msg.txt on the SD card
__CMD:Modem:<command>__ | Writes commands directly to the modem. See https://github.com/markqvist/MicroAPRS#serial-commands | CMD:Modem:cNOCALL
&nbsp;
__CMD:Settings:APRS:Beacon Frequency:<0 to 4,294,967,295>__ | Frequency at which position data will be transmitted in milliseconds | 60000
__CMD:Settings:APRS:Raw Packet:<alphanumeric 99 char max>__ | !:ZZ5ABC-1 :Hi there!{01 | See https://aprs.fi/?c=raw&call= (login required)
__CMD:Settings:APRS:Comment:<alphanumeric 99 char max>__ | APRS comment | Testing HamMessenger!
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
__CMD:Settings:APRS:Symbol:<alphanumeric 1 char max>__ | http://www.aprs.net/vm/DOS/SYMBOLS.HTM | n
__CMD:Settings:APRS:Table:<alphanumeric 1 char max>__ | http://www.aprs.net/vm/DOS/SYMBOLS.HTM | s
__CMD:Settings:APRS:Automatic ACK:<True/False>__ | Dictates if the modem will automatically ack messages | True (1)
__CMD:Settings:APRS:Preamble:<0 to 65,535>__ | Number of milliseconds of grey noise before APRS message |350
__CMD:Settings:APRS:Tail:<0 to 65,535>__ | Number of milliseconds of grey noise after APRS message | 80
__CMD:Settings:APRS:Retry Count:<0 to 65,535>__ | How many times HamMessenger will retry a message send | 5
__CMD:Settings:APRS:Retry Interval:<0 to 65,535>__ | Number of milliseconds in between message retries| 10000
&nbsp;
__CMD:Settings:GPS:Update Freq:<0 to 4,294,967,295>__ | Frequency that the GPS location is updated in memory | 10000
__CMD:Settings:GPS:Pos Tolerance:<0-100%>__ | GPS position must change this much to register as a change | 1.0
__CMD:Settings:GPS:Dest Latitude:<-3.4028235E+38 to 3.4028235E+38>__ | GPS latitude coordinates of destination | 51.508131
__CMD:Settings:GPS:Dest Longitude:<-3.4028235E+38 to 3.4028235E+38>__ | GPS longitude coordinates of destination | -0.128002
&nbsp;
__CMD:Settings:Display:Timeout:<0 to 4,294,967,295>__ | Amount of time before Messages and Live Feed displays timeout and go home | 2000
__CMD:Settings:Display:Brightness:<0 to 100>__ | Brightness of Oled (0 is as dim but not off) | 100
__CMD:Settings:Display:Show Position:<True/False>__ | Lat and Long position is shown on screen | True (1)
__CMD:Settings:Display:Scroll Messages:<True/False>__ | Messages are scrolled on Message display | True (1)
__CMD:Settings:Display:Scroll Speed:<0 to 65,535>__ | Speed in milliseconds that each pixel of messages is scrolled | 4
__CMD:Settings:Display:Invert:<True/False>__ | The colors of the display will be inverted | False
