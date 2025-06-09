/*

  https://github.com/dalethomas81/HamMessenger
  Copyright (C) 2021  Dale Thomas

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  DaleThomas@me.com

*/

const char version[] = __DATE__ " " __TIME__; 

#pragma region "STRUCTURES"

  // for some reason Ardunio will not allow a structure after a method
  struct APRSFormat_Raw {
    char src[15];
    char dst[15];
    char path[10];
    char data[125];
    uint32_t DateInt;
    uint32_t TimeInt;
  };

  struct APRSFormat_Msg {
    char from[15] = {'\0'};
    char to[15] = {'\0'};
    char msg[100] = {'\0'};
    int line = 0;
    bool ack = false;
    uint32_t DateInt;
    uint32_t TimeInt;
  };

  struct GPS_Date {
    byte Day = 1;
    byte Month = 1;
    int Year = 1970;
    uint32_t DateInt;
    //char DateString[9];
  };

  struct GPS_Time {
    byte Hour = 0;
    byte Minute = 0;
    byte Second = 0;
    byte CentiSecond = 0;
    uint32_t TimeInt;
    //char TimeString[9];
  };

  struct GPS {
    byte Satellites;
    GPS_Date Date;
    GPS_Time Time;
  } GPSData;

#pragma endregion

#pragma region "UTILITY"

  #define BATT_CHARGED            7200
  #define BATT_DISCHARGED         5600
  #define VOLTAGE_CHECK_RATE      10000

  unsigned char VoltagePercent = 0;
  long Voltage = 0;
  unsigned long voltage_check_timer;
  unsigned long processor_scan_time, scanTime;

  long readVcc(){
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
    #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
      ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
      ADMUX = _BV(MUX5) | _BV(MUX0);
    #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
      ADMUX = _BV(MUX3) | _BV(MUX2);
    #else
      ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    #endif  

    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring

    uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
    uint8_t high = ADCH; // unlocks both

    long result = (high<<8) | low;

    result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
    return result; // Vcc in millivolts
  }

  void handleVoltage(){
    if (millis() - voltage_check_timer > VOLTAGE_CHECK_RATE){
      Voltage = readVcc();
      VoltagePercent = constrain(map(Voltage,(int)BATT_DISCHARGED,(int)BATT_CHARGED,0,100),0,100);
      voltage_check_timer = millis();
    }
  }

  void handleStats(){
    scanTime = micros() - processor_scan_time;
    processor_scan_time = micros();
  }

#pragma endregion

#pragma region "KEYBOARD"

  // M5Stack Keyboard https://docs.m5stack.com/en/unit/cardkb
  #include <Wire.h> 
  #define CARDKB_ADDR 0x5F  
  char keyboardInputChar;
  bool wakeDisplay = false;
  bool displayDim = false;

  #define KEYBOARD_NUMBER_KEYS            (keyboardInputChar >= 48 && keyboardInputChar <= 57)
  #define KEYBOARD_DIRECTIONAL_KEYS       (keyboardInputChar >= -76 && keyboardInputChar <= -73)
  #define KEYBOARD_PRINTABLE_CHARACTERS   (keyboardInputChar >= 32 && keyboardInputChar <= 126)
  #define KEYBOARD_BACKSPACE_KEY          8
  #define KEYBOARD_ENTER_KEY              13
  #define KEYBOARD_ESCAPE_KEY             27
  #define KEYBOARD_MINUS_KEY              45
  #define KEYBOARD_PERIOD_KEY             46
  #define KEYBOARD_RIGHT_KEY              -73
  #define KEYBOARD_DOWN_KEY               -74
  #define KEYBOARD_UP_KEY                 -75
  #define KEYBOARD_LEFT_KEY               -76
  
  void handleKeyboard(){
  keyboardInputChar = 0;
  Wire.requestFrom(CARDKB_ADDR, 1);
  while(Wire.available())
  {
    char c = Wire.read();
    if (c != 0)
    {
      //Serial.println((int)c);
      if (!displayDim) { // key presses should only register if screen awake
        keyboardInputChar = c;
      }
      wakeDisplay = true;
    }
  }
}

#pragma endregion

