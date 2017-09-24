All of this assumes you are using a 15 led bar graph. I'm combining a 5 led and a 10 led bargraph from Digikey.

From the bargraph_code.ino you need to do the following:

What this chunk does is include the sx1509 libraries and sets up the configuration. This assumes the library is configured stock and the address has not been changed. It also defines the LED locations for the bar graph. Take this code and insert it into the header of the Neutrino_Wand.ino file

```c++
#include <Wire.h> // Include the I2C library (required)
#include <SparkFunSX1509.h> // Include SX1509 library

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address
SX1509 io; // Create an SX1509 object to be used throughout

// bargraph helper variables
int seq_1_current = 0;  // current led in sequence 1
const int num_led = 15; // total number of leds in bar graph

// SX1509 pin definitions for the leds on the graph:
const byte SX1509_BAR_01 = 0;
const byte SX1509_BAR_02 = 1;
const byte SX1509_BAR_03 = 2;
const byte SX1509_BAR_04 = 3;
const byte SX1509_BAR_05 = 4;
const byte SX1509_BAR_06 = 5;
const byte SX1509_BAR_07 = 6;
const byte SX1509_BAR_08 = 7;
const byte SX1509_BAR_09 = 8;
const byte SX1509_BAR_10 = 9;
const byte SX1509_BAR_11 = 10;
const byte SX1509_BAR_12 = 11;
const byte SX1509_BAR_13 = 12;
const byte SX1509_BAR_14 = 13;
const byte SX1509_BAR_15 = 14;
```

The next chunk is added to the setup() function. This initialized the sx1509 board and makes sure the arduino is communicating. It configures the pin modes for the LED's as inputs and shutdown all of the LED's so they are not lit

```c++
  // Call io.begin(<address>) to initialize the SX1509. If it
  // successfully communicates, it'll return 1.
  if (!io.begin(SX1509_ADDRESS)) {
    while (1) ; // If we fail to communicate, loop forever for now but it would be nice to warn the user somehow
  }
  
  // configuration for the bargraph LED's
  io.pinMode(SX1509_BAR_01, OUTPUT);
  io.pinMode(SX1509_BAR_02, OUTPUT);
  io.pinMode(SX1509_BAR_03, OUTPUT);
  io.pinMode(SX1509_BAR_04, OUTPUT);
  io.pinMode(SX1509_BAR_05, OUTPUT);
  io.pinMode(SX1509_BAR_06, OUTPUT);
  io.pinMode(SX1509_BAR_07, OUTPUT);
  io.pinMode(SX1509_BAR_08, OUTPUT);
  io.pinMode(SX1509_BAR_09, OUTPUT);
  io.pinMode(SX1509_BAR_10, OUTPUT);
  io.pinMode(SX1509_BAR_11, OUTPUT);
  io.pinMode(SX1509_BAR_12, OUTPUT);
  io.pinMode(SX1509_BAR_13, OUTPUT);
  io.pinMode(SX1509_BAR_14, OUTPUT);
  io.pinMode(SX1509_BAR_15, OUTPUT);

  // set everything off initially
  shutdown_leds();
```

Right above the Loop() function in neutrino_wand.ino there are a number of intervals defined. The interval for the bargraph is already defined in neutrino_wand.ino. What this does is defines how often the bar graph is updated. If you wanted to speed up the bargraph this is the variable you would adjust. We define it here because in the main loop() we update this parameter so the bargraph can go faster when firing 

```c++
long firing_interval = 40;    // interval at which to cycle firing lights on the bargraph. We update this in the loop to speed up the animation so must be declared here (milliseconds).
```

The last part is to replace the stub functions in neutrino_wand.ino. At the very bottom of the neutrino_wand.ino file you will see these stubs

```c++
/*************** Bar Graph Animations *********************/
void shutdown_leds() {
  // stubb function for when I re-enable to bargraph
}
void barGraphSequenceOne(int currentMillis) {
  // stubb function for when I re-enable to bargraph
}
void barGraphSequenceTwo(int currentMillis) {
  // stubb function for when I re-enable to bargraph
}
```

I kept the code that calls the bargraph in the file so all you have to do is update these stubs with the actual code.  Simply copy this code and replace the stubbs with this.

