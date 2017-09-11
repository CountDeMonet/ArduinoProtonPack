/*************************************************** 
  This is a library for the Adafruit Sound Boards in UART mode

  ----> http://www.adafruit.com/products/2342
  ----> http://www.adafruit.com/products/2341
  ----> http://www.adafruit.com/products/2217
  ----> http://www.adafruit.com/products/2210
  ----> http://www.adafruit.com/products/2133
  ----> http://www.adafruit.com/products/2220

  Check out the links above for our tutorials and wiring diagrams
  This sound fx driver uses TTL Serial to communicate

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#ifndef _ADAFRUIT_SOUNDBOARD_H_
#define _ADAFRUIT_SOUNDBOARD_H_

#include "Arduino.h"

#define LINE_BUFFER_SIZE  80
#define MAXFILES 25

class Adafruit_Soundboard : public Print {
 public:
  Adafruit_Soundboard(Stream *s, Stream *d, int8_t r);

  boolean reset(void);

  int     readLine(void);
  uint8_t listFiles(void);

  char *fileName(uint8_t n);
  uint32_t fileSize(uint8_t n);


  uint8_t volUp(void);
  uint8_t volDown(void);

  boolean playTrack(uint8_t n);
  boolean playTrack(char *name);
  boolean pause(void);
  boolean unpause(void);
  boolean stop(void);

  boolean trackTime(uint32_t *current, uint32_t *total);
  boolean trackSize(uint32_t *current, uint32_t *total);

 private:
  Stream   *stream;     // -> sound board, e.g. SoftwareSerial or Serial1
  Stream    *debug;      // -> host, e.g. Serial
  
  int8_t reset_pin;
  char line_buffer[LINE_BUFFER_SIZE];
  boolean writing;

  // File name & size caching
  uint8_t files;
  char filenames[MAXFILES][12];
  uint32_t filesizes[MAXFILES];


  virtual size_t write(uint8_t); // Because Print subclass
};
#endif