#pragma region "SETTINGS"

  #include <EEPROM.h>

  const char Initialized[] = {"Initialized 2025MAY27 v2"}; // change this to something unique if you want to re-init the EEPROM during flashing. useful when there has been a change to a settings array.

  #define EEPROM_SETTINGS_START_ADDR      1000

  // NOTE: these need to stay in contiguous order starting at 0.
  // otherwise the getDataTypeExample will break.
  // if changes are made here, just be sure to update that function.
  #define SETTINGS_EDIT_TYPE_NONE         0
  #define SETTINGS_EDIT_TYPE_BOOLEAN      1
  #define SETTINGS_EDIT_TYPE_INT          2
  #define SETTINGS_EDIT_TYPE_UINT         3
  #define SETTINGS_EDIT_TYPE_LONG         4
  #define SETTINGS_EDIT_TYPE_ULONG        5
  #define SETTINGS_EDIT_TYPE_FLOAT        6
  #define SETTINGS_EDIT_TYPE_STRING2      7
  #define SETTINGS_EDIT_TYPE_STRING3      8
  #define SETTINGS_EDIT_TYPE_STRING7      9
  #define SETTINGS_EDIT_TYPE_STRING100    10
  #define SETTINGS_EDIT_TYPE_ALT1         11
                          
  const char *MenuItems_Settings[] = {"APRS", "GPS", "Display"};
  const char *MenuItems_Settings_APRS[] = {"Beacon Enabled", "Beacon Distance", "Beacon Idle Time", "Raw Packet", "Comment",
                                          "Message Text", "Recipient Callsign", "Recipient SSID", "My Callsign", "My SSID", 
                                          "Dest Callsign", "Dest SSID", "PATH1 Callsign", "PATH1 SSID", "PATH2 Callsign", "PATH2 SSID",
                                          "Symbol", "Table", "Automatic ACK", "Preamble", "Tail", "Retry Count", "Retry Interval"};
  const char *MenuItems_Settings_GPS[] = {"Pos Tolerance", "Dest Latitude", "Dest Longitude"};
  const char *MenuItems_Settings_Display[] = {"Timeout", "Brightness", "Show Position", "Scroll Messages", "Scroll Speed", "Invert"};

  unsigned char Settings_Type_APRS[] = {SETTINGS_EDIT_TYPE_BOOLEAN, SETTINGS_EDIT_TYPE_FLOAT, SETTINGS_EDIT_TYPE_ULONG, SETTINGS_EDIT_TYPE_STRING100, SETTINGS_EDIT_TYPE_STRING100,
                                        SETTINGS_EDIT_TYPE_STRING100, SETTINGS_EDIT_TYPE_STRING7, SETTINGS_EDIT_TYPE_STRING3, SETTINGS_EDIT_TYPE_STRING7, SETTINGS_EDIT_TYPE_STRING3,
                                        SETTINGS_EDIT_TYPE_STRING7, SETTINGS_EDIT_TYPE_STRING3, SETTINGS_EDIT_TYPE_STRING7, SETTINGS_EDIT_TYPE_STRING3, SETTINGS_EDIT_TYPE_STRING7, SETTINGS_EDIT_TYPE_STRING3,
                                        SETTINGS_EDIT_TYPE_STRING2, SETTINGS_EDIT_TYPE_ALT1, SETTINGS_EDIT_TYPE_BOOLEAN, SETTINGS_EDIT_TYPE_UINT, SETTINGS_EDIT_TYPE_UINT, SETTINGS_EDIT_TYPE_UINT, SETTINGS_EDIT_TYPE_UINT};
  unsigned char Settings_Type_GPS[] = {SETTINGS_EDIT_TYPE_FLOAT, SETTINGS_EDIT_TYPE_FLOAT, SETTINGS_EDIT_TYPE_FLOAT};
  unsigned char Settings_Type_Display[] = {SETTINGS_EDIT_TYPE_ULONG, SETTINGS_EDIT_TYPE_UINT, SETTINGS_EDIT_TYPE_BOOLEAN, SETTINGS_EDIT_TYPE_BOOLEAN, SETTINGS_EDIT_TYPE_UINT, SETTINGS_EDIT_TYPE_BOOLEAN};
  unsigned char Settings_TypeIndex_APRS[] = { 0,0,0,0,1,
                                              2,0,0,1,1,
                                              2,2,3,3,4,4,
                                              0,1,3,1,2,4,5}; // this is the index in the array of the data arrays below
  unsigned char Settings_TypeIndex_GPS[] = {1,2,3};
  unsigned char Settings_TypeIndex_Display[] = {1,0,1,2,3,4};
  // data arrays
  bool Settings_TypeBool[5] = {true,true,true,true,false}; // aprs beacon enabled, display show position, scroll messages, auto ACK, invert
  int Settings_TypeInt[0] = {};
  unsigned int Settings_TypeUInt[6] = {100,400,80,4,5,10000}; // display brightness, aprs preamble, aprs tail, scroll speed, Retry Count, Retry Interval
  long Settings_TypeLong[0] = {};
  unsigned long Settings_TypeULong[2] = {300000, 2000}; // aprs beacon Idle Time, display timeout
  float Settings_TypeFloat[4] = {1.00000,0.00001,34.790040,-82.790672}; // aprs beacon distance, gps position tolerance, gps latitude, gps longitude
  char Settings_TypeString2[2][2] = {'n','\0'};
  char Settings_TypeString3[6][3] = {'0','0','\0'};
  char Settings_TypeString7[5][7] = {'N','O','C','A','L','L','\0'};
  char Settings_TypeString100[3][100] = {'T','e','s','t','\0'};
  char Settings_TempDispCharArr[100];

  #define SETTINGS_APRS_BEACON_ENABLED          Settings_TypeBool[Settings_TypeIndex_APRS[0]]         // beacon enabled
  #define SETTINGS_APRS_BEACON_DISTANCE         Settings_TypeFloat[Settings_TypeIndex_APRS[1]]        // beacon distance
  #define SETTINGS_APRS_BEACON_IDLE_TIME        Settings_TypeULong[Settings_TypeIndex_APRS[2]]        // beacon idle time
  #define SETTINGS_APRS_RAW_PACKET              Settings_TypeString100[Settings_TypeIndex_APRS[3]]    // raw packet
  #define SETTINGS_APRS_COMMENT                 Settings_TypeString100[Settings_TypeIndex_APRS[4]]    // comment
  #define SETTINGS_APRS_MESSAGE                 Settings_TypeString100[Settings_TypeIndex_APRS[5]]    // message
  #define SETTINGS_APRS_RECIPIENT_CALL          Settings_TypeString7[Settings_TypeIndex_APRS[6]]      // recipient
  #define SETTINGS_APRS_RECIPIENT_SSID          Settings_TypeString3[Settings_TypeIndex_APRS[7]]      // recipient ssid
  #define SETTINGS_APRS_CALLSIGN                Settings_TypeString7[Settings_TypeIndex_APRS[8]]      // callsign
  #define SETTINGS_APRS_CALLSIGN_SSID           Settings_TypeString3[Settings_TypeIndex_APRS[9]]      // callsign ssid
  #define SETTINGS_APRS_DESTINATION_CALL        Settings_TypeString7[Settings_TypeIndex_APRS[10]]     // Destination Callsign
  #define SETTINGS_APRS_DESTINATION_SSID        Settings_TypeString3[Settings_TypeIndex_APRS[11]]     // Destination SSID
  #define SETTINGS_APRS_PATH1_CALL              Settings_TypeString7[Settings_TypeIndex_APRS[12]]     // PATH1 Callsign
  #define SETTINGS_APRS_PATH1_SSID              Settings_TypeString3[Settings_TypeIndex_APRS[13]]     // PATH1 SSID
  #define SETTINGS_APRS_PATH2_CALL              Settings_TypeString7[Settings_TypeIndex_APRS[14]]     // PATH2 Callsign
  #define SETTINGS_APRS_PATH2_SSID              Settings_TypeString3[Settings_TypeIndex_APRS[15]]     // PATH2 SSID
  #define SETTINGS_APRS_SYMBOL                  Settings_TypeString2[Settings_TypeIndex_APRS[16]]     // Symbol
  #define SETTINGS_APRS_SYMBOL_TABLE            Settings_TypeString2[Settings_TypeIndex_APRS[17]]     // Symbol Table
  #define SETTINGS_APRS_AUTOMATIC_ACK           Settings_TypeBool[Settings_TypeIndex_APRS[18]]        // Automatic ACK
  #define SETTINGS_APRS_PREAMBLE                Settings_TypeUInt[Settings_TypeIndex_APRS[19]]        // Preamble
  #define SETTINGS_APRS_TAIL                    Settings_TypeUInt[Settings_TypeIndex_APRS[20]]        // Tail
  #define SETTINGS_APRS_RETRY_COUNT             Settings_TypeUInt[Settings_TypeIndex_APRS[21]]        // Retry Count
  #define SETTINGS_APRS_RETRY_INTERVAL          Settings_TypeUInt[Settings_TypeIndex_APRS[22]]        // Retry Interval

  #define SETTINGS_GPS_POSITION_TOLERANCE       Settings_TypeFloat[Settings_TypeIndex_GPS[0]]         // position tolerance
  #define SETTINGS_GPS_DESTINATION_LATITUDE     Settings_TypeFloat[Settings_TypeIndex_GPS[1]]         // destination latitude
  #define SETTINGS_GPS_DESTINATION_LONGITUDE    Settings_TypeFloat[Settings_TypeIndex_GPS[2]]         // destination longitute

  #define SETTINGS_DISPLAY_TIMEOUT              Settings_TypeULong[Settings_TypeIndex_Display[0]]      // timeout
  #define SETTINGS_DISPLAY_BRIGHTNESS           Settings_TypeUInt[Settings_TypeIndex_Display[1]]       // brightness
  #define SETTINGS_DISPLAY_SHOW_POSITION        Settings_TypeBool[Settings_TypeIndex_Display[2]]       // show position
  #define SETTINGS_DISPLAY_SCROLL_MESSAGES      Settings_TypeBool[Settings_TypeIndex_Display[3]]       // scroll messages
  #define SETTINGS_DISPLAY_SCROLL_SPEED         Settings_TypeUInt[Settings_TypeIndex_Display[4]]       // scroll speed
  #define SETTINGS_DISPLAY_INVERT               Settings_TypeBool[Settings_TypeIndex_Display[5]]       // invert

  bool applySettings=false, saveModemSettings=false;
  unsigned char applySettings_Seq=0;
  bool modemCmdFlag_Setc=false, modemCmdFlag_Setsc=false;
  bool modemCmdFlag_Setd=false, modemCmdFlag_Setsd=false;
  bool modemCmdFlag_Set1=false, modemCmdFlag_Sets1=false;
  bool modemCmdFlag_Set2=false, modemCmdFlag_Sets2=false;
  bool modemCmdFlag_Setls=false, modemCmdFlag_Setlt=false;
  bool modemCmdFlag_Setma=false, modemCmdFlag_Setw=false;
  bool modemCmdFlag_SetW=false, modemCmdFlag_Setmr=false;
  bool modemCmdFlag_SetS=false, modemCmdFlag_SetC=false;
  bool modemCmdFlag_SetH=false;
  
  #if !defined(ARRAY_SIZE)
      #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
  #endif

  void checkInit(){
    Serial.println(F("Checking initialization...")); // `F()` stores the string in flash memory and avoids heaps
    bool writeDefaults = false;
    for (byte i=0;i<sizeof(Initialized)-1;i++){
      if (Initialized[i] != EEPROM.read(i)) {
        writeDefaults = true;
        i = sizeof(Initialized); // get out
      }
    }
    /*for (byte i=0;i<sizeof(version)-1;i++){
      if (version[i] != EEPROM.read(i)) {
        writeDefaults = true;
        i = sizeof(version); // get out
      }
    }*/
    if (writeDefaults) {
      applyDefaultsToSettings();
      makeInitialized();
    }
  }

  void makeInitialized(){
    Serial.println(F("Initializing..."));
    for (byte i=0;i<sizeof(Initialized)-1;i++){
      EEPROM.write(i, Initialized[i]);
    }
    /*for (byte i=0;i<sizeof(version)-1;i++){
      EEPROM.write(i, version[i]);
    }*/
  }

  void applyDefaultsToSettings(){
    Serial.println(F("Applying defaults to settings..."));

    SETTINGS_APRS_BEACON_ENABLED = true;
    
    SETTINGS_APRS_BEACON_DISTANCE = 0.5000;

    SETTINGS_APRS_BEACON_IDLE_TIME = 30000;

    const char* strTemp1 = "NOCALL>APRS,WIDE1-1,WIDE2-1:!0000.00N/00000.00W>HamMessenger Ready";
    for (int i=0; i<sizeof(strTemp1);i++) {
      SETTINGS_APRS_RAW_PACKET[i] = strTemp1[i];
    }

    const char* strTemp2 = "https://github.com/dalethomas81/HamMessenger";
    for (int i=0; i<sizeof(strTemp2);i++) {
      SETTINGS_APRS_COMMENT[i] = strTemp2[i];
    }

    const char* strTemp3 = "Hi!";
    for (int i=0; i<sizeof(strTemp3);i++) {
      SETTINGS_APRS_MESSAGE[i] = strTemp3[i];
    }

    const char* strTemp4 = "NOCALL";
    for (int i=0; i<sizeof(strTemp4);i++) {
      SETTINGS_APRS_RECIPIENT_CALL[i] = strTemp4[i];
    }

    SETTINGS_APRS_RECIPIENT_SSID[0] = '0';
    SETTINGS_APRS_RECIPIENT_SSID[1] = '\0';

    const char* strTemp5 = "NOCALL";
    for (int i=0; i<sizeof(strTemp5);i++) {
      SETTINGS_APRS_CALLSIGN[i] = strTemp5[i];
    }

    SETTINGS_APRS_CALLSIGN_SSID[0] = '0';
    SETTINGS_APRS_CALLSIGN_SSID[1] = '\0';

    const char* strTemp6 = "APRS";
    for (int i=0; i<sizeof(strTemp6);i++) {
      SETTINGS_APRS_DESTINATION_CALL[i] = strTemp6[i];
    }

    SETTINGS_APRS_DESTINATION_SSID[0]  = '1';
    SETTINGS_APRS_DESTINATION_SSID[1] = '\0';

    const char* strTemp7 = "WIDE1";
    for (int i=0; i<sizeof(strTemp7);i++) {
      SETTINGS_APRS_PATH1_CALL[i] = strTemp7[i];
    }

    SETTINGS_APRS_PATH1_SSID[0] = '1';
    SETTINGS_APRS_PATH1_SSID[1] = '\0';

    const char* strTemp8 = "WIDE2";
    for (int i=0; i<sizeof(strTemp8)-1;i++) {
      SETTINGS_APRS_PATH2_CALL[i] = strTemp8[i];
    }

    SETTINGS_APRS_PATH2_SSID[0] = '2';
    SETTINGS_APRS_PATH2_SSID[1] = '\0';

    SETTINGS_APRS_SYMBOL[0] = 'n';
    SETTINGS_APRS_SYMBOL[1] = '\0';

    SETTINGS_APRS_SYMBOL_TABLE[0] = 's';
    SETTINGS_APRS_SYMBOL_TABLE[1] = '\0';

    SETTINGS_APRS_AUTOMATIC_ACK = true;

    SETTINGS_APRS_PREAMBLE = 350;

    SETTINGS_APRS_TAIL = 80;

    SETTINGS_APRS_RETRY_COUNT = 2;

    SETTINGS_APRS_RETRY_INTERVAL = 10000;
    
    SETTINGS_GPS_POSITION_TOLERANCE = 0.01;  // unit is in degrees

    // London  LAT:51.508131     LNG:-0.128002
    SETTINGS_GPS_DESTINATION_LATITUDE = 51.508131;

    SETTINGS_GPS_DESTINATION_LONGITUDE = -0.128002;
    
    SETTINGS_DISPLAY_TIMEOUT = 120000;

    SETTINGS_DISPLAY_BRIGHTNESS = 100;

    SETTINGS_DISPLAY_SHOW_POSITION = true;

    SETTINGS_DISPLAY_SCROLL_MESSAGES = true;

    SETTINGS_DISPLAY_SCROLL_SPEED = 4;

    SETTINGS_DISPLAY_INVERT= false;

    writeSettingsToEeprom();
  }

  void writeSettingsToEeprom(){
    Serial.println(F("Writing settings..."));
    
    /*Settings Settings_Default;
    int address = EEPROM_SETTINGS_START_ADDR;
    EEPROM.put(address, Settings_Default);
    address = address + sizeof(Settings_Default);
    Serial.print(F("Settings size =")); Serial.println(sizeof(Settings_Default));*/

    int address = EEPROM_SETTINGS_START_ADDR;
    for (int i=0;i<ARRAY_SIZE(Settings_TypeBool);i++){
      //Serial.print(F("bool:"));Serial.println(address);
      EEPROM.put(address, Settings_TypeBool[i]);
      address = address + sizeof(Settings_TypeBool[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeInt);i++){
      //Serial.print(F("int:"));Serial.println(address);
      EEPROM.put(address, Settings_TypeInt[i]);
      address = address + sizeof(Settings_TypeInt[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeUInt);i++){
      //Serial.print(F("uint:"));Serial.print(address);Serial.print(":");Serial.println(Settings_TypeUInt[i]);
      EEPROM.put(address, Settings_TypeUInt[i]);
      address = address + sizeof(Settings_TypeUInt[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeLong);i++){
      //Serial.print(F("long:"));Serial.println(address);
      EEPROM.put(address, Settings_TypeLong[i]);
      address = address + sizeof(Settings_TypeLong[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeULong);i++){
      //Serial.print(F("ulong:"));Serial.print(address);Serial.print(":");Serial.println(Settings_TypeULong[i]);
      EEPROM.put(address, Settings_TypeULong[i]);
      address = address + sizeof(Settings_TypeULong[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeFloat);i++){
      //Serial.print(F("float:"));Serial.println(address);
      EEPROM.put(address, Settings_TypeFloat[i]);
      address = address + sizeof(Settings_TypeFloat[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeString2);i++){
      //Serial.print(F("Settings_TypeString2:"));Serial.println(address);
      for (int j=0;j<sizeof(Settings_TypeString2[i]);j++){
        EEPROM.put(address, Settings_TypeString2[i][j]);
        address = address + sizeof(Settings_TypeString2[i][j]);
      }
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeString3);i++){
      //Serial.print(F("Settings_TypeString3:"));Serial.println(address);
      for (int j=0;j<sizeof(Settings_TypeString3[i]);j++){
        EEPROM.put(address, Settings_TypeString3[i][j]);
        address = address + sizeof(Settings_TypeString3[i][j]);
      }
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeString7);i++){
      //Serial.print(F("Settings_TypeString7:"));Serial.println(address);
      for (int j=0;j<sizeof(Settings_TypeString7[i]);j++){
        EEPROM.put(address, Settings_TypeString7[i][j]);
        address = address + sizeof(Settings_TypeString7[i][j]);
      }
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeString100);i++){
      //Serial.print(F("Settings_TypeString100:"));Serial.println(address);
      for (int j=0;j<sizeof(Settings_TypeString100[i]);j++){
        EEPROM.put(address, Settings_TypeString100[i][j]);
        address = address + sizeof(Settings_TypeString100[i][j]);
      }
    }
  }

  void readSettingsFromEeprom(){
    Serial.println(F("Reading settings..."));
    
    /*int address = EEPROM_SETTINGS_START_ADDR;
    EEPROM.get(address, settings);
    Serial.print(F("Settings size =")); Serial.println(sizeof(settings));*/
    
    int address = EEPROM_SETTINGS_START_ADDR;
    for (int i=0;i<ARRAY_SIZE(Settings_TypeBool);i++){
      //Serial.print(F("bool:"));Serial.println(address);
      EEPROM.get(address, Settings_TypeBool[i]);
      address = address + sizeof(Settings_TypeBool[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeInt);i++){
      //Serial.print(F("int:"));Serial.println(address);
      EEPROM.get(address, Settings_TypeInt[i]);
      address = address + sizeof(Settings_TypeInt[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeUInt);i++){
      //Serial.print(F("uint:"));Serial.print(address);Serial.print(":");Serial.println(Settings_TypeUInt[i]);
      EEPROM.get(address, Settings_TypeUInt[i]);
      address = address + sizeof(Settings_TypeUInt[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeLong);i++){
      //Serial.print(F("long:"));Serial.println(address);
      EEPROM.get(address, Settings_TypeLong[i]);
      address = address + sizeof(Settings_TypeLong[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeULong);i++){
      //Serial.print(F("ulong:"));Serial.print(address);Serial.print(":");Serial.println(Settings_TypeULong[i]);
      EEPROM.get(address, Settings_TypeULong[i]);
      address = address + sizeof(Settings_TypeULong[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeFloat);i++){
      //Serial.print(F("float:"));Serial.println(address);
      EEPROM.get(address, Settings_TypeFloat[i]);
      address = address + sizeof(Settings_TypeFloat[i]);
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeString2);i++){
      //Serial.print(F("Settings_TypeString2:"));Serial.println(address);
      for (int j=0;j<sizeof(Settings_TypeString2[i]);j++){
        EEPROM.get(address, Settings_TypeString2[i][j]);
        address = address + sizeof(Settings_TypeString2[i][j]);
      }
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeString3);i++){
      //Serial.print(F("Settings_TypeString3:"));Serial.println(address);
      for (int j=0;j<sizeof(Settings_TypeString3[i]);j++){
        EEPROM.get(address, Settings_TypeString3[i][j]);
        address = address + sizeof(Settings_TypeString3[i][j]);
      }
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeString7);i++){
      //Serial.print(F("Settings_TypeString7:"));Serial.println(address);
      for (int j=0;j<sizeof(Settings_TypeString7[i]);j++){
        EEPROM.get(address, Settings_TypeString7[i][j]);
        address = address + sizeof(Settings_TypeString7[i][j]);
      }
    }
    for (int i=0;i<ARRAY_SIZE(Settings_TypeString100);i++){
      for (int j=0;j<sizeof(Settings_TypeString100[i]);j++){
        EEPROM.get(address, Settings_TypeString100[i][j]);
        address = address + sizeof(Settings_TypeString100[i][j]);
      }
    }
  }

  void handleSettings(){  
    if (applySettings){
      applySettings=false;
      applySettings_Seq=1;
    }

    switch (applySettings_Seq){
      case 0:
        break;
      case 1:
        Serial.println(F("Applying settings..."));
        applySettings_Seq++;
        break;
      case 2:
        modemCmdFlag_Setc=true;  // callsign
        applySettings_Seq++;
        break;
      case 3:
        if(!modemCmdFlag_Setc){
          applySettings_Seq++;
        }
        break;
      case 4:
        modemCmdFlag_Setd=true;  // destination
        applySettings_Seq++;
        break;
      case 5:
        if(!modemCmdFlag_Setd){
          applySettings_Seq++;
        }
        break;
      case 6:
        modemCmdFlag_Set1=true;  // path1
        applySettings_Seq++;
        break;
      case 7:
        if(!modemCmdFlag_Set1){
          applySettings_Seq++;
        }
        break;
      case 8:
        modemCmdFlag_Set2=true;  // path2
        applySettings_Seq++;
        break;
      case 9:
        if(!modemCmdFlag_Set2){
          applySettings_Seq++;
        }
        break;
      case 10:
        modemCmdFlag_Setsc=true; // callsign ssid
        applySettings_Seq++;
        break;
      case 11:
        if(!modemCmdFlag_Setsc){
          applySettings_Seq++;
        }
        break;
      case 12:
        modemCmdFlag_Setsd=true; // destination ssid
        applySettings_Seq++;
        break;
      case 13:
        if(!modemCmdFlag_Setsd){
          applySettings_Seq++;
        }
        break;
      case 14:
        modemCmdFlag_Sets1=true; // path1 ssid
        applySettings_Seq++;
        break;
      case 15:
        if(!modemCmdFlag_Sets1){
          applySettings_Seq++;
        }
        break;
      case 16:
        modemCmdFlag_Sets2=true; // path2 ssid
        applySettings_Seq++;
        break;
      case 17:
        if(!modemCmdFlag_Sets2){
          applySettings_Seq++;
        }
        break;
      case 18:
        modemCmdFlag_Setls=true; // symbol
        applySettings_Seq++;
        break;
      case 19:
        if(!modemCmdFlag_Setls){
          applySettings_Seq++;
        }
        break;
      case 20:
        modemCmdFlag_Setlt=true; // table
        applySettings_Seq++;
        break;
      case 21:
        if(!modemCmdFlag_Setlt){
          applySettings_Seq++;
        }
        break;
      case 22:
        modemCmdFlag_Setma=true; // auto ack on/off
        applySettings_Seq++;
        break;
      case 23:
        if(!modemCmdFlag_Setma){
          applySettings_Seq++;
        }
        break;
      case 24:
        modemCmdFlag_Setw=true;  // preamble
        applySettings_Seq++;
        break;
      case 25:
        if(!modemCmdFlag_Setw){
          applySettings_Seq++;
        }
        break;
      case 26:
        modemCmdFlag_SetW=true;  // tail
        applySettings_Seq++;
        break;
      case 27:
        if(!modemCmdFlag_SetW){
          applySettings_Seq++;
        }
        break;
      case 28:
        if (saveModemSettings) {
          saveModemSettings=false;
          modemCmdFlag_SetS==true; // save the configuration
          applySettings_Seq++;
        } else {
          applySettings_Seq=0;
        }
        break;
      case 29:
        if(!modemCmdFlag_SetS){
          applySettings_Seq=0;
        }
        break;
    }
  }

#pragma endregion

#pragma region "GPS"

  #include <TinyGPS++.h>
  TinyGPSPlus gps;
  #define DESTINATION_REPORT_FREQUENCY 20000    // how often distance to target is sent to serial
  // http://ember2ash.com/lat.htm
  float currentLatDeg = 0;
  float currentLngDeg = 0;
  float lastLatDeg = 0.0;
  float lastLngDeg = 0.0;
  float lastLatDegSmartBeacon = 0.0;
  float lastLngDegSmartBeacon = 0.0;
  float smartBeaconDistanceLast = 0.0;
  char currentLat[9] = {'0','0','0','0','.','0','0','N','\0'};
  char currentLng[10] = {'0','0','0','0','0','.','0','0','N','\0'};
  bool modemCmdFlag_Lat=false, modemCmdFlag_Lng=false;
  unsigned long gps_report_timer, destination_report_timer, gps_initializing_timer;
  bool gpsInitialized = false; // we can set this when we get our first coordinate
  bool gpsInitializing;
  bool gpsSmartBeaconDistanceConditionMet = false;
  bool gpsLocationHasChanged = false;
  float smartBeaconDistance = 0.0;
  unsigned long currentIdleTime = 0.0;

  // London                                 LAT:51.508131     LNG:-0.128002
  //double DESTINATION_LAT = 51.508131, DESTINATION_LON = -0.128002;
    
  #define EARTH_RADIUS_MI 3958.8 // Earth's radius in miles
  float haversineMiles(float lat1, float lon1, float lat2, float lon2) {
    float toRad = PI / 180.0;
    float dLat = (lat2 - lat1) * toRad;
    float dLon = (lon2 - lon1) * toRad;

    float a = sin(dLat/2) * sin(dLat/2) +
              cos(lat1 * toRad) * cos(lat2 * toRad) *
              sin(dLon/2) * sin(dLon/2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return EARTH_RADIUS_MI * c;
  }
  float parseAprsLat(const char *aprsLat) {
    // Extract degrees (first 2 chars)
    uint8_t degrees = (aprsLat[0] - '0') * 10 + (aprsLat[1] - '0');

    // Extract minutes (next chars until hemisphere)
    float minutes = atof(&aprsLat[2]);

    // Hemisphere (N/S is at position 7 for 8-char string)
    char hemisphere = aprsLat[7];

    float decimalDegrees = degrees + (minutes / 60.0);

    if (hemisphere == 'S') decimalDegrees = -decimalDegrees;

    return decimalDegrees;
  }
  float parseAprsLon(const char *aprsLon) {
    // Extract degrees (first 3 chars)
    uint16_t degrees = (aprsLon[0] - '0') * 100 +
                      (aprsLon[1] - '0') * 10 +
                      (aprsLon[2] - '0');

    // Extract minutes (next chars until hemisphere)
    float minutes = atof(&aprsLon[3]);

    // Hemisphere (E/W is at position 8 for 9-char string)
    char hemisphere = aprsLon[8];

    float decimalDegrees = degrees + (minutes / 60.0);

    if (hemisphere == 'W') decimalDegrees = -decimalDegrees;

    return decimalDegrees;
  }

  void readGPS(){
    // monitor the serial port for data from the GPS
    while (Serial2.available() > 0){
      gps.encode(Serial2.read());
    }
    
    // blink display controls to indicate the GPS is still initializing
    if ((!gps.location.isUpdated() || !gps.location.isValid()) && millis() - gps_initializing_timer > 250){
      gps_initializing_timer = millis();
      gpsInitializing = !gpsInitializing;
    }

    // 
    if ( millis() - gps_report_timer > 1000){
        // reset timer
        gps_report_timer = millis();

        //
        if (gps.location.isUpdated() && gps.location.isValid())
        {
          gpsInitialized = true;
        /*Serial.print(F("LOCATION   Fix Age="));
          Serial.print(gps.location.age());
          Serial.print(F("ms Raw Lat="));
          Serial.print(gps.location.rawLat().negative ? "-" : "+");
          Serial.print(gps.location.rawLat().deg);
          Serial.print("[+");
          Serial.print(gps.location.rawLat().billionths);
          Serial.print(F(" billionths],  Raw Long="));
          Serial.print(gps.location.rawLng().negative ? "-" : "+");
          Serial.print(gps.location.rawLng().deg);
          Serial.print("[+");
          Serial.print(gps.location.rawLng().billionths);
          Serial.print(F(" billionths],  Lat="));
          Serial.print(F(" Lat="));
          Serial.print(gps.location.lat(), 6);
          Serial.print(F(" Long="));
          Serial.print(gps.location.lng(), 6); */
          
          // DDMM.MM http://ember2ash.com/lat.htm
          //Serial.print(F(" Lat2="));
          currentLatDeg = gps.location.rawLat().deg*100.0 + gps.location.rawLat().billionths*0.000000001*60.0;
          dtostrf(currentLatDeg, 8, 3, currentLat);
          currentLat[7] = gps.location.rawLat().negative ? 'S' : 'N';
          for (byte i = 0; i < sizeof(currentLat) - 1; i++) {
            if (currentLat[i] == ' ') currentLat[i] = '0';
          }
          if (currentLatDeg > lastLatDeg + abs(SETTINGS_GPS_POSITION_TOLERANCE)  ||
                currentLatDeg < lastLatDeg - abs(SETTINGS_GPS_POSITION_TOLERANCE)) {
            modemCmdFlag_Lat=true;
            gpsLocationHasChanged = true;
            lastLatDeg = currentLatDeg;
          }
          
          //Serial.print(F(" Long2="));
          currentLngDeg = gps.location.rawLng().deg*100.0 + gps.location.rawLng().billionths*0.000000001*60.0;
          dtostrf(currentLngDeg, 9, 3, currentLng);
          currentLng[8] = gps.location.rawLng().negative ? 'W' : 'E';
          for (byte i = 0; i < sizeof(currentLng) - 1; i++) {
            if (currentLng[i] == ' ') currentLng[i] = '0';
          }
          if (currentLngDeg > lastLngDeg + abs(SETTINGS_GPS_POSITION_TOLERANCE) ||
                currentLngDeg < lastLngDeg - abs(SETTINGS_GPS_POSITION_TOLERANCE)) {
            modemCmdFlag_Lng=true;
            gpsLocationHasChanged = true;
            lastLngDeg = currentLngDeg;
          }
          
        }
        if (gps.date.isUpdated())
        {
          /*
          Serial.print(F("DATE       Fix Age="));
          Serial.print(gps.date.age());
          Serial.print(F("ms Raw="));
          Serial.print(gps.date.value());
          Serial.print(F(" Year="));
          Serial.print(gps.date.year());
          Serial.print(F(" Month="));
          Serial.print(gps.date.month());
          Serial.print(F(" Day="));
          Serial.println(gps.date.day());
          */
          GPSData.Date.Day = gps.date.day();
          GPSData.Date.Month = gps.date.month();
          GPSData.Date.Year = gps.date.year();
          GPSData.Date.DateInt = gps.date.value();
        }
        if (gps.time.isUpdated())
        {
          /*
          Serial.print(F("TIME       Fix Age="));
          Serial.print(gps.time.age());
          Serial.print(F("ms Raw="));
          Serial.print(gps.time.value());
          Serial.print(F(" Hour="));
          Serial.print(gps.time.hour());
          Serial.print(F(" Minute="));
          Serial.print(gps.time.minute());
          Serial.print(F(" Second="));
          Serial.print(gps.time.second());
          Serial.print(F(" Hundredths="));
          Serial.println(gps.time.centisecond());
          */
          GPSData.Time.Hour = gps.time.hour();
          GPSData.Time.Minute = gps.time.minute();
          GPSData.Time.Second = gps.time.second();
          GPSData.Time.CentiSecond = gps.time.centisecond();
          GPSData.Time.TimeInt = gps.time.value();
        }
        if (gps.speed.isUpdated())
        {
          /*
          Serial.print(F("SPEED      Fix Age="));
          Serial.print(gps.speed.age());
          Serial.print(F("ms Raw="));
          Serial.print(gps.speed.value());
          Serial.print(F(" Knots="));
          Serial.print(gps.speed.knots());
          Serial.print(F(" MPH="));
          Serial.print(gps.speed.mph());
          Serial.print(F(" m/s="));
          Serial.print(gps.speed.mps());
          Serial.print(F(" km/h="));
          Serial.println(gps.speed.kmph());
          */
        }
        if (gps.course.isUpdated())
        {
          /*
          Serial.print(F("COURSE     Fix Age="));
          Serial.print(gps.course.age());
          Serial.print(F("ms Raw="));
          Serial.print(gps.course.value());
          Serial.print(F(" Deg="));
          Serial.println(gps.course.deg());
          */
        }
        if (gps.altitude.isUpdated())
        {
          /*
          Serial.print(F("ALTITUDE   Fix Age="));
          Serial.print(gps.altitude.age());
          Serial.print(F("ms Raw="));
          Serial.print(gps.altitude.value());
          Serial.print(F(" Meters="));
          Serial.print(gps.altitude.meters());
          Serial.print(F(" Miles="));
          Serial.print(gps.altitude.miles());
          Serial.print(F(" KM="));
          Serial.print(gps.altitude.kilometers());
          Serial.print(F(" Feet="));
          Serial.println(gps.altitude.feet());
          */
        }
        if (gps.satellites.isUpdated())
        {
          /*
          Serial.print(F("SATELLITES Fix Age="));
          Serial.print(gps.satellites.age());
          Serial.print(F("ms Value="));
          Serial.println(gps.satellites.value());
          */
          GPSData.Satellites = gps.satellites.value();
        }
        if (gps.hdop.isUpdated())
        {
          /*
          Serial.print(F("HDOP       Fix Age="));
          Serial.print(gps.hdop.age());
          Serial.print(F("ms raw="));
          Serial.print(gps.hdop.value());
          Serial.print(F(" hdop="));
          Serial.println(gps.hdop.hdop());
          */
        }
        if (millis() - destination_report_timer > DESTINATION_REPORT_FREQUENCY)
        {/*
          Serial.println();
          if (gps.location.isValid())
          {
            double distanceToDestination =
              TinyGPSPlus::distanceBetween(
                gps.location.lat(),
                gps.location.lng(),
                DESTINATION_LAT, 
                DESTINATION_LON);
            double courseToDestination =
              TinyGPSPlus::courseTo(
                gps.location.lat(),
                gps.location.lng(),
                SETTINGS_GPS_DESTINATION_LATITUDE, 
                SETTINGS_GPS_DESTINATION_LONGITUDE);
      
            Serial.print(F("DESTINATION     Distance="));
            Serial.print((distanceToDestination/1000.0)*0.621371, 6); // mile factor 0.621371
            Serial.print(F(" mi Course-to="));
            Serial.print(courseToDestination, 6);
            Serial.print(F(" degrees ["));
            Serial.print(TinyGPSPlus::cardinal(courseToDestination));
            Serial.println(F("]"));
          }
          
          /*
          Serial.print(F("DIAGS      Chars="));
          Serial.print(gps.charsProcessed());
          Serial.print(F(" Sentences-with-Fix="));
          Serial.print(gps.sentencesWithFix());
          Serial.print(F(" Failed-checksum="));
          Serial.print(gps.failedChecksum());
          Serial.print(F(" Passed-checksum="));
          Serial.println(gps.passedChecksum());
          */
      /*
          if (gps.charsProcessed() < 10)
            Serial.println(F("WARNING: No GPS data.  Check wiring."));
      
          destination_report_timer = millis();
          Serial.println();*/
        }

    }
  }

#pragma endregion

#pragma region "DISPLAYS"

  #include <Adafruit_GFX.h>
  #include <Adafruit_SH1106.h>

  #define CHAR_WIDTH                            6 // number of pixels that one character takes up on the screen
                                                  // TODO make this dynamic by getting from the display library

  #define CURSOR_MARGIN                         6 // number of pixels to have as a margin on the left of the display
  #define CHAR_GAP                              2 // number of pixels to have as a gap between chars

  #define DISPLAY_REFRESH_RATE                  100 // 33 is ~30fps | 100 is ~ 10fps
  #define DISPLAY_REFRESH_RATE_SCROLL           80  // during testing, i found that anything less than 60 causes performance issues
  #define DISPLAY_BLINK_RATE                    500
  #define UI_DISPLAY_HOME                       0
  #define UI_DISPLAY_MESSAGES                   1
  #define UI_DISPLAY_MESSAGES_NEW_MESSAGE       2
  #define UI_DISPLAY_LIVEFEED                   3
  #define UI_DISPLAY_SETTINGS                   4
  #define UI_DISPLAY_SETTINGS_APRS              5
  #define UI_DISPLAY_SETTINGS_GPS               6
  #define UI_DISPLAY_SETTINGS_DISPLAY           7
  #define UI_DISPLAY_SETTINGS_SAVE              8
  #define UI_DISPLAY_DEBUG                      9

  #define UI_DISPLAY_ROW_TOP                    0
  #define UI_DISPLAY_ROW_01                     12 // 8
  #define UI_DISPLAY_ROW_02                     22 // 16
  #define UI_DISPLAY_ROW_03                     30 // 14
  #define UI_DISPLAY_ROW_04                     38 // 32
  #define UI_DISPLAY_ROW_05                     46 // 40
  #define UI_DISPLAY_ROW_06                     54 // 48
  #define UI_DISPLAY_ROW_BOTTOM                 56

  unsigned char currentDisplay = UI_DISPLAY_HOME;
  unsigned char currentDisplayLast = currentDisplay;
  unsigned char previousDisplay = UI_DISPLAY_HOME;
  short int indexPosition_X = 0, indexPosition_X_Last = 0;
  uint32_t cursorPosition_X = 0, cursorPosition_X_Last = 0;
  uint32_t cursorPosition_Y = 0, cursorPosition_Y_Last = 0;
  short int ScrollingIndex_LiveFeed, ScrollingIndex_LiveFeed_minX;
  short int ScrollingIndex_MessageFeed, ScrollingIndex_MessageFeed_minX;

  // do something on first show of display
  bool displayInitialized = false;
  // leave the display after a timeout period
  bool leaveDisplay_MessageFeed = false, leaveDisplay_LiveFeed = false, leaveDisplay_Settings = false;
  // refresh the displays
  bool displayRefresh_Global = true, displayRefresh_Scroll = true;
  bool UnreadMessages = false;
  // go into edit mode in the settings
  bool editMode_Settings = false;
  bool displayBlink = false;
  unsigned char Settings_EditValueSize = 0;
  bool settingsChanged = false;
  bool sendMessage=false, sendMessage_Ack=false;
  unsigned long display_refresh_timer, display_refresh_timer_scroll, leave_display_timer;
  unsigned long display_blink_timer, display_timeout_timer, display_off_timer;

  // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
  #define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
  Adafruit_SH1106 display(OLED_RESET);

  #if (SH1106_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SH1106.h!");
  #endif

  // input pins
  #define rxPin A5 // using analog input pins
  #define txPin A6

  template <typename T> T numberOfDigits(T number){
    // https://studyfied.com/program/cpp-basic/count-number-of-digits-in-a-given-integer/
    // https://stackoverflow.com/questions/8627625/is-it-possible-to-make-function-that-will-accept-multiple-data-types-for-given-a/8627646
    /*
      This function assumes number is an integer type. Passing a float or double might lead to incorrect results or infinite loops because of how division works with floating-point numbers.
      If number is negative, it may not behave as expected unless you handle the sign first.
    */
      int count = 0;
      while(number != 0) {
        count++;
        number /= 10;
    }
    return count;
  }

  const uint8_t minBrightness = 32;
  const uint8_t maxBrightness = 255;
  const float breathingSpeedHz = 0.2;
  uint8_t calculateBreathingBrightness() {
    float t = millis() / 1000.0;  // Seconds
    float wave = (sin(2 * PI * breathingSpeedHz * t) + 1.0) / 2.0; // [0, 1]
    return map(wave * 100, 0, 100, minBrightness, maxBrightness);
  }
  void drawDitheredHeader(const String& text, uint8_t level) {
    // Assume cursor has already been set externally
    int startX = display.getCursorX();
    int startY = display.getCursorY();

    // Draw the text
    display.setTextColor(WHITE);
    display.print(text);

    // Width of text in pixels (6 px per char with default font)
    int textWidth = text.length() * CHAR_WIDTH;
    int textHeight = 8; // For default font height

    // Overlay a simulated dimming mask
    if (level < 255) {
      for (int y = 0; y < textHeight; y++) {
        for (int x = 0; x < textWidth; x++) {
          // 50% dither mask if brightness is low
          if (((x + y) % 2 == 0) && level < 128) {
            display.drawPixel(startX + x, startY + y, BLACK);
          }
          // 25% dither if brightness is very low
          else if (((x + y) % 4 == 0) && level < 64) {
            display.drawPixel(startX + x, startY + y, BLACK);
          }
        }
      }
    }
  }

  const char* screenNameOn;
  const char* screenNameOff;
  uint8_t headerXPos = 0;
  uint8_t getHeaderXPos(int16_t numHeaderChars){
      /*int16_t numberOfCharsThatCanFitOnScreen = (display.width() / CHAR_WIDTH);
      int16_t middleOfScreen = numberOfCharsThatCanFitOnScreen / 2;
      int16_t halfOfHeaderWidth = numHeaderChars / 2;
      return (middleOfScreen - halfOfHeaderWidth) * CHAR_WIDTH;*/
      return (((display.width() / CHAR_WIDTH) / 2) - (numHeaderChars / 2)) * CHAR_WIDTH;
  }

  // x bitmap
  const uint8_t noGpsIcon[8] = {
    0b10000001,
    0b01000010,
    0b00100100,
    0b00011000,
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001
  };

  void drawNoGpsIcon(int x, int y) {
    display.drawBitmap(x, y, noGpsIcon, 8, 8, WHITE);
  }

  void drawGpsStatusText(bool gpsConnected, uint32_t satellites, int16_t x, int16_t y) {
    display.setCursor(x, y);
    display.print("GPS-");

    //
    int textWidth = CHAR_WIDTH * 4;    // 6px per char, 4 letters: G P S |
    if (!gpsConnected) {
      // Strikethrough the word "GPS"
      int _x = x;                // Cursor X where "G" starts
      int _y = y + 3;           // Middle of text height (for strikethrough)
      display.drawLine(_x, _y, _x + textWidth - 1, _y, WHITE);
    }

    //
    display.setCursor(x + textWidth, y);
    display.print(satellites);
  }

  // while in edit mode, this method is used to edit a temporary character array that will later be
  // copied into memory. this makes it easier to manage editing a setting from the oled and keyboard.
  void handleDisplay_TempVarDisplay(int SettingsEditType){
    if (keyboardInputChar == KEYBOARD_LEFT_KEY) {
      // move the displayed value to the left
      if(indexPosition_X < 0){
        indexPosition_X++;
      }
      // move the cursor left as the x position is changed
      if (cursorPosition_X > 0) { 
        cursorPosition_X--;
      } else {
        cursorPosition_X=0;
      }
    }
    if (keyboardInputChar == KEYBOARD_RIGHT_KEY) {
      // move the displayed value to the right
      short int len = strlen(Settings_TempDispCharArr) * -1;
      if(indexPosition_X > len + 4){
        indexPosition_X--;
      }
      // move the cursor right as the x position is changed
      if (cursorPosition_X < Settings_EditValueSize) { 
        cursorPosition_X++;
      } else {
        cursorPosition_X=Settings_EditValueSize;
      }
    }
    bool characterDelete = false;
    if (keyboardInputChar == KEYBOARD_BACKSPACE_KEY){
      if (cursorPosition_X > 0) { 
        cursorPosition_X--;
        characterDelete = true;
      }
      // the beginning of the value is out of the window so lets start moving it back in
      // in the below graphic, the bar represents the edge of the screen
      // the dots are char positions
      // .....|     indexPosition_X = -5
      // .....|.... cursorPosition_X = 9
      if (indexPosition_X < 0 && cursorPosition_X - 4 <= abs(indexPosition_X)) {
        indexPosition_X++;
      }
    }
    switch (SettingsEditType) {
      case SETTINGS_EDIT_TYPE_ALT1:
        if (keyboardInputChar == KEYBOARD_DOWN_KEY) {
          if (Settings_TempDispCharArr[0] == 'S' || Settings_TempDispCharArr[0] == 's') {
            strcpy(Settings_TempDispCharArr, "Alternate");
          } else {
            strcpy(Settings_TempDispCharArr, "Standard");
          }
        }
        break;
      case SETTINGS_EDIT_TYPE_BOOLEAN:
        if (keyboardInputChar == KEYBOARD_DOWN_KEY) {
          if (Settings_TempDispCharArr[0] == 'T' || Settings_TempDispCharArr[0] == 't' || Settings_TempDispCharArr[0] == '1') {
            strcpy(Settings_TempDispCharArr, "False");
          } else {
            strcpy(Settings_TempDispCharArr, "True");
          }
        }
        break;
      case SETTINGS_EDIT_TYPE_INT:
          // TODO this is not validated because we have no settings of type int
          if (characterDelete) {
            if (cursorPosition_X >= 0) {
              Settings_TempDispCharArr[cursorPosition_X] = '\0';
            }
          } else if (KEYBOARD_NUMBER_KEYS) {
            if (cursorPosition_X < 5) { // int can be 5 digits -32,768 to 32,767
              Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
            }
          } else if (keyboardInputChar == KEYBOARD_MINUS_KEY) {
            int tempInt = strtoul(Settings_TempDispCharArr,NULL,10);
            tempInt = -tempInt;
            itoa(tempInt, Settings_TempDispCharArr, 10);
          }
        break;
      case SETTINGS_EDIT_TYPE_UINT:
          if (characterDelete) {
            if (cursorPosition_X >= 0) {
              Settings_TempDispCharArr[cursorPosition_X] = '\0';
            }
          } else if (KEYBOARD_NUMBER_KEYS) {
            if (cursorPosition_X < 5) { // uint can be 5 digits 0 to 65,535
              Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
            }
          }
        break;
      case SETTINGS_EDIT_TYPE_LONG:
          // TODO this is not validated because we have no settings of type long
          if (characterDelete) {
            if (cursorPosition_X >= 0) {
              Settings_TempDispCharArr[cursorPosition_X] = '\0';
            }
          } else if (KEYBOARD_NUMBER_KEYS) {
            if (cursorPosition_X < 10) { // long can be 10 digits -2,147,483,648 to 2,147,483,647
              Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
            }
          } else if (keyboardInputChar == KEYBOARD_MINUS_KEY) {
            int tempInt = strtoul(Settings_TempDispCharArr,NULL,10);
            tempInt = -tempInt;
            itoa(tempInt, Settings_TempDispCharArr, 10);
          }
        break;
      case SETTINGS_EDIT_TYPE_ULONG:
          if (characterDelete) {
            if (cursorPosition_X >= 0) {
              Settings_TempDispCharArr[cursorPosition_X] = '\0';
            }
          } else if (KEYBOARD_NUMBER_KEYS) {
            if (cursorPosition_X < 10) { // ulong can be 10 digits 0 to 4,294,967,295
              Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
            }
          }
        break;
      case SETTINGS_EDIT_TYPE_FLOAT:
          if (characterDelete) {
            if (cursorPosition_X >= 0) {
              Settings_TempDispCharArr[cursorPosition_X] = '\0';
            }
          } else if (KEYBOARD_NUMBER_KEYS || keyboardInputChar == KEYBOARD_PERIOD_KEY) {
            if (cursorPosition_X < 40) { // float can be 40 characters
              Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
            }
          } else if (keyboardInputChar == KEYBOARD_MINUS_KEY) {
            double tempDouble = strtod(Settings_TempDispCharArr,NULL);
            tempDouble = -tempDouble;
            dtostrf(tempDouble,3,6,Settings_TempDispCharArr); // https://www.programmingelectronics.com/dtostrf/
          }
        break;
      case SETTINGS_EDIT_TYPE_STRING2:
        if (characterDelete) {
          if (cursorPosition_X >= 0) {
            Settings_TempDispCharArr[cursorPosition_X] = '\0';
          }
        } else if KEYBOARD_PRINTABLE_CHARACTERS {
          if (cursorPosition_X < sizeof(Settings_TypeString2[0]) - 1) {
            Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
          }
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING3:
        if (characterDelete) {
          if (cursorPosition_X >= 0) {
            Settings_TempDispCharArr[cursorPosition_X] = '\0';
          }
        } else if KEYBOARD_PRINTABLE_CHARACTERS {
          if (cursorPosition_X < sizeof(Settings_TypeString3[0]) - 1) {
            Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
          }
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING7:
        if (characterDelete) {
          if (cursorPosition_X >= 0) {
            Settings_TempDispCharArr[cursorPosition_X] = '\0';
          }
        } else if KEYBOARD_PRINTABLE_CHARACTERS {
          if (cursorPosition_X < sizeof(Settings_TypeString7[0]) - 1) {
            Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
          }
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING100:
        if (characterDelete) {
          if (cursorPosition_X >= 0) {
            Settings_TempDispCharArr[cursorPosition_X] = '\0';
          }
        } else if KEYBOARD_PRINTABLE_CHARACTERS {
          if (cursorPosition_X < sizeof(Settings_TypeString100[0]) - 1) {
            Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
          }
          // the end of the string is out further than the edge of the screen.
          // move the index position to the left (negative) so that we can see the end of the string.
          if (cursorPosition_X >= display.width() / CHAR_WIDTH - 2) {
            indexPosition_X = (display.width() / CHAR_WIDTH - cursorPosition_X) - 2;
          }
        }
        break;
      default:
        break;
    }
    characterDelete = false;
  }

  // this method is used to copy over settings that are being modified.
  // while settings on the display are being modified, they are character arrays.
  // once the settings are applied, this setting converts them into the destination datatype.
  // this makes editing from the display and keyboard much easier.
  void handleDisplay_TempVarApply(int SettingsType, int SettingsTypeIndex){
    cursorPosition_X = 0;
    // apply edited values
    switch (SettingsType) {
      case SETTINGS_EDIT_TYPE_ALT1:
        if (Settings_TempDispCharArr[0] == 'S' || Settings_TempDispCharArr[0] == 's') {
          Settings_TypeString2[SettingsTypeIndex][0] = 's';
        } else {
          Settings_TypeString2[SettingsTypeIndex][0] = 'a';
        }
        Settings_TypeString2[SettingsTypeIndex][1] = '\0';
        break;
      case SETTINGS_EDIT_TYPE_BOOLEAN:
        if (Settings_TempDispCharArr[0] == 'T' || Settings_TempDispCharArr[0] == 't' || Settings_TempDispCharArr[0] == '1') {
          Settings_TypeBool[SettingsTypeIndex] = 1;
        } else {
          Settings_TypeBool[SettingsTypeIndex] = 0;
        }
        break;
      case SETTINGS_EDIT_TYPE_INT:
          Settings_TypeInt[SettingsTypeIndex] = strtoul(Settings_TempDispCharArr,NULL,10);
        break;
      case SETTINGS_EDIT_TYPE_UINT:
          Settings_TypeUInt[SettingsTypeIndex] = strtoul(Settings_TempDispCharArr,NULL,10);
        break;
      case SETTINGS_EDIT_TYPE_LONG:
          Settings_TypeLong[SettingsTypeIndex] = strtoul(Settings_TempDispCharArr,NULL,10);
        break;
      case SETTINGS_EDIT_TYPE_ULONG:
          Settings_TypeULong[SettingsTypeIndex] = strtoul(Settings_TempDispCharArr,NULL,10);
        break;
      case SETTINGS_EDIT_TYPE_FLOAT:
          Settings_TypeFloat[SettingsTypeIndex] = atof(Settings_TempDispCharArr);
        break;
      case SETTINGS_EDIT_TYPE_STRING2:
        for (int i=0; i<sizeof(Settings_TypeString2[SettingsTypeIndex]);i++) {
          Settings_TypeString2[SettingsTypeIndex][i] = Settings_TempDispCharArr[i];       
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING3:
        for (int i=0; i<sizeof(Settings_TypeString3[SettingsTypeIndex]);i++) {
          Settings_TypeString3[SettingsTypeIndex][i] = Settings_TempDispCharArr[i];       
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING7:
        for (int i=0; i<sizeof(Settings_TypeString7[SettingsTypeIndex]);i++) {
          Settings_TypeString7[SettingsTypeIndex][i] = Settings_TempDispCharArr[i];       
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING100:
        for (int i=0; i<sizeof(Settings_TypeString100[SettingsTypeIndex]);i++) {
          Settings_TypeString100[SettingsTypeIndex][i] = Settings_TempDispCharArr[i];       
        }
        break;
      default:
        break;
    }
  }

  // this method is used to copy the current setting from memory to the temporarily displayed
  // setting in the handleDisplay_TempVarDisplay method so that it can be edited in a temporary area
  // before being copied back to memory using the handleDisplay_TempVarApply method.
  void handleDisplay_TempVarCopy(int SettingsType, int SettingsTypeIndex){
    // clear the char array first
    for (int i=0; i<sizeof(Settings_TempDispCharArr);i++) {
      Settings_TempDispCharArr[i] = '\0';
    }
    // copy data to temp variable
    switch (SettingsType) {
      case SETTINGS_EDIT_TYPE_ALT1:
        if (Settings_TypeString2[SettingsTypeIndex][0] == 'S' || Settings_TypeString2[SettingsTypeIndex][0] == 's') {
          strcpy(Settings_TempDispCharArr, "Standard");
        } else {
          strcpy(Settings_TempDispCharArr, "Alternate");
        }
        break;
      case SETTINGS_EDIT_TYPE_BOOLEAN:
        if (Settings_TypeBool[SettingsTypeIndex]) {
          strcpy(Settings_TempDispCharArr, "True");
        } else {
          strcpy(Settings_TempDispCharArr, "False");
        }
        break;
      case SETTINGS_EDIT_TYPE_INT:
        itoa(Settings_TypeInt[SettingsTypeIndex],Settings_TempDispCharArr,10);
        break;
      case SETTINGS_EDIT_TYPE_UINT:
        ultoa(Settings_TypeUInt[SettingsTypeIndex],Settings_TempDispCharArr,10);
        break;
      case SETTINGS_EDIT_TYPE_LONG:
        ltoa(Settings_TypeLong[SettingsTypeIndex],Settings_TempDispCharArr,10);
        break;
      case SETTINGS_EDIT_TYPE_ULONG:
        ultoa(Settings_TypeULong[SettingsTypeIndex],Settings_TempDispCharArr,10);
        break;
      case SETTINGS_EDIT_TYPE_FLOAT:
        dtostrf(Settings_TypeFloat[SettingsTypeIndex],3,6,Settings_TempDispCharArr);
        break;
      case SETTINGS_EDIT_TYPE_STRING2:
        for (int i=0; i<strlen(Settings_TypeString2[SettingsTypeIndex]);i++) {
          Settings_TempDispCharArr[i] = Settings_TypeString2[SettingsTypeIndex][i];       
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING3:
        for (int i=0; i<strlen(Settings_TypeString3[SettingsTypeIndex]);i++) {
          Settings_TempDispCharArr[i] = Settings_TypeString3[SettingsTypeIndex][i];       
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING7:
        for (int i=0; i<strlen(Settings_TypeString7[SettingsTypeIndex]);i++) {
          Settings_TempDispCharArr[i] = Settings_TypeString7[SettingsTypeIndex][i];       
        }
        break;
      case SETTINGS_EDIT_TYPE_STRING100:
        short int len = strlen(Settings_TypeString100[SettingsTypeIndex]);
        for (int i=0; i < len;i++) {
          Settings_TempDispCharArr[i] = Settings_TypeString100[SettingsTypeIndex][i];       
        }
        // the string extends out past the edge of the screen so we set the index position more negative so we can see it.
        if (len > display.width() / CHAR_WIDTH){
          indexPosition_X = -(len - display.width() / CHAR_WIDTH);
        }
        break;
      default:
        break;
    }
  }

  // this method is used to print the current value of a setting onto the display for reference
  // while scrolling through the settings menu.
  void handleDisplay_PrintValStoredInMem(int SettingsType, int SettingsTypeIndex){
    //
    display.setCursor(cursorPosition_X * CHAR_WIDTH,UI_DISPLAY_ROW_BOTTOM);
    //
    switch (SettingsType) {
      case SETTINGS_EDIT_TYPE_ALT1:
        Settings_EditValueSize = 0;
        if (Settings_TypeString2[SettingsTypeIndex][0] == 'S' || Settings_TypeString2[SettingsTypeIndex][0] == 's') {
          display.print(F("Standard"));
        } else {
          display.print(F("Alternate"));
        }
        break;
      case SETTINGS_EDIT_TYPE_BOOLEAN:
        Settings_EditValueSize = 0;
        if (Settings_TypeBool[SettingsTypeIndex]) {
          display.print(F("True"));
        } else {
          display.print(F("False"));
        }
        break;
      case SETTINGS_EDIT_TYPE_INT:
        Settings_EditValueSize = numberOfDigits<int>(Settings_TypeInt[SettingsTypeIndex]);
        display.print(Settings_TypeInt[SettingsTypeIndex]);
        break;
      case SETTINGS_EDIT_TYPE_UINT:
        Settings_EditValueSize = numberOfDigits<unsigned int>(Settings_TypeUInt[SettingsTypeIndex]);
        display.print(Settings_TypeUInt[SettingsTypeIndex]);
        break;
      case SETTINGS_EDIT_TYPE_LONG:
        Settings_EditValueSize = numberOfDigits<long>(Settings_TypeLong[SettingsTypeIndex]);
        display.print(Settings_TypeLong[SettingsTypeIndex]);
        break;
      case SETTINGS_EDIT_TYPE_ULONG:
        Settings_EditValueSize = numberOfDigits<unsigned long>(Settings_TypeULong[SettingsTypeIndex]);
        display.print(Settings_TypeULong[SettingsTypeIndex]);
        break;
      case SETTINGS_EDIT_TYPE_FLOAT:
        Settings_EditValueSize = numberOfDigits<float>(Settings_TypeFloat[SettingsTypeIndex]);
        display.print(Settings_TypeFloat[SettingsTypeIndex],6);
        break;
      case SETTINGS_EDIT_TYPE_STRING2:
        Settings_EditValueSize = sizeof(Settings_TypeString2[SettingsTypeIndex]) - 1;
        display.print(Settings_TypeString2[SettingsTypeIndex]);
        break;
      case SETTINGS_EDIT_TYPE_STRING3:
        Settings_EditValueSize = sizeof(Settings_TypeString3[SettingsTypeIndex]) - 1;
        display.print(Settings_TypeString3[SettingsTypeIndex]);
        break;
      case SETTINGS_EDIT_TYPE_STRING7:
        Settings_EditValueSize = sizeof(Settings_TypeString7[SettingsTypeIndex]) - 1;
        display.print(Settings_TypeString7[SettingsTypeIndex]);
        break;
      case SETTINGS_EDIT_TYPE_STRING100:
        Settings_EditValueSize = sizeof(Settings_TypeString100[SettingsTypeIndex]) - 1;
        display.print(Settings_TypeString100[SettingsTypeIndex]);
        break;
      default:
        break;
    }
  }

  // while in edit mode, this method is used to temporarily display and edit a setting on the oled using a keyboard.
  // the setting displayed is a character array to make it easier to work with while editing.
  void handleDisplay_PrintTempVal(){
    //
    Settings_EditValueSize = sizeof(Settings_TempDispCharArr) - 1;
    //
    display.setCursor(indexPosition_X * CHAR_WIDTH, UI_DISPLAY_ROW_BOTTOM - 1);
    display.print(Settings_TempDispCharArr);
    cursorPosition_X = strlen(Settings_TempDispCharArr);
    //
    if (displayBlink) {
      display.setCursor((cursorPosition_X * CHAR_WIDTH) + (indexPosition_X * CHAR_WIDTH), UI_DISPLAY_ROW_BOTTOM);
      display.print('_');
    }
  }

  // this method is used to determine the currently selected row based on the cursor position.
  // as the up and down arrows are pressed, the cursor position is added or subtracted to navigate.
  // this assigns an actual pixel count based on global settings. in other words, if your cursor is 
  // equal to 0 then it is considered to be on "ROW 1" and will be printed at the position defined as 
  // "ROW 1" - in this case UI_DISPLAY_ROW_01 = 8
  int handleDisplay_GetSelectionRow(int cursorPosition){
    int selectionRow = 0;
    switch (cursorPosition) {
      case 0:
        selectionRow = UI_DISPLAY_ROW_01;
        break;
      case 1:
        selectionRow = UI_DISPLAY_ROW_02;
        break;
      case 2:
        selectionRow = UI_DISPLAY_ROW_03;
        break;
      case 3:
        selectionRow = UI_DISPLAY_ROW_04;
        break;
      default:
        selectionRow = UI_DISPLAY_ROW_04;
        break;
    }
    return selectionRow;
  }

  void handleDisplays(){ 
    // these timers are used to drive various display element. for example, there is a
    // timer that is used to refresh the display, scroll the display, blink elements like
    // a period, and even put the display to sleep
    if (millis() - display_refresh_timer > DISPLAY_REFRESH_RATE){
      displayRefresh_Global = true;
      display_refresh_timer = millis();
    }
    if (millis() - display_refresh_timer_scroll > DISPLAY_REFRESH_RATE_SCROLL){
      displayRefresh_Scroll = true;
      display_refresh_timer_scroll = millis();
    }
    if (millis() - display_blink_timer > DISPLAY_BLINK_RATE) {
      displayBlink = !displayBlink;
      display_blink_timer = millis();
    }
    if (millis() - display_timeout_timer > SETTINGS_DISPLAY_TIMEOUT && !displayDim) {
      displayDim = true;
      display.SH1106_command(SH1106_SETCONTRAST);
      display.SH1106_command(0);
    } else if (wakeDisplay){ // wakeDisplay is called in various places such as when the keyboard is pressed or a message is received
      wakeDisplay = false;
      displayDim = false;
      display.SH1106_command(SH1106_SETCONTRAST);
      display.SH1106_command((uint8_t)constrain(map(SETTINGS_DISPLAY_BRIGHTNESS,0,100,0,254),0,254));
      display.SH1106_command(SH1106_DISPLAYON);
      display_timeout_timer = millis();
    }

    // turn off the screen after display_timeout_timer timer has expired
    if (displayDim) {
      if (millis() - display_off_timer > 3000) {
        display.SH1106_command(SH1106_DISPLAYOFF);
      }
    } else {
      display_off_timer = millis();
    }

    // the display defaults to having a dark background and light text. that can be inverted here.
    display.invertDisplay(SETTINGS_DISPLAY_INVERT);

    // when display changes, call for the new display to initialize. the display logic will
    // see the displayInitialized bit drop and proceed to run its init logic.
    if (currentDisplay != currentDisplayLast) {
      currentDisplayLast = currentDisplay;
      displayInitialized = false;
    }

    // add display objects to buffer. all of the contents that make up each display are organized
    // in each one of the method calls below. the switch..case ensures that each are mutually exclusive 
    // and do not overwrite each other. the navigation between displays is controlled by the currentDisplay
    // variable. when the value changes, the display changes to the display accordingly. this value is 
    // written to from within the logic of each displays respective method.
    switch (currentDisplay) {
      case UI_DISPLAY_HOME:
        handleDisplay_Home();
        break;
      case UI_DISPLAY_MESSAGES:
        handleDisplay_Messages();
        break;
      case UI_DISPLAY_MESSAGES_NEW_MESSAGE:
        handleDisplay_Messages_NewMessage();
        break;
      case UI_DISPLAY_LIVEFEED:
        handleDisplay_LiveFeed();
        break;
      case UI_DISPLAY_SETTINGS:
        handleDisplay_Settings();
        break;
      case UI_DISPLAY_SETTINGS_APRS:
        handleDisplay_Settings_APRS();
        break;
      case UI_DISPLAY_SETTINGS_GPS:
        handleDisplay_Settings_GPS();
        break;
      case UI_DISPLAY_SETTINGS_DISPLAY:
        handleDisplay_Settings_Display();
        break;
      case UI_DISPLAY_SETTINGS_SAVE:
        handleDisplay_Settings_Save();
        break;
      case UI_DISPLAY_DEBUG:
        handleDisplay_Debug();
        break;
      default:
        handleDisplay_Home();
        break;
    }
    
    // clear these bits here at the end of this method since all other methods are called within and need a 
    // chance to see the value of these variables for at least one cycle.
    displayRefresh_Global = false;
    displayRefresh_Scroll = false;
  }

  // this method is called once in the setup() method of the controller and is used to initialize the
  // display with things such as text size, text color, user callsign, and device info like build date.
  void handleDisplay_Startup(){
    
    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    //display.display();
    //delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();
    
    display.setTextSize(1);      // Normal 1:1 pixel scale - default letter size is 5x8 pixels
    display.setTextColor(WHITE); 
    display.setTextWrap(false);
    
    display.setCursor(30,UI_DISPLAY_ROW_01); 
    display.println(F("HamMessenger"));
    
    display.setCursor(30,UI_DISPLAY_ROW_03);
    display.println(F("Build Date"));
    
    display.setCursor(0,UI_DISPLAY_ROW_04);
    display.println(version);
    
    display.setCursor(30,UI_DISPLAY_ROW_06); 
    display.println(F("Call:"));
    
    display.setCursor(60,UI_DISPLAY_ROW_06);
    display.println(SETTINGS_APRS_CALLSIGN);
    
    display.display();
    delay(3000);
  }

  void handleDisplay_Debug(){
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      screenNameOn = "[ DEBUG ]";
      screenNameOff = "[       ]";
      headerXPos = getHeaderXPos(strlen(screenNameOn));
    }
    // handle button context for current display
    if (keyboardInputChar == KEYBOARD_UP_KEY){
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=2;
      }
    }
    if (keyboardInputChar == KEYBOARD_DOWN_KEY){ 
      if (cursorPosition_Y < 2){
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == KEYBOARD_LEFT_KEY){
    }
    if (keyboardInputChar == KEYBOARD_RIGHT_KEY){
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      currentDisplay = UI_DISPLAY_HOME;
    }      
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();
      
      // add global objects to buffer
      handleDisplay_Global();

      // handle cursor
      //int selectionRow = handleDisplay_GetSelectionRow(cursorPosition_Y+1);
      //display.setCursor(0,selectionRow);
      //display.print(F(">"));

      // handle header
      display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
      display.print(screenNameOff);
      if(displayBlink || !displayInitialized){
        display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
        display.print(screenNameOn);
      }
      
      // handle body
      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_02);
      display.print(F("Idle:"));
      display.setCursor(36,UI_DISPLAY_ROW_02); // 6 pixels + 6 pixels * 5 chars = 36
      display.print(currentIdleTime);

      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_03);
      display.print(F("Dist:"));
      display.setCursor(36,UI_DISPLAY_ROW_03); // 6 pixels + 6 pixels * 5 chars = 36
      display.print(smartBeaconDistance);

      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_04);
      display.print(F("Loc:"));
      display.setCursor(30,UI_DISPLAY_ROW_04); // 6 pixels + 6 pixels * 4 chars = 30
      display.print((gpsLocationHasChanged ? "True" : "False"));

      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_05);
      display.print(F("Spd:"));
      display.setCursor(30,UI_DISPLAY_ROW_05); // 6 pixels + 6 pixels * 4 chars = 30
      display.print(gps.speed.mph());

      display.setCursor(66, UI_DISPLAY_ROW_05); // 30 pixels + 6 pixels * 6 chars = 66
      display.print(F("hdop:"));
      display.setCursor(96, UI_DISPLAY_ROW_05); // 66 pixels + 6 pixels * 5 chars = 96
      display.print(gps.hdop.value());

      
      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    }
  }

  // this method handles all of the global objects of the display. any objects that are common between
  // the displays will go here. things such as voltage, scan time, latitude/longitude, etc all are 
  // added to the display buffer here. each of the displays will call this method when building its
  // own display.
  void handleDisplay_Global(){
    //
    if (digitalRead(rxPin) == HIGH){
      display.setCursor(0,UI_DISPLAY_ROW_TOP);
      display.print(F("Rx"));
    }
    if (digitalRead(txPin) == HIGH){
      display.setCursor(13,UI_DISPLAY_ROW_TOP);
      display.print(F("Tx"));
    }
    if (gpsInitializing || gps.location.isValid()){
      //display.setCursor(31,UI_DISPLAY_ROW_TOP);
      //display.print(F("GPS"));
      drawGpsStatusText(gps.location.isValid(), gps.satellites.value(), 26, UI_DISPLAY_ROW_TOP);
    }
    /* TODO add this object and manage unread messages so that the user is notified
    if (UnreadMessages){
      display.setCursor(40,UI_DISPLAY_ROW_TOP);
      display.print(F("Msg"));
    }
    */

    // display the voltage
    display.setCursor(65,UI_DISPLAY_ROW_TOP);
    display.print(String((float)Voltage/1000.0,1) + F("V"));
    
    // display the cpu scan time
    display.setCursor(93,UI_DISPLAY_ROW_TOP);
    if (scanTime >= 10000) {
        float timeMs = scanTime / 1000.0;
        display.print(timeMs, 1);  // One decimal place
        display.print(F("ms"));
    } else {
        display.print(scanTime);
        display.print(F("us"));
    }

    // display gps location
    if (SETTINGS_DISPLAY_SHOW_POSITION) {
      int x_lat = 13;
      display.setCursor(x_lat,UI_DISPLAY_ROW_BOTTOM);
      display.print(currentLat); // DDMM.mmN

      int x_lng = x_lat + (8 * CHAR_WIDTH) + 5; // 8 letters times 6 pixels wide plus a gap of 5 (0000.00N)
      display.setCursor(x_lng, UI_DISPLAY_ROW_BOTTOM);
      display.print(currentLng); // DDDMM.mmW
    }
  }

  // this is the upper most level display and is mostly used for navigation.
  void handleDisplay_Home(){
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      screenNameOn = "[ HOME ]";
      screenNameOff = "[      ]";
      headerXPos = getHeaderXPos(strlen(screenNameOn));
    }
    // handle button context for current display
    if (keyboardInputChar == KEYBOARD_UP_KEY){
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=2;
      }
    }
    if (keyboardInputChar == KEYBOARD_DOWN_KEY){ 
      if (cursorPosition_Y < 2){
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == KEYBOARD_LEFT_KEY){
      currentDisplay = UI_DISPLAY_DEBUG;
      previousDisplay = UI_DISPLAY_HOME;
    }
    if (keyboardInputChar == KEYBOARD_RIGHT_KEY){
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
      switch (cursorPosition_Y){
        case 0:
          currentDisplay = UI_DISPLAY_MESSAGES;
          break;
        case 1:
          currentDisplay = UI_DISPLAY_LIVEFEED;
          break;
        case 2:
          currentDisplay = UI_DISPLAY_SETTINGS;
          break;
        default:
          currentDisplay = UI_DISPLAY_HOME;
          break;
      }
      previousDisplay = UI_DISPLAY_HOME;
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
    }      
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();
      
      // add global objects to buffer
      handleDisplay_Global();

      // handle cursor
      int selectionRow = handleDisplay_GetSelectionRow(cursorPosition_Y+1);
      display.setCursor(0,selectionRow);
      display.print(F(">"));

      // handle header
      display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
      display.print(screenNameOff);
      if(displayBlink || !displayInitialized){
        display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
        display.print(screenNameOn);
      }
      
      // handle body
      size_t charsPrinted, closeCursorPosition;
      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_02);
      charsPrinted = display.print(F("Messages"));
      if (selectionRow == UI_DISPLAY_ROW_02) closeCursorPosition = charsPrinted;

      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_03);
      charsPrinted = display.print(F("Live Feed"));
      if (selectionRow == UI_DISPLAY_ROW_03) closeCursorPosition = charsPrinted;

      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_04);
      charsPrinted = display.print(F("Settings"));
      if (selectionRow == UI_DISPLAY_ROW_04) closeCursorPosition = charsPrinted;

      // display closing cursor
      display.setCursor(CURSOR_MARGIN + CHAR_GAP + closeCursorPosition * CHAR_WIDTH + (CHAR_GAP - 1), selectionRow);
      display.print(F("<"));
      
      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    }
  }

  APRSFormat_Msg MsgData;
  uint32_t MsgDataRecordCount;
  void handleDisplay_Messages(){
    //  Radio 1: CMD: Modem:#Hi!
    //  Radio 1: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :Hi!{006
    //  Radio 2: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :ack006
    
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = cursorPosition_X;
      cursorPosition_Y_Last = -1;
      //MsgDataRecordCount = getMsgDataRecord(cursorPosition_Y + 1, MsgData);
      leave_display_timer = millis();
    }
    // handle button context for current display
    if (keyboardInputChar == KEYBOARD_UP_KEY){
      if (cursorPosition_Y < MsgDataRecordCount - 1){
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == KEYBOARD_DOWN_KEY){
      if (cursorPosition_Y > 0){ // dont scroll past the number of records in the array
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=(MsgDataRecordCount > 0 ? MsgDataRecordCount - 1 : 0); // wanna roll to the end. check if record count is not 0 first
      }
    }
    if (keyboardInputChar == KEYBOARD_LEFT_KEY){
    }
    if (keyboardInputChar == KEYBOARD_RIGHT_KEY){
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
      // TODO: we want to go into a new message display here.
      // if the currently highlighted message is valid, the new message will automatically
      // set the destination the same as the source of the currently highlighted message - effectively
      // implementing a "reply" feature. otherwise, the destination will be set to its last value.

      //
      currentDisplay = UI_DISPLAY_MESSAGES_NEW_MESSAGE;
      previousDisplay = UI_DISPLAY_MESSAGES;

      //
      memset(&SETTINGS_APRS_MESSAGE, 0, sizeof(SETTINGS_APRS_MESSAGE));
      SETTINGS_APRS_MESSAGE[sizeof(SETTINGS_APRS_MESSAGE) - 1] = '\0';

      //
      extractMsgCallParts(MsgData.from, SETTINGS_APRS_RECIPIENT_CALL, SETTINGS_APRS_RECIPIENT_SSID);

    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      currentDisplay = previousDisplay;
      return;
    }

    // build display
    if (displayRefresh_Scroll || keyboardInputChar != 0){
      if (cursorPosition_Y != cursorPosition_Y_Last){ // changed to new record (index)
        cursorPosition_Y_Last = cursorPosition_Y;

        //
        MsgDataRecordCount = getMsgDataRecord(cursorPosition_Y + 1, MsgData); // adding 1 here because cursorPosition_Y is zero indexed but getMsgDataRecord is not
        
        //
        int dataLen = strlen(MsgData.msg);
        ScrollingIndex_MessageFeed_minX = -CHAR_WIDTH * 2 * dataLen; // 12 = 6 pixels/character * text size 2
        if (!SETTINGS_DISPLAY_SCROLL_MESSAGES) {
          ScrollingIndex_MessageFeed = 0;
        } else {
          ScrollingIndex_MessageFeed = display.width(); // starting point for text 
        }

      }

      // clear the buffer
      display.clearDisplay();

      // add global objects to buffer
      handleDisplay_Global();

      //
      if (MsgDataRecordCount > 0){
        char to_from[24] = {'\0'};
        byte index = 0;
        for (byte i=0;i<sizeof(MsgData.from)-1;i++){
          if (MsgData.from[i] != '\0'){
            to_from[index] = MsgData.from[i];
            index++;
          } else {
            i = sizeof(MsgData.from); // get out
          }
        }
        to_from[index] = '>'; index++;
        for (byte i=0;i<sizeof(MsgData.to)-1;i++){
          if (MsgData.to[i] != '\0'){
            to_from[index] = MsgData.to[i];
            index++;
          } else {
            i = sizeof(MsgData.to); // get out
          }
        }
        // display who the message is to and from
        display.setCursor(0,UI_DISPLAY_ROW_01);
        display.print(to_from);
        // display message
        display.setCursor(ScrollingIndex_MessageFeed,UI_DISPLAY_ROW_02);
        display.setTextSize(2);
        display.print(MsgData.msg); 
        // go back to original text size
        display.setTextSize(1); // Normal 1:1 pixel scale - default letter size is 6x8 pixels
        // display the cursor position (represents record number in this case)
        display.setCursor(0,UI_DISPLAY_ROW_04); display.print(F("Record: "));
        display.setCursor(45,UI_DISPLAY_ROW_04); display.print(cursorPosition_Y+1);
        display.setCursor(45 + numberOfDigits<uint32_t>(cursorPosition_Y+1) * 5,UI_DISPLAY_ROW_04); display.print(F(" of "));
        display.setCursor(65 + numberOfDigits<uint32_t>(cursorPosition_Y+1) * 5,UI_DISPLAY_ROW_04); display.print(MsgDataRecordCount);
        // display the date and time
        display.setCursor(0,UI_DISPLAY_ROW_05); display.print(F("D:"));
        display.setCursor(15,UI_DISPLAY_ROW_05); display.print(MsgData.DateInt);
        display.setCursor(60,UI_DISPLAY_ROW_05); display.print(F("T:"));
        display.setCursor(75,UI_DISPLAY_ROW_05); display.print(MsgData.TimeInt);
        unsigned int scrollPixelCount = (SETTINGS_DISPLAY_SCROLL_MESSAGES ? SETTINGS_DISPLAY_SCROLL_SPEED : 32);
        if (keyboardInputChar == KEYBOARD_LEFT_KEY || SETTINGS_DISPLAY_SCROLL_MESSAGES){ //  scroll only when enter pressed TODO: this wont work because key press not persistent
          ScrollingIndex_MessageFeed = ScrollingIndex_MessageFeed - scrollPixelCount; // higher number here is faster scroll but choppy
          if(ScrollingIndex_MessageFeed < ScrollingIndex_MessageFeed_minX) ScrollingIndex_MessageFeed = display.width(); // makeshift scroll because startScrollleft truncates the string!
        } else if (keyboardInputChar == KEYBOARD_RIGHT_KEY){
          ScrollingIndex_MessageFeed = ScrollingIndex_MessageFeed + scrollPixelCount;
          if(ScrollingIndex_MessageFeed > display.width()) ScrollingIndex_MessageFeed = ScrollingIndex_MessageFeed_minX;
        }
      } else {
        display.setCursor(0, UI_DISPLAY_ROW_02);
        display.print(F("You have no messages :("));
        if (!leaveDisplay_MessageFeed) {
          leaveDisplay_MessageFeed = true;
          leave_display_timer = millis();
        }
      }
      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    }
    // timeout and leave
    if (millis() - leave_display_timer > SETTINGS_DISPLAY_TIMEOUT && leaveDisplay_MessageFeed){
      leaveDisplay_MessageFeed = false;
      currentDisplay = UI_DISPLAY_HOME;
      return;
    }
  }

  void handleDisplay_Messages_NewMessage(){
    //  Radio 1: CMD: Modem:#Hi!
    //  Radio 1: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :Hi!{006
    //  Radio 2: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :ack006
    
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      indexPosition_X = 0;
      indexPosition_X_Last = indexPosition_X;
      cursorPosition_X = 0;
      cursorPosition_Y = 5; // 5, 6, and 7 are message, recipient callsign, and recipient SSID
      cursorPosition_X_Last = cursorPosition_X;
      cursorPosition_Y_Last = -1;
      editMode_Settings = false;
      screenNameOn = "[ NEW MESSAGE ]";
      screenNameOff = "[             ]";
      headerXPos = getHeaderXPos(strlen(screenNameOn));
    }
    // handle button context for current display
    if (KEYBOARD_PRINTABLE_CHARACTERS || KEYBOARD_DIRECTIONAL_KEYS || keyboardInputChar == KEYBOARD_BACKSPACE_KEY) {
      if (editMode_Settings){
        handleDisplay_TempVarDisplay(Settings_Type_APRS[cursorPosition_Y]);
      } else if (keyboardInputChar == KEYBOARD_UP_KEY) {
        if (cursorPosition_Y > 5) { // 5 is message
          cursorPosition_Y--;
        } else {
          cursorPosition_Y = 7; // 7 is recipient ID
        }
      } else if (keyboardInputChar == KEYBOARD_DOWN_KEY) {
        if (cursorPosition_Y < 7) { // 7 is recipient ID
          cursorPosition_Y++;
        } else {
          cursorPosition_Y = 5; // 5 is message
        }
      }
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
      if (editMode_Settings) {
        //
        editMode_Settings = false;
        handleDisplay_TempVarApply(Settings_Type_APRS[cursorPosition_Y],Settings_TypeIndex_APRS[cursorPosition_Y]);
        if (cursorPosition_Y == 5){
          // trigger a message send
          sendMessage = true;
        }
      } else {
        // enable edit mode
        editMode_Settings = true;
        indexPosition_X = 0;
        handleDisplay_TempVarCopy(Settings_Type_APRS[cursorPosition_Y],Settings_TypeIndex_APRS[cursorPosition_Y]);
      }
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      editMode_Settings = false;
      cursorPosition_X = 0;
      currentDisplay = previousDisplay;
      previousDisplay = UI_DISPLAY_HOME;
      return;
    }
    // build display
    if (displayRefresh_Global){

      // clear the buffer
      display.clearDisplay();

      // add global objects to buffer
      bool showPositionMemory = SETTINGS_DISPLAY_SHOW_POSITION;
      SETTINGS_DISPLAY_SHOW_POSITION = false; // we dont want to show the lat long while editing
      handleDisplay_Global();
      SETTINGS_DISPLAY_SHOW_POSITION = showPositionMemory; // put it back to what it was

      // handle cursor
      int selectionRow = handleDisplay_GetSelectionRow(cursorPosition_Y - 4);
      display.setCursor(0,selectionRow);
      display.print(F(">"));

      // handle header
      display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
      display.print(screenNameOff);
      if(displayBlink || !displayInitialized){
        display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
        display.print(screenNameOn);
      }
      
      // handle body
      size_t charsPrinted, closeCursorPosition;
      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_02);
      charsPrinted = display.print(MenuItems_Settings_APRS[5]);
      if (selectionRow == UI_DISPLAY_ROW_02) closeCursorPosition = charsPrinted;

      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_03);
      charsPrinted = display.print(MenuItems_Settings_APRS[6]);
      if (selectionRow == UI_DISPLAY_ROW_03) closeCursorPosition = charsPrinted;

      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_04);
      charsPrinted = display.print(MenuItems_Settings_APRS[7]);
      if (selectionRow == UI_DISPLAY_ROW_04) closeCursorPosition = charsPrinted;

      // display closing cursor
      display.setCursor(CURSOR_MARGIN + CHAR_GAP + closeCursorPosition * CHAR_WIDTH + (CHAR_GAP - 1), selectionRow);
      display.print(F("<"));  

      // handle edit field
      if (editMode_Settings) {
        handleDisplay_PrintTempVal();
      } else {
        handleDisplay_PrintValStoredInMem(Settings_Type_APRS[cursorPosition_Y],Settings_TypeIndex_APRS[cursorPosition_Y]);
      }
      
      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    }
  }

  APRSFormat_Raw RawData;
  uint32_t RawDataRecordCount;
  void handleDisplay_LiveFeed(){
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = cursorPosition_X;
      cursorPosition_Y_Last = -1;
      //RawDataRecordCount = getRawDataRecord(cursorPosition_Y + 1, RawData);
      leave_display_timer = millis();
    }
    // handle button context for current display
    if (keyboardInputChar == KEYBOARD_UP_KEY){
      if (cursorPosition_Y < RawDataRecordCount - 1){
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == KEYBOARD_DOWN_KEY){
      if (cursorPosition_Y > 0){ // dont scroll past the number of records in the array
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=(RawDataRecordCount > 0 ? RawDataRecordCount - 1 : 0); // wanna roll to the end. check if record count is not 0 first
      }
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      currentDisplay = previousDisplay;
      return;
    }
    // build display
    if (displayRefresh_Scroll || keyboardInputChar != 0){
      if (cursorPosition_Y != cursorPosition_Y_Last){ // changed to new record (index)
        cursorPosition_Y_Last = cursorPosition_Y; 
        RawDataRecordCount = getRawDataRecord(cursorPosition_Y + 1, RawData); // adding 1 here because cursorPosition_Y is zero indexed but getRawDataRecord is not
        int dataLen = strlen(RawData.data);
        ScrollingIndex_LiveFeed_minX = -10 * dataLen; // 10 = 5 pixels/character * text size 2
        if (!SETTINGS_DISPLAY_SCROLL_MESSAGES) {
          ScrollingIndex_LiveFeed = 0;
        } else {
          ScrollingIndex_LiveFeed = display.width(); // starting point for text 
        }
      }
      // clear the buffer
      display.clearDisplay();
      // add global objects to buffer
      handleDisplay_Global();
      if (RawDataRecordCount > 0){
        char src_dst[24] = {'\0'};
        byte index = 0;
        for (byte i=0;i<sizeof(RawData.src)-1;i++){
          if (RawData.src[i] != '\0'){
            src_dst[index] = RawData.src[i];
            index++;
          } else {
            i = sizeof(RawData.src); // get out
          }
        }
        src_dst[index] = '>'; index++;
        for (byte i=0;i<sizeof(RawData.dst)-1;i++){
          if (RawData.dst[i] != '\0'){
            src_dst[index] = RawData.dst[i];
            index++;
          } else {
            i = sizeof(RawData.dst); // get out
          }
        }
        // display who the message is to and from
        display.setCursor(0,UI_DISPLAY_ROW_01);
        display.print(src_dst);
        // display message
        display.setCursor(ScrollingIndex_LiveFeed,UI_DISPLAY_ROW_02);
        display.setTextSize(2);
        display.print(RawData.data); 
        // go back to original text size
        display.setTextSize(1); // Normal 1:1 pixel scale - default letter size is 5x8 pixels
        // display the cursor position (represents record number in this case)
        display.setCursor(0,UI_DISPLAY_ROW_04); display.print(F("Record: "));
        display.setCursor(45,UI_DISPLAY_ROW_04); display.print(cursorPosition_Y+1);
        display.setCursor(45 + numberOfDigits<uint32_t>(cursorPosition_Y+1) * 5,UI_DISPLAY_ROW_04); display.print(F(" of "));
        display.setCursor(65 + numberOfDigits<uint32_t>(cursorPosition_Y+1) * 5,UI_DISPLAY_ROW_04); display.print(RawDataRecordCount);
        // display the date and time
        display.setCursor(0,UI_DISPLAY_ROW_05); display.print(F("D:"));
        display.setCursor(15,UI_DISPLAY_ROW_05); display.print(RawData.DateInt);
        display.setCursor(60,UI_DISPLAY_ROW_05); display.print(F("T:"));
        display.setCursor(75,UI_DISPLAY_ROW_05); display.print(RawData.TimeInt);
        unsigned int scrollPixelCount = (SETTINGS_DISPLAY_SCROLL_MESSAGES ? SETTINGS_DISPLAY_SCROLL_SPEED : 32);
        if (keyboardInputChar == KEYBOARD_LEFT_KEY || SETTINGS_DISPLAY_SCROLL_MESSAGES){ //  scroll only when enter pressed TODO: this wont work because key press not persistent
          ScrollingIndex_LiveFeed = ScrollingIndex_LiveFeed - scrollPixelCount; // higher number here is faster scroll but choppy
          if(ScrollingIndex_LiveFeed < ScrollingIndex_LiveFeed_minX) ScrollingIndex_LiveFeed = display.width(); // makeshift scroll because startScrollleft truncates the string!
        } else if (keyboardInputChar == KEYBOARD_RIGHT_KEY){
          ScrollingIndex_LiveFeed = ScrollingIndex_LiveFeed + scrollPixelCount;
          if(ScrollingIndex_LiveFeed > display.width()) ScrollingIndex_LiveFeed = ScrollingIndex_LiveFeed_minX;
        }
      } else {
        display.setCursor(0,UI_DISPLAY_ROW_02);
        display.print(F("Live feed is empty"));
        if (!leaveDisplay_LiveFeed) {
          leaveDisplay_LiveFeed = true;
          leave_display_timer = millis();
        }
      }

      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    }
    // timeout and leave
    if (millis() - leave_display_timer > SETTINGS_DISPLAY_TIMEOUT && leaveDisplay_LiveFeed){
      leaveDisplay_LiveFeed = false;
      currentDisplay = UI_DISPLAY_HOME;
      return;
    }
  }

  void handleDisplay_Settings_Save(){
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      screenNameOn = "[ Save changes? ]";
      screenNameOff = "[               ]";
      headerXPos = getHeaderXPos(strlen(screenNameOn));
    }
    // handle button context for current display
    if (keyboardInputChar == KEYBOARD_UP_KEY){
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=1;
      }
    }
    if (keyboardInputChar == KEYBOARD_DOWN_KEY){
      if (cursorPosition_Y < 1){
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
      if (cursorPosition_Y == 1) {
        writeSettingsToEeprom();
        applySettings=true;
        saveModemSettings=true;
      }
      currentDisplay = UI_DISPLAY_SETTINGS;
      previousDisplay = UI_DISPLAY_HOME;
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      currentDisplay = UI_DISPLAY_SETTINGS;
      previousDisplay = UI_DISPLAY_HOME;
      return;
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();

      // add global objects to buffer
      handleDisplay_Global();
      
      // handle cursor
      int selectionRow = handleDisplay_GetSelectionRow(cursorPosition_Y+1);
      display.setCursor(0,selectionRow);
      display.print(F(">"));

      // handle question
      display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
      display.print(screenNameOff);
      if(displayBlink || !displayInitialized){
        display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
        display.print(screenNameOn);
      }
      
      // handle body
      size_t charsPrinted, closeCursorPosition;
      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_02);
      charsPrinted = display.print(F("No"));
      if (selectionRow == UI_DISPLAY_ROW_02) closeCursorPosition = charsPrinted;
      
      display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_03);
      charsPrinted = display.print(F("Yes"));
      if (selectionRow == UI_DISPLAY_ROW_03) closeCursorPosition = charsPrinted;

      // display closing cursor
      display.setCursor(CURSOR_MARGIN + CHAR_GAP + closeCursorPosition * CHAR_WIDTH + (CHAR_GAP - 1), selectionRow);
      display.print(F("<"));  

      // display all content from buffer
      display.display();

      //
      displayInitialized= true;
    }
  }

  void handleDisplay_Settings(){
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      screenNameOn = "[ SETTINGS ]";
      screenNameOff = "[          ]";
      headerXPos = getHeaderXPos(strlen(screenNameOn));
    }
    // handle button context for current display
    if (keyboardInputChar == KEYBOARD_UP_KEY){
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=ARRAY_SIZE(MenuItems_Settings)-1;
      }
    }
    if (keyboardInputChar == KEYBOARD_DOWN_KEY){
      if (cursorPosition_Y < ARRAY_SIZE(MenuItems_Settings)-1){ // Size of array / size of array element
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
      switch (cursorPosition_Y) {
        case 0:
          currentDisplay = UI_DISPLAY_SETTINGS_APRS;
          break;
        case 1:
          currentDisplay = UI_DISPLAY_SETTINGS_GPS;
          break;
        case 2:
          currentDisplay = UI_DISPLAY_SETTINGS_DISPLAY;
          break;
        default:
          currentDisplay = UI_DISPLAY_SETTINGS;
          break;
      }
      previousDisplay = UI_DISPLAY_SETTINGS;
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      currentDisplay = UI_DISPLAY_HOME;
      return;
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();

      // add global objects to buffer
      handleDisplay_Global();
      
      // handle cursor
      int selectionRow = handleDisplay_GetSelectionRow(cursorPosition_Y+1);
      display.setCursor(0,selectionRow);
      display.print(F(">"));      
      
      // handle header
      display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
      display.print(screenNameOff);
      if(displayBlink || !displayInitialized){
        display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
        display.print(screenNameOn);
      }

      // handle body
      size_t charsPrinted, closeCursorPosition;
      int NumOfSettings = ARRAY_SIZE(MenuItems_Settings);
      if (NumOfSettings >= 1) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_02);
        charsPrinted = display.print(MenuItems_Settings[cursorPosition_Y>2 ? cursorPosition_Y-2 : 0]);
        if (selectionRow == UI_DISPLAY_ROW_02) closeCursorPosition = charsPrinted;
      }
      if (NumOfSettings >= 2) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_03);
        charsPrinted = display.print(MenuItems_Settings[cursorPosition_Y>2 ? cursorPosition_Y-1 : 1]);
        if (selectionRow == UI_DISPLAY_ROW_03) closeCursorPosition = charsPrinted;
      }
      if (NumOfSettings >= 3) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_04);
        charsPrinted = display.print(MenuItems_Settings[cursorPosition_Y>2 ? cursorPosition_Y-0 : 2]);
        if (selectionRow == UI_DISPLAY_ROW_04) closeCursorPosition = charsPrinted;
      }

      // display closing cursor
      display.setCursor(CURSOR_MARGIN + CHAR_GAP + closeCursorPosition * CHAR_WIDTH + (CHAR_GAP - 1), selectionRow);
      display.print(F("<"));  

      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    }
  }

  void handleDisplay_Settings_APRS(){
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      indexPosition_X = 0;
      indexPosition_X_Last = indexPosition_X;
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      editMode_Settings = false;
      Settings_EditValueSize = 0;
      settingsChanged = false;
      for (int i=0; i<sizeof(Settings_TempDispCharArr);i++) {
        Settings_TempDispCharArr[i] = '\0';
      }
      screenNameOn = "[ APRS ]";
      screenNameOff = "[      ]";
      headerXPos = getHeaderXPos(strlen(screenNameOn));
    }
    // monitor for changes
    if (editMode_Settings) {
      settingsChanged = true;
    }
    // handle button context for current display
    if (KEYBOARD_PRINTABLE_CHARACTERS || KEYBOARD_DIRECTIONAL_KEYS || keyboardInputChar == KEYBOARD_BACKSPACE_KEY) {
      if (editMode_Settings){
        handleDisplay_TempVarDisplay(Settings_Type_APRS[cursorPosition_Y]);
      } else if (keyboardInputChar == KEYBOARD_UP_KEY) {
        if (cursorPosition_Y > 0) {
          cursorPosition_Y--;
        } else {
          cursorPosition_Y=ARRAY_SIZE(MenuItems_Settings_APRS)-1;
        }
      } else if (keyboardInputChar == KEYBOARD_DOWN_KEY) {
        if (cursorPosition_Y < ARRAY_SIZE(MenuItems_Settings_APRS)-1) {
          cursorPosition_Y++;
        } else {
          cursorPosition_Y=0;
        }
      }
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
      if (editMode_Settings) {
        editMode_Settings = false;
        handleDisplay_TempVarApply(Settings_Type_APRS[cursorPosition_Y],Settings_TypeIndex_APRS[cursorPosition_Y]);
      } else {
        // enable edit mode
        editMode_Settings = true;
        indexPosition_X = 0;
        handleDisplay_TempVarCopy(Settings_Type_APRS[cursorPosition_Y],Settings_TypeIndex_APRS[cursorPosition_Y]);
      }
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      if (editMode_Settings) {
        // disable edit mode
        editMode_Settings = false;
        cursorPosition_X = 0;
      } else {
        if (settingsChanged) {
          currentDisplay = UI_DISPLAY_SETTINGS_SAVE;
          previousDisplay = UI_DISPLAY_SETTINGS_APRS;
        } else {
          currentDisplay = previousDisplay;
          return;
        }
      }
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();
      
      // add global objects to buffer
      bool showPositionMemory = SETTINGS_DISPLAY_SHOW_POSITION;
      SETTINGS_DISPLAY_SHOW_POSITION = false; // we dont want to show the lat long while editing
      handleDisplay_Global();
      SETTINGS_DISPLAY_SHOW_POSITION = showPositionMemory; // put it back to what it was

      // handle cursor
      int selectionRow = handleDisplay_GetSelectionRow(cursorPosition_Y+1);
      display.setCursor(0,selectionRow);
      display.print(F(">"));
      
      // handle header
      display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
      display.print(screenNameOff);
      if(displayBlink || !displayInitialized){
        display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
        display.print(screenNameOn);
      }

      // handle body
      size_t charsPrinted, closeCursorPosition;
      int NumOfSettings = ARRAY_SIZE(MenuItems_Settings_APRS);
      if (NumOfSettings >= 1) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_02);
        charsPrinted = display.print(MenuItems_Settings_APRS[cursorPosition_Y>2 ? cursorPosition_Y-2 : 0]);
        if (selectionRow == UI_DISPLAY_ROW_02) closeCursorPosition = charsPrinted;
      }
      if (NumOfSettings >= 2) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_03);
        charsPrinted = display.print(MenuItems_Settings_APRS[cursorPosition_Y>2 ? cursorPosition_Y-1 : 1]);
        if (selectionRow == UI_DISPLAY_ROW_03) closeCursorPosition = charsPrinted;
      }
      if (NumOfSettings >= 3) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_04);
        charsPrinted = display.print(MenuItems_Settings_APRS[cursorPosition_Y>2 ? cursorPosition_Y-0 : 2]);
        if (selectionRow == UI_DISPLAY_ROW_04) closeCursorPosition = charsPrinted;
      }

      // display closing cursor
      display.setCursor(CURSOR_MARGIN + CHAR_GAP + closeCursorPosition * CHAR_WIDTH + (CHAR_GAP - 1), selectionRow);
      display.print(F("<"));  

      // handle edit field
      if (editMode_Settings) {
        handleDisplay_PrintTempVal();
      } else {
        handleDisplay_PrintValStoredInMem(Settings_Type_APRS[cursorPosition_Y],Settings_TypeIndex_APRS[cursorPosition_Y]);
      }

      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    }
  }

  void handleDisplay_Settings_GPS(){
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      indexPosition_X = 0;
      indexPosition_X_Last = indexPosition_X;
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      editMode_Settings = false;
      Settings_EditValueSize = 0;
      settingsChanged = false;
      for (int i=0; i<sizeof(Settings_TempDispCharArr);i++) {
        Settings_TempDispCharArr[i] = '\0';
      }
      screenNameOn = "[ GPS ]";
      screenNameOff = "[     ]";
      headerXPos = getHeaderXPos(strlen(screenNameOn));
    }
    // monitor for changes
    if (editMode_Settings) {
      settingsChanged = true;
    }
    // handle button context for current display
    if (KEYBOARD_PRINTABLE_CHARACTERS || KEYBOARD_DIRECTIONAL_KEYS || keyboardInputChar == KEYBOARD_BACKSPACE_KEY) {
      if (editMode_Settings){
        handleDisplay_TempVarDisplay(Settings_Type_GPS[cursorPosition_Y]);
      } else if (keyboardInputChar == KEYBOARD_UP_KEY) {
        if (cursorPosition_Y > 0) {
          cursorPosition_Y--;
        } else {
          cursorPosition_Y=ARRAY_SIZE(MenuItems_Settings_GPS)-1;
        }
      } else if (keyboardInputChar == KEYBOARD_DOWN_KEY) {
        if (cursorPosition_Y < ARRAY_SIZE(MenuItems_Settings_GPS)-1) {
          cursorPosition_Y++;
        } else {
          cursorPosition_Y=0;
        }
      }
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
      if (editMode_Settings) {
        editMode_Settings = false;
        handleDisplay_TempVarApply(Settings_Type_GPS[cursorPosition_Y],Settings_TypeIndex_GPS[cursorPosition_Y]);
      } else {
        // enable edit mode
        editMode_Settings = true;
        indexPosition_X = 0;
        handleDisplay_TempVarCopy(Settings_Type_GPS[cursorPosition_Y],Settings_TypeIndex_GPS[cursorPosition_Y]);
      }
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      if (editMode_Settings) {
        // disable edit mode
        editMode_Settings = false;
        cursorPosition_X = 0;
      } else {
        if (settingsChanged) {
          currentDisplay = UI_DISPLAY_SETTINGS_SAVE;
          previousDisplay = UI_DISPLAY_SETTINGS_GPS;
        } else {
          currentDisplay = previousDisplay;
          return;
        }
      }
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();

      // add global objects to buffer
      bool showPositionMemory = SETTINGS_DISPLAY_SHOW_POSITION;
      SETTINGS_DISPLAY_SHOW_POSITION = false; // we dont want to show the lat long while editing
      handleDisplay_Global();
      SETTINGS_DISPLAY_SHOW_POSITION = showPositionMemory; // put it back to what it was

      // handle cursor
      int selectionRow = handleDisplay_GetSelectionRow(cursorPosition_Y+1);
      display.setCursor(0,selectionRow);
      display.print(F(">"));

      // handle header
      display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
      display.print(screenNameOff);
      if(displayBlink || !displayInitialized){
        display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
        display.print(screenNameOn);
      }

      // handle body
      size_t charsPrinted, closeCursorPosition;
      int NumOfSettings = ARRAY_SIZE(MenuItems_Settings_GPS);
      if (NumOfSettings >= 1) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_02);
        charsPrinted = display.print(MenuItems_Settings_GPS[cursorPosition_Y>2 ? cursorPosition_Y-2 : 0]);
        if (selectionRow == UI_DISPLAY_ROW_02) closeCursorPosition = charsPrinted;
      }
      if (NumOfSettings >= 2) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_03);
        charsPrinted = display.print(MenuItems_Settings_GPS[cursorPosition_Y>2 ? cursorPosition_Y-1 : 1]);
        if (selectionRow == UI_DISPLAY_ROW_03) closeCursorPosition = charsPrinted;
      }
      if (NumOfSettings >= 3) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_04);
        charsPrinted = display.print(MenuItems_Settings_GPS[cursorPosition_Y>2 ? cursorPosition_Y-0 : 2]);
        if (selectionRow == UI_DISPLAY_ROW_04) closeCursorPosition = charsPrinted;
      }

      // display closing cursor
      display.setCursor(CURSOR_MARGIN + CHAR_GAP + closeCursorPosition * CHAR_WIDTH + (CHAR_GAP - 1), selectionRow);
      display.print(F("<"));  

      // handle edit field
      if (editMode_Settings) {
        handleDisplay_PrintTempVal();
      } else {
        handleDisplay_PrintValStoredInMem(Settings_Type_GPS[cursorPosition_Y],Settings_TypeIndex_GPS[cursorPosition_Y]);
      }

      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    } 
  }

  void handleDisplay_Settings_Display(){
    // the displayInitialized variable gets cleared each time a display changes. this signals to the
    // display method that it is in "on first show" and needs to initialize. this is where we clear and 
    // initalize critical variables such as cursor positions.
    if (!displayInitialized){
      indexPosition_X = 0;
      indexPosition_X_Last = indexPosition_X;
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      editMode_Settings = false;
      Settings_EditValueSize = 0;
      settingsChanged = false;
      for (int i=0; i<sizeof(Settings_TempDispCharArr);i++) {
        Settings_TempDispCharArr[i] = '\0';
      }
      screenNameOn = "[ DISPLAY ]";
      screenNameOff = "[         ]";
      headerXPos = getHeaderXPos(strlen(screenNameOn));
    }
    // monitor for changes
    if (editMode_Settings) {
      settingsChanged = true;
    }
    // handle button context for current display
    if (KEYBOARD_PRINTABLE_CHARACTERS || KEYBOARD_DIRECTIONAL_KEYS || keyboardInputChar == KEYBOARD_BACKSPACE_KEY) {
      if (editMode_Settings){
        handleDisplay_TempVarDisplay(Settings_Type_Display[cursorPosition_Y]);
      } else if (keyboardInputChar == KEYBOARD_UP_KEY) {
        if (cursorPosition_Y > 0) {
          cursorPosition_Y--;
        } else {
          cursorPosition_Y=ARRAY_SIZE(MenuItems_Settings_Display)-1;
        }
      } else if (keyboardInputChar == KEYBOARD_DOWN_KEY) {
        if (cursorPosition_Y < ARRAY_SIZE(MenuItems_Settings_Display)-1) {
          cursorPosition_Y++;
        } else {
          cursorPosition_Y=0;
        }
      }
    }
    if (keyboardInputChar == KEYBOARD_ENTER_KEY){
      if (editMode_Settings) {
        editMode_Settings = false;
        handleDisplay_TempVarApply(Settings_Type_Display[cursorPosition_Y],Settings_TypeIndex_Display[cursorPosition_Y]);
      } else {
        // enable edit mode
        editMode_Settings = true;
        indexPosition_X = 0;
        handleDisplay_TempVarCopy(Settings_Type_Display[cursorPosition_Y],Settings_TypeIndex_Display[cursorPosition_Y]);
      }
    }
    if (keyboardInputChar == KEYBOARD_ESCAPE_KEY){
      if (editMode_Settings) {
        // disable edit mode
        editMode_Settings = false;
        cursorPosition_X = 0;
      } else {
        if (settingsChanged) {
          currentDisplay = UI_DISPLAY_SETTINGS_SAVE;
          previousDisplay = UI_DISPLAY_SETTINGS_DISPLAY;
        } else {
          currentDisplay = previousDisplay;
          return;
        }
      }
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();

      // add global objects to buffer
      bool showPositionMemory = SETTINGS_DISPLAY_SHOW_POSITION;
      SETTINGS_DISPLAY_SHOW_POSITION = false; // we dont want to show the lat long while editing
      handleDisplay_Global();
      SETTINGS_DISPLAY_SHOW_POSITION = showPositionMemory; // put it back to what it was

      // handle cursor
      int selectionRow = handleDisplay_GetSelectionRow(cursorPosition_Y+1);
      display.setCursor(0,selectionRow);
      display.print(F(">"));

      // handle header
      display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
      display.print(screenNameOff);
      if(displayBlink || !displayInitialized){
        display.setCursor(headerXPos, UI_DISPLAY_ROW_01);
        display.print(screenNameOn);
      }

      // handle body
      size_t charsPrinted, closeCursorPosition;
      int NumOfSettings = ARRAY_SIZE(MenuItems_Settings_Display);
      if (NumOfSettings >= 1) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_02);
        charsPrinted = display.print(MenuItems_Settings_Display[cursorPosition_Y>2 ? cursorPosition_Y-2 : 0]);
        if (selectionRow == UI_DISPLAY_ROW_02) closeCursorPosition = charsPrinted;
      }
      if (NumOfSettings >= 2) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_03);
        charsPrinted = display.print(MenuItems_Settings_Display[cursorPosition_Y>2 ? cursorPosition_Y-1 : 1]);
        if (selectionRow == UI_DISPLAY_ROW_03) closeCursorPosition = charsPrinted;
      }
      if (NumOfSettings >= 3) {
        display.setCursor(CURSOR_MARGIN + CHAR_GAP, UI_DISPLAY_ROW_04);
        charsPrinted = display.print(MenuItems_Settings_Display[cursorPosition_Y>2 ? cursorPosition_Y-0 : 2]);
        if (selectionRow == UI_DISPLAY_ROW_04) closeCursorPosition = charsPrinted;
      }

      // display closing cursor
      display.setCursor(CURSOR_MARGIN + CHAR_GAP + closeCursorPosition * CHAR_WIDTH + (CHAR_GAP - 1), selectionRow);
      display.print(F("<"));  
      
      // handle edit field
      if (editMode_Settings) {
        handleDisplay_PrintTempVal();
      } else {
        handleDisplay_PrintValStoredInMem(Settings_Type_Display[cursorPosition_Y],Settings_TypeIndex_Display[cursorPosition_Y]);
      }

      // display all content from buffer
      display.display();

      //
      displayInitialized = true;
    } 
  }

