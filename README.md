# Arduino Based Ghostbusters Proton Pack
Arduino Code for a Ghostbusters Neutrino Wand and Proton Pack

I'm in the process of building a proton pack for my son and this is the progress I have made so far. I have included the models for 3d printing, all sounds but the ghostbusters theme song, and the arduino code to control all of this. The models for 3d printing are scaled to 83%. The proton pack models are also mirrored for a left handed users pack. Once I have some time I'll post non-mirrored files. 

The models are based off the following things on thingiverse: 
* Main base model for the pack: https://www.thingiverse.com/thing:2479141
* Base model for the proton wand: https://www.thingiverse.com/thing:1128019
* Full scale proton wand the 83% one is based on: https://www.thingiverse.com/thing:2334883
* And a clippard valve with text: https://www.thingiverse.com/thing:2286284

This pack utilizes many off the shelf products for the electronics to make the build as easy as possible. Total cost is around $80 for the parts if sourced from amazon. You can find cheaper on ebay

* 1 arduino nano. You get 3 for $11.86: https://www.amazon.com/gp/product/B0713XK923
* 2 power converters to adjust the 11.1v lipo to 5v. You get 6 for $9.69: https://www.amazon.com/gp/product/B01MQGMOKI
* 1 Adafruit Audio FX Sound Board. It's the most expensive part at $23.80 but is awesome to work with: https://www.amazon.com/gp/product/B00Q3U42DM
* Audio amp $6.01 each: https://www.amazon.com/gp/product/B00PY2YSI4
* small speaker for my 83% scale build. You get 2 for $8.05. https://www.amazon.com/gp/product/B00NQ0LHNA
* 4 neopixel jewels for the cyclotron @ 5.49 each: https://www.amazon.com/gp/product/B0105VMT4S
* 1 neopixel jewels for the end of the wand @ 5.49 each: https://www.amazon.com/gp/product/B0105VMT4S
* two 8 neopixel light stick for the powercell. You can get 5 for $8.49 here: https://www.amazon.com/HiLetgo-5pcs-8-WS2812-Driver-Development/dp/B014F5VLN0
* 4 toggle switches:https://www.amazon.com/gp/product/B00HGAKDIQ
* 1 button:  https://www.amazon.com/gp/product/B01IU898QA
* 11.1v lipo to power the whole thing: https://www.amazon.com/Gens-ace-Battery-2200mAh-Airplane

For the optional bargraph we are using the SparkFun SX1509 expander board and some resitor networks/bargraphs from digikey. If you want to use the bargraph you need to move the code from the test file into the main file and overwrite the stub functions. Since I won't be using it in my son's pack I removed it from the main file 
https://learn.sparkfun.com/tutorials/sx1509-io-expander-breakout-hookup-guide

* https://www.digikey.com/product-detail/en/sparkfun-electronics/BOB-13601/1568-1186-ND/5673772
* https://www.digikey.com/product-detail/en/bourns-inc/4611X-101-331LF/4611X-101-331LF-ND/3787988
* https://www.digikey.com/product-detail/en/bourns-inc/4606X-101-331LF/4606X-1-331LF-ND/1088983
* https://www.digikey.com/product-detail/en/sunled/XGUYX10D/1497-1086-5-ND/4745985
* https://www.digikey.com/product-detail/en/sunled/XGUYX5D/1497-1087-5-ND/4745584

Here's a video of the whole shebang put together

[![IMAGE ALT TEXT](http://img.youtube.com/vi/IysHD7t0PW0/0.jpg)](https://www.youtube.com/watch?v=IysHD7t0PW0 "Ghostbusters Arduino Proton Pack Update")
