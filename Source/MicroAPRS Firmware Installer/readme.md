The Batch file in this folder uses AVRDUDE to flash the binarys of MicroAPRS to the Arduino Pro Mini. 
Originally I had the MicroAPRS binarys in this folder as well as a copy of AVRDUDE. 
However that is bad practice (and prob against their licenses) so I have removed them. I am leaving this batch file
here so that it can be used if needed. The MicroAPRS repo has good information on how to flash it anyway.

https://github.com/markqvist/MicroAPRS


Here is the command I used on my Mac to write the firmware to the modem. I'll just leave it here for now but will do something with it later.

```
/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avrdude -C/Applications/Arduino.app/Contents/Java/hardware/tools/avr/etc/avrdude.conf -v -patmega328p -carduino -P/dev/cu.usbserial-FTH9M3MG -b57600 -D -Uflash:w:/Users/dalethomas/Downloads/microaprs-3v-ss-latest.hex:i 
```