#pragma endregion

#pragma region "MODEM"

  // http://www.aprs.net/vm/DOS/PROTOCOL.HTM

  bool modemCmdFlag_Raw=false, modemCmdFlag_Cmt=false, modemCmdFlag_Msg=false;
  bool modemCmdFlag_MsgRecipient=false, modemCmdFlag_MsgRecipientSSID=false;
  unsigned char sendMessage_Seq=0, sendMessage_Retrys;
  #define MAXIMUM_MODEM_COMMAND_RATE 100        // maximum rate that commands can be sent to modem
  unsigned long modem_command_timer, aprs_beacon_timer, smart_beacon_timer;
  unsigned long message_retry_timer;

  void handleAprsBeacon(){
    
    if (millis() - smart_beacon_timer > 0){ 
      smart_beacon_timer = millis();

      // move inside else when done debugging
      currentIdleTime = millis() - aprs_beacon_timer;
      float i = parseAprsLat(currentLat);
      float j = parseAprsLon(currentLng);
      smartBeaconDistance = haversineMiles(i, j, lastLatDegSmartBeacon, lastLngDegSmartBeacon);

      // smart beaconing
      if (!gpsLocationHasChanged){
        aprs_beacon_timer = millis();

      } else {
        // as long as we are moving, reset the beacon timer
        if (modemCmdFlag_Lat || modemCmdFlag_Lng){
          aprs_beacon_timer = millis();
        }
        // if the beacon distance is satisfied we set the flag to beacon
        if (smartBeaconDistance > smartBeaconDistanceLast + abs(SETTINGS_APRS_BEACON_DISTANCE)
            || smartBeaconDistance < smartBeaconDistanceLast - abs(SETTINGS_APRS_BEACON_DISTANCE)) {
                // internal variables
                smartBeaconDistanceLast = smartBeaconDistance;
                lastLatDegSmartBeacon = i;
                lastLngDegSmartBeacon = j;
                // set flag
                gpsSmartBeaconDistanceConditionMet = true;
                // reset the beacon timer
                //aprs_beacon_timer = millis();
        }
        // if the beacon flag is set or the timer expires, we beacon
        if ((currentIdleTime > SETTINGS_APRS_BEACON_IDLE_TIME || gpsSmartBeaconDistanceConditionMet) && gps.location.isValid()==true){
          gpsSmartBeaconDistanceConditionMet = false;
          //
          gpsLocationHasChanged = false;
          //
          smartBeaconDistanceLast = smartBeaconDistance;
          lastLatDegSmartBeacon = i;
          lastLngDegSmartBeacon = j;
          //
          if (SETTINGS_APRS_BEACON_ENABLED==true) modemCmdFlag_Cmt=true;
        }

      }

    }

  }

  void handleSendMessage(){  
    if (sendMessage){
      sendMessage=false;
      sendMessage_Seq=1;
    }
    switch (sendMessage_Seq){
      case 0:
        sendMessage_Retrys=0;
        break;
      case 1: // init
        sendMessage_Ack=false;
        message_retry_timer = millis();
        sendMessage_Seq++;
        break;
      case 2: // set recipient callsign
        modemCmdFlag_MsgRecipient=true;
        sendMessage_Seq++;
        break;
      case 3: // wait for complete and set recipient ssid
        if (!modemCmdFlag_MsgRecipient) {
          modemCmdFlag_MsgRecipientSSID=true;
          sendMessage_Seq++;
        }
        break;
      case 4: // wait for complete and set message and end sequence
        if (!modemCmdFlag_MsgRecipientSSID) {
          modemCmdFlag_Msg=true;
          sendMessage_Seq++;
        }
        break;
      case 5: // wait for acknowledgment and end sequence
        if (millis() - message_retry_timer > SETTINGS_APRS_RETRY_INTERVAL && !sendMessage_Ack && sendMessage_Retrys < SETTINGS_APRS_RETRY_COUNT){
          sendMessage_Seq=1;
          sendMessage_Retrys++;
        } else if (sendMessage_Ack || sendMessage_Retrys >= SETTINGS_APRS_RETRY_COUNT) {
          sendMessage_Seq=0;
        }
        break;
    }
  }

  void handleModemCommands(){
      // send raw packet
      if (modemCmdFlag_Raw==true){
        if (sendModemCommand("!", 1, SETTINGS_APRS_RAW_PACKET, strlen(SETTINGS_APRS_RAW_PACKET)) == -1){
          modemCmdFlag_Raw=false;
        }
        return;
      }
      // send location
      // dont send if we dont have valid coordinates (modemCmdFlag_Cmt will remain true so we send when we get coordinates)
      if (modemCmdFlag_Cmt==true){
        if (sendModemCommand("@", 1, SETTINGS_APRS_COMMENT, strlen(SETTINGS_APRS_COMMENT)) == -1){
          modemCmdFlag_Cmt=false;
        }
        return;
      }
      // send message
      if (modemCmdFlag_Msg==true){
        if (sendModemCommand("#", 1, SETTINGS_APRS_MESSAGE, strlen(SETTINGS_APRS_MESSAGE)) == -1){
          modemCmdFlag_Msg=false;
        }
        return;
      }
      // set callsign                           ****************Setting*******************
      if (modemCmdFlag_Setc==true){
        if (sendModemCommand("c", 1, SETTINGS_APRS_CALLSIGN, strlen(SETTINGS_APRS_CALLSIGN)) == -1){
          modemCmdFlag_Setc=false;
        }
        return;
      }
      // set destination callsign               ****************Setting*******************
      if (modemCmdFlag_Setd==true){
        if (sendModemCommand("d", 1, SETTINGS_APRS_DESTINATION_CALL, strlen(SETTINGS_APRS_DESTINATION_CALL)) == -1){
          modemCmdFlag_Setd=false;
        }
        return;
      }
      // set PATH1 callsign                     ****************Setting*******************
      if (modemCmdFlag_Set1==true){
        if (sendModemCommand("1", 1, SETTINGS_APRS_PATH1_CALL, strlen(SETTINGS_APRS_PATH1_CALL)) == -1){
          modemCmdFlag_Set1=false;
        }
        return;
      }
      // set PATH2 callsign                     ****************Setting*******************
      if (modemCmdFlag_Set2==true){
        if (sendModemCommand("2", 1, SETTINGS_APRS_PATH2_CALL, strlen(SETTINGS_APRS_PATH2_CALL)) == -1){
          modemCmdFlag_Set2=false;
        }
        return;
      }
      // set your SSID                          ****************Setting*******************
      if (modemCmdFlag_Setsc==true){
        if (sendModemCommand("sc", 2, SETTINGS_APRS_CALLSIGN_SSID, strlen(SETTINGS_APRS_CALLSIGN_SSID)) == -1){
          modemCmdFlag_Setsc=false;
        }
        return;
      }
      // set destination SSID                   ****************Setting*******************
      if (modemCmdFlag_Setsd==true){
        if (sendModemCommand("sd", 2, SETTINGS_APRS_DESTINATION_SSID, strlen(SETTINGS_APRS_DESTINATION_SSID)) == -1){
          modemCmdFlag_Setsd=false;
        }
        return;
      }
      // set PATH1 SSID                         ****************Setting*******************
      if (modemCmdFlag_Sets1==true){
        if (sendModemCommand("s1", 2, SETTINGS_APRS_PATH1_SSID, strlen(SETTINGS_APRS_PATH1_SSID)) == -1){
          modemCmdFlag_Sets1=false;
        }
        return;
      }    
      // set PATH2 SSID                         ****************Setting*******************
      if (modemCmdFlag_Sets2==true){
        if (sendModemCommand("s2", 2, SETTINGS_APRS_PATH2_SSID, strlen(SETTINGS_APRS_PATH2_SSID)) == -1){
          modemCmdFlag_Sets2=false;
        }
        return;
      }
      // set latitude
      if (modemCmdFlag_Lat==true){
        if (sendModemCommand("lla", 3, currentLat, strlen(currentLat)) == -1){
          modemCmdFlag_Lat=false;
        }
        return;
      }
      // set longitude
      if (modemCmdFlag_Lng==true){
        if (sendModemCommand("llo", 3, currentLng, strlen(currentLng)) == -1){
          modemCmdFlag_Lng=false;
        }
        return;
      }
      // set symbol                             ****************Setting*******************
      if (modemCmdFlag_Setls==true){
        if (sendModemCommand("ls", 2, SETTINGS_APRS_SYMBOL, strlen(SETTINGS_APRS_SYMBOL)) == -1){
          modemCmdFlag_Setls=false;
        }
        return;
      }
      // set symbol table                       ****************Setting*******************
      if (modemCmdFlag_Setlt==true){
        if (sendModemCommand("lt", 2, SETTINGS_APRS_SYMBOL_TABLE, strlen(SETTINGS_APRS_SYMBOL_TABLE)) == -1){
          modemCmdFlag_Setlt=false;
        }
        return;
      }
      // set message recipient
      if (modemCmdFlag_MsgRecipient==true){
        if (sendModemCommand("mc", 2, SETTINGS_APRS_RECIPIENT_CALL, strlen(SETTINGS_APRS_RECIPIENT_CALL)) == -1){
          modemCmdFlag_MsgRecipient=false;
        }
        return;
      }
      // set message recipient ssid
      if (modemCmdFlag_MsgRecipientSSID==true){
        if (sendModemCommand("ms", 2, SETTINGS_APRS_RECIPIENT_SSID, strlen(SETTINGS_APRS_RECIPIENT_SSID)) == -1){
          modemCmdFlag_MsgRecipientSSID=false;
        }
        return;
      }
      // retry last message
      if (modemCmdFlag_Setmr==true){
        if (sendModemCommand("mr", 2, SETTINGS_APRS_RECIPIENT_SSID, strlen(SETTINGS_APRS_RECIPIENT_SSID)) == -1){
          modemCmdFlag_Setmr=false;
        }
        return;
      }
      // automatic ACK on/off                   ****************Setting*******************
      if (modemCmdFlag_Setma==true){
        char OnOff[2] = {'\0'};
        if (SETTINGS_APRS_AUTOMATIC_ACK) {
          OnOff[0] = '1';
        } else {
          OnOff[0] = '0';
        }
        if (sendModemCommand("ma", 2, OnOff, strlen(OnOff)) == -1){
          modemCmdFlag_Setma=false;
        }
        return;
      }
      // set preamble in ms                     ****************Setting*******************
      if (modemCmdFlag_Setw==true){
        //char *  itoa ( int value, char * str, int base );
        char timeBuffer[17];
        itoa(SETTINGS_APRS_PREAMBLE,timeBuffer,10);
        if (sendModemCommand("w", 1, timeBuffer, strlen(timeBuffer)) == -1){
          modemCmdFlag_Setw=false;
        }
        return;
      }
      // set tail in ms                         ****************Setting*******************
      if (modemCmdFlag_SetW==true){
        //char *  itoa ( int value, char * str, int base );
        char timeBuffer[17];
        itoa(SETTINGS_APRS_TAIL,timeBuffer,10);
        if (sendModemCommand("W", 1, timeBuffer, strlen(timeBuffer)) == -1){
          modemCmdFlag_SetW=false;
        }
        return;
      }
      // save settings                          ****************Setting*******************
      if (modemCmdFlag_SetS==true){
        if (sendModemCommand("S", 1, "", 0) == -1){
          modemCmdFlag_SetS=false;
        }
        return;
      } 
      // clear configuration                    ****************Setting*******************
      if (modemCmdFlag_SetC==true){
        if (sendModemCommand("C", 1, "", 0) == -1){
          modemCmdFlag_SetC=false;
        }
        return;
      }    
      // print configuration
      if (modemCmdFlag_SetH==true){
        if (sendModemCommand("H", 1, "", 0) == -1){
          modemCmdFlag_SetH=false;
        }
        return;
      }
  }

  int sendModemCommand(char *cmd, int const cmdLen, char *val, int const valLen){
    if ( millis() - modem_command_timer > MAXIMUM_MODEM_COMMAND_RATE ){
      for (byte i = 0; i < cmdLen; i++) {
        Serial1.write(cmd[i]);
      }
      for (byte i = 0; i < valLen; i++) {
        Serial1.write(val[i]);
      }
      modem_command_timer = millis();
      return -1;
    } else {
      return 1; // not enough time has elapsed
    }
  }

  void readModem(){
    bool gotFormatRaw = false; 
    char modemData[256]; // what is max APRS message length?
    if (Serial1.available()){
      memset(modemData,'\0',sizeof(modemData));
      int len = Serial1.readBytesUntil('\n', modemData, sizeof(modemData));
      modemData[len] = '\0';  // Null-terminate manually
      Serial.print(F("Modem Raw:"));Serial.println(modemData);
      gotFormatRaw = true;
    }

    APRSFormat_Raw Format_Raw_In = {'\0'};

    if (gotFormatRaw){
      bool foundSrc=false, foundDst=false, foundPath=false, foundData=false;
      for (int i = 0; i < sizeof(modemData) - 1; i++) {
        if (modemData[i] == 'S' && modemData[i+1] == 'R' && modemData[i+2] == 'C' && modemData[i+3] == ':' && modemData[i+4] == ' '){
          foundSrc=true;
          i = i + 5; // get past the 'SRC: '
          byte iSrc = 0;
          while(modemData[i] != ' '){
            Format_Raw_In.src[iSrc] = modemData[i];
            i++;
            iSrc++;
          }
          Format_Raw_In.src[iSrc+1] = '\0';
          //Serial.println();
          //Serial.print(F("SRC=")); Serial.println(Format_Raw_In.src);
        }
        if (modemData[i] == 'D' && modemData[i+1] == 'S' && modemData[i+2] == 'T' && modemData[i+3] == ':' && modemData[i+4] == ' '){
          foundDst=true;
          i = i + 5; // get past the 'DST: '
          byte iDst = 0;
          while(modemData[i] != ' '){
            Format_Raw_In.dst[iDst] = modemData[i];
            i++;
            iDst++;
          }
          Format_Raw_In.dst[iDst+1] = '\0';
          //Serial.print(F("DST=")); Serial.println(Format_Raw_In.dst);
        }
        if (modemData[i] == 'P' && modemData[i+1] == 'A' && modemData[i+2] == 'T' && modemData[i+3] == 'H' && modemData[i+4] == ':' && modemData[i+5] == ' '){
          foundPath=true;
          i = i + 6; // get past the 'PATH: '
          byte iPath = 0;
          while(modemData[i] != ' '){
            Format_Raw_In.path[iPath] = modemData[i];
            i++;
            iPath++;
          }
          Format_Raw_In.path[iPath+1] = '\0';
          //Serial.print(F("PATH=")); Serial.println(Format_Raw_In.path);
        }
        if (modemData[i] == 'D' && modemData[i+1] == 'A' && modemData[i+2] == 'T' && modemData[i+3] == 'A' && modemData[i+4] == ':' && modemData[i+5] == ' '){
          foundData=true;
          i = i + 6; // get past the 'DATA: '
          byte iData = 0;
          while(modemData[i] != '\0'){ // data gets the rest
            Format_Raw_In.data[iData] = modemData[i];
            i++;
            iData++;
          }
          Format_Raw_In.data[iData+1] = '\0';
          //Serial.print(F("DATA=")); Serial.println(Format_Raw_In.data);
        }
        if (!foundSrc && !foundDst && !foundPath && !foundData) {
          //Serial.println(F("    !APRS"));
          //Serial.println();
          i = sizeof(modemData); // setting i to size of modem data will make loop exit sooner
        }
        if (foundSrc && foundDst && foundPath && foundData) {
          Format_Raw_In.DateInt = GPSData.Date.DateInt;
          Format_Raw_In.TimeInt = GPSData.Time.TimeInt;
          writeRawDataToSd(Format_Raw_In);
          i = sizeof(modemData); // setting i to size of modem data will make loop exit sooner
        }
      }
      
      // check if message here
      // Radio 2: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :Hi!{006
      // Radio 1: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :ack006
      if (Format_Raw_In.data[0] == ':') { // TODO see if message is to user not just any message
        APRSFormat_Msg Format_Msg_In;
        // set the date and time
        Format_Msg_In.DateInt = Format_Raw_In.DateInt;
        Format_Msg_In.TimeInt = Format_Raw_In.TimeInt;
        // get the 'from' 
        int j=0;
        for (int i=0;i<sizeof(Format_Raw_In.src);i++) {
          if (Format_Raw_In.src[i] != '[' && Format_Raw_In.src[i] != ']') {
            Format_Msg_In.from[j] = Format_Raw_In.src[i];
            j++;
          }
        }
        // get the 'to'
        j=0;
        for (int i=1;Format_Raw_In.data[i]!=' ' && Format_Raw_In.data[i]!='\0';i++) {
          Format_Msg_In.to[j] = Format_Raw_In.data[i]; 
          j++;
        }
        // get the message
        j=0;
        int k=0;
        bool foundLine = false;
        char line[6] = {'\0'};
        for (int i=11;Format_Raw_In.data[i]!='\0';i++) {
          if (Format_Raw_In.data[i]!='{' && !foundLine) {
            Format_Msg_In.msg[j] = Format_Raw_In.data[i];
            j++;
            // check if acknowledge
            if (Format_Msg_In.msg[0] == 'a' && 
                  Format_Msg_In.msg[1] == 'c' && 
                    Format_Msg_In.msg[2] == 'k') {
              foundLine = true;
              Format_Msg_In.ack = true;
            }
          } else {
            foundLine = true;
            if (isdigit(Format_Raw_In.data[i])) {
              line[k] = Format_Raw_In.data[i];
              k++;
            }
          }
        }
        // get the line
        Format_Msg_In.line = atoi(line);
        // print out the message
        Serial.println(F("Received a message!"));
        Serial.print(F("To="));Serial.println(Format_Msg_In.to);
        Serial.print(F("From="));Serial.println(Format_Msg_In.from);
        Serial.print(F("Msg="));Serial.println(Format_Msg_In.msg);
        Serial.print(F("Line="));Serial.println(Format_Msg_In.line);
        Serial.print(F("Ack="));Serial.println(Format_Msg_In.ack);
        if (strstr(Format_Msg_In.to, SETTINGS_APRS_CALLSIGN) != NULL) {
          // if message is an acknowledge don't add.
          if (!Format_Msg_In.ack) {
            wakeDisplay = true;
            Format_Msg_In.DateInt = GPSData.Date.DateInt;
            Format_Msg_In.TimeInt = GPSData.Time.TimeInt;
            writeMsgDataToSd(Format_Msg_In);
            // switch to message display
            if (currentDisplay == UI_DISPLAY_HOME) currentDisplay = UI_DISPLAY_MESSAGES;
          } else {
            // for now any acknowledge will set this. in the future, 
            // we will need to see if it is a response to our message.
            sendMessage_Ack=true;
          }
        }
      }
    }
  }

