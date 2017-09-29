#include <QueueArray.h>

// for the sound board
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"

#include <Adafruit_NeoPixel.h>

// for led triggers
#define HIGH 0x1
#define LOW  0x0

// neopixel pins / setup
#define NEO_POWER 2 // for cyclotron and powercell
Adafruit_NeoPixel powerStick = Adafruit_NeoPixel(48, NEO_POWER, NEO_GRB + NEO_KHZ800);

#define NEO_NOSE 3 // for nose of wand
Adafruit_NeoPixel noseJewel = Adafruit_NeoPixel(7, NEO_NOSE, NEO_GRB + NEO_KHZ800);

#define NEO_WAND 4 // for nose of wand
Adafruit_NeoPixel wandLights = Adafruit_NeoPixel(4, NEO_WAND, NEO_GRB + NEO_KHZ800);

// LED indexes into the neopixel powerstick chain for the cyclotron
const int powercellLedCount = 14;                                         // total number of led's in the animation
const int powercellIndexOffset = 1;                                       // first led offset into the led chain for the animation

int c1Start = 16;
int c1End = 22;
int c2Start = 23;
int c2End = 29;
int c3Start = 30;
int c3End = 36;
int c4Start = 37;
int c4End = 43;
int ventStart = 44;
int ventEnd = 47;

// inputs for switches and buttons
const int THEME_SWITCH = 5;
const int STARTUP_SWITCH = 6;
const int SAFETY_SWITCH = 7;
const int FIRE_BUTTON = 8;

// soundboard pins and setup
#define SFX_RST 10
#define SFX_RX 11
#define SFX_TX 12
const int ACT = 13;// this allows us to know if the audio is playing

SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard( & ss, NULL, SFX_RST);

// ##############################
// available options
// ##############################
const bool useGameCyclotronEffect = true;   // set this to true to get the fading previous cyclotron light in the idle sequence
const bool useCyclotronFadeInEffect = false; // Instead of the yellow alternate flashing on boot/vent this fades the cyclotron in from off to red
const bool useDialogTracks = true;          // set to true if you want the dialog tracks to play after firing for 5 seconds

// Possible Pack states
bool powerBooted = false;   // has the pack booted up
bool isFiring = false;      // keeps track of the firing state
bool shouldWarn = false;    // track the warning state for alert audio
bool shuttingDown = false;  // is the pack in the process of shutting down
bool poweredDown = true;    // is the pack powered down
bool venting = false;       // is the pack venting

// physical switch states
bool startup = false;
bool theme = false;
bool safety = false;
bool fire = false;
bool warning = false;

// audio track names on soundboard
const String startupTrack =   "T00     WAV";
const String blastTrack =     "T01     WAV";
const String endTrack =       "T02     WAV";
const String idleTrack =      "T03     WAV";
const String shutdownTrack =  "T04     WAV";
const String clickTrack =     "T05     WAV";
const String chargeTrack =    "T06     WAV";
const String warnTrack =      "T07     WAV";
const String ventTrack =      "T08     WAV";
const String texTrack =       "T09     WAV";
const String choreTrack =     "T10     WAV";
const String toolsTrack =     "T11     WAV";
const String listenTrack =    "T12     WAV";
const String thatTrack =      "T14     WAV";
const String neutronizedTrack="T17     WAV";
const String boxTrack =       "T18     WAV";
const String themeTrack =     "T19     OGG";

// this queue holds a shuffled list of dialog tracks we can pull from so we don't
// play the same ones twice
QueueArray <int> dialogQueue;
int numDialog = 7;

// timer trigger times/states
unsigned long firingStateMillis;
const int firingWarmWaitTime = 5000;  // how long to hold down fire for lights to speed up
const int firingWarnWaitTime = 10000;  // how long to hold down fire before warning sounds

