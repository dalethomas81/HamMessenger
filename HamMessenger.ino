// http://www.aprs.net/vm/DOS/PROTOCOL.HTM

#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_SH1106.h>
#include <TinyGPS++.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>

#include <Wire.h>         // M5Stack Keyboard 
#define CARDKB_ADDR 0x5F  // M5Stack Keyboard https://docs.m5stack.com/en/unit/cardkb
char keyboardInputChar, keyboardInputCharLast;

// sketch will write default settings if new build
//const char version[] = "build "  __DATE__ " " __TIME__; 
const char version[] = __DATE__ " " __TIME__; 

#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

//#define SCREEN_WIDTH 128 // OLED display width, in pixels
//#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DISPLAY_REFRESH_RATE                  100
#define DISPLAY_REFRESH_RATE_SCROLL           80       // min 60
#define DISPLAY_BLINK_RATE                    500
#define CHARACTER_INCREMENT_RATE              350
#define UI_DISPLAY_HOME                       10000
#define UI_DISPLAY_MESSAGES                   11000
#define UI_DISPLAY_LIVEFEED                   12000
#define UI_DISPLAY_SETTINGS                   13000
#define UI_DISPLAY_SETTINGS_APRS              13100
#define UI_DISPLAY_SETTINGS_GPS               13200
#define UI_DISPLAY_SETTINGS_DISPLAY           13300
#define UI_DISPLAY_SETTINGS_SAVE              13400

#define UI_DISPLAY_ROW_TOP                    0
#define UI_DISPLAY_ROW_01                     8
#define UI_DISPLAY_ROW_02                     16
#define UI_DISPLAY_ROW_03                     24
#define UI_DISPLAY_ROW_04                     32
#define UI_DISPLAY_ROW_05                     40
#define UI_DISPLAY_ROW_06                     48
#define UI_DISPLAY_ROW_BOTTOM                 56

long currentDisplay = UI_DISPLAY_HOME;
long previousDisplay_Messages = UI_DISPLAY_HOME;
long previousDisplay_LiveFeed = UI_DISPLAY_HOME;
long previousDisplay_Settings = UI_DISPLAY_HOME;
long previousDisplay_Settings_Save = UI_DISPLAY_SETTINGS;
long previousDisplay_Settings_APRS = UI_DISPLAY_SETTINGS;
long previousDisplay_Settings_GPS = UI_DISPLAY_SETTINGS;
long previousDisplay_Settings_Display = UI_DISPLAY_SETTINGS;
int cursorPosition_X = 0, cursorPosition_X_Last = 0;
int cursorPosition_Y = 0, cursorPosition_Y_Last = 0;
int ScrollingIndex_LiveFeed, ScrollingIndex_LiveFeed_minX;
int ScrollingIndex_MessageFeed, ScrollingIndex_MessageFeed_minX;

// do something on first show of display
bool displayInitialized_Home = false, displayInitialized_Messages = false;
bool displayInitialized_LiveFeed = false, displayInitialized_Settings = false, displayInitialized_Settings_Save = false;
bool displayInitialized_Settings_APRS = false, displayInitialized_Settings_GPS = false;
bool displayInitialized_Settings_Display = false;
// leave the display after a timeout period
bool leaveDisplay_MessageFeed = false, leaveDisplay_LiveFeed = false, leaveDisplay_Settings = false, leaveDisplay_Settings_Save = false;
bool leaveDisplay_Settings_APRS = false, leaveDisplay_Settings_GPS = false, leaveDisplay_Settings_Display = false;
// refresh the displays
bool displayRefresh_Global = true, displayRefresh_Scroll = true;
// go into edit mode in the settings
bool editMode_Settings_APRS = false, editMode_Settings_GPS = false, editMode_Settings_Display = false;
bool displayBlink = false, characterIncrement = false;
int Settings_EditType = 0;
int Settings_EditValueSize = 0;
bool settingsChanged = false;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//Adafruit_SSH1106 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SH1106 display(OLED_RESET);

#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

TinyGPSPlus gps;

//#define GPS_REPORT_FREQUENCY 10000            // how often to check for updated GPS info
#define DESTINATION_REPORT_FREQUENCY 20000    // how often distance to target is sent to serial - will be removed
//#define APRS_COMMENT_FREQUENCY 300000         // how often a comment is sent with position
#define MAXIMUM_MODEM_COMMAND_RATE 100        // maximum rate that commands can be sent to modem

unsigned long gps_report_timer, destination_report_timer, modem_command_timer, aprs_beacon_timer, display_refresh_timer, display_refresh_timer_scroll;
unsigned long leave_display_timer_MessageFeed, leave_display_timer_Livefeed, leave_display_timer_Settings;
unsigned long leave_display_timer_Settings_APRS, leave_display_timer_Settings_GPS, leave_display_timer_Settings_Display;
unsigned long voltage_check_timer;
unsigned long processor_scan_time, scanTime;
unsigned long display_blink_timer;
unsigned long button_hold_timer_down, character_increment_timer;
unsigned long character_increment_rate = CHARACTER_INCREMENT_RATE;

// http://ember2ash.com/lat.htm
float fltCurrentLatDeg = 0;
float fltCurrentLngDeg = 0;
//float fltPositionTolerance = 0.00001; // (%) represents the change in position that warrants an update to the modem.
float fltLastLatDeg = 0.0;
float fltLastLngDeg = 0.0;
char strCurrentLatHeading[] = "N";
char strCurrentLngHeading[] = "N";
char strCurrentLat[9] = {'0','0','0','0','.','0','0','N','\0'};
char strCurrentLng[10] = {'0','0','0','0','0','.','0','0','N','\0'};
bool blnModemCmdFlag_Lat=false, blnModemCmdFlag_Lng=false;
bool blnModemCmdFlag_Raw=false, blnModemCmdFlag_Cmt=false, blnModemCmdFlag_Msg=false;
bool blnModemCmdFlag_MsgRecipient=false, blnModemCmdFlag_MsgRecipientSSID=false;
bool blnModemCmdFlag_Setc=false, blnModemCmdFlag_Setsc=false;
bool blnModemCmdFlag_Setd=false, blnModemCmdFlag_Setsd=false;
bool blnModemCmdFlag_Set1=false, blnModemCmdFlag_Sets1=false;
bool blnModemCmdFlag_Set2=false, blnModemCmdFlag_Sets2=false;
bool blnModemCmdFlag_Setls=false, blnModemCmdFlag_Setlt=false;
bool blnModemCmdFlag_Setma=false, blnModemCmdFlag_Setw=false;
bool blnModemCmdFlag_SetW=false, blnModemCmdFlag_Setmr=false;
bool blnModemCmdFlag_SetS=false, blnModemCmdFlag_SetC=false;
bool blnModemCmdFlag_SetH=false;
bool blnAprsAutomaticCommentEnabled = true;
bool applySettings=false, saveModemSettings=false;
int applySettings_Seq=0;

// London                                 LAT:51.508131     LNG:-0.128002
//double DESTINATION_LAT = 51.508131, DESTINATION_LON = -0.128002;

struct APRSFormat_Raw {
  char src[15];
  char dst[15];
  char path[10];
  char data[125];
};

struct APRSFormat_Msg {
  char from[15] = {'\0'};
  char to[15] = {'\0'};
  char msg[100] = {'\0'};
  int line = 0;
  bool ack = false;
};

struct GPS_Date {
  byte Day = 1;
  byte Month = 1;
  int Year = 1970;
  //char DateString[] = "01-01-1970";
};

struct GPS_Time {
  byte Hour = 0;
  byte Minute = 0;
  byte Second = 0;
  byte CentiSecond = 0;
  //char TimeString[] = "00:00:00.00";
};

struct GPS {
  byte Satellites;
  GPS_Date Date;
  GPS_Time Time;
} GPSData;

#define LIVEFEED_BUFFER_SIZE  5
int liveFeedBufferIndex = -1, oldliveFeedBufferIndex = -1, liveFeedBufferIndex_RecordCount = 0;
bool liveFeedIsEmpty = true;
APRSFormat_Raw LiveFeedBuffer[LIVEFEED_BUFFER_SIZE] = {'\0'};

#define INCOMING_MESSAGE_BUFFER_SIZE 5
int incomingMessageBufferIndex = -1, oldIncomingMessageBufferIndex = -1, incomingMessageBufferIndex_RecordCount = 0;
bool messageFeedIsEmpty = true;
APRSFormat_Msg IncomingMessageBuffer[INCOMING_MESSAGE_BUFFER_SIZE];

// input pins
#define rxPin A5 // using analog input pins
#define txPin A6

// voltage settings
int VoltagePercent = 0;
long Voltage = 0;
#define BATT_CHARGED            7200
#define BATT_DISCHARGED         5600
#define VOLTAGE_CHECK_RATE      10000

// settings
#define EEPROM_SETTINGS_START_ADDR    1000
#define SETTINGS_EDIT_TYPE_NONE        0
#define SETTINGS_EDIT_TYPE_BOOLEAN     1
#define SETTINGS_EDIT_TYPE_INT         2
#define SETTINGS_EDIT_TYPE_UINT        3
#define SETTINGS_EDIT_TYPE_LONG        4
#define SETTINGS_EDIT_TYPE_ULONG       5
#define SETTINGS_EDIT_TYPE_FLOAT       6
#define SETTINGS_EDIT_TYPE_STRING2     7
#define SETTINGS_EDIT_TYPE_STRING7     8
#define SETTINGS_EDIT_TYPE_STRING100   9
                        
const char *MenuItems_Settings[] = {"APRS","GPS","Display"};
const char *MenuItems_Settings_APRS[] = {"Beacon Frequency","Raw Packet","Comment","Message","Recipient Callsign","Recipient SSID", "My Callsign","Callsign SSID", 
                                        "Destination Callsign", "Destination SSID", "PATH1 Callsign", "PATH1 SSID", "PATH2 Callsign", "PATH2 SSID",
                                        "Symbol", "Table", "Automatic ACK", "Preamble", "Tail"};
const char *MenuItems_Settings_GPS[] = {"Update Frequency","Position Tolerance","Destination Latitude","Destination Longitude"};
const char *MenuItems_Settings_Display[] = {"Timeout", "Brightness", "Show Position", "Scroll Messages", "Scroll Speed"};

int Settings_Type_APRS[] = {SETTINGS_EDIT_TYPE_ULONG,SETTINGS_EDIT_TYPE_STRING100,SETTINGS_EDIT_TYPE_STRING100,SETTINGS_EDIT_TYPE_STRING100,SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,
                            SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,
                            SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_BOOLEAN,SETTINGS_EDIT_TYPE_UINT,SETTINGS_EDIT_TYPE_UINT};
int Settings_Type_GPS[] = {SETTINGS_EDIT_TYPE_ULONG,SETTINGS_EDIT_TYPE_FLOAT,SETTINGS_EDIT_TYPE_FLOAT,SETTINGS_EDIT_TYPE_FLOAT};
int Settings_Type_Display[] = {SETTINGS_EDIT_TYPE_ULONG, SETTINGS_EDIT_TYPE_UINT, SETTINGS_EDIT_TYPE_BOOLEAN, SETTINGS_EDIT_TYPE_BOOLEAN, SETTINGS_EDIT_TYPE_UINT};
int Settings_TypeIndex_APRS[] = {0,0,1,2,0,0,1,1,2,2,3,3,4,4,5,6,2,1,2}; // this is the index in the array of the data arrays below
int Settings_TypeIndex_GPS[] = {1,0,1,2};
int Settings_TypeIndex_Display[] = {2,0,0,1,3};
// data arrays
bool Settings_TypeBool[3] = {true,true,true}; // display show position, scroll messages, auto ACK
int Settings_TypeInt[0] = {};
unsigned int Settings_TypeUInt[4] = {100,400,80,4}; // display brightness, aprs preamble, aprs tail, scroll speed
long Settings_TypeLong[0] = {};
unsigned long Settings_TypeULong[3] = {300000,10000,2000}; // aprs beacon frequency, gps update frequency, display timeout
float Settings_TypeFloat[3] = {0.00001,34.790040,-82.790672}; // gps position tolerance, gps latitude, gps longitude
char Settings_TypeString2[7][2] = {'0','\0'};
char Settings_TypeString7[5][7] = {'N','O','C','A','L','L','\0'};
char Settings_TypeString100[3][100] = {'T','e','s','t','\0'};
char Settings_TempDispCharArr[100];