#pragma endregion

#pragma region "TERMINAL"

  // store common strings here just to save space instead of recreating them
  const char* InvalidCommand = "Invalid command.";
  const char* InvalidData_UnsignedInt = "Invalid data. Expected unsigned integer 0-65535 instead got ";
  const char* InvalidData_Float = "Invalid data. Expected float -3.4028235E+38-3.4028235E+38 instead got ";
  const char* InvalidData_UnsignedLong = "Invalid data. Expected unsigned long 0-4294967295 instead got ";
  const char* InvalidData_TrueFalse = "Invalid data. Expected True/False or 1/0";
  const char* InvalidData_String99 = "Invalid data. Expected string <= 99 chars";
  const char* InvalidData_String6 = "Invalid data. Expected string <= 6 chars";
  const char* InvalidData_String2 = "Invalid data. Expected string <= 2 chars";
  const char* InvalidData_String1 = "Invalid data. Expected string <= 1 chars";

  const char exNone[]      PROGMEM = ":<Unknown>";
  const char exBool[]      PROGMEM = ":<True/False>";
  const char exInt[]       PROGMEM = ":<-32,768 to 32,767>";
  const char exUInt[]      PROGMEM = ":<0 to 65,535>";
  const char exLong[]      PROGMEM = ":<-2,147,483,648 to 2,147,483,647>";
  const char exULong[]     PROGMEM = ":<0 to 4,294,967,295>";
  const char exFloat[]     PROGMEM = ":<-3.4028235E+38 to 3.4028235E+38>";
  const char exString2[]   PROGMEM = ":<Alpha numeric up to 1 character>";
  const char exString3[]   PROGMEM = ":<Alpha numeric up to 2 character>";
  const char exString7[]   PROGMEM = ":<Alpha numeric up to 6 characters>";
  const char exString100[] PROGMEM = ":<Alpha numeric up to 99 characters>";
  const char exAlt1[]      PROGMEM = ":<Standard/Alternate>";

  // Lookup table of pointers to PROGMEM strings
  const char* const dataTypeExamples[] PROGMEM = {
    exNone,      // SETTINGS_EDIT_TYPE_NONE       = 0
    exBool,      // SETTINGS_EDIT_TYPE_BOOLEAN    = 1
    exInt,       // SETTINGS_EDIT_TYPE_INT        = 2
    exUInt,      // SETTINGS_EDIT_TYPE_UINT       = 3
    exLong,      // SETTINGS_EDIT_TYPE_LONG       = 4
    exULong,     // SETTINGS_EDIT_TYPE_ULONG      = 5
    exFloat,     // SETTINGS_EDIT_TYPE_FLOAT      = 6
    exString2,   // SETTINGS_EDIT_TYPE_STRING2    = 7
    exString3,   // SETTINGS_EDIT_TYPE_STRING3    = 8
    exString7,   // SETTINGS_EDIT_TYPE_STRING7    = 9
    exString100, // SETTINGS_EDIT_TYPE_STRING100  = 10
    exAlt1,      // SETTINGS_EDIT_TYPE_ALT1       = 11
  };

  void getDataTypeExample(int dataType, char* outExample) {
    if (dataType >= 0 && dataType < (int)(sizeof(dataTypeExamples) / sizeof(dataTypeExamples[0]))) {
      strcpy_P(outExample, (PGM_P)pgm_read_word(&dataTypeExamples[dataType])); // strcpy_P reads directly from flash, not from RAM
    } else {
      strcpy(outExample, ""); // default to empty string
    }
  }

  void printOutSerialCommands(){

    Serial.println();
    Serial.println(F("CMD:Settings:Print:"));  // prints all settings to terminal
    Serial.println(F("CMD:Settings:Save:"));   // saves all settings to eeprom
    Serial.println(F("CMD:Beacon:"));          // immediatley sends a beacon per current settings
    Serial.println(F("CMD:Message:<Recipient Callsign>:<Recipient SSID>:<Message>"));   // sends a message to a recipient
    Serial.println(F("CMD:SD:Raw:Print:"));    // prints all raw data to terminal
    Serial.println(F("CMD:SD:Raw:Delete:"));   // deletes all raw data from sd
    Serial.println(F("CMD:SD:Msg:Print:"));    // prints all messages to terminal
    Serial.println(F("CMD:SD:Msg:Delete:"));   // deletes all messages from sd
    Serial.println(F("CMD:Modem:<command>"));  // writes a command directly to the modem (see MicroAPRS github for examples)
    Serial.println();

    char example[40] = {'\0'};
    for (int i=0;i<ARRAY_SIZE(MenuItems_Settings);i++) {
      switch (i) {
        case 0: // APRS
          for (int j=0;j<ARRAY_SIZE(MenuItems_Settings_APRS);j++) {
            getDataTypeExample(Settings_Type_APRS[j],example);
            Serial.print(F("CMD:Settings:"));Serial.print(MenuItems_Settings[i]);Serial.print(F(":"));Serial.print(MenuItems_Settings_APRS[j]);Serial.println(example);
          }
          break;
          
        case 1: // GPS
          Serial.println();
          for (int k=0;k<ARRAY_SIZE(MenuItems_Settings_GPS);k++) {
            getDataTypeExample(Settings_Type_GPS[k],example);
            Serial.print(F("CMD:Settings:"));Serial.print(MenuItems_Settings[i]);Serial.print(F(":"));Serial.print(MenuItems_Settings_GPS[k]);Serial.println(example);
          }
          break;
          
        case 2: // Display
          Serial.println();
          for (int l=0;l<ARRAY_SIZE(MenuItems_Settings_Display);l++) {
            getDataTypeExample(Settings_Type_Display[l],example);
            Serial.print(F("CMD:Settings:"));Serial.print(MenuItems_Settings[i]);Serial.print(F(":"));Serial.print(MenuItems_Settings_Display[l]);Serial.println(example);
          }
          break;
          
        default:
          break;
          
      }
    }
  }

  void printOutSettings(){
    Serial.println();
    Serial.println(F("Current Settings"));
    Serial.println(F("APRS:"));
    Serial.print(MenuItems_Settings_APRS[0]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_BEACON_ENABLED);
    Serial.print(MenuItems_Settings_APRS[1]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_BEACON_DISTANCE);
    Serial.print(MenuItems_Settings_APRS[2]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_BEACON_IDLE_TIME);
    Serial.print(MenuItems_Settings_APRS[3]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_RAW_PACKET);
    Serial.print(MenuItems_Settings_APRS[4]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_COMMENT);
    Serial.print(MenuItems_Settings_APRS[5]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_MESSAGE);
    Serial.print(MenuItems_Settings_APRS[6]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_RECIPIENT_CALL);
    Serial.print(MenuItems_Settings_APRS[7]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_RECIPIENT_SSID);
    Serial.print(MenuItems_Settings_APRS[8]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_CALLSIGN);
    Serial.print(MenuItems_Settings_APRS[9]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_CALLSIGN_SSID);
    Serial.print(MenuItems_Settings_APRS[10]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_DESTINATION_CALL);
    Serial.print(MenuItems_Settings_APRS[11]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_DESTINATION_SSID);
    Serial.print(MenuItems_Settings_APRS[12]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_PATH1_CALL);
    Serial.print(MenuItems_Settings_APRS[13]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_PATH1_SSID);
    Serial.print(MenuItems_Settings_APRS[14]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_PATH2_CALL);
    Serial.print(MenuItems_Settings_APRS[15]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_PATH2_SSID);
    Serial.print(MenuItems_Settings_APRS[16]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_SYMBOL);
    Serial.print(MenuItems_Settings_APRS[17]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_SYMBOL_TABLE);
    Serial.print(MenuItems_Settings_APRS[18]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_AUTOMATIC_ACK);
    Serial.print(MenuItems_Settings_APRS[19]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_PREAMBLE);
    Serial.print(MenuItems_Settings_APRS[20]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_TAIL);
    Serial.print(MenuItems_Settings_APRS[21]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_RETRY_COUNT);
    Serial.print(MenuItems_Settings_APRS[22]); Serial.print(F("=")); Serial.println(SETTINGS_APRS_RETRY_INTERVAL);
    Serial.println(F("GPS:"));
    Serial.print(MenuItems_Settings_GPS[0]); Serial.print(F("=")); Serial.println(SETTINGS_GPS_POSITION_TOLERANCE, 2);
    Serial.print(MenuItems_Settings_GPS[1]); Serial.print(F("=")); Serial.println(SETTINGS_GPS_DESTINATION_LATITUDE, 6);
    Serial.print(MenuItems_Settings_GPS[2]); Serial.print(F("=")); Serial.println(SETTINGS_GPS_DESTINATION_LONGITUDE, 6);
    Serial.println(F("Display:"));
    Serial.print(MenuItems_Settings_Display[0]); Serial.print(F("=")); Serial.println(SETTINGS_DISPLAY_TIMEOUT);
    Serial.print(MenuItems_Settings_Display[1]); Serial.print(F("=")); Serial.println(SETTINGS_DISPLAY_BRIGHTNESS);
    Serial.print(MenuItems_Settings_Display[2]); Serial.print(F("=")); Serial.println(SETTINGS_DISPLAY_SHOW_POSITION);
    Serial.print(MenuItems_Settings_Display[3]); Serial.print(F("=")); Serial.println(SETTINGS_DISPLAY_SCROLL_MESSAGES);
    Serial.print(MenuItems_Settings_Display[4]); Serial.print(F("=")); Serial.println(SETTINGS_DISPLAY_SCROLL_SPEED);
    Serial.print(MenuItems_Settings_Display[5]); Serial.print(F("=")); Serial.println(SETTINGS_DISPLAY_INVERT);
    Serial.println();
    Serial.print(F("Version=")); Serial.println(version);
    Serial.println();
  }

  const byte MAX_LEN = 250;
  char inData[MAX_LEN] = {'\0'};
  byte index = 0;
  const char CMD_DELETE[] PROGMEM = "Delete";
  const char CMD_PRINT[]  PROGMEM = "Print";
  const char CMD_MODEM[]  PROGMEM = "Modem";
  void handleSerial(){
    // the gps module continuously prints to the cpu
    readGPS();
    // when the modem processes a message or responds to a command it will print to the cpu
    readModem();
    // usb serial commands are handled here
    bool gotCMD = false;
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') {
        inData[index] = '\0';
        Serial.print(F("ECHO ["));Serial.print(index+1);Serial.print(F("] "));Serial.println(inData);
        if (inData[0]=='C' && inData[1]=='M' && inData[2]=='D' && inData[3]==':') gotCMD=true;
        if (inData[0]=='?') printOutSerialCommands();
        index = 0;
      } else {
        if (index < MAX_LEN - 1) {
          inData[index++] = c;
        } else {
          // we got to the end of our buffer and never got a \n so lets start over
          index = 0;
          break;
        }
      }
    }

    if (gotCMD){
      // get the command
      char CMD[30]={'\0'};
      int i=4; // start at 5 to skip "CMD:"
      while (inData[i] != ':') {
        if (inData[i] == '\0' || inData[i] == '\n') return;
        CMD[i-4] = inData[i];
        i++;
      }
      i++; // i should be sitting at the ':'. go ahead and skip that.
      if (strstr(CMD, "Modem") != NULL) { // if intended to write to modem
        while (inData[i] != '\0'){
          Serial1.print(inData[i]);
          i++;
        }
      } else if (strstr(CMD, "SD") != NULL) {
        // get the SD group
        char SDGroup[5]={'\0'};
        int j_sdg = 0;
        while (inData[i] != ':') {
          if (inData[i] == '\0' || inData[i] == '\n') return;
          SDGroup[j_sdg] = inData[i];
          i++; j_sdg++;
        }
        i++; // i should be sitting at the ':'. go ahead and skip that.
        if (strstr(SDGroup, "Raw") != NULL) {
          // get the SD Command
          char SD_cmd[10]={'\0'};
          int j_sdc = 0;
          while (inData[i] != ':') {
            if (inData[i] == '\0' || inData[i] == '\n') return;
            SD_cmd[j_sdc] = inData[i];
            i++; j_sdc++;
          }
          if (strstr(SD_cmd, "Delete") != NULL) {
            deleteAllRawData();
          } else if (strstr(SD_cmd, "Print") != NULL) {
            //printRawDataFromSd(0);
            startReadingFromSd(0);
          } else if (strstr(SD_cmd, "Test") != NULL) {
            RawDataRecordCount = getRawDataRecord(1, RawData);
            Serial.print(F("Raw Data Record Count")); Serial.println(RawDataRecordCount);
            Serial.print(F("src:")); Serial.print(RawData.src);
            Serial.print(F("\tdst:")); Serial.print(RawData.dst);
            Serial.print(F("\tpath:")); Serial.print(RawData.path);
            Serial.print(F("\tdata:")); Serial.print(RawData.data);
            Serial.print(F("\tdate:")); Serial.print(RawData.DateInt);
            Serial.print(F("\ttime:")); Serial.print(RawData.TimeInt);
            
          } else {
            Serial.println(InvalidCommand);
          }
        
        } else if (strstr(SDGroup, "Msg") != NULL) {
          // get the SD Command
          char SD_cmd[10]={'\0'};
          int j_sdc = 0;
          while (inData[i] != ':') {
            if (inData[i] == '\0' || inData[i] == '\n') return;
            SD_cmd[j_sdc] = inData[i];
            i++; j_sdc++;
          }
          if (strstr(SD_cmd, "Delete") != NULL) {
            deleteAllMsgData();
          } else if (strstr(SD_cmd, "Print") != NULL) {
            printMsgDataFromSd(0);
          } else if (strstr(SD_cmd, "Test") != NULL) {
            MsgDataRecordCount = getMsgDataRecord(1, MsgData);
            Serial.print(F("Msg Data Record Count")); Serial.println(MsgDataRecordCount);
            Serial.print(F("to:")); Serial.print(MsgData.to);
            Serial.print(F("\tfrom:")); Serial.print(MsgData.from);
            Serial.print(F("\tmsg:")); Serial.print(MsgData.msg);
            Serial.print(F("\tline:")); Serial.print(MsgData.line);
            Serial.print(F("\tack:")); Serial.print(MsgData.ack);
            Serial.print(F("\tdate:")); Serial.print(MsgData.DateInt);
            Serial.print(F("\ttime:")); Serial.print(MsgData.TimeInt);
          } else {
            Serial.println(InvalidCommand);
          }
        } else {
          Serial.println(InvalidCommand);
        }
      } else if (strstr(CMD, "Message") != NULL) {
        // get the Recipient
        memset(SETTINGS_APRS_RECIPIENT_CALL,'\0',sizeof(SETTINGS_APRS_RECIPIENT_CALL)); 
        int j_r = 0;
        while (inData[i] != ':') {
          if (inData[i] == '\0' || inData[i] == '\n') return;
          SETTINGS_APRS_RECIPIENT_CALL[j_r] = inData[i];
          i++; j_r++;
        }
        i++; // i should be sitting at the ':'. go ahead and skip that.
        // get the Recipient SSID
        memset(SETTINGS_APRS_RECIPIENT_SSID,'\0',sizeof(SETTINGS_APRS_RECIPIENT_SSID)); 
        int j_rs = 0;
        while (inData[i] != ':') {
          if (inData[i] == '\0' || inData[i] == '\n') return;
          SETTINGS_APRS_RECIPIENT_SSID[j_rs] = inData[i];
          i++; j_rs++;
        }
        i++; // i should be sitting at the ':'. go ahead and skip that.
        // get the message
        memset(SETTINGS_APRS_MESSAGE,'\0',sizeof(SETTINGS_APRS_MESSAGE)); 
        int j_m = 0;
        while (inData[i] != '\0') {
          if (inData[i] == '\0' || inData[i] == '\n') return;
          SETTINGS_APRS_MESSAGE[j_m] = inData[i];
          i++; j_m++;
        }
        i++; // i should be sitting at the ':'. go ahead and skip that.
        sendMessage=true;

      } else if (strstr(CMD, "Beacon") != NULL) {
        // send a beacon now
        modemCmdFlag_Cmt=true;
      } else if (strstr(CMD, "Settings") != NULL) {
        // get the setting group
        char SettingGroup[30]={'\0'};
        int j_sg = 0;
        while (inData[i] != ':') {
          if (inData[i] == '\0' || inData[i] == '\n') return;
          SettingGroup[j_sg] = inData[i];
          i++; j_sg++;
        }
        i++; // i should be sitting at the ':'. go ahead and skip that.
        char inData_Value[300] = {'\0'};
        int k = 0;
        if (strstr(SettingGroup, MenuItems_Settings[0]) != NULL) { // APRS
          // get the setting
          char Setting[30]={'\0'};
          int j_s_APRS = 0;
          while (inData[i] != ':') {
            if (inData[i] == '\0' || inData[i] == '\n') return;
            Setting[j_s_APRS] = inData[i];
            i++; j_s_APRS++;
          }
          i++; // i should be sitting at the ':'. go ahead and skip that.
          if (strstr(Setting, MenuItems_Settings_APRS[0]) != NULL) { // "Beacon Enabled"
            while (inData[i] != '\n' && inData[i] != '\0') {
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            if (inData_Value[0]=='1' || inData_Value[0]=='T' || inData_Value[0]=='t'){
              SETTINGS_APRS_BEACON_ENABLED = true;
            } else if (inData_Value[0]=='0' || inData_Value[0]=='F' || inData_Value[0]=='f'){
              SETTINGS_APRS_BEACON_ENABLED = false;
            } else {
              Serial.println(InvalidData_TrueFalse);
            }
          } else if (strstr(Setting, MenuItems_Settings_APRS[1]) != NULL) { // "Beacon Distance"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>40) { // float would be no longer than 40 digits (this is ridiculous)
                Serial.print(InvalidData_Float);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i]) && !'.' && !'-') {
                Serial.print(InvalidData_Float);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //SETTINGS_APRS_BEACON_DISTANCE = strtod(inData_Value,NULL); // should we use strtod or atof? which is best?
            SETTINGS_APRS_BEACON_DISTANCE = atof(inData_Value);

          } else if (strstr(Setting, MenuItems_Settings_APRS[2]) != NULL) { // "Beacon Idle Time"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>9) { // unsigned long would be no longer than 10 digits
                Serial.print(InvalidData_UnsignedLong);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i])) {
                Serial.print(InvalidData_UnsignedLong);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value);
            SETTINGS_APRS_BEACON_IDLE_TIME = atol(inData_Value);
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[3]) != NULL) { // "Raw Packet"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>99) { // string100 should be no longer than 99 chars (plus \n)
                Serial.print(InvalidData_String99);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            memset(SETTINGS_APRS_RAW_PACKET,'\0',sizeof(SETTINGS_APRS_RAW_PACKET)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_RAW_PACKET[i] = inData_Value[i];
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[4]) != NULL) { // "Comment"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>99) { // string100 should be no longer than 99 chars (plus \n)
                Serial.print(InvalidData_String99);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            memset(SETTINGS_APRS_COMMENT,'\0',sizeof(SETTINGS_APRS_COMMENT)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_COMMENT[i] = inData_Value[i];
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[5]) != NULL) { // "Message"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>99) { // string100 should be no longer than 99 chars (plus \n)
                Serial.print(InvalidData_String99);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            memset(SETTINGS_APRS_MESSAGE,'\0',sizeof(SETTINGS_APRS_MESSAGE)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_MESSAGE[i] = inData_Value[i];
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[6]) != NULL) { // "Recipient Callsign"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>6) { // string7 should be no longer than 6 chars (plus \n)
                Serial.print(InvalidData_String6);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            memset(SETTINGS_APRS_RECIPIENT_CALL,'\0',sizeof(SETTINGS_APRS_RECIPIENT_CALL)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_RECIPIENT_CALL[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[7]) != NULL) { // "Recipient SSID"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>2) { // string3 should be no longer than 2 chars (plus \n)
                Serial.print(InvalidData_String2);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            //SETTINGS_APRS_RECIPIENT_SSID[0] = inData_Value[0];
            memset(SETTINGS_APRS_RECIPIENT_SSID,'\0',sizeof(SETTINGS_APRS_RECIPIENT_SSID)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_RECIPIENT_SSID[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[8]) != NULL) { // "My Callsign"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>6) { // string7 should be no longer than 6 chars (plus \n)
                Serial.print(InvalidData_String6);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            memset(SETTINGS_APRS_CALLSIGN,'\0',sizeof(SETTINGS_APRS_CALLSIGN)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_CALLSIGN[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[9]) != NULL) { // "Callsign SSID"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>2) { // string3 should be no longer than 2 chars (plus \n)
                Serial.print(InvalidData_String2);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            //SETTINGS_APRS_CALLSIGN_SSID[0] = inData_Value[0];
            memset(SETTINGS_APRS_CALLSIGN_SSID,'\0',sizeof(SETTINGS_APRS_CALLSIGN_SSID)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_CALLSIGN_SSID[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[10]) != NULL) { // "Destination Callsign"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>6) { // string7 should be no longer than 6 chars (plus \n)
                Serial.print(InvalidData_String6);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            memset(SETTINGS_APRS_DESTINATION_CALL,'\0',sizeof(SETTINGS_APRS_DESTINATION_CALL)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_DESTINATION_CALL[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[11]) != NULL) { // "Destination SSID"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>2) { // string3 should be no longer than 2 chars (plus \n)
                Serial.print(InvalidData_String2);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            //SETTINGS_APRS_DESTINATION_SSID[0] = inData_Value[0]; 
            memset(SETTINGS_APRS_DESTINATION_SSID,'\0',sizeof(SETTINGS_APRS_DESTINATION_SSID)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_DESTINATION_SSID[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[12]) != NULL) { // "PATH1 Callsign"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>6) { // string7 should be no longer than 6 chars (plus \n)
                Serial.print(InvalidData_String6);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            memset(SETTINGS_APRS_PATH1_CALL,'\0',sizeof(SETTINGS_APRS_PATH1_CALL)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_PATH1_CALL[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[13]) != NULL) { // "PATH1 SSID"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>2) { // string3 should be no longer than 2 chars (plus \n)
                Serial.print(InvalidData_String2);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            //SETTINGS_APRS_PATH1_SSID[0] = inData_Value[0];
            memset(SETTINGS_APRS_PATH1_SSID,'\0',sizeof(SETTINGS_APRS_PATH1_SSID)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_PATH1_SSID[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[14]) != NULL) { // "PATH2 Callsign"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>6) { // string7 should be no longer than 6 chars (plus \n)
                Serial.print(InvalidData_String6);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            memset(SETTINGS_APRS_PATH2_CALL,'\0',sizeof(SETTINGS_APRS_PATH2_CALL)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_PATH2_CALL[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[15]) != NULL) { // "PATH2 SSID"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>2) { // string3 should be no longer than 2 chars (plus \n)
                Serial.print(InvalidData_String2);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            //SETTINGS_APRS_PATH2_SSID[0] = inData_Value[0];
            memset(SETTINGS_APRS_PATH2_SSID,'\0',sizeof(SETTINGS_APRS_PATH2_SSID)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_PATH2_SSID[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[16]) != NULL) { // "Symbol"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>1) { // string2 should be no longer than 1 chars (plus \n)
                Serial.print(InvalidData_String1);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            //SETTINGS_APRS_SYMBOL[0] = inData_Value[0];
            memset(SETTINGS_APRS_SYMBOL,'\0',sizeof(SETTINGS_APRS_SYMBOL)); 
            for (int i=0; inData_Value[i]!='\0';i++) {
              SETTINGS_APRS_SYMBOL[i] = inData_Value[i]; 
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[17]) != NULL) { // "Table"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>1) { // string2 should be no longer than 1 chars (plus \n)
                Serial.print(InvalidData_String1);Serial.println(inData_Value);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            if (inData_Value[0]=='1' || inData_Value[0]=='S' || inData_Value[0]=='s'){
              SETTINGS_APRS_SYMBOL_TABLE[0] = 's';
            } else if (inData_Value[0]=='0' || inData_Value[0]=='A' || inData_Value[0]=='a'){
              SETTINGS_APRS_SYMBOL_TABLE[0] = 'a';
            } else {
              Serial.println(InvalidData_TrueFalse);
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[18]) != NULL) { // "Automatic ACK"
            while (inData[i] != '\n' && inData[i] != '\0') {
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            if (inData_Value[0]=='1' || inData_Value[0]=='T' || inData_Value[0]=='t'){
              SETTINGS_APRS_AUTOMATIC_ACK = true;
            } else if (inData_Value[0]=='0' || inData_Value[0]=='F' || inData_Value[0]=='f'){
              SETTINGS_APRS_AUTOMATIC_ACK = false;
            } else {
              Serial.println(InvalidData_TrueFalse);
            }
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[19]) != NULL) { // "Preamble"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>4) { // unsigned int would be no longer than 5 digits
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i])) {
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value);
            SETTINGS_APRS_PREAMBLE = atoi(inData_Value);
            
          } else if (strstr(Setting, MenuItems_Settings_APRS[20]) != NULL) { // "Tail"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>4) { // unsigned int would be no longer than 5 digits
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i])) {
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value);
            SETTINGS_APRS_TAIL = atoi(inData_Value);

          } else if (strstr(Setting, MenuItems_Settings_APRS[21]) != NULL) { // "Retry Count"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>4) { // unsigned int would be no longer than 5 digits
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i])) {
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value);
            SETTINGS_APRS_RETRY_COUNT = atoi(inData_Value);

          } else if (strstr(Setting, MenuItems_Settings_APRS[22]) != NULL) { // "Retry Interval"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>4) { // unsigned int would be no longer than 5 digits
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i])) {
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value);
            SETTINGS_APRS_RETRY_INTERVAL = atoi(inData_Value);

          } else {
            Serial.println(InvalidCommand);
          }
        } else if (strstr(SettingGroup, MenuItems_Settings[1]) != NULL) { // GPS
          // get the setting
          char Setting[30]={'\0'};
          int j_s_GPS = 0;
          while (inData[i] != ':') {
            Setting[j_s_GPS] = inData[i];
            i++; j_s_GPS++;
          }
          i++; // i should be sitting at the ':'. go ahead and skip that.
          if (strstr(Setting, MenuItems_Settings_GPS[0]) != NULL) { // Position Tolerance"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>40) { // float would be no longer than 40 digits (this is ridiculous)
                Serial.print(InvalidData_Float);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i]) && !'.' && !'-') {
                Serial.print(InvalidData_Float);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //SETTINGS_GPS_POSITION_TOLERANCE = strtod(inData_Value,NULL); // should we use strtod or atof? which is best?
            SETTINGS_GPS_POSITION_TOLERANCE = atof(inData_Value);

          } else if (strstr(Setting, MenuItems_Settings_GPS[1]) != NULL) { // Destination Latitude
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>40) { // float would be no longer than 40 digits (this is ridiculous)
                Serial.print(InvalidData_Float);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i]) && !'.' && !'-') {
                Serial.print(InvalidData_Float);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //SETTINGS_GPS_DESTINATION_LATITUDE = strtod(inData_Value,NULL); // should we use strtod or atof? which is best?
            SETTINGS_GPS_DESTINATION_LATITUDE = atof(inData_Value);

          } else if (strstr(Setting, MenuItems_Settings_GPS[2]) != NULL) { // Destination Longitude
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>40) { // float would be no longer than 40 digits (this is ridiculous)
                Serial.print(InvalidData_Float);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i]) && !'.' && !'-') {
                Serial.print(InvalidData_Float);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //SETTINGS_GPS_DESTINATION_LONGITUDE = strtod(inData_Value,NULL); // should we use strtod or atof? which is best?
            SETTINGS_GPS_DESTINATION_LONGITUDE = atof(inData_Value);

          } else {
            Serial.println(InvalidCommand);
          }
        } else if (strstr(SettingGroup, MenuItems_Settings[2]) != NULL) { // Display
          // get the setting
          char Setting[30]={'\0'};
          int j_s_Display = 0;
          while (inData[i] != ':') {
            Setting[j_s_Display] = inData[i];
            i++; j_s_Display++;
          }
          i++; // i should be sitting at the ':'. go ahead and skip that.
          if (strstr(Setting, MenuItems_Settings_Display[0]) != NULL) { // "Timeout"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>9) { // unsigned long would be no longer than 10 digits 0 to 4,294,967,295
                Serial.print(InvalidData_UnsignedLong);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i])) {
                Serial.print(InvalidData_UnsignedLong);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value);
            SETTINGS_DISPLAY_TIMEOUT = atol(inData_Value);
            
          } else if (strstr(Setting, MenuItems_Settings_Display[1]) != NULL) { // "Brightness"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>4) { // unsigned int would be no longer than 5 digits
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i])) {
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value);
            SETTINGS_DISPLAY_BRIGHTNESS = atoi(inData_Value);
            
          } else if (strstr(Setting, MenuItems_Settings_Display[2]) != NULL) { // "Show Position"
            while (inData[i] != '\n' && inData[i] != '\0') {
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            if (inData_Value[0]=='1' || inData_Value[0]=='T' || inData_Value[0]=='t'){
              SETTINGS_DISPLAY_SHOW_POSITION = true;
            } else if (inData_Value[0]=='0' || inData_Value[0]=='F' || inData_Value[0]=='f'){
              SETTINGS_DISPLAY_SHOW_POSITION = false;
            } else {
              Serial.println(InvalidData_TrueFalse);
            }
            
          } else if (strstr(Setting, MenuItems_Settings_Display[3]) != NULL) { // "Scroll Messages"
            while (inData[i] != '\n' && inData[i] != '\0') {
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            if (inData_Value[0]=='1' || inData_Value[0]=='T' || inData_Value[0]=='t'){
              SETTINGS_DISPLAY_SCROLL_MESSAGES = true;
            } else if (inData_Value[0]=='0' || inData_Value[0]=='F' || inData_Value[0]=='f'){
              SETTINGS_DISPLAY_SCROLL_MESSAGES = false;
            } else {
              Serial.println(InvalidData_TrueFalse);
            }
            
          } else if (strstr(Setting, MenuItems_Settings_Display[4]) != NULL) { // "Scroll Speed"
            while (inData[i] != '\n' && inData[i] != '\0') {
              if (k>4) { // unsigned int would be no longer than 5 digits
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData_Value);
                return;
              }
              if (!isDigit(inData[i])) {
                Serial.print(InvalidData_UnsignedInt);Serial.println(inData[i]);
                return;
              }
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value);
            SETTINGS_DISPLAY_SCROLL_SPEED = atoi(inData_Value);

          } else if (strstr(Setting, MenuItems_Settings_Display[5]) != NULL) { // "Invert"
            while (inData[i] != '\n' && inData[i] != '\0') {
              inData_Value[k] = inData[i];
              i++; k++;
            }
            //Serial.print(DataEntered);Serial.println(inData_Value); 
            if (inData_Value[0]=='1' || inData_Value[0]=='T' || inData_Value[0]=='t'){
              SETTINGS_DISPLAY_INVERT = true;
            } else if (inData_Value[0]=='0' || inData_Value[0]=='F' || inData_Value[0]=='f'){
              SETTINGS_DISPLAY_INVERT = false;
            } else {
              Serial.println(InvalidData_TrueFalse);
            }
            
          } else {
            Serial.println(InvalidCommand);
          }
          
        } else if (strstr(SettingGroup, "Save") != NULL) { // CMD:Settings:Save:
          writeSettingsToEeprom();
          applySettings=true;
          saveModemSettings=true;
          printOutSettings();
          
        } else if (strstr(SettingGroup, "Print") != NULL) { // CMD:Settings:Print:
          printOutSettings();
          
        } else {
          Serial.println(InvalidCommand);
        }
      } else {
        Serial.println(InvalidCommand);
      }
    }
  }

