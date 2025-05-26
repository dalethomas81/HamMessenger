# HamMessenger # 

Click [here](https://github.com/dalethomas81/HamMessenger#updates) for updates.

Hands on article about HamMessenger in IEEE Spectrum magazine [here](https://spectrum.ieee.org/ham-radio-text-hacking)  
Arduino blog post on HamMessenger [here](https://blog.arduino.cc/2021/07/27/send-text-messages-over-ham-radio-with-the-hammessenger/?queryID=undefined)  
Hackaday blog post on HamMessenger [here](https://hackaday.com/blog/?s=hammessenger)  


HamMessenger is a portable, battery powered device that runs on a microcontroller and interfaces with an inexpensive ham radio to send and receive text messages and provide position updates using the [APRS](http://www.aprs.org/doc/APRS101.PDF) protocol. Messages and position updates sent via HamMessenger can be viewed on sites such as [aprs.fi](https://aprs.fi). HamMessenger messages are NOT encrypted!  

HamMessenger is intended to be used by licensed ham radio operators. For more information you can check out the [ARRL](http://www.arrl.org/what-is-ham-radio) website.  

The goal of the HamMessenger project is to create a device that uses ham radio as a medium for sending and receiving text messages. HamMessenger exists also to promote amateur radio, electronics, and programming. The project is currently in a beta prototyping stage but does function quite well. 

At the core of the the project is the [MicroAPRS](https://github.com/markqvist/MicroAPRS) modem created by [markqvist](https://github.com/markqvist). The primary controller is an Arduino Mega 2560 with the MicroAPRS Modem running on an Arduino Pro Mini. HamMessenger includes GPS functionality supplied by a Neo-6M GPS radio. For peripherals, an M5Stack CardKB keyboard is used for input and an SSD1106 Oled display for output.  

I will add some videos and pictures of the device in action asap. Please feel free to contribute to the project. I am very busy outside of this project so any help is greatly appreciated :)  

<table>
  <tr>
    <td align="center">
      <img src="https://github.com/dalethomas81/HamMessenger/blob/master/Media/Screenshots/GUI.png" alt="Screenshot 1" width="300px" style="border:1px solid #ccc; border-radius:8px;" /><br/>
      <sub>Raw Serial Data Output</sub>
    </td>
    <td align="center">
      <img src="https://github.com/dalethomas81/HamMessenger/blob/master/Media/Screenshots/GUI2.png" alt="Screenshot 2" width="300px" style="border:1px solid #ccc; border-radius:8px;" /><br/>
      <sub>Station Markers on Map Tab</sub>
    </td>
  </tr>
</table>

<table>
  <tr>
    <td align="center">
      <img src="https://github.com/dalethomas81/HamMessenger/blob/master/Media/Table-colors.jpg" alt="Screenshot 1" width="300px" style="border:1px solid #ccc; border-radius:8px;" /><br/>
      <sub>Full Enclosure</sub>
    </td>
    <td align="center">
      <img src="https://github.com/dalethomas81/HamMessenger/blob/master/Media/Board-v1/all-together-layout.jpeg" alt="Screenshot 2" width="300px" style="border:1px solid #ccc; border-radius:8px;" /><br/>
      <sub>Component Layout</sub>
    </td>
    <td align="center">
      <img src="https://github.com/dalethomas81/HamMessenger/blob/master/Media/CAD/Enclosure2.jpeg" alt="Screenshot 2" width="300px" style="border:1px solid #ccc; border-radius:8px;" /><br/>
      <sub>CAD Isometric</sub>
    </td>
  </tr>
</table>

## Operation ##

Full instructions on how to operate HamMessenger will go here. For now, just a couple notes :)  

HamMessenger will beacon your location and comment according to the 'Beacon' settings in `APRS` Settings.  
- `Beacon Distance` is the distance in miles that will trigger a beacon.  
- `Beacon Idle Time` is the time in milliseconds that the station must be in the same location after having moved from the last beacon location before a beacon will be sent.
- `Position Tolerance` under `GPS` settings is the +/- value that will trigger a location update.  

To send a direct message to a station, select `Messages` on the `HOME` screen and press `Enter` on the keyboard. This will open the `NEW MESSAGE` screen.  
- `Message Text` is the message that will be sent.  
- `Recipient Callsign` is the callsign of the station that will recieve the message. Example 'NOCALL-ID'  
- `Recipient SSID` is the ID of the station that will recieve the message. Example 'NOCALL-ID'  

Screen navigation information can be found [here](https://github.com/dalethomas81/HamMessenger/blob/master/Documentation/Operating%20Instructions/Screen%20Navigation.md).  

Radio setup instructions can be found [here](https://github.com/dalethomas81/HamMessenger/blob/master/Documentation/Operating%20Instructions/Radio%20Setup.md).

Serial interface information can be found [here](https://github.com/dalethomas81/HamMessenger/blob/master/Documentation/Operating%20Instructions/Serial%20Interface.md).

## TODO ##

hardware:
- improve enclosure design  

software:
- add ability to program the radio once connected for 'plug-n-play' experience (is this possible?)  
- add feature to be selective about message acknowledgments. currently any acknowlegment (from a second conversation) will reset the messaging sequencer.  

## Libraries ##

Here are links to the non-standard libraries that I am using for this project. All other libraries are standard and can be installed using the library manager built into the Arduino IDE.  
https://github.com/wonho-maker/Adafruit_SH1106  
https://github.com/mikalhart/TinyGPSPlus/releases/tag/v1.02b  
https://github.com/adafruit/Adafruit-GFX-Library/releases/tag/1.10.10  

## Compilation ##

The easiest way to compile HamMessenger is to use the Ardunio IDE. After all libraries are installed you can use the Verify button to compile or the Upload button to compile and write the binary to the main controller M1. 

The Arduino source code for HamMessenger can be found [here](/Source/HamMessenger).

The modem controller does not need to be compiled as the binary files are already made available [here](https://github.com/markqvist/MicroAPRS/tree/master/precompiled).  

You can use AVRDude to write the binary to the modem controller. Instructions on how to do that will be coming soon. In the meantime, if you already know how to install and use AVRDude then feel free to make use of the [batch](/Source/MicroAPRS%20Firmware%20Installer) file I created and an [FTDI](https://www.amazon.com/gp/product/B00DDF8TV6/ref=ox_sc_act_title_4?smid=A2SXV8GJXX3WPH&psc=1) serial cable to write to the modem.

## Parts ##

Find BOM information on parts [here](/Documentation/Parts/BOMs)  
Find Drawing information on parts [here](/Documentation/Parts/Drawings)  
Find Vendor information on parts [here](/Documentation/Parts/Vendors)  

### Part Specs ###

Arduino:  
https://gallery.autodesk.com/fusion360/projects/arduino-mega-2560-r3  

Keyboard:  
https://github.com/m5stack/M5-ProductExampleCodes/tree/master/Unit/CARDKB  

OLED Display:    
https://grabcad.com/library/0-96-oled-display-4-pin-1  
https://grabcad.com/library/ecran-oled-1-3-1

Modem:  
https://github.com/markqvist/MicroAPRS  
https://github.com/markqvist/MicroAPRS/blob/master/precompiled/microaprs-5v-ss-latest.hex  
https://github.com/markqvist/MicroAPRS/blob/master/documentation/Hardware%20Schematic.pdf  
https://unsigned.io/shop  
https://unsigned.io/product/micromodem-r23b  

GPS:    
https://grabcad.com/library/gps-module-ublox_neo6mv2-1  
https://grabcad.com/library/ecran-oled-1-3-1  

Grove Connector:  
https://grabcad.com/library/grove-connectors-stand-and-flat-male-plugs-1/details?folder_id=2820551  

Battery:  
Panasonic 18650 - don't have a link but you can get them on eBay or Amazon  

## Updates ##

### 08-SEP-2021: Troubleshooting freezing

There is an issue where during transmit the unit will freeze or restart. I did some experimenting and found out that its the cable that connects to the radio. It is not shielded at all. I thought the small ferrites would be enough to combat this but obviously not. I did some experimenting and found that if I use larger ferrites and wrap them once that the issue goes away. [Here](/Media/Install/large-ferrites.jpeg/) is a picture of the installation. I will now look at sourcing a shielded cable for this application.

### 05-SEP-2021: First version of the enclosure

So I started learning 3D modeling so that I can create an enclosure for HamMessenger. Its been a bit of a learning curve but I'm getting the hang of it. I decided that for the first iteration I would go with an open unibody frame. I chose this style for the first version because it is easy and I'd like to have something working while I learn more about 3D modeling.

You can find the Fusion 360 archive file and the stl [here](/Documentation/CAD/Fusion-360/). It needs to be printed on its side like [this](/Media/Unibody-Frame-v1/cura.png/) to minimize the amount of support needed during printing. I used 0.2mm layers with 20% infill as well as 5% support infill. The print took about 6.5 hours.

A few pics:  
[Front](/Media/Unibody-Frame-v1/front.jpeg/)  
[Side](/Media/Unibody-Frame-v1/side.jpeg/)  
[Back](/Media/Unibody-Frame-v1/back.jpeg/)  

### 14-AUG-2021: Rev v1.1 has arrived and it works!

Ok, great news. Revision v1.1 of the HamMessenger board came in from PCBWay and they are great. I built 2 units and was able to encode and decode messages well. I will add some more information later but [here](/Media/Board-v1.1/) is a few pictures for now.

### 24-JUL-2021: Rev v1.1 is being fabbed!!

I was able to work through all of the issues with the first version. I placed an order with jlcpcb this time (osh park was great but too expensive). I ordered 20 boards this time and if they work well I will give some away.  

I'll update later with how I resolved the issues. For now, going to have a beer :)

### 22-JUL-2021: Put together some information on the board for your review.

[Here](/Media/Board-v1/analysis.md) is an in-depth analysis of the problems with version 1.0 of the board. Also, does any body know how to add correct shielding for the board? I am having issues with the Arduino locking up on transmit sometimes.

### 21-JUL-2021: Made revisons to the board resolving and improving the following:

1. add ground to SD card
2. add OE connection to line leveler
3. reverse I2C connections
4. change GPS I/O to correct serial
5. purchase correct GPS (added new link in BOM)
6. move SD card away from USB.
7. move keyboard grove to bottom of board and out of way
8. move 4 pin terminal for cable such that it is out of the way of larger OLED
9. ground plane improvements.  

still left to do:

1. board does not receive (soldering issues?)  
2. board some times locks up during transmit (ground plane issues?)  


### 20-JUL-2021: Bad news on the boards :( Lots of problems. I will begin immediately working on a revision. Here is a list of the issues:

1. SD Card missing ground
2. Line leveler missing OE connection.
3. I2C connections are reversed.
4. GPS is pointing to wrong serial ports (can fix with code but might as well make it right).
5. GPS pins do not match board. (worked around this by shifting pins)
6. Cannot not receive (don't know why at this point).
7. SD card hits Arduino USB. Will move it and the Grove connector to make room.
8. Arduino periodically locks up. Seemingly due to RF? Had this issue with prototype board but fixed it with ferrites.


### 18-JUL-2021: The printed circuit boards from OSH will be arriving tomorrow! I have added all of the parts needed to build HamMessenger [here](/Documentation/Parts/BOMs).

04-JUL-2021: Version 1 of the pcb for HamMessenger has be sent for fabrication. The estimated delivery date is the 22nd of July. In the meantime I will be ordering the components to populate the board. As I do that I will make sure to get the links for the components and add them to the BOM in the Eagle folder.  
