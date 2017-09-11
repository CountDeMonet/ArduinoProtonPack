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

#include "Adafruit_Soundboard.h"

//#define DEBUG 1

// Constructor
Adafruit_Soundboard::Adafruit_Soundboard(Stream *s, Stream *d, int8_t r) 
  : stream(s), debug(d), reset_pin(r) 
{
  stream->setTimeout(500);
  writing = false;
  files = 0;
}


int Adafruit_Soundboard::readLine(void) {
  int x = stream->readBytesUntil('\n', line_buffer, LINE_BUFFER_SIZE);
  line_buffer[x] = 0;

  if (stream->peek() == '\r') stream->read();
  //stream->readBytesUntil('\r', line_buffer, LINE_BUFFER_SIZE);
  return x;
}


// Anything printed to Soundboard object will be split to both the sound card
// and debug streams.  Saves having to print everything twice in debug code.
size_t Adafruit_Soundboard::write(uint8_t c) {
  if(debug) {
    if(!writing) {
      debug->print(F("---> "));
      writing = true;
    }
    debug->write(c);
  }
  return stream->write(c);
}

// Do a hard reset by bringing the RST pin low
// then read out the output lines
boolean Adafruit_Soundboard::reset(void) {
  digitalWrite(reset_pin, LOW);
  pinMode(reset_pin, OUTPUT);
  delay(10);
  pinMode(reset_pin, INPUT);
  delay(1000); // give a bit of time to 'boot up'
  
  // eat new line
  readLine();
#ifdef DEBUG
  Serial.println(line_buffer);   // Date and name
#endif

  readLine();
  // "Adafruit FX Sound Board 9/10/14"
#ifdef DEBUG
  Serial.println(line_buffer);   // Date and name
#endif
  if (! strstr(line_buffer, "Adafruit FX Sound Board")) return false;

  delay(250);

  readLine();
  //Serial.print("3>"); Serial.println(line_buffer);   // FAT type
  readLine();
  //Serial.print("4>"); Serial.println(line_buffer);   // # of files
  
  return true;
}



// Query the board for the # of files and names/sizes
uint8_t Adafruit_Soundboard::listFiles(void) {
  uint32_t filesize;

  while (stream->available())
    stream->read();

  stream->println('L'); // 'L' for 'l'ist

  files=0;
  
  while (stream->readBytesUntil('\n', line_buffer, LINE_BUFFER_SIZE)) {
    // copy over the file name
    memcpy(filenames[files], line_buffer, 12); 
    filenames[files][11] = 0;

    // parse out the file size after the name + tab
    filesizes[files] = 0;
    for (uint8_t i=0; i<16; i++) {
       uint8_t c = line_buffer[12 + i];
       if ((c > '9') || (c < '0')) {
           break;
       }
       
       filesizes[files] *= 10;
       
       filesizes[files] += c - '0';
    }
    
    files++;
    if (files >= MAXFILES) break;
  }
  return files;
}

// for public consumption
char * Adafruit_Soundboard::fileName(uint8_t n) {
  if (n >= files)  return NULL;

  return filenames[n];
}

uint32_t Adafruit_Soundboard::fileSize(uint8_t n) {
  if (n >= files)  return 0;

  return filesizes[n];
}


boolean Adafruit_Soundboard::playTrack(uint8_t n) {
  while (stream->available())
    stream->read();

  stream->print("#"); stream->println(n);

  readLine();  // eat return

  readLine();

#ifdef DEBUG
  Serial.print("<---"); Serial.println(line_buffer);
#endif

  // check we got "play" back
  if (strstr(line_buffer, "play") == 0) {
    return false;
  }
  // check the # is correct
  int playing = atoi(line_buffer+5);
#ifdef DEBUG
  Serial.print("# = "); Serial.println(playing);
#endif
  if (n != playing) return false;

  return true;
}

boolean Adafruit_Soundboard::playTrack(char *name) {
  while (stream->available())
    stream->read();

  stream->print("P"); stream->println(name);

  readLine();  // eat return
#ifdef DEBUG
  Serial.print("\n\r<--- "); Serial.println(line_buffer);
#endif

  readLine();

#ifdef DEBUG
  Serial.print("\n\r<--- "); Serial.println(line_buffer);
#endif

  // check we got "play" back
  if (strstr(line_buffer, "play") == 0) {
    return false;
  }
  return true;
}


uint8_t Adafruit_Soundboard::volUp() {
  while (stream->available())
    stream->read();

  stream->println("+");
  readLine();
  //Serial.println(line_buffer);

  uint8_t v = atoi(line_buffer);

  return v;
}

uint8_t Adafruit_Soundboard::volDown() {
  while (stream->available())
    stream->read();

  stream->println("-");
  readLine();
  //Serial.println(line_buffer);

  uint8_t v = atoi(line_buffer);

  return v;
}

boolean Adafruit_Soundboard::pause() {
  while (stream->available())
    stream->read();

  stream->print("=");
  if (! stream->readBytes(line_buffer, 1)) 
    return false;

  if (line_buffer[0] != '=')
    return false;
  return true;
}


boolean Adafruit_Soundboard::unpause() {
  while (stream->available())
    stream->read();

  stream->print(">");
  if (! stream->readBytes(line_buffer, 1)) 
    return false;

  if (line_buffer[0] != '>')
    return false;
  return true;
}

boolean Adafruit_Soundboard::stop() {
  while (stream->available())
    stream->read();

  stream->print("q");
  readLine();

  if (line_buffer[0] != 'q') 
    return false;

  return true;
}


boolean Adafruit_Soundboard::trackTime(uint32_t *current, uint32_t *total) {
  while (stream->available())
    stream->read();

  stream->print('t');
  readLine();
  //Serial.println(line_buffer);
  //Serial.println(strlen(line_buffer));
  if (strlen(line_buffer) != 12) return false;
  *current = atoi(line_buffer);
  *total = atoi(line_buffer+6);

  return true;
}

boolean Adafruit_Soundboard::trackSize(uint32_t *remain, uint32_t *total) {
  while (stream->available())
    stream->read();

  stream->print('s');
  readLine();
  //Serial.println(line_buffer);
  //Serial.println(strlen(line_buffer));
  if (strlen(line_buffer) != 22) return false;
  *remain = atol(line_buffer);
  *total = atol(line_buffer+11);

  return true;
}