#define SETTINGS_APRS_BEACON_FREQUENCY        Settings_TypeULong[Settings_TypeIndex_APRS[0]]        // beacon frequency
#define SETTINGS_APRS_RAW_PACKET              Settings_TypeString100[Settings_TypeIndex_APRS[1]]    // raw packet
#define SETTINGS_APRS_COMMENT                 Settings_TypeString100[Settings_TypeIndex_APRS[2]]    // comment
#define SETTINGS_APRS_MESSAGE                 Settings_TypeString100[Settings_TypeIndex_APRS[3]]    // message
#define SETTINGS_APRS_RECIPIENT_CALL          Settings_TypeString7[Settings_TypeIndex_APRS[4]]      // recipient
#define SETTINGS_APRS_RECIPIENT_SSID          Settings_TypeString2[Settings_TypeIndex_APRS[5]]      // recipient ssid
#define SETTINGS_APRS_CALLSIGN                Settings_TypeString7[Settings_TypeIndex_APRS[6]]      // callsign
#define SETTINGS_APRS_CALLSIGN_SSID           Settings_TypeString2[Settings_TypeIndex_APRS[7]]      // callsign ssid
#define SETTINGS_APRS_DESTINATION_CALL        Settings_TypeString7[Settings_TypeIndex_APRS[8]]      // Destination Callsign
#define SETTINGS_APRS_DESTINATION_SSID        Settings_TypeString2[Settings_TypeIndex_APRS[9]]      // Destination SSID
#define SETTINGS_APRS_PATH1_CALL              Settings_TypeString7[Settings_TypeIndex_APRS[10]]     // PATH1 Callsign
#define SETTINGS_APRS_PATH1_SSID              Settings_TypeString2[Settings_TypeIndex_APRS[11]]     // PATH1 SSID
#define SETTINGS_APRS_PATH2_CALL              Settings_TypeString7[Settings_TypeIndex_APRS[12]]     // PATH2 Callsign
#define SETTINGS_APRS_PATH2_SSID              Settings_TypeString2[Settings_TypeIndex_APRS[13]]     // PATH2 SSID
#define SETTINGS_APRS_SYMBOL                  Settings_TypeString2[Settings_TypeIndex_APRS[14]]     // Symbol
#define SETTINGS_APRS_SYMBOL_TABLE            Settings_TypeString2[Settings_TypeIndex_APRS[15]]     // Symbol Table
#define SETTINGS_APRS_AUTOMATIC_ACK           Settings_TypeBool[Settings_TypeIndex_APRS[16]]        // Automatic ACK
#define SETTINGS_APRS_PREAMBLE                Settings_TypeUInt[Settings_TypeIndex_APRS[17]]        // Preamble
#define SETTINGS_APRS_TAIL                    Settings_TypeUInt[Settings_TypeIndex_APRS[18]]        // Tail

#define SETTINGS_GPS_UPDATE_FREQUENCY         Settings_TypeULong[Settings_TypeIndex_GPS[0]]       // update frequency
#define SETTINGS_GPS_POSITION_TOLERANCE       Settings_TypeFloat[Settings_TypeIndex_GPS[1]]       // position tolerance
#define SETTINGS_GPS_DESTINATION_LATITUDE     Settings_TypeFloat[Settings_TypeIndex_GPS[2]]       // destination latitude
#define SETTINGS_GPS_DESTINATION_LONGITUDE    Settings_TypeFloat[Settings_TypeIndex_GPS[3]]       // destination longitute

#define SETTINGS_DISPLAY_TIMEOUT              Settings_TypeULong[Settings_TypeIndex_Display[0]]        // timeout
#define SETTINGS_DISPLAY_BRIGHTNESS           Settings_TypeUInt[Settings_TypeIndex_Display[1]]         // brightness
#define SETTINGS_DISPLAY_SHOW_POSITION        Settings_TypeBool[Settings_TypeIndex_Display[2]]         // show position
#define SETTINGS_DISPLAY_SCROLL_MESSAGES      Settings_TypeBool[Settings_TypeIndex_Display[3]]         // scroll messages
#define SETTINGS_DISPLAY_SCROLL_SPEED         Settings_TypeUInt[Settings_TypeIndex_Display[4]]         // scroll speed


// store common strings here
const char DataEntered[] = {"Data entered="};
const char InvalidCommand[] = {"Invalid command."};
const char InvalidData_UnsignedInt[] = {"Invalid data. Expected unsigned integer 0-65535 instead got "};
const char InvalidData_UnsignedLong[] = {"Invalid data. Expected unsigned long 0-4294967295 instead got "};
const char InvalidData_TrueFalse[] = {"Invalid data. Expected True/False or 1/0"};
const char Initialized[] = {"Initialized01"};

// SD card 
File myFile;

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

