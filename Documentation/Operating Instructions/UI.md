
```
  SPLASH
┌─────────────────────────┐
│       HamMessenger      │
│                         │
│ Month Day Year HH:mm:ss │
│                         │
│       Csll:NOCALL       │
│                         │
└───────────┬─────────────┘
            │
            │
  HOME      │                                    Messages
┌───────────┴─────────────┐                    ┌─────────────────────────┐
│[Tx][Rx][Msg]  5.2V   0ms│                    │[Tx][Rx][Msg]  5.2V   0ms│
│                         │                    │ 1 [NOCALL-0]>[NOCALL-1] │
│ >Messages               ├──────────┬─────────┤                         │
│  Live Feed              │          │         │ Hi, Dale!               │
│  Settings               │          │         │                         │
│                         │          │         │                         │
│ LT:9999.99 LG:99999.99  │          │         │ LT:9999.99 LG:99999.99  │
└─────────────────────────┘          │         └─────────────────────────┘
                                     │
                                     │
                                     │           Live Feed
                                     │         ┌─────────────────────────┐
                                     │         │[Tx][Rx][Msg]  5.2V   0ms│
                                     │         │ 1 [NOCALL-0]>[NOCALL-1] │
                                     ├─────────┤                         │
                                     │         │ The weather report for F│
                                     │         │                         │
                                     │         │                         │
                                     │         │ LT:9999.99 LG:99999.99  │
                                     │         └─────────────────────────┘
                                     │
                                     │
                                     │           Settings                            APRS
                                     │         ┌─────────────────────────┐         ┌─────────────────────────┐
                                     │         │[Tx][Rx][Msg]  5.2V   0ms│         │[Tx][Rx][Msg]  5.2V   0ms│
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
                                                                              │    │[Tx][Rx][Msg]  5.2V   0ms│
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
                                                                              │    │[Tx][Rx][Msg]  5.2V   0ms│
                                                                              │    │ >Timeout                │
                                                                              └────┤  Brightness             │
                                                                                   │  Show Position          │
                                                                                   │  Scroll Messages        │
                                                                                   │                         │
                                                                                   │  10000_                 │
                                                                                   └─────────────────────────┘

```

The above text was generated at https://asciiflow.com/