// Arduino setup function
void setup() {  
  // softwareserial at 9600 baud for the audio board
  ss.begin(9600);

  // see if we have the soundboard
  // If we fail to communicate, loop forever for now but it would be nice to warn the user somehow
  if (!sfx.reset()) {
    while (1);
  }

  // set act modes for the fx board
  pinMode(ACT, INPUT);

  // configure nose jewel
  noseJewel.begin();
  noseJewel.setBrightness(50);
  noseJewel.show(); // Initialize all pixels to 'off'

  // configure powercell/cyclotron
  powerStick.begin();
  powerStick.setBrightness(10);
  powerStick.show(); // Initialize all pixels to 'off'

  // configure wand lights
  wandLights.begin();
  wandLights.setBrightness(10);
  wandLights.show();

  // set the modes for the switches/buttons
  pinMode(THEME_SWITCH, INPUT);
  digitalWrite(THEME_SWITCH, HIGH);
  pinMode(STARTUP_SWITCH, INPUT);
  digitalWrite(STARTUP_SWITCH, HIGH);
  pinMode(SAFETY_SWITCH, INPUT);
  digitalWrite(SAFETY_SWITCH, HIGH);
  pinMode(FIRE_BUTTON, INPUT);
  digitalWrite(FIRE_BUTTON, HIGH);
}

/* ************* Audio Board Helper Functions ************* */
// helper function to play a track by name on the audio board
void playTrack(String trackname) {
  // stop track if one is going
  int playing = digitalRead(ACT);
  if (playing == 0) {
    sfx.stop();
  }

  char charName[20];
  trackname.toCharArray(charName, 20);
  
  // now go play
  if (sfx.playTrack(charName)) {
    sfx.unpause();
  }
}

void playDialogTrack(){
  // if the queue is empty reseed it
  if ( dialogQueue.isEmpty() ){
    for (int i=1; i<=numDialog; i++){
      dialogQueue.enqueue(i);
    }
  }
  
  switch (dialogQueue.dequeue()){
    case (1):
      playTrack(texTrack);
      break;
    case (2):
      playTrack(listenTrack);
      break;
    case (3):
      playTrack(choreTrack);
      break;
    case (4):
      playTrack(boxTrack);
      break;
    case (6):
      playTrack(thatTrack);
      break;
    case (8):
      playTrack(neutronizedTrack);
      break;
    case (9):
      playTrack(toolsTrack);
      break;
    default: 
      playTrack(endTrack);
      break;
  }
}

/* ************* Main Loop ************* */
int cyclotronRunningFadeOut = 255;  // we reset this variable every time we change the cyclotron index so the fade effect works
int cyclotronRunningFadeIn = 0;     // we reset this to 0 to fade the cyclotron in from nothing

// intervals that can be adjusted in real time to speed up animations 
int pwr_interval = 60;        // interval at which to cycle lights for the powercell. We update this in the loop to speed up the animation so must be declared here (milliseconds)
int cyc_interval = 1000;      // interval at which to cycle lights for the cyclotron.
int cyc_fade_interval = 15;   // fade the inactive cyclotron to light to nothing
int firing_interval = 40;     // interval at which to cycle firing lights on the bargraph. We update this in the loop to speed up the animation so must be declared here (milliseconds).

