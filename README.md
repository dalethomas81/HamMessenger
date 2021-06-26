# HamMessenger

The goal of the HamMessenger project is to create a device that uses ham radio as a medium for sending and receiving text messages. The project is currently in a beta prototyping stage with minimum functionality. 

At the core of the the project is the MicroAPRS modem created by markqvist (link below). The primary controller is an Arduino Mega with the MicroAPRS Modem running on an Arduino Pro Mini. HamMessenger includes GPS functionality supplied by a Neo-6M GPS radio. For peripherals, an M5Stack CardKB keyboard is used for input and an SSD1106 Oled display for output.

I will add some videos and pictures of the device in action asap. Please feel free to contribute to the project. I am very busy outside of this project so any help is greatly appreciated :)

![alt text](https://github.com/dalethomas81/HamMessenger/blob/master/Media/IMG_3982.jpeg)

TODO (repo):
- add schematics
- add parts list

TODO (hardware):
- add shielding to components (controller freezes if radio too close during Txing)
- design pcb
- design case

TODO (software):
- add edit functionality to GPS and Display settings

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