#pragma endregion

#pragma region "SD"
  #include <Base64.h> //  Base64 library by Xander Electronics
  #include <SPI.h>
  #include <SD.h>

  // these files are opened once on boot
  // they are never closed but the data is saved via the .flush() method
  File RawDataFile;
  File MsgDataFile;
  const char* RawDataFileName = "raw.txt";
  const char* MsgDataFileName = "msg.txt";
  bool readingSD = false;
  APRSFormat_Raw rawData;
  byte* buff = (byte*)&rawData; // to access RawData as bytes

  // Base64-encode any binary buffer into a caller-provided static buffer
  // Returns: number of characters written (not including null terminator)
  int encodeBase64(char* output, size_t outputSize, const void* input, size_t inputSize) {
    // Calculate required encoded length
    size_t requiredSize = Base64.encodedLength(inputSize) + 1;  // +1 for null terminator

    if (outputSize < requiredSize) {
      // Not enough space in output buffer
      return -1;
    }

    // Perform Base64 encoding
    int actualLen = Base64.encode(output, (const char*)input, inputSize);
    output[actualLen] = '\0';  // Ensure null-termination

    return actualLen;
  }

  void printFormattedSize(uint32_t bytes) {
    if (bytes < 1024) {
      Serial.print(bytes);
      Serial.print(F(" B"));
    } else if (bytes < 1024 * 1024) {
      Serial.print(bytes / 1024.0, 2);
      Serial.print(F(" KB"));
    } else if (bytes < 1024UL * 1024UL * 1024UL) {
      Serial.print(bytes / 1024.0 / 1024.0, 2);
      Serial.print(F(" MB"));
    } else {
      Serial.print(bytes / 1024.0 / 1024.0 / 1024.0, 2);
      Serial.print(F(" GB"));
    }
  }

  // 1 GB (gigabyte) = 1,073,741,824 bytes
  // 2 GB = 2 × 1,073,741,824 = 2,147,483,648 bytes
  // 90% of 2 GB = 0.9 × 2,147,483,648 = 1,932,735,283.2 bytes
  //#define MAX_FILE_BYTES 2147483648UL
  #define WARN_AT_BYTES 1932735283UL
  void writeStructToSd(File& file, void* dataPtr, size_t dataSize, const char* label, bool addNewline = true) {
    if (file) {
      uint32_t size = file.size();
      if (size >= WARN_AT_BYTES) {
        Serial.print(F("[WARNING] "));
        Serial.print(label);
        Serial.print(F(" file is getting large: "));
        printFormattedSize(size);
        Serial.println(F("). Consider offloading or deleting."));
      }
      file.seek(size);
      file.write((byte*)dataPtr, dataSize);
      if (addNewline) file.write('\n');
      file.flush();
    } else {
      Serial.print(F("error opening "));
      Serial.print(label);
      Serial.println(F(".txt"));
    }
  }

  void writeRawDataToSd(APRSFormat_Raw RawData) {
    writeStructToSd(RawDataFile, &RawData, sizeof(APRSFormat_Raw), "raw");
  }

  void writeMsgDataToSd(APRSFormat_Msg MsgData) {
    writeStructToSd(MsgDataFile, &MsgData, sizeof(APRSFormat_Msg), "msg");
  }

  void startReadingFromSd(uint32_t StartPosition) {
    RawDataFile.seek(StartPosition);
    readingSD = true;
    Serial.println(F("Started reading new data from raw.txt..."));
  }

  void printRawDataFromSd() {
    // exit once the end of the file is reached
    if (!readingSD || !RawDataFile.available()) {
      readingSD = false;
      return;
    }

    // read the size of the APRS structure into buffer
    RawDataFile.read(buff, sizeof(APRSFormat_Raw));
    // burn the \n
    byte nextByte = RawDataFile.read(); 

    // Allocate space for Base64-encoded output:
    // Every 3 bytes of binary input becomes 4 ASCII characters in Base64.
    // We add 2 to the input size to round up to the next multiple of 3.
    // Then multiply by 4/3 (implemented as *4/3 here).
    // +1 for the null terminator at the end.
    char encodedBuffer[((sizeof(APRSFormat_Raw) + 2) / 3) * 4 + 1];
    // Encode and print the record
    int len = encodeBase64(encodedBuffer, sizeof(encodedBuffer), buff, sizeof(APRSFormat_Raw));

    Serial.print(F("SD Raw:"));
    Serial.println(encodedBuffer);
  }

  void printMsgDataFromSd(uint32_t StartPosition){
    APRSFormat_Msg MsgData;
    byte *buff = (byte *) &MsgData; // to access MsgData as bytes
    MsgDataFile.seek(StartPosition);

    // if the file opened okay, write to it:
    if (MsgDataFile) {
      Serial.println(F("Reading from msg.txt..."));
      while (MsgDataFile.available()) {
        for (int count=0;count<sizeof(APRSFormat_Msg); count++) { 
          if (MsgDataFile.available()) {
            *(buff+count) = MsgDataFile.read();
          }
        }
        Serial.print(F("to:")); Serial.print(MsgData.to);
        Serial.print(F("\tfrom:")); Serial.print(MsgData.from);
        Serial.print(F("\tmsg:")); Serial.print(MsgData.msg);
        Serial.print(F("\tline:")); Serial.print(MsgData.line);
        Serial.print(F("\tack:")); Serial.print(MsgData.ack);
        Serial.print(F("\tdate:")); Serial.print(MsgData.DateInt);
        Serial.print(F("\ttime:")); Serial.print(MsgData.TimeInt);
        Serial.println(MsgDataFile.read()); // take care of the '\n' (maybe not write this in future)
      }
    } else {
      // if the file didn't open, print an error:
      Serial.println(F("error opening msg.txt"));
    }
    
    MsgDataFile.flush(); // will save data
  }

  // THIS METHOD HAS NOT BEEN TESTED YET
  void deleteAllMsgData(){
    if (SD.exists(MsgDataFileName)) {
      MsgDataFile.close();
      SD.remove(MsgDataFileName);
      MsgDataFile = SD.open(MsgDataFileName, FILE_WRITE);
      Serial.println(F("msg.txt has been deleted."));
    } else {
      Serial.println(F("msg.txt does not exist."));
    }
  }

  // THIS METHOD HAS NOT BEEN TESTED YET
  void deleteAllRawData(){
    if (SD.exists(RawDataFileName)) {
      RawDataFile.close();
      SD.remove(RawDataFileName);
      RawDataFile = SD.open(RawDataFileName, FILE_WRITE);
      Serial.println(F("raw.txt has been deleted."));
    } else {
      Serial.println(F("raw.txt does not exist."));
    }
  }

  uint32_t getRawDataRecord(uint32_t RecordNumber, APRSFormat_Raw &RawData){
    uint32_t RecordCount = 0;
    byte *buff = (byte *) &RawData; // to access RawData as bytes
    // get size of APRSFormat_Raw so we know how long each record is
    uint32_t RecordSize = 0;
    RecordSize = sizeof(APRSFormat_Raw) + 1; // add one for line feed
    // get the size of the file so we can move backwards and get the latest messages in order
    uint32_t RawDataFileSize = 0;
    RawDataFileSize = RawDataFile.size();
    // if the file is opened okay, read from it:
    if (RawDataFile) {
      // print out how many records there are
      RecordCount = RawDataFileSize/RecordSize;
      if (RecordNumber <= RecordCount && RecordNumber > 0) {
        // seek to end of file minus total number of records size to get the record of interest
        RawDataFile.seek(RawDataFileSize-(RecordSize*RecordNumber));
        for (int count=0;count<sizeof(APRSFormat_Raw); count++) { 
          if (RawDataFile.available()) {
            *(buff+count) = RawDataFile.read();
          }
        }
      }
    } else {
      // if the file didn't open, print an error:
      Serial.println(F("error opening file"));
    }
    RawDataFile.flush(); // will save data
    return RecordCount;
  }

  uint32_t getMsgDataRecord(uint32_t RecordNumber, APRSFormat_Msg &MsgData){
    uint32_t RecordCount = 0;
    byte *buff = (byte *) &MsgData; // to access MsgData as bytes
    // get size of APRSFormat_Msg so we know how long each record is
    uint32_t RecordSize = 0;
    RecordSize = sizeof(APRSFormat_Msg) + 1; // add one for line feed
    // get the size of the file so we can move backwards and get the latest messages in order
    uint32_t MsgDataFileSize = 0;
    MsgDataFileSize = MsgDataFile.size();
    // if the file is opened okay, read from it:
    if (MsgDataFile) {
      // print out how many records there are
      RecordCount = MsgDataFileSize/RecordSize;
      if (RecordNumber <= RecordCount && RecordNumber > 0) {
        // seek to end of file minus total number of records size to get the record of interest
        MsgDataFile.seek(MsgDataFileSize-(RecordSize*RecordNumber));
        for (int count=0;count<sizeof(APRSFormat_Msg); count++) { 
          if (MsgDataFile.available()) {
            *(buff+count) = MsgDataFile.read();
          }
        }
      }
    } else {
      // if the file didn't open, print an error:
      Serial.println(F("error opening file"));
    }
    MsgDataFile.flush(); // will save data
    return RecordCount;
  }

