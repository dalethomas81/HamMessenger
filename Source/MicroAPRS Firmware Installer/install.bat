@echo off
"%~dp0\avr\bin\avrdude" -C "%~dp0\avr\etc\avrdude.conf" -c arduino -v -p ATMEGA328P -P COM25  -b 57600 -D -U flash:w:"%~dp0\microaprs-5v-ss-latest.hex":i