void loop() {
  // get the current time
  int currentMillis = millis();

  // find out of the audio board is playing audio
  int playing = digitalRead(ACT);

  // get the current switch states
  int theme_switch = digitalRead(THEME_SWITCH);

  // if the theme switch has recently changed from off to on we 
  // should play the full ghostbusters theme song
  if (theme_switch == 1) {
    if (theme == false) {
      playTrack(themeTrack);
      theme = true;
    }
  } else {
    theme = false;
  }

  int startup_switch = digitalRead(STARTUP_SWITCH);
  int safety_switch = digitalRead(SAFETY_SWITCH);
  int fire_button = digitalRead(FIRE_BUTTON);
  
  // while the startup switch is set on
  if (startup_switch == 1) {   
    // in general we always try to play the idle sound if started
    if (playing == 1 && startup == true) {
      playTrack(idleTrack);
    }
    
    // choose the right powercell animation sequence for booted/on
    if ( powerBooted == true ) {
      // standard idle power sequence for the pack
      poweredDown = false;
      shuttingDown = false;
      venting = false;
      setWandLightState(3, 0, 0); //set sloblow red
      setVentLightState(ventStart, ventEnd, 2);
      powerSequenceOne(currentMillis, pwr_interval, cyc_interval, cyc_fade_interval);
    } else {
      // boot up the pack. powerSequenceBoot will set powerBooted when complete
      powerSequenceBoot(currentMillis);
      setWandLightState(3, 7, currentMillis);   //set sloblow red blinking
    }

    // if we are not started up we should play the startup sound and begin the boot sequence
    if (startup == false) {
      startup = true;
      playTrack(startupTrack);

      // get the current safety switch state
      if (safety_switch == 1 && safety == false) {
        safety = true;
      }
    }
    
    if( startup == true && safety_switch == 1 ){
      if( venting == false && powerBooted == true ){
        setWandLightState(1, 2, 0);    //  set back light orange
        setWandLightState(2, 3, 0);    //  set body led blue
      }else{
        setWandLightState(1, 4, 0);    //  set back light off
        setWandLightState(2, 4, 0);    //  set body led off
      }

      // if the safety switch is set off then we can fire when the button is pressed
      if ( fire_button == 0) {
        // if the button is just pressed we clear all led's to start the firing animations
        if ( isFiring == false ) {
          shutdown_leds();
          isFiring = true;
        }
  
        // show the firing bargraph sequence
        barGraphSequenceTwo(currentMillis);

        // strobe the nose pixels
        fireStrobe(currentMillis); 

        // now powercell/cyclotron/wand lights
        // if this is the first time reset some variables and play the blast track
        if (fire == false) {
          shouldWarn = false;
          fire = true;
          firingStateMillis = millis();
          playTrack(blastTrack);
        } else {
          // find out what our timing is
          long diff = millis() - firingStateMillis;
          
          if ( diff > firingWarnWaitTime) { // if we are in the fire warn interval
            pwr_interval = 10;      // speed up the powercell animation
            firing_interval = 20;   // speed up the bar graph animation
            cyc_interval = 50;      // really speed up cyclotron
            cyc_fade_interval = 5;  // speed up the fade of the cyclotron
            if (playing == 1 || shouldWarn == false ) {
              shouldWarn = true;
              playTrack(warnTrack); // play the firing track with the warning
            }
            setWandLightState(0, 8, currentMillis);    // set top light red flashing fast
          } else if ( diff > firingWarmWaitTime) { // if we are in the dialog playing interval
            pwr_interval = 30;      // speed up the powercell animation
            firing_interval = 30;   // speed up the bar graph animation
            cyc_interval = 200;     // speed up cyclotron
            cyc_fade_interval = 10; // speed up the fade of the cyclotron
            if (playing == 1) {
              playTrack(blastTrack); // play the normal blast track
            }
            setWandLightState(0, 6, currentMillis);    // set top light orange flashing
          }
        }
      } else { // if we were firing and are no longer reset the leds
        if ( isFiring == true ) {
          shutdown_leds();
          isFiring = false;
        }
  
        // and do the standard bargraph sequence
        barGraphSequenceOne(currentMillis);

        if (fire == true) { // if we were firing let's reset the animations and play the correct final firing track
          clearFireStrobe();
          setWandLightState(0, 4, currentMillis);    // set top light off
          
          pwr_interval = 60;
          firing_interval = 40;
          cyc_interval = 1000;
          cyc_fade_interval = 15;
          fire = false;

          // see if we've been firing long enough to get the dialog or vent sounds
          long diff = millis() - firingStateMillis;

          if ( diff > firingWarnWaitTime) { // if we are past the warning let's vent the pack
            playTrack(ventTrack);
            venting = true;
            clearPowerStrip(); // play the boot animation on the powercell
          } else if ( diff > firingWarmWaitTime) { // if in the dialog time play the dialog in sequence
            if( useDialogTracks == true ){
              playDialogTrack();
            }else{
              playTrack(endTrack);
            }
          } else {
            // otherwise play the standard power down track
            playTrack(endTrack);
          }
        }
      }

      // if the safety was just changed play the click track
      if (safety == false) {
        safety = true;
        playTrack(chargeTrack);
      }
    } else {
      // if the safety is switched off play the click track
      if (safety == true) {
        setWandLightState(1, 4, 0);    // set back light off
        setWandLightState(2, 4, 0);    // set body off
        safety = false;
        playTrack(clickTrack);
      }
    }
  } else { // if we are powering down
    if( poweredDown == false ){
      if( shuttingDown == false ){
        playTrack(shutdownTrack); // play the pack shutdown track
        shuttingDown = true;
      }
      cyclotronRunningFadeOut = 255;
      powerSequenceShutdown(currentMillis);
    }else{
      if (startup == true) { // if started reset the variables
        clearPowerStrip(); // clear all led's
        shutdown_leds();
        startup = false;
        safety = false;
        fire = false;
      }
    }
  }
  delay(1);
}

