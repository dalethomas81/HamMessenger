# HamMessenger

HamMessenger is a portable, battery powered device that runs on a microcontroller and interfaces with an inexpensive ham radio to send and recieve text messages and provide position updates using the [APRS](http://www.aprs.org/doc/APRS101.PDF) protocol. Messages and position updates sent via HamMessenger can be viewed on sites such as [aprs.fi](https://aprs.fi). HamMessenger messages are NOT encrypted!

The goal of the HamMessenger project is to create a device that uses ham radio as a medium for sending and receiving text messages. HamMessenger exists also to promote amateur radio, electronics, and programming. The project is currently in a beta prototyping stage with minimum functionality. 

At the core of the the project is the [MicroAPRS](https://github.com/markqvist/MicroAPRS) modem created by [markqvist](https://github.com/markqvist). The primary controller is an Arduino Mega with the MicroAPRS Modem running on an Arduino Pro Mini. HamMessenger includes GPS functionality supplied by a Neo-6M GPS radio. For peripherals, an M5Stack CardKB keyboard is used for input and an SSD1106 Oled display for output.

I will add some videos and pictures of the device in action asap. Please feel free to contribute to the project. I am very busy outside of this project so any help is greatly appreciated :)


![alt text](https://github.com/dalethomas81/HamMessenger/blob/master/Media/926EDB43-D454-4EA1-B1C2-569C25EFE7AD.jpeg)

![alt text](https://github.com/dalethomas81/HamMessenger/blob/master/Media/IMG_3982.jpeg)

# Operation

Full instructions on how to operate HamMessenger will go here. For now, just a few notes :)

-HamMessenger will beacon your location and comment according to the 'Beacon Frequency' settings in APRS Settings.
-To send a direct message to a station, select 'Messages' on the home screen and press the right arrow on the keyboard. A message will be sent according to the settings in APRS Settings.

## Serial commands

Command | Description | Example
--- | --- | :---
__?__ | Displays serial command examples
__CMD:Settings:Print:__ | Prints the current settings
__CMD:Settings:Save:__ | Saves settings to EEPROM
__CMD:Modem:__ | Writes commands directly to the modem. See https://github.com/markqvist/MicroAPRS#serial-commands | CMD:Modem:cNOCALL
&nbsp;
__CMD:Settings:APRS:Beacon Frequency:<0 to 4,294,967,295>__ | Frequency at which position data will be transmitted in milliseconds | 60000
__CMD:Settings:APRS:Raw Packet:<alphanumeric 99 char max>__ | !:ZZ5ABC-1 :Hi there!{01 | See https://aprs.fi/?c=raw&call= (login required)
__CMD:Settings:APRS:Comment:<alphanumeric 99 char max>__ | APRS comment | Testing HamMessenger!
__CMD:Settings:APRS:Message:<alphanumeric 99 char max>__ | APRS message to recipient | Hi!
__CMD:Settings:APRS:Recipient Callsign:<alphanumeric 6 char max>__ | Callsign of the recipient | NOCALL
__CMD:Settings:APRS:Recipient SSID:<alphanumeric 1 char max>__ | SSID of recipient i.e. the number 3 in 'NOCALL-3' | 3
__CMD:Settings:APRS:My Callsign:<alphanumeric 6 char max>__ | Callsign of the sender (me) | NOCALL
__CMD:Settings:APRS:Callsign SSID:<alphanumeric 1 char max>__ | SSID of the sender i.e. the number 4 in 'NOCALL-4' | 4
__CMD:Settings:APRS:Destination Callsign:<alphanumeric 6 char max>__ | Callsign of destination when sending | APRS
__CMD:Settings:APRS:Destination SSID:<alphanumeric 1 char max>__ | SSID if the destination station | 0
__CMD:Settings:APRS:PATH1 Callsign:<alphanumeric 6 char max>__ | http://wa8lmf.net/DigiPaths/ | WIDE1
__CMD:Settings:APRS:PATH1 SSID:<alphanumeric 1 char max>__ | http://wa8lmf.net/DigiPaths/ | 1
__CMD:Settings:APRS:PATH2 Callsign:<alphanumeric 6 char max>__ | http://wa8lmf.net/DigiPaths/ | WIDE2
__CMD:Settings:APRS:PATH2 SSID:<alphanumeric 1 char max>__ | http://wa8lmf.net/DigiPaths/ | 2
__CMD:Settings:APRS:Symbol:<alphanumeric 1 char max>__ | http://www.aprs.net/vm/DOS/SYMBOLS.HTM | n
__CMD:Settings:APRS:Table:<alphanumeric 1 char max>__ | http://www.aprs.net/vm/DOS/SYMBOLS.HTM | s
__CMD:Settings:APRS:Automatic ACK:<True/False>__ | Dictates if the modem will automatically ack messages | True (1)
__CMD:Settings:APRS:Preamble:<0 to 65,535>__ | Number of milliseconds of grey noise before APRS message |350
__CMD:Settings:APRS:Tail:<0 to 65,535>__ | Number of milliseconds of grey noise after APRS message | 80
&nbsp;
__CMD:Settings:GPS:Update Frequency:<0 to 4,294,967,295>__ | Frequency that the GPS location is updated in memory | 10000
__CMD:Settings:GPS:Position Tolerance:<-3.4028235E+38 to 3.4028235E+38>__ | GPS position must change this much to register as a change | 0.000010
__CMD:Settings:GPS:Destination Latitude:<-3.4028235E+38 to 3.4028235E+38>__ | GPS latitude coordinates of destination | 51.508131
__CMD:Settings:GPS:Destination Longitude:<-3.4028235E+38 to 3.4028235E+38>__ | GPS longitude coordinates of destination | -0.128002
&nbsp;
__CMD:Settings:Display:Timeout:<0 to 4,294,967,295>__ | Amount of time before Messages and Live Feed displays timeout and go home | 2000
__CMD:Settings:Display:Brightness:<0 to 100>__ | Brightness of Oled | 100
__CMD:Settings:Display:Show Position:<True/False>__ | Lat and Long position is shown on screen | True (1)
__CMD:Settings:Display:Scroll Messages:<True/False>__ | Messages are scrolled on Message display | True (1)
__CMD:Settings:Display:Scroll Speed:<0 to 65,535>__ | Speed in milliseconds that each pixel of messages is scrolled | 4

TODO (repo):
- add schematics
- add parts list
- add menu layout
- add instructions

TODO (hardware):
- add shielding to components (controller freezes if radio too close during Txing)
- design pcb
- design case
- remove buttons

TODO (software):
- add "quick message" functionality. currently to send a message you highlight 'Messages' on the home screen and press the right key.
- add ability to program the radio for 'plug-n-play' experience (is this possible?)
- add store and recall messages in SD card to save dynamic memory

Keyboard:
https://github.com/m5stack/M5-ProductExampleCodes/tree/master/Unit/CARDKB
https://www.ebay.com/itm/124774337790?hash=item1d0d214cfe:g:kEkAAOSwEZlg0302

Display:
https://github.com/wonho-maker/Adafruit_SH1106
https://www.ebay.com/itm/391731053045

Modem:
https://github.com/markqvist/MicroAPRS
https://github.com/markqvist/MicroAPRS/blob/master/precompiled/microaprs-5v-ss-latest.hex
https://github.com/markqvist/MicroAPRS/blob/master/documentation/Hardware%20Schematic.pdf
https://unsigned.io/shop/
https://unsigned.io/product/micromodem-r23b/

GPS:
https://github.com/mikalhart/TinyGPSPlus

https://www.ebay.com/sch/i.html?_id=221791352715&_nkw=Ublox+NEO-6M+GPS+Module+Aircraft+Flight+Controller+For+Arduino+MWC+IMU+APM2
