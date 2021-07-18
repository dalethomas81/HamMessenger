# Radio Setup #

To be able to transmit on your radio using HamMessenger, you will need to properly set up your radio. This document will describe how to do that.  

## Choosing a radio ##

My preferred radio is the Baofeng UV-5R5. You can buy this radio for about $25-$30 from [Amazon](https://www.amazon.com/BAOFENG-Two-Way-136-174MHz-400-520MHz-Battery/dp/B091YLJ6CN/ref=sr_1_4?dchild=1&keywords=uv-5r5&qid=1626584757&sr=8-4). I recommend the 2-pack.  

There are also slightly nicer versions of this radio that you can buy directly from [BTECH](https://baofengtech.com). The [BF-F8HP](https://baofengtech.com/product/bf-f8hp/) is the 8 watt version and the [UV-5X3](https://baofengtech.com/product/uv-5x3/) is the tri-band version.  

You can also use many others radios with HamMessenger - you will just need to use a different cable between HamMessenger and the radio.  

## Frequency ##

Technically, there is no reason that you can't use just about any frequency that you would like with HamMessenger. However, the agreed upon frequency for 2 meter APRS (the protcol that HamMessenger uses) is 144.390 Mhz. Using this frequency will give you the most benefit because you will be able to communicate with other stations and even be able to hit APRS repeaters.  

## Programming ##

To program the UV-5R5 (or the other 2 variants above), you can use an open source program called [CHIRP](https://chirp.danplanet.com/projects/chirp/wiki/Home) or you can program directly from the front of the radio. I won't go into how to use CHIRP but I will show you how to program the Baofeng from the keypad. Knowing how to program it using this method will give you the most benefit as you will be able to program the radio in the field and make changes on the fly without a computer.  

### Programming a Channel Using the Keypad ###

1. First, determine the channel that you would like to program. You can do this by cycling through the radio and determining which channel is not in use.  
2. Put the radio in VFO mode by pressing the **[VFO/MR]** button. You can tell the radio is in VFO mode if there are no channel numbers displayed on the right. If there are, press the button again and you should see them disappear.  
3. Next, enter the APRS frequency so that it shows up on the display. Note: pay attention here to what VFO you are using. You can tell by the up or down arrow on the left side. It doesn't matter which one but you will want to make sure it stays the same from this point forward. You can change it using the **[A/B]** button.
4. Next, press the **[Menu]** button to get to the settings of the radio. The Baofeng gets a bad rap for having a menu system that is difficult to navigate and understand. I will tell you exactly what to enter but if you want to know more about why then hands down the best place to do that is the [Miklor](https://www.miklor.com/uv5r/UV5R-MenuDef.php) site.  
5. Lastly, follow this table to enter the correct settings for each item. Set them in this order and if they are not listed then don't worry about them. Also note that most of these are not required for APRS - some may reduce difficulties when Txing or Rxing and some just make life easier on you.  

    number | item | required | value | description
    ------ | ------- | -------- | ----- | -----------
    0 | SQL | *N* | 4 | recommended squelch
    2 | TXP | *N* | HIGH | transmit power
    3 | SAVE | *N* | OFF | power save mode
    4 | VOX | *N* | OFF | voice activation
    5 | WN | **Y** | NARR | bandwidth
    7 | TDR | *N* | OFF | dual watch dual reception
    9 | TOT | *N* | 60 | transmission time out time
    10 | R-DCS | **Y** | OFF | receive digital coded squelch
    11 | R-CTCS | **Y** | OFF | receive continuouse tone coded squelch
    12 | T-DCS | **Y** | OFF | transmit digital coded squelch
    13 | T-CTCS | **Y** | OFF | transmit continuouse tone coded squelch
    16 | DTMFST | *N* | OFF | DTMF tone of transmit
    19 | PTT-ID | *N* | OFF | when to send the PTT-ID
    20 | MDF-A | *N* | NAME | channel mode A display
    21 | MDF-B | *N* | NAME | channel mode B display
    23 | BCL | *N* | ON | busy channel lockout (won't interfere if someone already broadcasting)
    25 | SFT-D | **Y** | OFF | frequency shift direction
    26 | OFFSET | *N*| 000.000 | frequency shift amount
    35 | STE | *N* | OFF | squelch tail elimination
    36 | RP-STE | *N* | OFF | repeater squelch tail elimination
    37 | RPT-RL | *N* | OFF | delay the squelch tail of repeater
    39 | ROGER | *N* | OFF | tone at the end of transmission
    27 | MEM-CH | **Y** | empty channel you chose in step 1 | Note: do this last as it will trigger the save!

## Radio Use ##

I have found that the volume of the radio works well at around 30%. I actually havent done much testing here but this works well.

HamMessenger is designed so that you can wear the radio on your belt while you use HamMessenger as a texting device in your hand.