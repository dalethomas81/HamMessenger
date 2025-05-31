I will start working on videos and tutorials and add links to them here.

For now, there is something I want to address about the GPS module. Originally, I chose a GPS module that had a really small antenna. After using it for a while, I realize the antenna is very poor at reception. So, I decided to use a different model with a larger antenna.  

As is turns out, the model I originally used has a different pinout than most other models that have the larger and better antenna.  

However, there is a solution for the current versions of the board (currently v1.2). You can simply shift the board one pin over and solder a jumper from the vcc line to the pps through-hole (not used otherwise).  

<img src="../../Media/gps-alternate.jpeg" alt="GPS Alt" width="400">

Additionally, I do plan to change this in the next board revision.  