/*************** Wand Light Helpers *********************/
unsigned long prevFlashMillis = 0; // last time we changed a powercell light in the idle sequence
bool flashState = false;
int wandFastFlashInterval = 100; // interval at which we flash the top led on the wand
int wandMediumFlashInterval = 500; // interval at which we flash the top led on the wand

void setWandLightState(int lednum, int state, int currentMillis){
  switch ( state ) {
    case 0: // set led red
      wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
      break;
    case 1: // set led white
      wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
      break;
    case 2: // set led orange
      wandLights.setPixelColor(lednum, wandLights.Color(255, 127, 0));
      break;
    case 3: // set led blue
      wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 255));
      break;
    case 4: // set led off
      wandLights.setPixelColor(lednum, 0);
      break;
    case 5: // fast white flashing    
      if (currentMillis - prevFlashMillis > wandFastFlashInterval) {    
        prevFlashMillis = currentMillis;    
        if( flashState == false ){    
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));    
          flashState = true;    
        }else{    
          wandLights.setPixelColor(lednum, 0);    
          flashState = false;   
        }   
      }   
      break;
    case 6: // slower orange flashing    
      if (currentMillis - prevFlashMillis > wandMediumFlashInterval) {    
        prevFlashMillis = currentMillis;    
        if( flashState == false ){    
          wandLights.setPixelColor(lednum, wandLights.Color(255, 127, 0));    
          flashState = true;    
        }else{    
          wandLights.setPixelColor(lednum, 0);    
          flashState = false;   
        }   
      }   
      break;
    case 7: // medium red flashing    
      if (currentMillis - prevFlashMillis > wandMediumFlashInterval) {    
        prevFlashMillis = currentMillis;    
        if( flashState == false ){    
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));    
          flashState = true;    
        }else{    
          wandLights.setPixelColor(lednum, 0);    
          flashState = false;   
        }   
      }   
      break;
    case 8: // fast red flashing    
      if (currentMillis - prevFlashMillis > wandFastFlashInterval) {    
        prevFlashMillis = currentMillis;    
        if( flashState == false ){    
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));    
          flashState = true;    
        }else{    
          wandLights.setPixelColor(lednum, 0);    
          flashState = false;   
        }   
      }   
      break;
  }

  wandLights.show();
}

/*************** Vent Light *************************/
void setVentLightState(int startLed, int endLed, int state ){
  switch ( state ) {
    case 0: // set all leds to white
      for(int i=startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, powerStick.Color(255, 255, 255));
      }
      break;
    case 1: // set all leds to blue
      for(int i=startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, powerStick.Color(0, 0, 255));
      }
      break;
    case 2: // set all leds off
      for(int i=startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, 0);
      }
      break;
  }
}

/*************** Powercell/Cyclotron Animations *********************/
// timer helpers and intervals for the animations
unsigned long prevPwrBootMillis = 0;    // the last time we changed a powercell light in the boot sequence
const int pwr_boot_interval = 41;       // interval at which to cycle lights (milliseconds).

unsigned long prevCycBootMillis = 0;    // the last time we changed a cyclotron light in the boot sequence
const int cyc_boot_interval = 400;      // interval at which to cycle lights (milliseconds).
const int cyc_boot_alt_interval = 600;      // interval at which to cycle lights (milliseconds).

unsigned long prevShtdMillis = 0;       // last time we changed a light in the idle sequence
const int pwr_shutdown_interval = 200;  // interval at which to cycle lights (milliseconds).

unsigned long prevPwrMillis = 0;        // last time we changed a powercell light in the idle sequence
unsigned long prevCycMillis = 0;        // last time we changed a cyclotron light in the idle sequence
unsigned long prevFadeCycMillis = 0;    // last time we changed a cyclotron light in the idle sequence