void checkInit(){
  Serial.println(F("Checking initialization..."));
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

void applyDefaultsToSettings() {
  Serial.println(F("Applying defaults to settings..."));
  SETTINGS_APRS_BEACON_FREQUENCY = 300000;                // beacon frequency
  char strTemp1[] = {"HamMessenger!"};
  for (int i=0; i<sizeof(strTemp1);i++) {
     SETTINGS_APRS_RAW_PACKET[i] = strTemp1[i]; // raw packet
  }
  char strTemp2[] = {"Testing HamMessenger!"};
  for (int i=0; i<sizeof(strTemp2);i++) {
     SETTINGS_APRS_COMMENT[i] = strTemp2[i]; // comment
  }
  char strTemp3[] = {"Hi!"};
  for (int i=0; i<sizeof(strTemp3);i++) {
     SETTINGS_APRS_MESSAGE[i] = strTemp3[i]; // message
  }
  char strTemp4[] = {"NOCALL"};
  for (int i=0; i<sizeof(strTemp4);i++) {
     SETTINGS_APRS_RECIPIENT_CALL[i] = strTemp4[i];   // recipient
  }
  SETTINGS_APRS_RECIPIENT_SSID[0] = '3';              // recipient ssid
  char strTemp5[] = {"NOCALL"};
  for (int i=0; i<sizeof(strTemp5);i++) {
     SETTINGS_APRS_CALLSIGN[i] = strTemp5[i];   // callsign
  }
  SETTINGS_APRS_CALLSIGN_SSID[0] = '3';              // callsign ssid
  char strTemp6[] = {"APRS"};
  for (int i=0; i<sizeof(strTemp6);i++) {
     SETTINGS_APRS_DESTINATION_CALL[i] = strTemp6[i];   // Destination Callsign
  }
  SETTINGS_APRS_DESTINATION_SSID[0]  = '0';             // Destination SSID
  char strTemp7[] = {"WIDE1"};
  for (int i=0; i<sizeof(strTemp7);i++) {
     SETTINGS_APRS_PATH1_CALL[i] = strTemp7[i];  // PATH1 Callsign
  }
  SETTINGS_APRS_PATH1_SSID[0] = '1';             // PATH1 SSID
  char strTemp8[] = {"WIDE2"};
  for (int i=0; i<sizeof(strTemp8)-1;i++) {
     SETTINGS_APRS_PATH2_CALL[i] = strTemp8[i];  // PATH2 Callsign
  }
  SETTINGS_APRS_PATH2_SSID[0] = '2';             // PATH2 SSID
  SETTINGS_APRS_SYMBOL[0] = 'n';             // Symbol
  SETTINGS_APRS_SYMBOL_TABLE[0] = 's';             // Symbol Table
  SETTINGS_APRS_AUTOMATIC_ACK = true;                  // Automatic ACK
  SETTINGS_APRS_PREAMBLE = 350;                   // Preamble
  SETTINGS_APRS_TAIL = 80;                    // Tail
  
  // London  LAT:51.508131     LNG:-0.128002
  SETTINGS_GPS_UPDATE_FREQUENCY = 10000;                  // update frequency
  SETTINGS_GPS_POSITION_TOLERANCE = 0.00001;                // position tolerance
  SETTINGS_GPS_DESTINATION_LATITUDE = 51.508131;              // destination latitude
  SETTINGS_GPS_DESTINATION_LONGITUDE = -0.128002;              // destination longitute
  
  SETTINGS_DISPLAY_TIMEOUT = 2000;                    // timeout
  SETTINGS_DISPLAY_BRIGHTNESS = 100;                      // brightness
  SETTINGS_DISPLAY_SHOW_POSITION = true;                     // show position
  SETTINGS_DISPLAY_SCROLL_MESSAGES = true;                     // scroll messages
  SETTINGS_DISPLAY_SCROLL_SPEED = 4;                        // scroll speed

  writeSettingsToEeprom();
}

void writeSettingsToEeprom() {
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

void printOutSettings(){
  Serial.println();
  Serial.println(F("////////  Current Settings  ////////"));
  Serial.println(F("APRS:"));
  Serial.print(F("Beacon Frequency=")); Serial.println(SETTINGS_APRS_BEACON_FREQUENCY);
  Serial.print(F("Raw Packet=")); Serial.println(SETTINGS_APRS_RAW_PACKET);
  Serial.print(F("Comment=")); Serial.println(SETTINGS_APRS_COMMENT);
  Serial.print(F("Message=")); Serial.println(SETTINGS_APRS_MESSAGE);
  Serial.print(F("Message Recipient=")); Serial.println(SETTINGS_APRS_RECIPIENT_CALL);
  Serial.print(F("Message Recipient SSID=")); Serial.println(SETTINGS_APRS_RECIPIENT_SSID);
  Serial.print(F("Callsign=")); Serial.println(SETTINGS_APRS_CALLSIGN);
  Serial.print(F("Callsign SSID=")); Serial.println(SETTINGS_APRS_CALLSIGN_SSID);
  Serial.print(F("Destination Callsign=")); Serial.println(SETTINGS_APRS_DESTINATION_CALL);
  Serial.print(F("Destination SSID=")); Serial.println(SETTINGS_APRS_DESTINATION_SSID);
  Serial.print(F("PATH1 Callsign=")); Serial.println(SETTINGS_APRS_PATH1_CALL);
  Serial.print(F("PATH1 SSID=")); Serial.println(SETTINGS_APRS_PATH1_SSID);
  Serial.print(F("PATH2 Callsign=")); Serial.println(SETTINGS_APRS_PATH2_CALL);
  Serial.print(F("PATH2 SSID=")); Serial.println(SETTINGS_APRS_PATH2_SSID);
  Serial.print(F("Symbol=")); Serial.println(SETTINGS_APRS_SYMBOL);
  Serial.print(F("Symbol Table=")); Serial.println(SETTINGS_APRS_SYMBOL_TABLE);
  Serial.print(F("Automatic ACK=")); Serial.println(SETTINGS_APRS_AUTOMATIC_ACK);
  Serial.print(F("Preamble=")); Serial.println(SETTINGS_APRS_PREAMBLE);
  Serial.print(F("Tail=")); Serial.println(SETTINGS_APRS_TAIL);
  Serial.println(F("GPS:"));
  Serial.print(F("Update Frequency=")); Serial.println(SETTINGS_GPS_UPDATE_FREQUENCY);
  Serial.print(F("Position Tolerance=")); Serial.println(SETTINGS_GPS_POSITION_TOLERANCE, 6);
  Serial.print(F("Destination Latitude=")); Serial.println(SETTINGS_GPS_DESTINATION_LATITUDE, 6);
  Serial.print(F("Destination Longitude=")); Serial.println(SETTINGS_GPS_DESTINATION_LONGITUDE, 6);
  Serial.println(F("Display:"));
  Serial.print(F("Timeout=")); Serial.println(SETTINGS_DISPLAY_TIMEOUT);
  Serial.print(F("Brightness=")); Serial.println(SETTINGS_DISPLAY_BRIGHTNESS);
  Serial.print(F("Show Position=")); Serial.println(SETTINGS_DISPLAY_SHOW_POSITION);
  Serial.print(F("Scroll Messages=")); Serial.println(SETTINGS_DISPLAY_SCROLL_MESSAGES);
  Serial.print(F("Scroll Speed=")); Serial.println(SETTINGS_DISPLAY_SCROLL_SPEED);
  Serial.println();
  Serial.print(F("Version=")); Serial.println(version);
  Serial.println();
}

#pragma region "Displays"
  void handleDisplays(){ 
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

    // add display objects to buffer
    switch (currentDisplay) {
      case UI_DISPLAY_HOME:
        handleDisplay_Home();
        break;
      case UI_DISPLAY_MESSAGES:
        handleDisplay_Messages();
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
      default:
        handleDisplay_Home();
        break;
    }
    
    displayRefresh_Global = false;
    displayRefresh_Scroll = false;
  }

  void handleDisplay_Startup(){
    
    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    //display.display();
    //delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();
    
    display.setTextSize(1);                     // Normal 1:1 pixel scale - default letter size is 5x8 pixels
    //display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setTextColor(WHITE);        // Draw white text
    display.setTextWrap(false);
    
    display.setCursor(30,UI_DISPLAY_ROW_01);                     // Start at top-left corner 0 pixels right, 0 pixels down
    display.println(F("HamMessenger"));
    
    //display.setCursor(0,UI_DISPLAY_ROW_02);                     // Start at top-left corner 0 pixels right, 0 pixels down
    //display.println(F("Build Date"));
    
    display.setCursor(0,UI_DISPLAY_ROW_03);                     // Start at top-left corner 0 pixels right, 0 pixels down
    display.println(version);
    
    display.setCursor(30,UI_DISPLAY_ROW_05);                    // 0 pixels right, 13 pixels down
    display.println(F("Call:"));
    
    display.setCursor(60,UI_DISPLAY_ROW_05);                   // 50 pixels right, 13 pixels down
    display.println(SETTINGS_APRS_CALLSIGN);
    
    display.display();
    delay(5000); // Pause for 2 seconds
  }

  void handleDisplay_Global(){ 
    if (digitalRead(rxPin) == HIGH){
      //Serial.println(F("[Rx]"));
      display.setCursor(0,UI_DISPLAY_ROW_TOP);
      display.print(F("Rx"));
      }
    
    if (digitalRead(txPin) == HIGH){
      //Serial.println(F("[Tx]"));
      display.setCursor(15,UI_DISPLAY_ROW_TOP);
      display.print(F("Tx"));
    }
    
    display.setCursor(45,UI_DISPLAY_ROW_TOP);
    display.print(String(Voltage) + F("mV"));

    // show keyboard input on top. remove later and uncomment voltage above
    //display.setCursor(45,UI_DISPLAY_ROW_TOP);
    //display.print(String(keyboardInputCharLast) + F(" ")); display.print(keyboardInputCharLast, DEC);
    
    display.setCursor(100,UI_DISPLAY_ROW_TOP);
    display.print(String(scanTime) + F("ms"));

    if (SETTINGS_DISPLAY_SHOW_POSITION) {
      display.setCursor(0,UI_DISPLAY_ROW_BOTTOM);
      display.print(F("LT:"));
      
      display.setCursor(0+18,UI_DISPLAY_ROW_BOTTOM);
      display.print(fltCurrentLatDeg);
      
      display.setCursor(59,UI_DISPLAY_ROW_BOTTOM);
      display.print(F(" LG:"));
      
      display.setCursor(59+24,UI_DISPLAY_ROW_BOTTOM);
      display.print(fltCurrentLngDeg);
    }
  }

  void handleDisplay_Home(){
    // on first show
    if (!displayInitialized_Home){
      // reset other displays
      displayInitialized_Messages = false;
      displayInitialized_LiveFeed = false;
      displayInitialized_Settings = false;
      displayInitialized_Settings_APRS = false;
      displayInitialized_Settings_GPS = false;
      displayInitialized_Settings_Display = false;
      displayInitialized_Home = true;
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      //Serial.print(F("Entered Display Home:")); Serial.println(currentDisplay);
    }
    // handle button context for current display
    if (keyboardInputChar == -75){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=2;
      }
    }
    if (keyboardInputChar == -74){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y < 2){
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == 13){ // 13 DEC - Enter Key // 8 DEC - Backspace Key
      switch (cursorPosition_Y){
        case 0:
          currentDisplay = UI_DISPLAY_MESSAGES;
          previousDisplay_Messages = UI_DISPLAY_HOME;
          break;
        case 1:
          currentDisplay = UI_DISPLAY_LIVEFEED;
          previousDisplay_LiveFeed = UI_DISPLAY_HOME;
          break;
        case 2:
          currentDisplay = UI_DISPLAY_SETTINGS;
          previousDisplay_Settings = UI_DISPLAY_HOME;
          break;
        default:
          currentDisplay = UI_DISPLAY_HOME;
          break;
      }
    }
    if (keyboardInputChar == 27){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
    }      
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();
      // add global objects to buffer
      handleDisplay_Global();
      int selectionRow = 0;
      switch (cursorPosition_Y) {
        case 0:
          selectionRow = UI_DISPLAY_ROW_02;
          break;
        case 1:
          selectionRow = UI_DISPLAY_ROW_03;
          break;
        case 2:
          selectionRow = UI_DISPLAY_ROW_04;
          break;
        default:
          selectionRow = UI_DISPLAY_ROW_02;
          break;
      }
      display.setCursor(0,selectionRow);
      display.print(F(">"));
      
      display.setCursor(6,UI_DISPLAY_ROW_02);                    // 0 pixels right, 25 pixels down
      display.print(F("Messages"));
      
      display.setCursor(6,UI_DISPLAY_ROW_03);
      display.print(F("Live Feed"));
      
      display.setCursor(6,UI_DISPLAY_ROW_04);
      display.print(F("Settings"));
      
      // display all content from buffer
      display.display();
    }
  }

  void handleDisplay_Messages(){
    //  Radio 1: CMD: Modem:#Hi!
    //  Radio 2: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :Hi!{006
    //  Radio 1: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :ack006
    
    // on first show
    if (!displayInitialized_Messages){
      // reset other displays
      displayInitialized_Home = false;
      displayInitialized_Messages = true;
      displayInitialized_Settings = false;
      displayInitialized_Settings_APRS = false;
      displayInitialized_Settings_GPS = false;
      displayInitialized_Settings_Display = false;
      displayInitialized_LiveFeed = false;
      cursorPosition_X = 0;
      if (incomingMessageBufferIndex >= 0) {
        cursorPosition_Y = incomingMessageBufferIndex;
      } else {
        cursorPosition_Y = 0;
      }
      cursorPosition_X_Last = cursorPosition_X;
      cursorPosition_Y_Last = -1;
      oldIncomingMessageBufferIndex = incomingMessageBufferIndex; // set these equal to monitor changes
      //Serial.print(F("Entered Display Messages:")); Serial.println(currentDisplay);
    }
    // change cursor position as new mesasages arrive
    if (incomingMessageBufferIndex != oldIncomingMessageBufferIndex) {
      oldIncomingMessageBufferIndex = incomingMessageBufferIndex;
      cursorPosition_Y = incomingMessageBufferIndex;
    }
    // handle button context for current display
    if (keyboardInputChar == -75){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=incomingMessageBufferIndex_RecordCount - 1;
      }
    }
    if (keyboardInputChar == -74){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y < incomingMessageBufferIndex_RecordCount - 1){ // dont scroll past the number of records in the array
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == 13){ // 13 DEC - Enter Key // 8 DEC - Backspace Key
    }
    if (keyboardInputChar == 27){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
      currentDisplay = previousDisplay_Messages;
      return;
    }
    // build display
    if (displayRefresh_Scroll){
      if (cursorPosition_Y != cursorPosition_Y_Last){ // changed to new record (index)
        cursorPosition_Y_Last = cursorPosition_Y; 
        int dataLen = 0;
        for(int i=0;i<sizeof(IncomingMessageBuffer[cursorPosition_Y].msg);i++){
          if (IncomingMessageBuffer[cursorPosition_Y].msg[i] != '\0'){
            dataLen++;
          } else {
            i = sizeof(IncomingMessageBuffer[cursorPosition_Y].msg); // get out
          }
        }
        ScrollingIndex_MessageFeed_minX = -10 * dataLen; // 10 = 5 pixels/character * text size 2
        ScrollingIndex_MessageFeed = display.width(); // starting point for text 
      }
      // clear the buffer
      display.clearDisplay();
      // add global objects to buffer
      handleDisplay_Global();
      if (!messageFeedIsEmpty){
        char to_from[24] = {'\0'};
        byte index = 0;
        for (byte i=0;i<sizeof(IncomingMessageBuffer[cursorPosition_Y].from)-1;i++){
          if (IncomingMessageBuffer[cursorPosition_Y].from[i] != '\0'){
            to_from[index] = IncomingMessageBuffer[cursorPosition_Y].from[i];
            index++;
          } else {
            i = sizeof(IncomingMessageBuffer[cursorPosition_Y].from); // get out
          }
        }
        to_from[index] = '>'; index++;
        for (byte i=0;i<sizeof(IncomingMessageBuffer[cursorPosition_Y].to)-1;i++){
          if (IncomingMessageBuffer[cursorPosition_Y].to[i] != '\0'){
            to_from[index] = IncomingMessageBuffer[cursorPosition_Y].to[i];
            index++;
          } else {
            i = sizeof(IncomingMessageBuffer[cursorPosition_Y].to); // get out
          }
        }
        display.setCursor(0,UI_DISPLAY_ROW_01);
        display.print(to_from);

        //byte indexRow = 0;
        //if (cursorPosition_Y < 10) indexRow = 6;
        //display.setCursor(indexRow,UI_DISPLAY_ROW_02);
        //display.print(cursorPosition_Y);
        //display.setCursor(11,UI_DISPLAY_ROW_02);
        //display.print(F(":"));
        display.setCursor(ScrollingIndex_MessageFeed,UI_DISPLAY_ROW_02);
        display.setTextSize(2);                     // Normal 1:1 pixel scale - default letter size is 5x8 pixels
        display.print(IncomingMessageBuffer[cursorPosition_Y].msg); 
        display.setTextSize(1);                     // Normal 1:1 pixel scale - default letter size is 5x8 pixels
        if (keyboardInputChar == 13 || SETTINGS_DISPLAY_SCROLL_MESSAGES){ //  scroll only when enter pressed TODO: this wont work because key press not persistent
          ScrollingIndex_MessageFeed = ScrollingIndex_MessageFeed - SETTINGS_DISPLAY_SCROLL_SPEED; // higher number here is faster scroll but choppy
          if(ScrollingIndex_MessageFeed < ScrollingIndex_MessageFeed_minX) ScrollingIndex_MessageFeed = display.width(); // makeshift scroll because startScrollleft truncates the string!
        }
      } else {
        display.setCursor(0,UI_DISPLAY_ROW_02);
        display.print(F("You have no messages"));
        if (!leaveDisplay_MessageFeed) {
          leaveDisplay_MessageFeed = true;
          leave_display_timer_MessageFeed = millis();
        }
      }
      // display all content from buffer
      display.display();
    }
    // timeout and leave
    if (millis() - leave_display_timer_MessageFeed > SETTINGS_DISPLAY_TIMEOUT && leaveDisplay_MessageFeed){
      leaveDisplay_MessageFeed = false;
      currentDisplay = UI_DISPLAY_HOME;
      return;
    }
  }

  void handleDisplay_LiveFeed(){
    // on first show
    if (!displayInitialized_LiveFeed){
      // reset other displays
      displayInitialized_Home = false;
      displayInitialized_Messages = false;
      displayInitialized_Settings = false;
      displayInitialized_Settings_APRS = false;
      displayInitialized_Settings_GPS = false;
      displayInitialized_Settings_Display = false;
      displayInitialized_LiveFeed = true;
      cursorPosition_X = 0;
      if (liveFeedBufferIndex >= 0) {
        cursorPosition_Y = liveFeedBufferIndex;
      } else {
        cursorPosition_Y = 0;
      }
      cursorPosition_X_Last = cursorPosition_X;
      cursorPosition_Y_Last = -1;
      oldliveFeedBufferIndex = liveFeedBufferIndex; // set these equal to monitor changes
      //Serial.print(F("Entered Display Live Feed:")); Serial.println(currentDisplay);
    }
    // change cursor position as new mesasages arrive
    if (liveFeedBufferIndex != oldliveFeedBufferIndex) {
      oldliveFeedBufferIndex = liveFeedBufferIndex;
      cursorPosition_Y = liveFeedBufferIndex;
    }
    // handle button context for current display
    if (keyboardInputChar == -75){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=liveFeedBufferIndex_RecordCount - 1;
      }
    }
    if (keyboardInputChar == -74){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y < liveFeedBufferIndex_RecordCount - 1){ // dont scroll past the number of records in the array
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == 13){ // 13 DEC - Enter Key // 8 DEC - Backspace Key
    }
    if (keyboardInputChar == 27){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
      currentDisplay = previousDisplay_LiveFeed;
      return;
    }
    // build display
    if (displayRefresh_Scroll){
      if (cursorPosition_Y != cursorPosition_Y_Last){ // changed to new record (index)
        cursorPosition_Y_Last = cursorPosition_Y; 
        int dataLen = 0;
        for(int i=0;i<sizeof(LiveFeedBuffer[cursorPosition_Y].data);i++){
          if (LiveFeedBuffer[cursorPosition_Y].data[i] != '\0'){
            dataLen++;
          } else {
            i = sizeof(LiveFeedBuffer[cursorPosition_Y].data); // get out
          }
        }
        ScrollingIndex_LiveFeed_minX = -10 * dataLen; // 10 = 5 pixels/character * text size 2
        ScrollingIndex_LiveFeed = display.width(); // starting point for text 
      }
      // clear the buffer
      display.clearDisplay();
      // add global objects to buffer
      handleDisplay_Global();
      if (!liveFeedIsEmpty){
        char src_dst[24] = {'\0'};
        byte index = 0;
        for (byte i=0;i<sizeof(LiveFeedBuffer[cursorPosition_Y].src)-1;i++){
          if (LiveFeedBuffer[cursorPosition_Y].src[i] != '\0'){
            src_dst[index] = LiveFeedBuffer[cursorPosition_Y].src[i];
            index++;
          } else {
            i = sizeof(LiveFeedBuffer[cursorPosition_Y].src); // get out
          }
        }
        src_dst[index] = '>'; index++;
        for (byte i=0;i<sizeof(LiveFeedBuffer[cursorPosition_Y].dst)-1;i++){
          if (LiveFeedBuffer[cursorPosition_Y].dst[i] != '\0'){
            src_dst[index] = LiveFeedBuffer[cursorPosition_Y].dst[i];
            index++;
          } else {
            i = sizeof(LiveFeedBuffer[cursorPosition_Y].dst); // get out
          }
        }
        display.setCursor(0,UI_DISPLAY_ROW_01);
        display.print(src_dst);

        //byte indexRow = 0;
        //if (cursorPosition_Y < 10) indexRow = 6;
        //display.setCursor(indexRow,UI_DISPLAY_ROW_02);
        //display.print(cursorPosition_Y);
        //display.setCursor(11,UI_DISPLAY_ROW_02);
        //display.print(F(":"));
        display.setCursor(ScrollingIndex_LiveFeed,UI_DISPLAY_ROW_02);
        display.setTextSize(2);                     // Normal 1:1 pixel scale - default letter size is 5x8 pixels
        display.print(LiveFeedBuffer[cursorPosition_Y].data); 
        display.setTextSize(1);                     // Normal 1:1 pixel scale - default letter size is 5x8 pixels
        if (keyboardInputChar == 13 || SETTINGS_DISPLAY_SCROLL_MESSAGES){ //  scroll only when enter pressed TODO: this wont work because key press not persistent
          ScrollingIndex_LiveFeed = ScrollingIndex_LiveFeed - SETTINGS_DISPLAY_SCROLL_SPEED; // higher number here is faster scroll but choppy
          if(ScrollingIndex_LiveFeed < ScrollingIndex_LiveFeed_minX) ScrollingIndex_LiveFeed = display.width(); // makeshift scroll because startScrollleft truncates the string!
        }
      } else {
        display.setCursor(0,UI_DISPLAY_ROW_02);
        display.print(F("Live feed is empty"));
        if (!leaveDisplay_LiveFeed) {
          leaveDisplay_LiveFeed = true;
          leave_display_timer_Livefeed = millis();
        }
      }
      // display all content from buffer
      display.display();
    }
    // timeout and leave
    if (millis() - leave_display_timer_Livefeed > SETTINGS_DISPLAY_TIMEOUT && leaveDisplay_LiveFeed){
      leaveDisplay_LiveFeed = false;
      currentDisplay = UI_DISPLAY_HOME;
      return;
    }
  }

  void handleDisplay_Settings_Save() {
    // on first show
    if (!displayInitialized_Settings_Save){
      // reset other displays
      displayInitialized_Home = false;
      displayInitialized_Messages = false;
      displayInitialized_LiveFeed = false;
      displayInitialized_Settings = false;
      displayInitialized_Settings_APRS = false;
      displayInitialized_Settings_GPS = false;
      displayInitialized_Settings_Display = false;
      displayInitialized_Settings_Save = true;
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      //Serial.print(F("Entered Settings Save:")); Serial.println(currentDisplay);
    }
    // handle button context for current display
    if (keyboardInputChar == -75){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=1;
      }
    }
    if (keyboardInputChar == -74){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y < 1){
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == 13){ // 13 DEC - Enter Key // 8 DEC - Backspace Key
      if (cursorPosition_Y == 1) {
        writeSettingsToEeprom();
        applySettings=true;
        saveModemSettings=true;
      }
      currentDisplay = UI_DISPLAY_SETTINGS;
    }
    if (keyboardInputChar == 27){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
      currentDisplay = UI_DISPLAY_SETTINGS;
      return;
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();
      
      int selectionRow = 0;
      switch (cursorPosition_Y) {
        case 0:
          selectionRow = UI_DISPLAY_ROW_02; // no
          break;
        case 1:
          selectionRow = UI_DISPLAY_ROW_03; // yes
          break;
        default:
          selectionRow = UI_DISPLAY_ROW_02;
          break;
      }
        
      display.setCursor(0,selectionRow);
      display.print(F(">"));
      
      display.setCursor(6,UI_DISPLAY_ROW_01);
      display.print(F("Save changes?"));
      
      display.setCursor(6,UI_DISPLAY_ROW_02);
      display.print(F("No"));
      
      display.setCursor(6,UI_DISPLAY_ROW_03);
      display.print(F("Yes"));

      // display all content from buffer
      display.display();
    }
  }

  void handleDisplay_Settings(){
    // on first show
    if (!displayInitialized_Settings){
      // reset other displays
      displayInitialized_Home = false;
      displayInitialized_Messages = false;
      displayInitialized_LiveFeed = false;
      displayInitialized_Settings = true;
      displayInitialized_Settings_APRS = false;
      displayInitialized_Settings_GPS = false;
      displayInitialized_Settings_Display = false;
      displayInitialized_Settings_Save = false;
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      //Serial.print(F("Entered Settings:")); Serial.println(currentDisplay);
    }
    // handle button context for current display
    if (keyboardInputChar == -75){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=ARRAY_SIZE(MenuItems_Settings) - 1;
      }
    }
    if (keyboardInputChar == -74){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y < ARRAY_SIZE(MenuItems_Settings) - 1){ // Size of array / size of array element
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == 13){ // 13 DEC - Enter Key // 8 DEC - Backspace Key
      switch (cursorPosition_Y) {
        case 0:
          currentDisplay = UI_DISPLAY_SETTINGS_APRS;
          previousDisplay_Settings_APRS = UI_DISPLAY_SETTINGS;
          break;
        case 1:
          currentDisplay = UI_DISPLAY_SETTINGS_GPS;
          previousDisplay_Settings_GPS = UI_DISPLAY_SETTINGS;
          break;
        case 2:
          currentDisplay = UI_DISPLAY_SETTINGS_DISPLAY;
          previousDisplay_Settings_Display = UI_DISPLAY_SETTINGS;
          break;
        default:
          currentDisplay = UI_DISPLAY_SETTINGS;
          break;
      }
    }
    if (keyboardInputChar == 27){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
      currentDisplay = previousDisplay_Settings;
      return;
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();
      
      int selectionRow = 0;
      switch (cursorPosition_Y) {
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
          selectionRow = UI_DISPLAY_ROW_BOTTOM;
          break;
      }
        
      display.setCursor(0,selectionRow);
      display.print(F(">"));
      
      display.setCursor(6,UI_DISPLAY_ROW_01);
      display.print(MenuItems_Settings[cursorPosition_Y>3 ? cursorPosition_Y-3 : 0]); // if greater than 3, follow else normal
      
      display.setCursor(6,UI_DISPLAY_ROW_02);
      display.print(MenuItems_Settings[cursorPosition_Y>3 ? cursorPosition_Y-2 : 1]);
      
      display.setCursor(6,UI_DISPLAY_ROW_03);
      display.print(MenuItems_Settings[cursorPosition_Y>3 ? cursorPosition_Y-1 : 2]);
      
      //display.setCursor(6,UI_DISPLAY_ROW_04);
      //display.print(MenuItems_Settings[cursorPosition_Y>3 ? cursorPosition_Y-0 : 3]);

      // display all content from buffer
      display.display();
    }
  }

  void handleDisplay_Settings_APRS(){
    // on first show
    if (!displayInitialized_Settings_APRS){
      // reset other displays
      displayInitialized_Home = false;
      displayInitialized_Messages = false;
      displayInitialized_LiveFeed = false;
      displayInitialized_Settings = false;
      displayInitialized_Settings_APRS = true;
      displayInitialized_Settings_GPS = false;
      displayInitialized_Settings_Display = false;
      displayInitialized_Settings_Save = false;
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      editMode_Settings_APRS = false;
      Settings_EditValueSize = 0;
      settingsChanged = false;
      //Settings_EditType = SETTINGS_EDIT_TYPE_NONE;
      //Serial.print(F("Entered APRS Settings:")); Serial.println(currentDisplay);
      for (int i=0; i<sizeof(Settings_TempDispCharArr);i++) {
        Settings_TempDispCharArr[i] = '\0';
      }
    }
    // monitor for changes
    if (editMode_Settings_APRS) {
      settingsChanged = true;
    }
    // handle button context for current display
    if ((keyboardInputChar >= 32 && keyboardInputChar <= 126) || (keyboardInputChar >= -76 && keyboardInputChar <= -73) || keyboardInputChar == 8) { // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (editMode_Settings_APRS){
        bool characterDelete = false;
        if (keyboardInputChar == -76) { // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
          if (cursorPosition_X > 0) { 
            cursorPosition_X--;
          } else {
            cursorPosition_X=Settings_EditValueSize;
          }
        }
        if (keyboardInputChar == -73) { // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
          if (cursorPosition_X < Settings_EditValueSize) { 
            cursorPosition_X++;
          } else {
            cursorPosition_X=0;
          }
        }
        if (keyboardInputChar == 8){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
          if (cursorPosition_X > 0) { 
            cursorPosition_X--;
            characterDelete = true;
          } else {

          }
        }
        /////////////// change values here ////////////////////
        //int chrCode = 0;
        switch (Settings_Type_APRS[cursorPosition_Y]) {
          case SETTINGS_EDIT_TYPE_BOOLEAN:
            if (keyboardInputChar == -74) {
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
              } else if (keyboardInputChar >= 48 && keyboardInputChar <= 57) {
                Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
              } else if (keyboardInputChar <= 45) { // 45 = '-'
                int tempInt = strtoul(Settings_TempDispCharArr[cursorPosition_X],NULL,10);
                tempInt = tempInt / -1;
                itoa(tempInt, Settings_TempDispCharArr[cursorPosition_X], 10);
              }
            break;
          case SETTINGS_EDIT_TYPE_UINT:
              if (characterDelete) {
                if (cursorPosition_X >= 0) {
                  Settings_TempDispCharArr[cursorPosition_X] = '\0';
                }
              } else if (keyboardInputChar >= 48 && keyboardInputChar <= 57) {
                Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
              }
            break;
          case SETTINGS_EDIT_TYPE_LONG:
              // TODO this is not validated because we have no settings of type long
              if (characterDelete) {
                if (cursorPosition_X >= 0) {
                  Settings_TempDispCharArr[cursorPosition_X] = '\0';
                }
              } else if (keyboardInputChar >= 48 && keyboardInputChar <= 57) {
                Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
              } else if (keyboardInputChar <= 45) { // 45 = '-'
                int tempInt = strtoul(Settings_TempDispCharArr[cursorPosition_X],NULL,10);
                tempInt = tempInt / -1;
                itoa(tempInt, Settings_TempDispCharArr[cursorPosition_X], 10);
              }
            break;
          case SETTINGS_EDIT_TYPE_ULONG:
              if (characterDelete) {
                if (cursorPosition_X >= 0) {
                  Settings_TempDispCharArr[cursorPosition_X] = '\0';
                }
              } else if (keyboardInputChar >= 48 && keyboardInputChar <= 57) {
                Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
              }
            break;
          case SETTINGS_EDIT_TYPE_FLOAT:
              // TODO this is not validated because we have no settings of type float in aprs settings
              if (characterDelete) {
                if (cursorPosition_X >= 0) {
                  Settings_TempDispCharArr[cursorPosition_X] = '\0';
                }
              } else if ((keyboardInputChar >= 48 && keyboardInputChar <= 57) || keyboardInputChar <= 46) { // 46 = '.'
                Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
              } else if (keyboardInputChar <= 45) { // 45 = '-'
                double tempDouble = strtod(Settings_TempDispCharArr[cursorPosition_X],NULL);
                tempDouble = tempDouble / -1.0;
                dtostrf(tempDouble,3,6,Settings_TempDispCharArr[cursorPosition_X]); // https://www.programmingelectronics.com/dtostrf/
              }
            break;
          case SETTINGS_EDIT_TYPE_STRING2:
            if (characterDelete) {
              if (cursorPosition_X >= 0) {
                Settings_TempDispCharArr[cursorPosition_X] = '\0';
              }
            } else if (keyboardInputChar >= 32 && keyboardInputChar <= 126) {
              Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
            }
            break;
          case SETTINGS_EDIT_TYPE_STRING7:
            if (characterDelete) {
              if (cursorPosition_X >= 0) {
                Settings_TempDispCharArr[cursorPosition_X] = '\0';
              }
            } else if (keyboardInputChar >= 32 && keyboardInputChar <= 126) {
              Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
            }
            break;
          case SETTINGS_EDIT_TYPE_STRING100:
            if (characterDelete) {
              if (cursorPosition_X >= 0) {
                Settings_TempDispCharArr[cursorPosition_X] = '\0';
              }
            } else if (keyboardInputChar >= 32 && keyboardInputChar <= 126) {
              Settings_TempDispCharArr[cursorPosition_X] = keyboardInputChar;
            }
            break;
          default:
            break;
        }
        characterDelete = false;
      } else if (keyboardInputChar == -75) { // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
        if (cursorPosition_Y > 0) { // Size of array / size of array element
          cursorPosition_Y--;
        } else {
          cursorPosition_Y=ARRAY_SIZE(MenuItems_Settings_APRS) - 1;
        }
      } else if (keyboardInputChar == -74) { // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
        if (cursorPosition_Y < ARRAY_SIZE(MenuItems_Settings_APRS) - 1) { // Size of array / size of array element
          cursorPosition_Y++;
        } else {
          cursorPosition_Y=0;
        }
      }
    }
    if (keyboardInputChar == 13){ // 13 DEC - Enter Key // 8 DEC - Backspace Key
      if (editMode_Settings_APRS) {
        editMode_Settings_APRS = false;
        cursorPosition_X = 0;
        // apply edited values
        switch (Settings_Type_APRS[cursorPosition_Y]) {
          case SETTINGS_EDIT_TYPE_BOOLEAN:
            if (Settings_TempDispCharArr[0] == 'T' || Settings_TempDispCharArr[0] == 't' || Settings_TempDispCharArr[0] == '1') {
              Settings_TypeBool[Settings_TypeIndex_APRS[cursorPosition_Y]] = 1;
            } else {
              Settings_TypeBool[Settings_TypeIndex_APRS[cursorPosition_Y]] = 0;
            }
            break;
          case SETTINGS_EDIT_TYPE_INT:
              Settings_TypeInt[Settings_TypeIndex_APRS[cursorPosition_Y]] = strtoul(Settings_TempDispCharArr,NULL,10);
            break;
          case SETTINGS_EDIT_TYPE_UINT:
              Settings_TypeUInt[Settings_TypeIndex_APRS[cursorPosition_Y]] = strtoul(Settings_TempDispCharArr,NULL,10);
            break;
          case SETTINGS_EDIT_TYPE_LONG:
              Settings_TypeLong[Settings_TypeIndex_APRS[cursorPosition_Y]] = strtoul(Settings_TempDispCharArr,NULL,10);
            break;
          case SETTINGS_EDIT_TYPE_ULONG:
              Settings_TypeULong[Settings_TypeIndex_APRS[cursorPosition_Y]] = strtoul(Settings_TempDispCharArr,NULL,10);
            break;
          case SETTINGS_EDIT_TYPE_FLOAT:
              Settings_TypeFloat[Settings_TypeIndex_APRS[cursorPosition_Y]] = atof(Settings_TempDispCharArr);
            break;
          case SETTINGS_EDIT_TYPE_STRING2:
            for (int i=0; i<sizeof(Settings_TypeString2[Settings_TypeIndex_APRS[cursorPosition_Y]]);i++) {
              Settings_TypeString2[Settings_TypeIndex_APRS[cursorPosition_Y]][i] = Settings_TempDispCharArr[i];       
            }
            break;
          case SETTINGS_EDIT_TYPE_STRING7:
            for (int i=0; i<sizeof(Settings_TypeString7[Settings_TypeIndex_APRS[cursorPosition_Y]]);i++) {
              Settings_TypeString7[Settings_TypeIndex_APRS[cursorPosition_Y]][i] = Settings_TempDispCharArr[i];       
            }
            break;
          case SETTINGS_EDIT_TYPE_STRING100:
            for (int i=0; i<sizeof(Settings_TypeString100[Settings_TypeIndex_APRS[cursorPosition_Y]]);i++) {
              Settings_TypeString100[Settings_TypeIndex_APRS[cursorPosition_Y]][i] = Settings_TempDispCharArr[i];       
            }
            break;
          default:
            break;
        }
      } else {
        // enable edit mode
        editMode_Settings_APRS = true;
        // clear the char array first
        for (int i=0; i<sizeof(Settings_TempDispCharArr);i++) {
          Settings_TempDispCharArr[i] = '\0';
        }
        // copy data to temp variable
        switch (Settings_Type_APRS[cursorPosition_Y]) {
          case SETTINGS_EDIT_TYPE_BOOLEAN:
            if (Settings_TypeBool[Settings_TypeIndex_APRS[cursorPosition_Y]]) {
              strcpy(Settings_TempDispCharArr, "True");
            } else {
              strcpy(Settings_TempDispCharArr, "False");
            }
            break;
          case SETTINGS_EDIT_TYPE_INT:
            itoa(Settings_TypeInt[Settings_TypeIndex_APRS[cursorPosition_Y]],Settings_TempDispCharArr,10);
            break;
          case SETTINGS_EDIT_TYPE_UINT:
            // TODO find a way to do this with an unsigned integer (dont use itoa)
            itoa(Settings_TypeUInt[Settings_TypeIndex_APRS[cursorPosition_Y]],Settings_TempDispCharArr,10);
            break;
          case SETTINGS_EDIT_TYPE_LONG:
            ltoa(Settings_TypeLong[Settings_TypeIndex_APRS[cursorPosition_Y]],Settings_TempDispCharArr,10);
            break;
          case SETTINGS_EDIT_TYPE_ULONG:
            ultoa(Settings_TypeULong[Settings_TypeIndex_APRS[cursorPosition_Y]],Settings_TempDispCharArr,10);
            break;
          case SETTINGS_EDIT_TYPE_FLOAT:
            break;
          case SETTINGS_EDIT_TYPE_STRING2:
            for (int i=0; i<strlen(Settings_TypeString2[Settings_TypeIndex_APRS[cursorPosition_Y]]);i++) {
              Settings_TempDispCharArr[i] = Settings_TypeString2[Settings_TypeIndex_APRS[cursorPosition_Y]][i];       
            }
            break;
          case SETTINGS_EDIT_TYPE_STRING7:
            for (int i=0; i<strlen(Settings_TypeString7[Settings_TypeIndex_APRS[cursorPosition_Y]]);i++) {
              Settings_TempDispCharArr[i] = Settings_TypeString7[Settings_TypeIndex_APRS[cursorPosition_Y]][i];       
            }
            break;
          case SETTINGS_EDIT_TYPE_STRING100:
            for (int i=0; i<strlen(Settings_TypeString100[Settings_TypeIndex_APRS[cursorPosition_Y]]);i++) {
              Settings_TempDispCharArr[i] = Settings_TypeString100[Settings_TypeIndex_APRS[cursorPosition_Y]][i];       
            }
            break;
          default:
            break;
        }
      }
    }
    if (keyboardInputChar == 27){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
      if (editMode_Settings_APRS) {
        // disable edit mode
        editMode_Settings_APRS = false;
        cursorPosition_X = 0;
      } else {
        if (settingsChanged) {
          currentDisplay = UI_DISPLAY_SETTINGS_SAVE;
          previousDisplay_Settings_Save = UI_DISPLAY_SETTINGS_APRS;
        } else {
          currentDisplay = previousDisplay_Settings_APRS;
          return;
        }
      }
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();
      if (editMode_Settings_APRS) {
        if (displayBlink) {
          display.setCursor(cursorPosition_X*6,UI_DISPLAY_ROW_BOTTOM);
          display.print('_');
        }
      }
      // place the cursor
      display.setCursor(0,UI_DISPLAY_ROW_BOTTOM-1);
      // print
      if (editMode_Settings_APRS) {
        Settings_EditValueSize = sizeof(Settings_TempDispCharArr) - 1;
        display.print(Settings_TempDispCharArr);
        cursorPosition_X = strlen(Settings_TempDispCharArr);
      } else {
        switch (Settings_Type_APRS[cursorPosition_Y]) {
          case SETTINGS_EDIT_TYPE_BOOLEAN:
            Settings_EditValueSize = 0;
            if (Settings_TypeBool[Settings_TypeIndex_APRS[cursorPosition_Y]]) {
              display.print(F("True"));
            } else {
              display.print(F("False"));
            }
            break;
          case SETTINGS_EDIT_TYPE_INT:
            Settings_EditValueSize = numberOfDigits<int>(Settings_TypeInt[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            display.print(Settings_TypeInt[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            break;
          case SETTINGS_EDIT_TYPE_UINT:
            Settings_EditValueSize = numberOfDigits<unsigned int>(Settings_TypeUInt[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            display.print(Settings_TypeUInt[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            break;
          case SETTINGS_EDIT_TYPE_LONG:
            Settings_EditValueSize = numberOfDigits<long>(Settings_TypeLong[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            display.print(Settings_TypeLong[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            break;
          case SETTINGS_EDIT_TYPE_ULONG:
            Settings_EditValueSize = numberOfDigits<unsigned long>(Settings_TypeULong[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            display.print(Settings_TypeULong[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            break;
          case SETTINGS_EDIT_TYPE_FLOAT:
            Settings_EditValueSize = numberOfDigits<float>(Settings_TypeFloat[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            display.print(Settings_TypeFloat[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            break;
          case SETTINGS_EDIT_TYPE_STRING2:
            Settings_EditValueSize = sizeof(Settings_TypeString2[Settings_TypeIndex_APRS[cursorPosition_Y]]) - 1;
            display.print(Settings_TypeString2[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            break;
          case SETTINGS_EDIT_TYPE_STRING7:
            Settings_EditValueSize = sizeof(Settings_TypeString7[Settings_TypeIndex_APRS[cursorPosition_Y]]) - 1;
            display.print(Settings_TypeString7[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            break;
          case SETTINGS_EDIT_TYPE_STRING100:
            Settings_EditValueSize = sizeof(Settings_TypeString100[Settings_TypeIndex_APRS[cursorPosition_Y]]) - 1;
            display.print(Settings_TypeString100[Settings_TypeIndex_APRS[cursorPosition_Y]]);
            break;
          default:
            break;
        }
      }
      
      int selectionRow = 0;
      switch (cursorPosition_Y) {
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
        
      display.setCursor(0,selectionRow);
      display.print(F(">"));
      
      display.setCursor(6,UI_DISPLAY_ROW_01);
      display.print(MenuItems_Settings_APRS[cursorPosition_Y>3 ? cursorPosition_Y-3 : 0]); // if greater than 3, follow else normal
      
      display.setCursor(6,UI_DISPLAY_ROW_02);
      display.print(MenuItems_Settings_APRS[cursorPosition_Y>3 ? cursorPosition_Y-2 : 1]);
      
      display.setCursor(6,UI_DISPLAY_ROW_03);
      display.print(MenuItems_Settings_APRS[cursorPosition_Y>3 ? cursorPosition_Y-1 : 2]);
      
      display.setCursor(6,UI_DISPLAY_ROW_04);
      display.print(MenuItems_Settings_APRS[cursorPosition_Y>3 ? cursorPosition_Y-0 : 3]);

      // display all content from buffer
      display.display();
    }
  }

  void handleDisplay_Settings_GPS(){
    // on first show
    if (!displayInitialized_Settings_GPS){
      // reset other displays
      displayInitialized_Home = false;
      displayInitialized_Messages = false;
      displayInitialized_LiveFeed = false;
      displayInitialized_Settings = false;
      displayInitialized_Settings_APRS = false;
      displayInitialized_Settings_GPS = true;
      displayInitialized_Settings_Display = false;
      displayInitialized_Settings_Save = false;
      cursorPosition_X = 0;
      cursorPosition_Y = 0;
      cursorPosition_X_Last = 0;
      cursorPosition_Y_Last = 0;
      settingsChanged = false;
      //Serial.print(F("Entered GPS Settings:")); Serial.println(currentDisplay);
    }
    // handle button context for current display
    if (keyboardInputChar == -75){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y > 0){
        cursorPosition_Y--;
      } else {
        cursorPosition_Y=ARRAY_SIZE(MenuItems_Settings_GPS) - 1;
      }
    }
    if (keyboardInputChar == -74){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
      if (cursorPosition_Y < ARRAY_SIZE(MenuItems_Settings_GPS) - 1){ // Size of array / size of array element
        cursorPosition_Y++;
      } else {
        cursorPosition_Y=0;
      }
    }
    if (keyboardInputChar == 13){ // 13 DEC - Enter Key // 8 DEC - Backspace Key
      ////////////////// handle modifying the setting here ////////////////
    }
    if (keyboardInputChar == 27){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
      currentDisplay = previousDisplay_Settings_GPS;
      return;
    }
    // build display
    if (displayRefresh_Global){
      // clear the buffer
      display.clearDisplay();
      
      int selectionRow = 0;
      switch (cursorPosition_Y) {
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
        
      display.setCursor(0,selectionRow);
      display.print(F(">"));
      
      display.setCursor(6,UI_DISPLAY_ROW_01);
      display.print(MenuItems_Settings_GPS[cursorPosition_Y>3 ? cursorPosition_Y-3 : 0]); // if greater than 3, follow else normal
      
      display.setCursor(6,UI_DISPLAY_ROW_02);
      display.print(MenuItems_Settings_GPS[cursorPosition_Y>3 ? cursorPosition_Y-2 : 1]);
      
      display.setCursor(6,UI_DISPLAY_ROW_03);
      display.print(MenuItems_Settings_GPS[cursorPosition_Y>3 ? cursorPosition_Y-1 : 2]);
      
      display.setCursor(6,UI_DISPLAY_ROW_04);
      display.print(MenuItems_Settings_GPS[cursorPosition_Y>3 ? cursorPosition_Y-0 : 3]);

      // display all content from buffer
      display.display();
    } 
  }

  void handleDisplay_Settings_Display(){
  // on first show
  if (!displayInitialized_Settings_Display){
    // reset other displays
    displayInitialized_Home = false;
    displayInitialized_Messages = false;
    displayInitialized_LiveFeed = false;
    displayInitialized_Settings = false;
    displayInitialized_Settings_APRS = false;
    displayInitialized_Settings_GPS = false;
    displayInitialized_Settings_Display = true;
    displayInitialized_Settings_Save = false;
    cursorPosition_X = 0;
    cursorPosition_Y = 0;
    cursorPosition_X_Last = 0;
    cursorPosition_Y_Last = 0;
    settingsChanged = false;
    //Serial.print(F("Entered Display Settings:")); Serial.println(currentDisplay);
  }
  // handle button context for current display
  if (keyboardInputChar == -75){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
    if (cursorPosition_Y > 0){
      cursorPosition_Y--;
    } else {
      cursorPosition_Y=ARRAY_SIZE(MenuItems_Settings_Display) - 1;
    }
  }
  if (keyboardInputChar == -74){ // -74 DEC - Down Key // -75 DEC - Up Key // -73 DEC - Right Key // -76 DEC - Left Key
    if (cursorPosition_Y < ARRAY_SIZE(MenuItems_Settings_Display) - 1){ // Size of array / size of array element
      cursorPosition_Y++;
    } else {
      cursorPosition_Y=0;
    }
  }
  if (keyboardInputChar == 13){ // 13 DEC - Enter Key // 8 DEC - Backspace Key
    ////////////////// handle modifying the setting here ////////////////
  }
  if (keyboardInputChar == 27){ // 13 DEC - Enter Key // 8 DEC - Backspace Key // 27 DEC - ESC Key
    currentDisplay = previousDisplay_Settings_Display;
    return;
  }
  // build display
  if (displayRefresh_Global){
    // clear the buffer
    display.clearDisplay();
    
    int selectionRow = 0;
    switch (cursorPosition_Y) {
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
      
    display.setCursor(0,selectionRow);
    display.print(F(">"));
    
    display.setCursor(6,UI_DISPLAY_ROW_01);
    display.print(MenuItems_Settings_Display[cursorPosition_Y>3 ? cursorPosition_Y-3 : 0]); // if greater than 3, follow else normal
    
    display.setCursor(6,UI_DISPLAY_ROW_02);
    display.print(MenuItems_Settings_Display[cursorPosition_Y>3 ? cursorPosition_Y-2 : 1]);
    
    display.setCursor(6,UI_DISPLAY_ROW_03);
    display.print(MenuItems_Settings_Display[cursorPosition_Y>3 ? cursorPosition_Y-1 : 2]);
    
    display.setCursor(6,UI_DISPLAY_ROW_04);
    display.print(MenuItems_Settings_Display[cursorPosition_Y>3 ? cursorPosition_Y-0 : 3]);

    // display all content from buffer
    display.display();
  } 
}
#pragma endregion

template <typename T>
T numberOfDigits(T number) {
  // https://studyfied.com/program/cpp-basic/count-number-of-digits-in-a-given-integer/
  // https://stackoverflow.com/questions/8627625/is-it-possible-to-make-function-that-will-accept-multiple-data-types-for-given-a/8627646

    int count = 0;
    while(number != 0) {
      count++;
      number /= 10;
  }
  return count;
}

void handleAprsBeacon(){
  if ( millis() - aprs_beacon_timer > SETTINGS_APRS_BEACON_FREQUENCY ){
    if (blnAprsAutomaticCommentEnabled==true){
      blnModemCmdFlag_Cmt=true;
    }
    aprs_beacon_timer = millis();
  }
}

void handleModemCommands(){
    // send raw packet
    if (blnModemCmdFlag_Raw==true){
      if (sendModemCommand("!", 1, SETTINGS_APRS_RAW_PACKET, strlen(SETTINGS_APRS_RAW_PACKET)) == -1){
        blnModemCmdFlag_Raw=false;
      }
      return;
    }
    // send location
    if (blnModemCmdFlag_Cmt==true){
      if (sendModemCommand("@", 1, SETTINGS_APRS_COMMENT, strlen(SETTINGS_APRS_COMMENT)) == -1){
        blnModemCmdFlag_Cmt=false;
      }
      return;
    }
    // send message
    if (blnModemCmdFlag_Msg==true){
      if (sendModemCommand("#", 1, SETTINGS_APRS_MESSAGE, strlen(SETTINGS_APRS_MESSAGE)) == -1){
        blnModemCmdFlag_Msg=false;
      }
      return;
    }
    // set callsign                           ****************Setting*******************
    if (blnModemCmdFlag_Setc==true){
      if (sendModemCommand("c", 1, SETTINGS_APRS_CALLSIGN, strlen(SETTINGS_APRS_CALLSIGN)) == -1){
        blnModemCmdFlag_Setc=false;
      }
      return;
    }
    // set destination callsign               ****************Setting*******************
    if (blnModemCmdFlag_Setd==true){
      if (sendModemCommand("d", 1, SETTINGS_APRS_DESTINATION_CALL, strlen(SETTINGS_APRS_DESTINATION_CALL)) == -1){
        blnModemCmdFlag_Setd=false;
      }
      return;
    }
    // set PATH1 callsign                     ****************Setting*******************
    if (blnModemCmdFlag_Set1==true){
      if (sendModemCommand("1", 1, SETTINGS_APRS_PATH1_CALL, strlen(SETTINGS_APRS_PATH1_CALL)) == -1){
        blnModemCmdFlag_Set1=false;
      }
      return;
    }
    // set PATH2 callsign                     ****************Setting*******************
    if (blnModemCmdFlag_Set2==true){
      if (sendModemCommand("2", 1, SETTINGS_APRS_PATH2_CALL, strlen(SETTINGS_APRS_PATH2_CALL)) == -1){
        blnModemCmdFlag_Set2=false;
      }
      return;
    }
    // set your SSID                          ****************Setting*******************
    if (blnModemCmdFlag_Setsc==true){
      if (sendModemCommand("sc", 2, SETTINGS_APRS_CALLSIGN_SSID, strlen(SETTINGS_APRS_CALLSIGN_SSID)) == -1){
        blnModemCmdFlag_Setsc=false;
      }
      return;
    }
    // set destination SSID                   ****************Setting*******************
    if (blnModemCmdFlag_Setsd==true){
      if (sendModemCommand("sd", 2, SETTINGS_APRS_DESTINATION_SSID, strlen(SETTINGS_APRS_DESTINATION_SSID)) == -1){
        blnModemCmdFlag_Setsd=false;
      }
      return;
    }
    // set PATH1 SSID                         ****************Setting*******************
    if (blnModemCmdFlag_Sets1==true){
      if (sendModemCommand("s1", 2, SETTINGS_APRS_PATH1_SSID, strlen(SETTINGS_APRS_PATH1_SSID)) == -1){
        blnModemCmdFlag_Sets1=false;
      }
      return;
    }    
    // set PATH2 SSID                         ****************Setting*******************
    if (blnModemCmdFlag_Sets2==true){
      if (sendModemCommand("s2", 2, SETTINGS_APRS_PATH2_SSID, strlen(SETTINGS_APRS_PATH2_SSID)) == -1){
        blnModemCmdFlag_Sets2=false;
      }
      return;
    }
    // set latitude
    if (blnModemCmdFlag_Lat==true){
      if (sendModemCommand("lla", 3, strCurrentLat, strlen(strCurrentLat)) == -1){
        blnModemCmdFlag_Lat=false;
      }
      return;
    }
    // set longitude
    if (blnModemCmdFlag_Lng==true){
      if (sendModemCommand("llo", 3, strCurrentLng, strlen(strCurrentLng)) == -1){
        blnModemCmdFlag_Lng=false;
      }
      return;
    }
    // set symbol                             ****************Setting*******************
    if (blnModemCmdFlag_Setls==true){
      if (sendModemCommand("ls", 2, SETTINGS_APRS_SYMBOL, strlen(SETTINGS_APRS_SYMBOL)) == -1){
        blnModemCmdFlag_Setls=false;
      }
      return;
    }
    // set symbol table                       ****************Setting*******************
    if (blnModemCmdFlag_Setlt==true){
      if (sendModemCommand("lt", 2, SETTINGS_APRS_SYMBOL_TABLE, strlen(SETTINGS_APRS_SYMBOL_TABLE)) == -1){
        blnModemCmdFlag_Setlt=false;
      }
      return;
    }
    // set message recipient
    if (blnModemCmdFlag_MsgRecipient==true){
      if (sendModemCommand("mc", 2, SETTINGS_APRS_RECIPIENT_CALL, strlen(SETTINGS_APRS_RECIPIENT_CALL)) == -1){
        blnModemCmdFlag_MsgRecipient=false;
      }
      return;
    }
    // set message recipient ssid
    if (blnModemCmdFlag_MsgRecipientSSID==true){
      if (sendModemCommand("ms", 2, SETTINGS_APRS_RECIPIENT_SSID, strlen(SETTINGS_APRS_RECIPIENT_SSID)) == -1){
        blnModemCmdFlag_MsgRecipientSSID=false;
      }
      return;
    }
    // retry last message
    if (blnModemCmdFlag_Setmr==true){
      if (sendModemCommand("mr", 2, SETTINGS_APRS_RECIPIENT_SSID, strlen(SETTINGS_APRS_RECIPIENT_SSID)) == -1){
        blnModemCmdFlag_Setmr=false;
      }
      return;
    }
    // automatic ACK on/off                   ****************Setting*******************
    if (blnModemCmdFlag_Setma==true){
      char OnOff[2] = {'\0'};
      if (SETTINGS_APRS_AUTOMATIC_ACK) {
        OnOff[0] = '1';
      } else {
        OnOff[0] = '0';
      }
      if (sendModemCommand("ma", 2, OnOff, strlen(OnOff)) == -1){
        blnModemCmdFlag_Setma=false;
      }
      return;
    }
    // set preamble in ms                     ****************Setting*******************
    if (blnModemCmdFlag_Setw==true){
      //char *  itoa ( int value, char * str, int base );
      char timeBuffer[17];
      itoa(SETTINGS_APRS_PREAMBLE,timeBuffer,10);
      if (sendModemCommand("w", 1, timeBuffer, strlen(timeBuffer)) == -1){
        blnModemCmdFlag_Setw=false;
      }
      return;
    }
    // set tail in ms                         ****************Setting*******************
    if (blnModemCmdFlag_SetW==true){
      //char *  itoa ( int value, char * str, int base );
      char timeBuffer[17];
      itoa(SETTINGS_APRS_TAIL,timeBuffer,10);
      if (sendModemCommand("W", 1, timeBuffer, strlen(timeBuffer)) == -1){
        blnModemCmdFlag_SetW=false;
      }
      return;
    }
    // save settings                          ****************Setting*******************
    if (blnModemCmdFlag_SetS==true){
      if (sendModemCommand("S", 1, "", 0) == -1){
        blnModemCmdFlag_SetS=false;
      }
      return;
    } 
    // clear configuration                    ****************Setting*******************
    if (blnModemCmdFlag_SetC==true){
      if (sendModemCommand("C", 1, "", 0) == -1){
        blnModemCmdFlag_SetC=false;
      }
      return;
    }    
    // print configuration
    if (blnModemCmdFlag_SetH==true){
      if (sendModemCommand("H", 1, "", 0) == -1){
        blnModemCmdFlag_SetH=false;
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
  char modemData[500]; // this should be smaller. what is max APRS message length?
  if (Serial1.available()){
    memset(modemData,'\0',sizeof(modemData));
    Serial1.readBytesUntil('\n', modemData, sizeof(modemData));
    Serial.print(modemData);
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
        Serial.println();
        Serial.print(F("SRC=")); Serial.println(Format_Raw_In.src);
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
        Serial.print(F("DST=")); Serial.println(Format_Raw_In.dst);
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
        Serial.print(F("PATH=")); Serial.println(Format_Raw_In.path);
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
        Serial.print(F("DATA=")); Serial.println(Format_Raw_In.data);
      }
      if (!foundSrc && !foundDst && !foundPath && !foundData) {
        //Serial.println(F("    !APRS"));
        Serial.println();
        i = sizeof(modemData); // setting i to size of modem data will make loop exit sooner
      }
      if (foundSrc && foundDst && foundPath && foundData) {
        // handle the live feed index circular buffer
        if (liveFeedBufferIndex < LIVEFEED_BUFFER_SIZE - 1){
          liveFeedBufferIndex++;
          if (liveFeedBufferIndex_RecordCount == 0) {
            liveFeedBufferIndex_RecordCount = 1;
          } else if (liveFeedBufferIndex_RecordCount <= liveFeedBufferIndex) {
            liveFeedBufferIndex_RecordCount = liveFeedBufferIndex + 1; // get total number of records stored
          }
        } else {
          liveFeedBufferIndex=0;
        }
        // write data to live feed
        LiveFeedBuffer[liveFeedBufferIndex] = Format_Raw_In;
        // let other systems know there is data in the live feed
        liveFeedIsEmpty = false;
        
        // write live feed to SD card here
        
        //Serial.print(F("Total APRS Message Length="));Serial.println(i-1);
        i = sizeof(modemData); // setting i to size of modem data will make loop exit sooner
      }
    }
    
    // check if message here
    // Radio 2: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :Hi!{006
    // Radio 1: SRC: [NOCALL-3] DST: [APRS-0] PATH: [WIDE1-1] [WIDE2-2] DATA: :NOCALL-3 :ack006
    if (Format_Raw_In.data[0] == ':') {
      APRSFormat_Msg Format_Msg_In;
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
      Serial.println("Received a message!");
      Serial.print("To=");Serial.println(Format_Msg_In.to);
      Serial.print("From=");Serial.println(Format_Msg_In.from);
      Serial.print("Msg=");Serial.println(Format_Msg_In.msg);
      Serial.print("Line=");Serial.println(Format_Msg_In.line);
      Serial.print("Ack=");Serial.println(Format_Msg_In.ack);
      // if message to users callsign add to feed
      // SETTINGS_APRS_CALLSIGN      callsign
      // SETTINGS_APRS_CALLSIGN_SSID      callsign ssid
      if (strstr(Format_Msg_In.to, SETTINGS_APRS_CALLSIGN) != NULL) {
        // if message is an acknowledge don't add.
        if (!Format_Msg_In.ack) {
          // handle the incoming message index circular buffer
          if (incomingMessageBufferIndex < INCOMING_MESSAGE_BUFFER_SIZE - 1){
            incomingMessageBufferIndex++;
            if (incomingMessageBufferIndex_RecordCount == 0) {
              incomingMessageBufferIndex_RecordCount = 1;
            } else if (incomingMessageBufferIndex_RecordCount <= incomingMessageBufferIndex) {
              incomingMessageBufferIndex_RecordCount = incomingMessageBufferIndex + 1; // get total number of records stored
            }
          } else {
            incomingMessageBufferIndex=0;
          }
          // add message to feed
          IncomingMessageBuffer[incomingMessageBufferIndex] = Format_Msg_In;
          // let other systems know there is data in the live feed
          messageFeedIsEmpty = false;
          // switch to message display
          currentDisplay = UI_DISPLAY_MESSAGES;
        } else {
          // do something here if message is an acknowledge
        }
      }
    }
  }
}

void readGPS(){
/*
// degrees minutes seconds (dms) to decimal degrees (dd)
// DD = d + (min/60) + (sec/3600)
// if lat positive (+) then north (N) else south (S)
// if lon positive (+) then east (E) else west (W)

// decimal degrees (dd) to degrees minutes seconds (dms)
// d = int(DD)
// m = int((DD-d) * 60)
// s = (DD-d-m/60) * 3600

float long = 45.124783;
int deglong = long;
long -= deglong; // remove the degrees from the calculation
long *= 60; // convert to minutes
int minlong = long;
long -= minlong; // remove the minuts from the calculation
long *= 60; // convert to seconds
*/
  //// For one second we parse GPS data and report some key values
  //for (unsigned long start = millis(); millis() - start < 1000;)
  //{
      while (Serial2.available() > 0)
        //Serial.write(Serial2.read()); // uncomment this line if you want to see the GPS data flowing
        gps.encode(Serial2.read());
  //}
      
  if ( millis() - gps_report_timer > SETTINGS_GPS_UPDATE_FREQUENCY){
      // reset timer
      gps_report_timer = millis();
      if (gps.location.isUpdated())
      {
        //Serial.print(F("LOCATION   Fix Age="));
        //Serial.print(gps.location.age());
        //Serial.print(F("ms Raw Lat="));
        //Serial.print(gps.location.rawLat().negative ? "-" : "+");
        //Serial.print(gps.location.rawLat().deg);
        //Serial.print("[+");
        //Serial.print(gps.location.rawLat().billionths);
        //Serial.print(F(" billionths],  Raw Long="));
        //Serial.print(gps.location.rawLng().negative ? "-" : "+");
        //Serial.print(gps.location.rawLng().deg);
        //Serial.print("[+");
        //Serial.print(gps.location.rawLng().billionths);
        //Serial.print(F(" billionths],  Lat="));
        //Serial.print(F(" Lat="));
        //Serial.print(gps.location.lat(), 6);
        //Serial.print(F(" Long="));
        //Serial.print(gps.location.lng(), 6);
        
        // DDMM.MM http://ember2ash.com/lat.htm
        //Serial.print(F(" Lat2="));
        fltCurrentLatDeg = gps.location.rawLat().deg*100.0 + gps.location.rawLat().billionths*0.000000001*60.0;
        dtostrf(fltCurrentLatDeg, 8, 3, strCurrentLat);
        strCurrentLat[7] = gps.location.rawLat().negative ? 'S' : 'N';
        for (byte i = 0; i < sizeof(strCurrentLat) - 1; i++) {
          if (strCurrentLat[i] == ' ') strCurrentLat[i] = '0';
        }
        if (fltCurrentLatDeg > fltLastLatDeg + fltLastLatDeg*SETTINGS_GPS_POSITION_TOLERANCE  || fltCurrentLatDeg < fltLastLatDeg - fltLastLatDeg*SETTINGS_GPS_POSITION_TOLERANCE) {
          blnModemCmdFlag_Lat=true;
          fltLastLatDeg = fltCurrentLatDeg;
        }
        
        //Serial.print(F(" Long2="));
        fltCurrentLngDeg = gps.location.rawLng().deg*100.0 + gps.location.rawLng().billionths*0.000000001*60.0;
        dtostrf(fltCurrentLngDeg, 9, 3, strCurrentLng);
        strCurrentLng[8] = gps.location.rawLng().negative ? 'W' : 'E';
        for (byte i = 0; i < sizeof(strCurrentLng) - 1; i++) {
          if (strCurrentLng[i] == ' ') strCurrentLng[i] = '0';
        }
        if (fltCurrentLngDeg > fltLastLngDeg + fltLastLngDeg*SETTINGS_GPS_POSITION_TOLERANCE || fltCurrentLngDeg < fltLastLngDeg - fltLastLngDeg*SETTINGS_GPS_POSITION_TOLERANCE) {
          blnModemCmdFlag_Lng=true;
          fltLastLngDeg = fltCurrentLngDeg;
        }
        /*
        Serial.println();
        Serial.print("Comment=");
        byte j = 0;
        for (byte i = 0; i < sizeof(strAprsComment) - 1; i++) {
          if (strAprsComment[i] != 0){
            Serial.print(strAprsComment[i]);
            j++;
          }
        }
        Serial.print(" Size=");
        Serial.print(j);
        */
        //Serial.println();
        //Serial.print("CommentLength=");
        //Serial.println(stringLength(strAprsComment,sizeof(strAprsComment)));
        
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

void handleVoltage(){
  if (millis() - voltage_check_timer > VOLTAGE_CHECK_RATE){
    Voltage = readVcc();
    VoltagePercent = constrain(map(Voltage,(int)BATT_DISCHARGED,(int)BATT_CHARGED,0,100),0,100);
    //Serial.print("Voltage: "); Serial.print(Voltage); Serial.println("mv");
    //Serial.print("Voltage: "); Serial.print(VoltagePercent); Serial.println("%");
    voltage_check_timer = millis();
  }
}

void handleSettings() {  
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
      blnModemCmdFlag_Setc=true;  // callsign
      applySettings_Seq++;
      break;
    case 3:
      if(!blnModemCmdFlag_Setc){
        applySettings_Seq++;
      }
      break;
    case 4:
      blnModemCmdFlag_Setd=true;  // destination
      applySettings_Seq++;
      break;
    case 5:
      if(!blnModemCmdFlag_Setd){
        applySettings_Seq++;
      }
      break;
    case 6:
      blnModemCmdFlag_Set1=true;  // path1
      applySettings_Seq++;
      break;
    case 7:
      if(!blnModemCmdFlag_Set1){
        applySettings_Seq++;
      }
      break;
    case 8:
      blnModemCmdFlag_Set2=true;  // path2
      applySettings_Seq++;
      break;
    case 9:
      if(!blnModemCmdFlag_Set2){
        applySettings_Seq++;
      }
      break;
    case 10:
      blnModemCmdFlag_Setsc=true; // callsign ssid
      applySettings_Seq++;
      break;
    case 11:
      if(!blnModemCmdFlag_Setsc){
        applySettings_Seq++;
      }
      break;
    case 12:
      blnModemCmdFlag_Setsd=true; // destination ssid
      applySettings_Seq++;
      break;
    case 13:
      if(!blnModemCmdFlag_Setsd){
        applySettings_Seq++;
      }
      break;
    case 14:
      blnModemCmdFlag_Sets1=true; // path1 ssid
      applySettings_Seq++;
      break;
    case 15:
      if(!blnModemCmdFlag_Sets1){
        applySettings_Seq++;
      }
      break;
    case 16:
      blnModemCmdFlag_Sets2=true; // path2 ssid
      applySettings_Seq++;
      break;
    case 17:
      if(!blnModemCmdFlag_Sets2){
        applySettings_Seq++;
      }
      break;
    case 18:
      blnModemCmdFlag_Setls=true; // symbol
      applySettings_Seq++;
      break;
    case 19:
      if(!blnModemCmdFlag_Setls){
        applySettings_Seq++;
      }
      break;
    case 20:
      blnModemCmdFlag_Setlt=true; // table
      applySettings_Seq++;
      break;
    case 21:
      if(!blnModemCmdFlag_Setlt){
        applySettings_Seq++;
      }
      break;
    case 22:
      blnModemCmdFlag_Setma=true; // auto ack on/off
      applySettings_Seq++;
      break;
    case 23:
      if(!blnModemCmdFlag_Setma){
        applySettings_Seq++;
      }
      break;
    case 24:
      blnModemCmdFlag_Setw=true;  // preamble
      applySettings_Seq++;
      break;
    case 25:
      if(!blnModemCmdFlag_Setw){
        applySettings_Seq++;
      }
      break;
    case 26:
      blnModemCmdFlag_SetW=true;  // tail
      applySettings_Seq++;
      break;
    case 27:
      if(!blnModemCmdFlag_SetW){
        applySettings_Seq++;
      }
      break;
    case 28:
      if (saveModemSettings) {
        saveModemSettings=false;
        blnModemCmdFlag_SetS==true; // save the configuration
        applySettings_Seq++;
      } else {
        applySettings_Seq=0;
      }
      break;
    case 29:
      if(!blnModemCmdFlag_SetS){
        applySettings_Seq=0;
      }
      break;
  }
}

void getDataTypeExample(int dataType, char* outExample){

  const char chrNone[] = {":<Unknown>"};
  const char chrBool[] = {":<True/False>"};
  const char chrInt[] = {":<-32,768 to 32,767>"};
  const char chrUInt[] = {":<0 to 65,535>"};
  const char chrLong[] = {":<-2,147,483,648 to 2,147,483,647>"};
  const char chrULong[] = {":<0 to 4,294,967,295>"};
  const char chrFloat[] = {":<-3.4028235E+38 to 3.4028235E+38>"};
  const char chrString2[] = {":<Alpha numeric up to 1 character max>"};
  const char chrString7[] = {":<Alpha numeric up to 6 characters max>"};
  const char chrString100[] = {":<Alpha numeric up to 99 characters max>"};
  
  switch (dataType) {
    case (int)SETTINGS_EDIT_TYPE_NONE: {
      for (int i=0;i<ARRAY_SIZE(chrNone);i++) outExample[i] = chrNone[i];
      break;}
    case (int)SETTINGS_EDIT_TYPE_BOOLEAN:
      for (int i=0;i<ARRAY_SIZE(chrBool);i++) outExample[i] = chrBool[i];
      break;
    case (int)SETTINGS_EDIT_TYPE_INT:
      for (int i=0;i<ARRAY_SIZE(chrInt);i++) outExample[i] = chrInt[i];
      break;
    case (int)SETTINGS_EDIT_TYPE_UINT:
      for (int i=0;i<ARRAY_SIZE(chrUInt);i++) outExample[i] = chrUInt[i];
      break;
    case (int)SETTINGS_EDIT_TYPE_LONG:
      for (int i=0;i<ARRAY_SIZE(chrLong);i++) outExample[i] = chrLong[i];
      break;
    case (int)SETTINGS_EDIT_TYPE_ULONG:
      for (int i=0;i<ARRAY_SIZE(chrULong);i++) outExample[i] = chrULong[i];
      break;
    case (int)SETTINGS_EDIT_TYPE_FLOAT:
      for (int i=0;i<ARRAY_SIZE(chrFloat);i++) outExample[i] = chrFloat[i];
      break;
    case (int)SETTINGS_EDIT_TYPE_STRING2:
      for (int i=0;i<ARRAY_SIZE(chrString2);i++) outExample[i] = chrString2[i];
      break;
    case (int)SETTINGS_EDIT_TYPE_STRING7:
      for (int i=0;i<ARRAY_SIZE(chrString7);i++) outExample[i] = chrString7[i];
      break;
    case (int)SETTINGS_EDIT_TYPE_STRING100:
      for (int i=0;i<ARRAY_SIZE(chrString100);i++) outExample[i] = chrString100[i];
      break;
    default:
      break;
  }
}

void printOutSerialCommands() {

  // CMD:Modem:cNOCALL
  // CMD:Settings:Save:
  // CMD:Settings:Print:
  // CMD:Settings:APRS:Beacon Frequency:300000
  // CMD:Settings:APRS:Raw Packet:<raw data here>
  // CMD:Settings:APRS:Comment:Testing HamMessenger!
  // CMD:Settings:APRS:Message:Hi!

/*
# define SETTINGS_EDIT_TYPE_NONE        0
# define SETTINGS_EDIT_TYPE_BOOLEAN     1
# define SETTINGS_EDIT_TYPE_INT         2
# define SETTINGS_EDIT_TYPE_UINT        3
# define SETTINGS_EDIT_TYPE_LONG        4
# define SETTINGS_EDIT_TYPE_ULONG       5
# define SETTINGS_EDIT_TYPE_FLOAT       6
# define SETTINGS_EDIT_TYPE_STRING2     7
# define SETTINGS_EDIT_TYPE_STRING7     8
# define SETTINGS_EDIT_TYPE_STRING100   9
                        
const char *MenuItems_Settings[] = {"APRS","GPS","Display"};
const char *MenuItems_Settings_APRS[] = {"Beacon Frequency","Raw Packet","Comment","Message","Recipient Callsign","Recipient SSID", "My Callsign","Callsign SSID", 
                                        "Destination Callsign", "Destination SSID", "PATH1 Callsign", "PATH1 SSID", "PATH2 Callsign", "PATH2 SSID",
                                        "Symbol", "Table", "Automatic ACK", "Preamble", "Tail"};
const char *MenuItems_Settings_GPS[] = {"Update Frequency","Position Tolerance","Destination Latitude","Destination Longitude"};
const char *MenuItems_Settings_Display[] = {"Timeout", "Brightness", "Show Position", "Scroll Messages", "Scroll Speed"};

int Settings_Type_APRS[] = {SETTINGS_EDIT_TYPE_ULONG,SETTINGS_EDIT_TYPE_STRING100,SETTINGS_EDIT_TYPE_STRING100,SETTINGS_EDIT_TYPE_STRING100,SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,
                            SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_STRING7,SETTINGS_EDIT_TYPE_STRING2,
                            SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_STRING2,SETTINGS_EDIT_TYPE_BOOLEAN,SETTINGS_EDIT_TYPE_UINT,SETTINGS_EDIT_TYPE_UINT};
int Settings_Type_GPS[] = {SETTINGS_EDIT_TYPE_ULONG,SETTINGS_EDIT_TYPE_FLOAT,SETTINGS_EDIT_TYPE_FLOAT,SETTINGS_EDIT_TYPE_FLOAT};
int Settings_Type_Display[] = {SETTINGS_EDIT_TYPE_ULONG, SETTINGS_EDIT_TYPE_UINT, SETTINGS_EDIT_TYPE_BOOLEAN, SETTINGS_EDIT_TYPE_BOOLEAN, SETTINGS_EDIT_TYPE_UINT};
*/

  Serial.println();
  Serial.println("CMD:Settings:Print:");
  Serial.println("CMD:Settings:Save:");
  Serial.println("CMD:Modem:<command>");
  Serial.println();
  for (int i=0;i<ARRAY_SIZE(MenuItems_Settings);i++) {
    switch (i) {
      case 0: // APRS
        for (int j=0;j<ARRAY_SIZE(MenuItems_Settings_APRS);j++) {
          char example[50] = {'\0'};
          getDataTypeExample(Settings_Type_APRS[j],example);
          Serial.print("CMD:Settings:");Serial.print(MenuItems_Settings[i]);Serial.print(":");Serial.print(MenuItems_Settings_APRS[j]);Serial.println(example);
        }
        break;
        
      case 1: // GPS
        Serial.println();
        for (int k=0;k<ARRAY_SIZE(MenuItems_Settings_GPS);k++) {
          char example[50] = {'\0'};
          getDataTypeExample(Settings_Type_GPS[k],example);
          Serial.print("CMD:Settings:");Serial.print(MenuItems_Settings[i]);Serial.print(":");Serial.print(MenuItems_Settings_GPS[k]);Serial.println(example);
        }
        break;
        
      case 2: // Display
        Serial.println();
        for (int l=0;l<ARRAY_SIZE(MenuItems_Settings_Display);l++) {
          char example[50] = {'\0'};
          getDataTypeExample(Settings_Type_Display[l],example);
          Serial.print("CMD:Settings:");Serial.print(MenuItems_Settings[i]);Serial.print(":");Serial.print(MenuItems_Settings_Display[l]);Serial.println(example);
        }
        break;
        
      default:
        break;
        
    }
  }
}

void handleSerial() {
  // the gps module continuously prints to the cpu
  readGPS();
  // when the modem processes a message or responds to a command it will print to the cpu
  readModem();
  // usb serial commands are handled here
  bool gotCMD = false;
  char inData[500];
  if (Serial.available()) {
    memset(inData,'\0',sizeof(inData));
    Serial.readBytesUntil('\n', inData, sizeof(inData));
    Serial.print(F("ECHO  "));Serial.println(inData);
    if (inData[0]=='C' && inData[1]=='M' && inData[2]=='D' && inData[3]==':') gotCMD=true;
    if (inData[0]=='?') printOutSerialCommands();
  }
  if (gotCMD){
    //Serial.println("Got cmd");
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
      if (strstr(SettingGroup, "APRS") != NULL) {
        // get the setting
        char Setting[30]={'\0'};
        int j_s_APRS = 0;
        while (inData[i] != ':') {
          if (inData[i] == '\0' || inData[i] == '\n') return;
          Setting[j_s_APRS] = inData[i];
          i++; j_s_APRS++;
        }
        i++; // i should be sitting at the ':'. go ahead and skip that.
        if (strstr(Setting, MenuItems_Settings_APRS[0]) != NULL) { // "Beacon Frequency"
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
          SETTINGS_APRS_BEACON_FREQUENCY = atol(inData_Value);
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[1]) != NULL) { // "Raw Packet"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          memset(SETTINGS_APRS_RAW_PACKET,'\0',sizeof(SETTINGS_APRS_RAW_PACKET)); 
          for (int i=0; inData_Value[i]!='\0';i++) {
             SETTINGS_APRS_RAW_PACKET[i] = inData_Value[i];
          }
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[2]) != NULL) { // "Comment"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          memset(SETTINGS_APRS_COMMENT,'\0',sizeof(SETTINGS_APRS_COMMENT)); 
          for (int i=0; inData_Value[i]!='\0';i++) {
             SETTINGS_APRS_COMMENT[i] = inData_Value[i];
          }
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[3]) != NULL) { // "Message"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          memset(SETTINGS_APRS_MESSAGE,'\0',sizeof(SETTINGS_APRS_MESSAGE)); 
          for (int i=0; inData_Value[i]!='\0';i++) {
             SETTINGS_APRS_MESSAGE[i] = inData_Value[i];
          }
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[4]) != NULL) { // "Recipient Callsign"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          memset(SETTINGS_APRS_RECIPIENT_CALL,'\0',sizeof(SETTINGS_APRS_RECIPIENT_CALL)); 
          for (int i=0; inData_Value[i]!='\0';i++) {
             SETTINGS_APRS_RECIPIENT_CALL[i] = inData_Value[i]; 
          }
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[5]) != NULL) { // "Recipient SSID"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          SETTINGS_APRS_RECIPIENT_SSID[0] = inData_Value[0];
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[6]) != NULL) { // "My Callsign"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          memset(SETTINGS_APRS_CALLSIGN,'\0',sizeof(SETTINGS_APRS_CALLSIGN)); 
          for (int i=0; inData_Value[i]!='\0';i++) {
             SETTINGS_APRS_CALLSIGN[i] = inData_Value[i]; 
          }
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[7]) != NULL) { // "Callsign SSID"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          SETTINGS_APRS_CALLSIGN_SSID[0] = inData_Value[0];
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[8]) != NULL) { // "Destination Callsign"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          memset(SETTINGS_APRS_DESTINATION_CALL,'\0',sizeof(SETTINGS_APRS_DESTINATION_CALL)); 
          for (int i=0; inData_Value[i]!='\0';i++) {
             SETTINGS_APRS_DESTINATION_CALL[i] = inData_Value[i]; 
          }
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[9]) != NULL) { // "Destination SSID"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          SETTINGS_APRS_DESTINATION_SSID[0] = inData_Value[0];
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[10]) != NULL) { // "PATH1 Callsign"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          memset(SETTINGS_APRS_PATH1_CALL,'\0',sizeof(SETTINGS_APRS_PATH1_CALL)); 
          for (int i=0; inData_Value[i]!='\0';i++) {
             SETTINGS_APRS_PATH1_CALL[i] = inData_Value[i]; 
          }
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[11]) != NULL) { // "PATH1 SSID"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          SETTINGS_APRS_PATH1_SSID[0] = inData_Value[0];
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[12]) != NULL) { // "PATH2 Callsign"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          memset(SETTINGS_APRS_PATH2_CALL,'\0',sizeof(SETTINGS_APRS_PATH2_CALL)); 
          for (int i=0; inData_Value[i]!='\0';i++) {
             SETTINGS_APRS_PATH2_CALL[i] = inData_Value[i]; 
          }
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[13]) != NULL) { // "PATH2 SSID"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          SETTINGS_APRS_PATH2_SSID[0] = inData_Value[0];
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[14]) != NULL) { // "Symbol"
          while (inData[i] != '\n' && inData[i] != '\0') {
            inData_Value[k] = inData[i];
            i++; k++;
          }
          //Serial.print(DataEntered);Serial.println(inData_Value); 
          SETTINGS_APRS_SYMBOL[0] = inData_Value[0];
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[15]) != NULL) { // "Table"
          while (inData[i] != '\n' && inData[i] != '\0') {
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
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[16]) != NULL) { // "Automatic ACK"
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
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[17]) != NULL) { // "Preamble"
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
          
        } else if (strstr(Setting, MenuItems_Settings_APRS[18]) != NULL) { // "Tail"
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
          
        } else {
          Serial.println(InvalidCommand);
        }
      } else if (strstr(SettingGroup, "GPS") != NULL) {
        // get the setting
        char Setting[30]={'\0'};
        int j_s_GPS = 0;
        while (inData[i] != ':') {
          Setting[j_s_GPS] = inData[i];
          i++; j_s_GPS++;
        }
        i++; // i should be sitting at the ':'. go ahead and skip that.
        if (strstr(Setting, "Update Frequency") != NULL) {
          
        } else if (strstr(Setting, "Position Tolerance") != NULL) {
          
        } else if (strstr(Setting, "Destination Latitude") != NULL) {
          
        } else if (strstr(Setting, "Destination Longitude") != NULL) {
          
        } else {
          Serial.println(InvalidCommand);
        }
      } else if (strstr(SettingGroup, "Display") != NULL) {
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
        } else {
          Serial.println(InvalidCommand);
        }
        
      } else if (strstr(SettingGroup, "Save") != NULL) { // CMD:Settings:Save:
        writeSettingsToEeprom();
        applySettings=true;
        saveModemSettings=true;
        printOutSettings();
        
      } else if (strstr(SettingGroup, "Print") != NULL) { //CMD:Settings:Print:
        printOutSettings();
        
      } else {
        Serial.println(InvalidCommand);
      }
    } else {
      Serial.println(InvalidCommand);
    }
  }
}

void handleKeyboard() {
  keyboardInputChar = 0;
  Wire.requestFrom(CARDKB_ADDR, 1);
  while(Wire.available())
  {
    char c = Wire.read(); // receive a byte as characterif
    if (c != 0)
    {
      keyboardInputChar = c;
      keyboardInputCharLast = keyboardInputChar;
    }
  }
}

void setup(){
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial1.begin(9600); // modem
  Serial2.begin(9600); // gps
  //while (!Serial) // wait for serial port to connect. Needed for Native USB only
  while (!Serial1) // wait for modem
  
  Wire.begin(); // M5Stack Keyboard
  
  delay(2000);
  
  // check if this is a new device
  checkInit();

  // read in the settings from the eeprom
  readSettingsFromEeprom();
  applySettings=true;
  //saveModemSettings=true; // comment out so that the modem doesnt write to its eeprom on every boot!
  printOutSettings();

  // initialize SD card controller
  if (!SD.begin(53)) {
    Serial.println(F("Could not connect to SD card controller. Halting."));
    while (1);
  }
  
  /*
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print(F("Writing to test.txt..."));
    myFile.println(F("testing 1, 2, 3."));
    // close the file:
    myFile.close();
    Serial.println(F("done."));
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening test.txt"));
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println(F("test.txt:"));

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening test.txt"));
  }
*/

  // inputs
  pinMode(rxPin, INPUT);
  pinMode(txPin, INPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  //if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
  //  Serial.println(F("SSD1306 allocation failed"));
  //  //for(;;); // Don't proceed, loop forever
  //}
  
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64), 0x3C (for the 128x32)

  handleDisplay_Startup();
  
  // start gps_read_timer
  gps_report_timer = millis();

}

void loop(){
  // run handle routines
  handleKeyboard();
  handleSettings();
  handleDisplays();
  handleSerial();
  handleModemCommands();
  handleAprsBeacon();
  handleVoltage();
  // calculate cpu scan time
  scanTime = millis() - processor_scan_time;
  processor_scan_time = millis();
}
