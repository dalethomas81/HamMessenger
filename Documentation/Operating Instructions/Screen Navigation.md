## Screen Navigation ##

-The **[Up]** and **[Down]** keys move the cursor through the selections.\
-Pressing the **[Enter]** key will navigate to the screen selected by the cursor.\
-To exit a screen, press the **[Escape]** key.
#### Editing a Setting ####
-Select the setting using the **[Up]** and **[Down]** keys.\
-Press the **[Enter]** key. This will put the device in edit mode.\
-Use the **[Up]** and **[Down]** keys to modify a boolean value.\
-Use the **[-]** key to change the sign of a float value.\
-Use the **[Backspace]** key to delete alphanumeric values and enter new values.\
-Press the **[Enter]** key to accept a new value or press the **[Escape]** key to revert to the original value.

```text
  SPLASH
┌─────────────────────────┐
│      HamMessenger       │
│                         │
│         Build           │
│ Month Day Year HH:mm:ss │
│                         │
│       Call:NOCALL       │
└───────────┬─────────────┘
            │
            │
  HOME      │                                    MESSAGES                            NEW MESSAGE
┌───────────┴─────────────┐                    ┌─────────────────────────┐         ┌─────────────────────────┐
│ Tx Rx GPS-10 5.2V 9999us│                    │ Tx Rx GPS-10 5.2V 9999us│         │ Tx Rx GPS-10 5.2V 9999us│
│  [ HOME ]               │                    │  [NOCALL-0]>[NOCALL-1]  │         │  [ NEW MESSAGE ]        │
│ >Messages               ├──────────┬─────────┤                         ├─────────┤ >Message Text           | 
│  Live Feed              │          │         │ Hi, Dale!               │         │  Recipient Callsign     │
│  Settings               │          │         │ Record: 1 of 100        │         │  Recipient SSID         │
│                         │          │         │ D: 110721   T: 9271700  │         │                         │
│ LT:9999.99 LG:99999.99  │          │         │ LT:9999.99 LG:99999.99  │         │  Type msg then <ENTER>_ │
└─────────────────────────┘          │         └─────────────────────────┘         └─────────────────────────┘
                                     │
                                     │
                                     │           LIVE FEED
                                     │         ┌─────────────────────────┐
                                     │         │ Tx Rx GPS-10 5.2V 9999us│
                                     │         │  [NOCALL-0]>[NOCALL-1]  │
                                     ├─────────┤                         │
                                     │         │ The weather report for F│
                                     │         │ Record: 1 of 100        │
                                     │         │ D: 110721   T: 9271700  │
                                     │         │ LT:9999.99 LG:99999.99  │
                                     │         └─────────────────────────┘
                                     │
                                     │
                                     │           SETTINGS                            APRS
                                     │         ┌─────────────────────────┐         ┌─────────────────────────┐
                                     │         │ Tx Rx GPS-10 5.2V 9999us│         │ Tx Rx GPS-10 5.2V 9999us│
                                     │         │  [ SETTINGS ]           │         │  [ APRS ]               │
                                     └─────────┤ >APRS                   ├────┬────┤ >Beacon Enabled         │
                                               │  GPS                    │    │    │  Beacon Distance        │
                                               │  Display                │    │    │  Beacon Idle Time       │
                                               │                         │    │    │                         │
                                               │ LT:9999.99 LG:99999.99  │    │    │  120000_                │
                                               └─────────────────────────┘    │    └─────────────────────────┘
                                                                              │
                                                                              │
                                                                              │      GPS
                                                                              │    ┌─────────────────────────┐
                                                                              │    │ Tx Rx GPS-10 5.2V 9999us│
                                                                              │    │  [ GPS ]                │
                                                                              ├────┤ >Position Tolerance     │
                                                                              │    │  Dest Latitude          │
                                                                              │    │  Dest Longitude         │
                                                                              │    │                         │
                                                                              │    │  10000_                 │
                                                                              │    └─────────────────────────┘
                                                                              │
                                                                              │
                                                                              │      DISPLAY
                                                                              │    ┌─────────────────────────┐
                                                                              │    │ Tx Rx GPS-10 5.2V 9999us│
                                                                              │    │  [ DISPLAY ]            │
                                                                              └────┤ >Timeout                │
                                                                                   │  Brightness             │
                                                                                   │  Show Position          │
                                                                                   │                         │
                                                                                   │  10000_                 │
                                                                                   └─────────────────────────┘

```

The above text was generated at https://asciiflow.com/