// LED tracking variables
const int powerSeqTotal = powercellLedCount;                              // total number of led's for powercell 0 based
int powerSeqNum = powercellIndexOffset;                                   // current running powercell sequence led
int powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;       // shutdown sequence counts down

// animation level trackers for the boot and shutdown
int currentBootLevel = powercellIndexOffset;                              // current powercell boot level sequence led
int currentLightLevel = powercellLedCount - powercellIndexOffset;         // current powercell boot light sequence led

void setCyclotronLightState(int startLed, int endLed, int state ){
  switch ( state ) {
    case 0: // set all leds to red
      for(int i=startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, powerStick.Color(255, 0, 0));
      }
      break;
    case 1: // set all leds to orange
      for(int i=startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, powerStick.Color(255, 106, 0));
      }
      break;
    case 2: // set all leds off
      for(int i=startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, 0);
      }
      break;
    case 3: // fade all leds from red
      for(int i=startLed; i <= endLed; i++) {
        if( cyclotronRunningFadeOut >= 0 ){
          powerStick.setPixelColor(i, 255 * cyclotronRunningFadeOut/255, 0, 0);
          cyclotronRunningFadeOut--;
        }else{
          powerStick.setPixelColor(i, 0);
        }
      }
      break;
    case 4: // fade all leds to red
      for(int i=startLed; i <= endLed; i++) {
        if( cyclotronRunningFadeIn < 255 ){
          powerStick.setPixelColor(i, 255 * cyclotronRunningFadeIn/255, 0, 0);
          cyclotronRunningFadeIn++;
        }else{
          powerStick.setPixelColor(i, powerStick.Color(255, 0, 0));
        }
      }
      break;
  }
}

// shuts off and resets the powercell/cyclotron leds
void clearPowerStrip() {
  // reset vars
  powerBooted = false;
  poweredDown = true;
  powerSeqNum = powercellIndexOffset;
  powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;
  currentLightLevel = powercellLedCount;
  currentBootLevel = powercellIndexOffset;
  cyclotronRunningFadeIn = 0;
  
  // shutoff the leds
  for ( int i = 0; i <= c4End; i++) {
    powerStick.setPixelColor(i, 0);
  }
  powerStick.show();

  for ( int j=0; j<=3; j++ ){
    wandLights.setPixelColor(j, 0);
  }
  wandLights.show();

  if( venting == true ){
    setVentLightState(ventStart, ventEnd, 0);
  }
}

// boot animation on the powercell/cyclotron
bool reverseBootCyclotron = false;
void powerSequenceBoot(int currentMillis) {
  bool doUpdate = false;

  // START CYCLOTRON
  if( useCyclotronFadeInEffect == false ){
    if (currentMillis - prevCycBootMillis > cyc_boot_interval) {
      prevCycBootMillis = currentMillis;

      if( reverseBootCyclotron == false ){
        setCyclotronLightState(c1Start, c1End, 1);
        setCyclotronLightState(c2Start, c2End, 2);
        setCyclotronLightState(c3Start, c3End, 1);
        setCyclotronLightState(c4Start, c4End, 2);
        
        doUpdate = true;
        reverseBootCyclotron = true;
      }else{
        setCyclotronLightState(c1Start, c1End, 2);
        setCyclotronLightState(c2Start, c2End, 1);
        setCyclotronLightState(c3Start, c3End, 2);
        setCyclotronLightState(c4Start, c4End, 1);
        
        doUpdate = true;
        reverseBootCyclotron = false;
      }
    }
  }else{
    if (currentMillis - prevCycBootMillis > cyc_boot_alt_interval) {
      prevCycBootMillis = currentMillis;
      setCyclotronLightState(c1Start, c4End, 4);
      doUpdate = true;
    }
  }
  // END CYCLOTRON
  
  if (currentMillis - prevPwrBootMillis > pwr_boot_interval) {
    // save the last time you blinked the LED
    prevPwrBootMillis = currentMillis;

    // START POWERCELL
    if( currentBootLevel != powerSeqTotal ){
      if( currentBootLevel == currentLightLevel){
        if(currentLightLevel+1 <= powerSeqTotal){
          powerStick.setPixelColor(currentLightLevel+1, 0);
        }
        powerStick.setPixelColor(currentBootLevel, powerStick.Color(0, 0, 255));
        currentLightLevel = powerSeqTotal;
        currentBootLevel++;
      }else{
        if(currentLightLevel+1 <= powerSeqTotal){
          powerStick.setPixelColor(currentLightLevel+1, 0);
        }
        powerStick.setPixelColor(currentLightLevel, powerStick.Color(0, 0, 255));
        currentLightLevel--;
      }
      doUpdate = true;
    }else{
      powerBooted = true;
      currentBootLevel = powercellIndexOffset;
      currentLightLevel = powercellLedCount - powercellIndexOffset;
    }
    // END POWERCELL
  }

  // if we have changed an led
  if( doUpdate == true ){
    powerStick.show(); // commit all of the changes
  }
}