void extractRawCallParts(const char input[15], char callsign[7], char ssid[3]) {
    // Initialize outputs to empty strings
    callsign[0] = '\0';
    ssid[0] = '\0';

    const char* start = strchr(input, '[');
    const char* dash = strchr(input, '-');
    const char* end  = strchr(input, ']');

    // Validate format: must contain all three and in correct order
    if (start && dash && end && start < dash && dash < end) {
        size_t callLen = dash - (start + 1);
        size_t ssidLen = end - (dash + 1);

        if (callLen < 7) {
            strncpy(callsign, start + 1, callLen);
            callsign[callLen] = '\0';  // null-terminate
        }

        if (ssidLen < 3) {
            strncpy(ssid, dash + 1, ssidLen);
            ssid[ssidLen] = '\0';  // null-terminate
        }
    }
}

void extractMsgCallParts(const char input[15], char callsign[7], char ssid[3]) {
    // Clear outputs
    callsign[0] = '\0';
    ssid[0] = '\0';

    const char* dash = strchr(input, '-');

    if (dash) {
        size_t callLen = dash - input;
        size_t ssidLen = strlen(dash + 1);

        if (callLen < 7) {
            strncpy(callsign, input, callLen);
            callsign[callLen] = '\0';
        }

        if (ssidLen < 3) {  // changed to 3 for 2-digit SSID + null
            strncpy(ssid, dash + 1, ssidLen);
            ssid[ssidLen] = '\0';
        }
    }
}

