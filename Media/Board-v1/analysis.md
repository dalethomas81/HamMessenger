# Analysis of HamMessenger v1.0 #  

After receiving the board and soldering the SMD components, it became very apparent that a new revision would be in order. Here is what I found...  

# Form #

1. At first sight you can see that the board will not mate flush with the Arduino due to poor placement of the SD card mechanism and the Grove connector. I will be moving these as one of the first improvements.
2. Next, you can see that the modem (Arduino Pro Mini) has all of its supporting hardware mounted underneath it. This was done on purpose to save space but I think a better option would have been to place the components on the other side of the board. I may do that in a later revision. But, for now I have added female 2.54mm headers so that the Pro Mini can be removed for troubleshooting and repair.
3. The last major form issue is the display. It is just too small. The prototype version uses a non-standard 1.2" OLED that is controlled by a sh1106 instead of a ssd1306 (and consequently needs a modified library). I decided that I would give the standard 0.96" display a try since they are very common and use the out-of-the-box version of the Adafruit ssd1306 library. However, its just too damn small!
4. Bonus: while we're on the subject of form - you will notice how terrible of a job I did on the smd soldering. It was my first time but I think (hope) the next board will look better :)

# Function #

1. The first functional issue was with the SD card. This actually counts as 2 issues. One was that I missed a ground on my SD card mechanism. The other was that I missed feeding vcc to the OE pin of the line-level shifter. I repaired both of those with jumpers so that I could troubleshoot the rest of the board.
2. The next functional issue is that the OLED display and the keyboard would not work. I had gotten the SDA and SCL lines for I2C crossed. You'll see a picture below of the disgusting job I did of fixing that.
3. Next, you can see that I ordered the wrong GPS unit (correct one on the BOM now). Luckily, it worked out that the correct one has an extra pin (pps) and it just so happened that I could scoot this one over a pin and run a jumper for the power and get it to work. So technically, this isn't an issue with the board but I'm listing it here.
4. After I got the GPS installed and powered, I noticed that the coordinates were not being populated in the bottom of the display. This was because I routed the serial wires to the wrong serial ports of the Arduino. Doh. That was an easy change in the software to get this one working but I will make it right on the next version of the board.
5. Moving on to encoding and decoding messages - it doesn't encode messages as good as the prototypes and I haven't gotten it to *decode* a message at all yet. I'm going to remove the Arduino Pro Mini and try to reflow some of those components. Admittedly, I did not do a very good job soldering these.
6. Oh, and lastly - sometimes the Arduino will lock up on transmit. This was an issue that I saw with the prototype boards and thought it must be an issue with not having a ground plane or shielding. I "fixed" the issue by adding ferrite cores to the cable between HamMessenger and the radio. The problem went away but it is now present in this board even with the ferrite cores. If anyone knows a thing or 2 about how to shield this board from RF, I would love to hear from you.

# Pictures #

![all-together-layout](/Media/Board-v1/all-together-layout.jpeg)  

![front-and-back](/Media/Board-v1/front-and-back.jpeg)  

![front](/Media/Board-v1/front.jpeg)  

![side](/Media/Board-v1/side.jpeg)  

![isometric](/Media/Board-v1/isometric.jpeg)  

![i2c-repair](/Media/Board-v1/i2c-repair.jpeg)  

![oe-pin-jumper](/Media/Board-v1/oe-pin-jumper.jpeg)  

![larger-display](/Media/Board-v1/larger-display.jpeg)  