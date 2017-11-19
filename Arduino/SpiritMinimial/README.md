I had a bunch of extra components lying around so I decided I would use them to upgrade the lighting on my Spirit pack. I wanted the standard powercell and cyclotron animations while I walk around. I kept the flashing light in the powercell when firing the wand but removed them on the cyclotron.  The result can be seen here:

[![IMAGE ALT TEXT](http://img.youtube.com/vi/f6RJXqUei0Q/0.jpg)](https://www.youtube.com/watch?v=f6RJXqUei0Q "Spirit Halloween Ghostbusters Proton Pack Lighting Upgrade")

There are options in the code to use the video game cyclotron animation or just the regular light cycle. 
```
const bool useGameCyclotronEffect = true;     // set this to true to get the fading previous cyclotron light in the idle sequence
const bool useCyclotronFadeInEffect = true;   // Instead of the yellow alternate flashing on boot this fades the cyclotron in from off to red
```

You can also adjust the speed of the animations by updating these variables:
```
// ##############################
// bootup animation speeds
// ##############################
const int pwr_boot_interval = 30;       // How fast to do the powercell drop animation on bootup 
const int cyc_boot_interval = 400;      // If useCyclotronFadeInEffect is false this alternates the cycltron lights yellow 
const int cyc_boot_alt_interval = 100;  // How fast to fade in the cyclotron lights from black to red on bootup

// ##############################
// idle animation speeds
// ##############################
const int pwr_interval = 50;            // how fast the powercell cycles
const int cyc_interval = 750;           // how fast the cycltron cycles from one cell to the next
const int cyc_fade_interval = 1;        // if useGameCyclotronEffect is true this is how fast to fade the previous cyclotron to light to nothing
```

Example components used for this update

* [Optional power converter set to 5V output: 6 for $9.69](https://www.amazon.com/gp/product/B01MQGMOKI)
* [Arduino Nano: 3 for $11.86](https://www.amazon.com/gp/product/B0713XK923)
* [Two Neopixel Sticks: 1 for $7.99](https://www.amazon.com/ACROBOTIC-8-Pixel-Addressable-NeoPixels-WS2812B/dp/B06XRPLCV1)
* [Individual Neopixel: 100 for $15.88](https://www.amazon.com/gp/product/B01AG8X1X8)

You can probably find these components cheaper if you look around. For instance I have found packages of the sticks for under $10 for 5 of them before

Here is a fritzing diagram of the whole setup with the power converter:

[![IMAGE ALT TEXT](https://raw.githubusercontent.com/CountDeMonet/ArduinoProtonPack/master/Spirit%20Halloween%20Proton%20Pack%20-%20Pack%20Light%20Circuit.png)

If you wanted to not use the power converter and are ok with the lights being a bit dimmer you can use this setup which would be simpler to solder up. The code would be the same
[![IMAGE ALT TEXT](https://raw.githubusercontent.com/CountDeMonet/ArduinoProtonPack/master/Spirit%20Halloween%20Proton%20Pack%20-%20Alternate%20Pack%20Light%20Circuit.png)

For my pack I'll be using an old 11.1v 1800mah lipo I had laying around. You could easily use 4 AAA's as well for this setup and they would last a long time. Something like this: https://www.amazon.com/Sackorange-Battery-Spring-Plastic-Holder/dp/B071ZQ3DGT would work well if you wanted to use that.

I like to use the power converter set to 5v output as the neopixels like more stable voltage than the 5v out on the arduino can handle. It allows you to go brighter with the neopixels. You could simplify this even more and remove the converter and use the arduino vin to take in the battery. I also like to use connectors but this whole setup could be easily soldered all together since it only uses 2 pins on the arduino. 

I also used some red and blue transparent folders to upgrade the light windows. Here are some pictures of the board I made and the installation. I created 2 models for 3d printing to make the installation easier. Those are also included in the github repo 

https://github.com/CountDeMonet/ArduinoProtonPack/tree/master/Model/SpiritHalloweenProtonPackMods

I installed everything with hot glue. Easy and effective. I'll probably add a switch to turn the lights on and off instead of plugging the battery in each time but the install only took a few minutes. 

[![IMAGE ALT TEXT](https://i2.wp.com/vineripesoftware.files.wordpress.com/2017/10/img_20171015_095239274-copy.jpg?ssl=1&w=850)

[![IMAGE ALT TEXT](https://i1.wp.com/vineripesoftware.files.wordpress.com/2017/10/img_20171015_095416093-copy.jpg?ssl=1&w=850)

[![IMAGE ALT TEXT](https://i1.wp.com/vineripesoftware.files.wordpress.com/2017/10/img_20171015_102713199-copy.jpg?ssl=1&w=850)

[![IMAGE ALT TEXT](https://i1.wp.com/vineripesoftware.files.wordpress.com/2017/10/img_20171015_102542397-copy.jpg?ssl=1&w=850)

[![IMAGE ALT TEXT](https://i2.wp.com/vineripesoftware.files.wordpress.com/2017/10/img_20171015_102544974-copy.jpg?ssl=1&w=850)

[![IMAGE ALT TEXT](https://i0.wp.com/vineripesoftware.files.wordpress.com/2017/10/img_20171015_104157688-copy.jpg?ssl=1&w=850)