// idle/firing animation for the powercell/cyclotron
int cycOrder = 0;     // which cyclotron led will be lit next
int cycFading = -1;   // which cyclotron led is fading out for game style
void powerSequenceOne(int currentMillis, int anispeed, int cycspeed, int cycfadespeed) {
  bool doUpdate = false;  // keep track of if we changed something so we only update on changes

  // START CYCLOTRON 
  if( useGameCyclotronEffect == true ){ // if we are doing the video game style cyclotron
    if (currentMillis - prevCycMillis > cycspeed) {
      prevCycMillis = currentMillis;
      
      switch ( cycOrder ) {
        case 0:
          setCyclotronLightState(c1Start, c1End, 0);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c3Start, c3End, 2);
          cycFading = 0;
          cyclotronRunningFadeOut = 255;
          cycOrder = 1;
          break;
        case 1:
          setCyclotronLightState(c2Start, c2End, 0);
          setCyclotronLightState(c3Start, c3End, 2);
          setCyclotronLightState(c4Start, c4End, 2);
          cycFading = 1;
          cyclotronRunningFadeOut = 255;
          cycOrder = 2;
          break;
        case 2:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c3Start, c3End, 0);
          setCyclotronLightState(c4Start, c4End, 2);
          cycFading = 2;
          cyclotronRunningFadeOut = 255;
          cycOrder = 3;
          break;
        case 3:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c4Start, c4End, 0);
          cycFading = 3;
          cyclotronRunningFadeOut = 255;
          cycOrder = 0;
          break;
      }
  
      doUpdate = true;
    }
  
    // now figure out the fading light
    if( currentMillis - prevFadeCycMillis > cycfadespeed ){
      prevFadeCycMillis = currentMillis;
      if( cycFading != -1 ){
        switch ( cycFading ) {
          case 0:
            setCyclotronLightState(c4Start, c4End, 3);
            break;
          case 1:
            setCyclotronLightState(c1Start, c1End, 3);
            break;
          case 2:
            setCyclotronLightState(c2Start, c2End, 3);
            break;
          case 3:
            setCyclotronLightState(c3Start, c3End, 3);
            break;
        }
        doUpdate = true;
      }
    }
  }else{ // otherwise this is the standard version
    if (currentMillis - prevCycMillis > cycspeed) {
      prevCycMillis = currentMillis;
      
      switch ( cycOrder ) {
        case 0:
          setCyclotronLightState(c4Start, c4End, 2);
          setCyclotronLightState(c1Start, c1End, 0);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c3Start, c3End, 2);
          cycFading = 0;
          cyclotronRunningFadeOut = 255;
          cycOrder = 1;
          break;
        case 1:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c2Start, c2End, 0);
          setCyclotronLightState(c3Start, c3End, 2);
          setCyclotronLightState(c4Start, c4End, 2);
          cycFading = 1;
          cyclotronRunningFadeOut = 255;
          cycOrder = 2;
          break;
        case 2:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c3Start, c3End, 0);
          setCyclotronLightState(c4Start, c4End, 2);
          cycFading = 2;
          cyclotronRunningFadeOut = 255;
          cycOrder = 3;
          break;
        case 3:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c3Start, c3End, 2);
          setCyclotronLightState(c4Start, c4End, 0);
          cycFading = 3;
          cyclotronRunningFadeOut = 255;
          cycOrder = 0;
          break;
      }
  
      doUpdate = true;
    }
  }
  // END CYCLOTRON

  // START POWERCELL
  if (currentMillis - prevPwrMillis > anispeed) {
    // save the last time you blinked the LED
    prevPwrMillis = currentMillis;

    for ( int i = powercellIndexOffset; i <= powerSeqTotal; i++) {
      if ( i <= powerSeqNum ) {
        powerStick.setPixelColor(i, powerStick.Color(0, 0, 150));
      } else {
        powerStick.setPixelColor(i, 0);
      }
    }
    
    if ( powerSeqNum <= powerSeqTotal) {
      powerSeqNum++;
    } else {
      powerSeqNum = powercellIndexOffset;
    }

    doUpdate = true;
  }
  // END POWERCELL

  // if we changed anything update
  if( doUpdate == true ){
    powerStick.show();
  }
}

