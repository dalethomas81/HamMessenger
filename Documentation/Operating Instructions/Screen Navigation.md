## Screen Navigation ##

-The **[Up]** and **[Down]** keys move the cursor through the selections.\
-Pressing the **[Enter]** key will navigate to the screen selected by the cursor.\
-To exit a screen, press the **[Escape]** key.
#### Editing a Setting ####
-Select the setting using the **[Up]** and **[Down]** keys.\
-Press the **[Enter]** key. This will put the device in edit mode.\
-Use the **[Up]** and **[Down]** keys to modify a boolean value.\
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
  HOME      │                                    Messages
┌───────────┴─────────────┐                    ┌─────────────────────────┐
│ Tx  Rx  Msg   5.2V   0ms│                    │ Tx  Rx  Msg   5.2V   0ms│
│                         │                    │  [NOCALL-0]>[NOCALL-1]  │
│ >Messages               ├──────────┬─────────┤                         │
│  Live Feed              │          │         │ Hi, Dale!               │
│  Settings               │          │         │ Record: 1 of 100        │
│                         │          │         │ D: 110721   T: 9271700  │
│ LT:9999.99 LG:99999.99  │          │         │ LT:9999.99 LG:99999.99  │
└─────────────────────────┘          │         └─────────────────────────┘
                                     │
                                     │
                                     │           Live Feed
                                     │         ┌─────────────────────────┐
                                     │         │ Tx  Rx  Msg   5.2V   0ms│
                                     │         │  [NOCALL-0]>[NOCALL-1]  │
                                     ├─────────┤                         │
                                     │         │ The weather report for F│
                                     │         │ Record: 1 of 100        │
                                     │         │ D: 110721   T: 9271700  │
                                     │         │ LT:9999.99 LG:99999.99  │
                                     │         └─────────────────────────┘
                                     │
                                     │
                                     │           Settings                            APRS
                                     │         ┌─────────────────────────┐         ┌─────────────────────────┐
                                     │         │ Tx  Rx  Msg   5.2V   0ms│         │ Tx  Rx  Msg   5.2V   0ms│
                                     │         │                         │         │ >Beacon Freq            │
                                     └─────────┤ >APRS                   ├────┬────┤  Raw Packet             │
                                               │  GPS                    │    │    │  Comment                │
                                               │  Display                │    │    │  Message                │
                                               │                         │    │    │                         │
                                               │ LT:9999.99 LG:99999.99  │    │    │  120000_                │
                                               └─────────────────────────┘    │    └─────────────────────────┘
                                                                              │
                                                                              │
                                                                              │      GPS
                                                                              │    ┌─────────────────────────┐
                                                                              │    │ Tx  Rx  Msg   5.2V   0ms│
                                                                              │    │ >Update Frequency       │
                                                                              ├────┤  Position Tolerance     │
                                                                              │    │  Dest Latitude          │
                                                                              │    │  Dest Longitude         │
                                                                              │    │                         │
                                                                              │    │  10000_                 │
                                                                              │    └─────────────────────────┘
                                                                              │
                                                                              │
                                                                              │      Display
                                                                              │    ┌─────────────────────────┐
                                                                              │    │ Tx  Rx  Msg   5.2V   0ms│
                                                                              │    │ >Timeout                │
                                                                              └────┤  Brightness             │
                                                                                   │  Show Position          │
                                                                                   │  Scroll Messages        │
                                                                                   │                         │
                                                                                   │  10000_                 │
                                                                                   └─────────────────────────┘

```

The above text was generated at https://asciiflow.com/