#pragma endregion

void setup(){
  // chose the baud rate wisely
  // the arduino has a serial buffer size of 64 bytes.
  // if the data is received too fast, the buffer may overflow before the arduino gets to Serial.read() it
  // at 9600 baud, we can read a 131 char string without losing data. this is long enough for our 99 char limit for strings (raw, comment, and message) plus the command structure
  // for example: CMD:Settings:APRS:Message Text:0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
  Serial.begin(9600);
  Serial1.begin(9600); // modem
  Serial2.begin(9600); // gps
  while (!Serial1) // wait for modem

  Serial.println();
  Serial.println(F("HamMessenger Copyright (C) 2021-2025  Dale Thomas"));
  Serial.println(F("This program comes with ABSOLUTELY NO WARRANTY."));
  Serial.println(F("This is free software, and you are welcome to redistribute it"));
  Serial.println(F("under certain conditions."));
  Serial.println();

  if (!SD.begin(53)) {
    Serial.println(F("sd card init failed!"));
    while (1);
  }

  // go ahead and open the files for read/write access
  RawDataFile = SD.open(RawDataFileName, FILE_WRITE);
  MsgDataFile = SD.open(MsgDataFileName, FILE_WRITE);
  
  Wire.begin(); // I2C Keyboard
  
  // check if this is a new device
  checkInit();
  readSettingsFromEeprom();
  applySettings=true;
  printOutSettings();

  // inputs
  pinMode(rxPin, INPUT);
  pinMode(txPin, INPUT);
  
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64), 0x3C (for the 128x32)
  handleDisplay_Startup();

  // reset timers
  gps_report_timer = millis();
  display_timeout_timer = millis();
  voltage_check_timer = millis() + VOLTAGE_CHECK_RATE;

}

void loop(){
  handleKeyboard();
  handleSettings();
  handleDisplays();
  handleSerial();
  handleSendMessage();
  handleModemCommands();
  handleAprsBeacon();
  handleVoltage();
  handleStats();

  // printing all contents from SD card is non-blocking
  if (readingSD) {
    printRawDataFromSd();
  }
}
