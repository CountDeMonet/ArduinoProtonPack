#include <Adafruit_NeoPixel.h>

#define NEO_POWER 2 // for cyclotron and powercell
Adafruit_NeoPixel powerStick = Adafruit_NeoPixel(44, NEO_POWER, NEO_GRB + NEO_KHZ800);

// ##############################
// available options
// ##############################
const bool useGameCyclotronEffect = true; // set this to true to get the fading previous cyclotron light in the idle sequence

void setup() {
  // configure powercell/cyclotron
  powerStick.begin();
  powerStick.setBrightness(80);
  powerStick.show(); // Initialize all pixels to 'off'
}

int pwr_interval = 60;       // interval at which to cycle lights for the powercell.
int cyc_interval = 1000;      // interval at which to cycle lights for the cyclotron.
int cyc_fade_interval = 15;   // fade the inactive cyclotron to light to nothing

void loop() {
  // get the current time
  int currentMillis = millis();

  powerSequenceOne(currentMillis, pwr_interval, cyc_interval, cyc_fade_interval);
}

int cyclotronRunningFadeOut = 255;
void setCyclotronLightState(int startLed, int endLed, int state ){
  switch ( state ) {
    case 0:
      for(int i=startLed; i <= endLed; i++) { // red
        powerStick.setPixelColor(i, powerStick.Color(150, 0, 0));
      }
      break;
    case 1:
      for(int i=startLed; i <= endLed; i++) { // orange
        powerStick.setPixelColor(i, powerStick.Color(255, 106, 0));
      }
      break;
    case 2:
      for(int i=startLed; i <= endLed; i++) { // off
        powerStick.setPixelColor(i, 0);
      }
      break;
    case 3:
      for(int i=startLed; i <= endLed; i++) {
        if( cyclotronRunningFadeOut >= 0 ){
          powerStick.setPixelColor(i, 255 * cyclotronRunningFadeOut/255, 0, 0);
          cyclotronRunningFadeOut--;
        }else{
          powerStick.setPixelColor(i, 0);
        }
      }
      break;
  }
}

/*************** Powercell/Cyclotron Animations *********************/
int c1Start = 16;
int c1End = 22;
int c2Start = 23;
int c2End = 29;
int c3Start = 30;
int c3End = 36;
int c4Start = 37;
int c4End = 44;

unsigned long prevPwrMillis = 0;        // last time we changed a powercell light in the idle sequence
unsigned long prevCycMillis = 0;        // last time we changed a cyclotron light in the idle sequence
unsigned long prevFadeCycMillis = 0;    // last time we changed a fading cyclotron light in the idle sequence

int powerSeqTotal = 15;       // total number of led's for powercell 0 based
int powerSeqNum = 0;          // current running powercell sequence led

int cycOrder = 0;
int cycFading = -1;

// normal animation on the bar graph
void powerSequenceOne(int currentMillis, int anispeed, int cycspeed, int cycfadespeed) {
  bool doUpdate = false;
  
  // START CYCLOTRON 
  if( useGameCyclotronEffect == true ){
    // figure out main light
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
  }else{
    // figure out main light
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

    
    for ( int i = 0; i <= powerSeqTotal; i++) {
      if ( i <= powerSeqNum ) {
        powerStick.setPixelColor(i, powerStick.Color(0, 0, 150));
      } else {
        powerStick.setPixelColor(i, 0);
      }
    }
    
    if ( powerSeqNum <= powerSeqTotal) {
      powerSeqNum++;
    } else {
      powerSeqNum = 0;
    }

    doUpdate = true;
  }
  // END POWERCELL

  // if we changed anything update
  if( doUpdate == true ){
    powerStick.show();
  }
}