```c++
/*************** Bar Graph Animations *********************/
// This is the idle sequence
unsigned long prevBarMillis_on = 0;   // bargraph on tracker
const int pwrcl_interval = 60;     // interval at which to cycle lights (milliseconds).
bool reverseSequenceOne = false;

void barGraphSequenceOne(int currentMillis) {
  // normal sync animation on the bar graph
  if (currentMillis - prevBarMillis_on > pwrcl_interval) {
    // save the last time you blinked the LED
    prevBarMillis_on = currentMillis;

    if ( reverseSequenceOne == false ) {
      switch_graph_led(seq_1_current, HIGH);
      seq_1_current++;
      if ( seq_1_current > num_led ) {
        reverseSequenceOne = true;
      }
    } else {
      switch_graph_led(seq_1_current, LOW);
      seq_1_current--;
      if ( seq_1_current < 0  ) {
        reverseSequenceOne = false;
      }
    }
  }
}

// This is the firing sequence
unsigned long prevBarMillis_fire = 0; // bargraph firing tracker
int fireSequenceNum = 1;

void barGraphSequenceTwo(int currentMillis) {
  if (currentMillis - prevBarMillis_fire > firing_interval) {
    // save the last time you blinked the LED
    prevBarMillis_fire = currentMillis;

    switch (fireSequenceNum) {
      case 1:
        switch_graph_led(2, LOW);
        switch_graph_led(14, LOW);
        switch_graph_led(1, HIGH);
        switch_graph_led(15, HIGH);
        fireSequenceNum++;
        break;
      case 2:
        switch_graph_led(1, LOW);
        switch_graph_led(15, LOW);
        switch_graph_led(2, HIGH);
        switch_graph_led(14, HIGH);
        fireSequenceNum++;
        break;
      case 3:
        switch_graph_led(2, LOW);
        switch_graph_led(14, LOW);
        switch_graph_led(3, HIGH);
        switch_graph_led(13, HIGH);
        fireSequenceNum++;
        break;
      case 4:
        switch_graph_led(3, LOW);
        switch_graph_led(13, LOW);
        switch_graph_led(4, HIGH);
        switch_graph_led(12, HIGH);
        fireSequenceNum++;
        break;
      case 5:
        switch_graph_led(4, LOW);
        switch_graph_led(12, LOW);
        switch_graph_led(5, HIGH);
        switch_graph_led(11, HIGH);
        fireSequenceNum++;
        break;
      case 6:
        switch_graph_led(5, LOW);
        switch_graph_led(11, LOW);
        switch_graph_led(6, HIGH);
        switch_graph_led(10, HIGH);
        fireSequenceNum++;
        break;
      case 7:
        switch_graph_led(6, LOW);
        switch_graph_led(10, LOW);
        switch_graph_led(7, HIGH);
        switch_graph_led(9, HIGH);
        fireSequenceNum++;
        break;
      case 8:
        switch_graph_led(7, LOW);
        switch_graph_led(9, LOW);
        switch_graph_led(6, HIGH);
        switch_graph_led(10, HIGH);
        fireSequenceNum++;
        break;
      case 9:
        switch_graph_led(6, LOW);
        switch_graph_led(10, LOW);
        switch_graph_led(5, HIGH);
        switch_graph_led(11, HIGH);
        fireSequenceNum++;
        break;
      case 10:
        switch_graph_led(5, LOW);
        switch_graph_led(11, LOW);
        switch_graph_led(4, HIGH);
        switch_graph_led(12, HIGH);
        fireSequenceNum++;
        break;
      case 11:
        switch_graph_led(4, LOW);
        switch_graph_led(12, LOW);
        switch_graph_led(3, HIGH);
        switch_graph_led(13, HIGH);
        fireSequenceNum++;
        break;
      case 12:
        switch_graph_led(3, LOW);
        switch_graph_led(13, LOW);
        switch_graph_led(2, HIGH);
        switch_graph_led(14, HIGH);
        fireSequenceNum = 1;
        break;
    }
  }
}

/************************* Shutdown and helper functions ****************************/
void shutdown_leds() {
  // reset the sequence
  seq_1_current = 1;
  fireSequenceNum = 1;

  // shut all led's off
  for (int i = 1; i <= 15; i++) {
    switch_graph_led(i, LOW);
  }
}

void switch_graph_led(int num, int state) {
  switch (num) {
    case 1:
      io.digitalWrite(SX1509_BAR_01, state);
      break;
    case 2:
      io.digitalWrite(SX1509_BAR_02, state);
      break;
    case 3:
      io.digitalWrite(SX1509_BAR_03, state);
      break;
    case 4:
      io.digitalWrite(SX1509_BAR_04, state);
      break;
    case 5:
      io.digitalWrite(SX1509_BAR_05, state);
      break;
    case 6:
      io.digitalWrite(SX1509_BAR_06, state);
      break;
    case 7:
      io.digitalWrite(SX1509_BAR_07, state);
      break;
    case 8:
      io.digitalWrite(SX1509_BAR_08, state);
      break;
    case 9:
      io.digitalWrite(SX1509_BAR_09, state);
      break;
    case 10:
      io.digitalWrite(SX1509_BAR_10, state);
      break;
    case 11:
      io.digitalWrite(SX1509_BAR_11, state);
      break;
    case 12:
      io.digitalWrite(SX1509_BAR_12, state);
      break;
    case 13:
      io.digitalWrite(SX1509_BAR_13, state);
      break;
    case 14:
      io.digitalWrite(SX1509_BAR_14, state);
      break;
    case 15:
      io.digitalWrite(SX1509_BAR_15, state);
      break;
  }
}
```

And with that everything should work. You'll have the idle animation on the bar graph and the firing animation with the speed increasing as you hold the button down as I showed in some of the videos.