// shutdown animation for the powercell/cyclotron
int cyclotronFadeOut = 255;
void powerSequenceShutdown(int currentMillis) {
  if (currentMillis - prevShtdMillis > pwr_shutdown_interval) {
    prevShtdMillis = currentMillis;

    // START CYCLOTRON
    for(int i=c1Start; i <= c4End; i++) {
      if( cyclotronFadeOut >= 0 ){
        powerStick.setPixelColor(i, 255 * cyclotronFadeOut/255, 0, 0);
        cyclotronFadeOut--;
      }else{
        powerStick.setPixelColor(i, 0);
      }
    }
    // END CYCLOTRON
    
    // START POWERCELL
    for ( int i = powerSeqTotal; i >= powercellIndexOffset; i--) {
      if ( i <= powerShutdownSeqNum ) {
        powerStick.setPixelColor(i, powerStick.Color(0, 0, 150));
      } else {
        powerStick.setPixelColor(i, 0);
      }
    }
    
    powerStick.show();
    
    if ( powerShutdownSeqNum >= powercellIndexOffset) {
      powerShutdownSeqNum--;
    } else {
      poweredDown = true;
      powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;
      cyclotronFadeOut = 255;
    }
    // END POWERCELL
  }
}

/*************** Nose Jewel Firing Animations *********************/
unsigned long prevFireMillis = 0;
const int fire_interval = 50;     // interval at which to cycle lights (milliseconds).
int fireSeqNum = 0;
int fireSeqTotal = 5;

void clearFireStrobe() {
  for ( int i = 0; i < 7; i++) {
    noseJewel.setPixelColor(i, 0);
  }
  noseJewel.show();
  fireSeqNum = 0;
}

void fireStrobe(int currentMillis) {
  if (currentMillis - prevFireMillis > fire_interval) {
    prevFireMillis = currentMillis;
    
    switch ( fireSeqNum ) {
      case 0:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(2, 0);
        noseJewel.setPixelColor(3, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(4, 0);
        noseJewel.setPixelColor(5, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(6, 0);
        break;
      case 1:
        noseJewel.setPixelColor(0, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 2:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(1, 0);
        noseJewel.setPixelColor(2, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(3, 0);
        noseJewel.setPixelColor(4, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(5, 0);
        noseJewel.setPixelColor(6, noseJewel.Color(255, 0, 0));
        break;
      case 3:
        noseJewel.setPixelColor(0, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 4:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(1, 0);
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, 0);
        noseJewel.setPixelColor(4, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(5, 0);
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 5:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(0, 255, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(3, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 0, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(6, noseJewel.Color(0, 0, 255));
        break;
    }
  
    noseJewel.show();
  
    fireSeqNum++;
    if ( fireSeqNum > fireSeqTotal ) {
      fireSeqNum = 0;
    }
  }
}

/*************** Bar Graph Animations *********************/
void shutdown_leds() {
  // stub function for when I re-enable to bargraph
}
void barGraphSequenceOne(int currentMillis) {
  // stub function for when I re-enable to bargraph
}
void barGraphSequenceTwo(int currentMillis) {
  // stub function for when I re-enable to bargraph
}

