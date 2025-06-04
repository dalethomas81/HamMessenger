@echo off

REM Get the directory this script is in
set SCRIPT_DIR=%~dp0

REM Paths to avrdude and hex file
set AVRDUDE_EXE=%SCRIPT_DIR%avrdude.exe
set AVRDUDE_CONF=%SCRIPT_DIR%avrdude.conf
set HEX_FILE=%SCRIPT_DIR%microaprs-5v-ss-latest.hex

REM Serial port (adjust if needed)
set PORT=COM8

REM Run avrdude
"%AVRDUDE_EXE%" -C "%AVRDUDE_CONF%" -c arduino -v -p ATMEGA328P -P %PORT% -b 57600 -D -U flash:w:"%HEX_FILE%":i