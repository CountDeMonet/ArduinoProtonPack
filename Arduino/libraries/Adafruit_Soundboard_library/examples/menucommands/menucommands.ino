/* 
  Menu driven control of a sound board over UART.
  Commands for playing by # or by name (full 11-char name)
  Hard reset and List files (when not playing audio)
  Vol + and - (only when not playing audio)
  Pause, unpause, quit playing (when playing audio)
  Current play time, and bytes remaining & total bytes (when playing audio)

  Connect UG to ground to have the sound board boot into UART mode
*/

#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"


// Choose any two pins that can be used with SoftwareSerial to RX & TX
#define SFX_TX 5
#define SFX_RX 6

// Connect to the RST pin on the Sound Board
#define SFX_RST 4

// You can also monitor the ACT pin for when audio is playing!

// we'll be using software serial
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);

// pass the software serial to Adafruit_soundboard, the second
// argument is the debug port (not used really) and the third 
// arg is the reset pin
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);
// can also try hardware serial with
// Adafruit_Soundboard sfx = Adafruit_Soundboard(&Serial1, NULL, SFX_RST);

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit Sound Board!");
  
  // softwareserial at 9600 baud
  ss.begin(9600);
  // can also do Serial1.begin(9600)

  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
  Serial.println("SFX board found");
}


void loop() {
  flushInput();
  
  Serial.println(F("What would you like to do?"));
  Serial.println(F("[r] - reset"));
  Serial.println(F("[+] - Vol +"));
  Serial.println(F("[-] - Vol -"));
  Serial.println(F("[L] - List files"));
  Serial.println(F("[P] - play by file name"));
  Serial.println(F("[#] - play by file number"));
  Serial.println(F("[=] - pause playing"));
  Serial.println(F("[>] - unpause playing"));
  Serial.println(F("[q] - stop playing"));
  Serial.println(F("[t] - playtime status"));
  Serial.println(F("> "));
  
  while (!Serial.available());
  char cmd = Serial.read();
  
  flushInput();
  
  switch (cmd) {
    case 'r': {
      if (!sfx.reset()) {
        Serial.println("Reset failed");
      }
      break; 
    }
    
    case 'L': {
      uint8_t files = sfx.listFiles();
    
      Serial.println("File Listing");
      Serial.println("========================");
      Serial.println();
      Serial.print("Found "); Serial.print(files); Serial.println(" Files");
      for (uint8_t f=0; f<files; f++) {
        Serial.print(f); 
        Serial.print("\tname: "); Serial.print(sfx.fileName(f));
        Serial.print("\tsize: "); Serial.println(sfx.fileSize(f));
      }
      Serial.println("========================");
      break; 
    }
    
    case '#': {
      Serial.print("Enter track #");
      uint8_t n = readnumber();

      Serial.print("\nPlaying track #"); Serial.println(n);
      if (! sfx.playTrack(n) ) {
        Serial.println("Failed to play track?");
      }
      break;
    }
    
    case 'P': {
      Serial.print("Enter track name (full 12 character name!) >");
      char name[20];
      readline(name, 20);

      Serial.print("\nPlaying track \""); Serial.print(name); Serial.print("\"");
      if (! sfx.playTrack(name) ) {
        Serial.println("Failed to play track?");
      }
      break;
   }

   case '+': {
      Serial.println("Vol up...");
      uint16_t v;
      if (! (v = sfx.volUp()) ) {
        Serial.println("Failed to adjust");
      } else {
        Serial.print("Volume: "); Serial.println(v);
      }
      break;
   }

   case '-': {
      Serial.println("Vol down...");
      uint16_t v;
      if (! (v=sfx.volDown()) ) {
        Serial.println("Failed to adjust");
      } else { 
        Serial.print("Volume: "); 
        Serial.println(v);
      }
      break;
   }
   
   case '=': {
      Serial.println("Pausing...");
      if (! sfx.pause() ) Serial.println("Failed to pause");
      break;
   }
   
   case '>': {
      Serial.println("Unpausing...");
      if (! sfx.unpause() ) Serial.println("Failed to unpause");
      break;
   }
   
   case 'q': {
      Serial.println("Stopping...");
      if (! sfx.stop() ) Serial.println("Failed to stop");
      break;
   }  

   case 't': {
      Serial.print("Track time: ");
      uint32_t current, total;
      if (! sfx.trackTime(&current, &total) ) Serial.println("Failed to query");
      Serial.print(current); Serial.println(" seconds");
      break;
   }  

   case 's': {
      Serial.print("Track size (bytes remaining/total): ");
      uint32_t remain, total;
      if (! sfx.trackSize(&remain, &total) ) 
        Serial.println("Failed to query");
      Serial.print(remain); Serial.print("/"); Serial.println(total); 
      break;
   }  

  }
}






/************************ MENU HELPERS ***************************/

void flushInput() {
  // Read all available serial input to flush pending data.
  uint16_t timeoutloop = 0;
  while (timeoutloop++ < 40) {
    while(ss.available()) {
      ss.read();
      timeoutloop = 0;  // If char was received reset the timer
    }
    delay(1);
  }
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}

uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff) {
  uint16_t buffidx = 0;
  
  while (true) {
    if (buffidx > maxbuff) {
      break;
    }

    if (Serial.available()) {
      char c =  Serial.read();
      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0) {  // the first 0x0A is ignored
          continue;
        }
        buff[buffidx] = 0;  // null term
        return buffidx;
      }
      buff[buffidx] = c;
      buffidx++;
    }
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
/************************ MENU HELPERS ***************************/