/******************************************************************************
SparkFunSX1509.cpp
SparkFun SX1509 I/O Expander Library Source File
Jim Lindblom @ SparkFun Electronics
Original Creation Date: September 21, 2015
https://github.com/sparkfun/SparkFun_SX1509_Arduino_Library

Here you'll find the Arduino code used to interface with the SX1509 I2C
16 I/O expander. There are functions to take advantage of everything the
SX1509 provides - input/output setting, writing pins high/low, reading 
the input value of pins, LED driver utilities (blink, breath, pwm), and
keypad engine utilites.

Development environment specifics:
	IDE: Arduino 1.6.5
	Hardware Platform: Arduino Uno
	SX1509 Breakout Version: v2.0

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include <Wire.h>
#include "Arduino.h"
#include "SparkFunSX1509.h"
#include "util/sx1509_registers.h"

SX1509::SX1509()
{
	_clkX = 0;
}

SX1509::SX1509(byte address, byte resetPin, byte interruptPin, byte oscillatorPin)
{
	// Store the received parameters into member variables
	deviceAddress =  address;
	pinInterrupt = interruptPin;
	pinOscillator = oscillatorPin;
	pinReset = resetPin;
}

byte SX1509::begin(byte address, byte resetPin)
{
	// Store the received parameters into member variables
	deviceAddress =  address;
	pinReset = resetPin;
	
	return init();
}

byte SX1509::init(void)
{
	// Begin I2C
	Wire.begin();
	
	// If the reset pin is connected
	if (pinReset != 255)
		reset(1);
	else
		reset(0);
		
	// Communication test. We'll read from two registers with different
	// default values to verify communication.
	unsigned int testRegisters = 0;
	testRegisters = readWord(REG_INTERRUPT_MASK_A);	// This should return 0xFF00
	
	// Then read a byte that should be 0x00
	if (testRegisters == 0xFF00)
	{
		// Set the clock to a default of 2MHz using internal
		clock(INTERNAL_CLOCK_2MHZ);
		
		return 1;
	}
	
	return 0;	
}

void SX1509::reset(bool hardware)
{
	// if hardware bool is set
	if (hardware)
	{
		// Check if bit 2 of REG_MISC is set
		// if so nReset will not issue a POR, we'll need to clear that bit first
		byte regMisc = readByte(REG_MISC);
		if (regMisc & (1<<2))
		{
			regMisc &= ~(1<<2);
			writeByte(REG_MISC, regMisc);
		}
		// Reset the SX1509, the pin is active low
		pinMode(pinReset, OUTPUT);	// set reset pin as output
		digitalWrite(pinReset, LOW);	// pull reset pin low
		delay(1);	// Wait for the pin to settle
		digitalWrite(pinReset, HIGH);	// pull reset pin back high
	}
	else
	{
		// Software reset command sequence:
		writeByte(REG_RESET, 0x12);
		writeByte(REG_RESET, 0x34);
	}
}

void SX1509::pinDir(byte pin, byte inOut)
{
	// The SX1509 RegDir registers: REG_DIR_B, REG_DIR_A
	//	0: IO is configured as an output
	//	1: IO is configured as an input
	byte modeBit;
	if ((inOut == OUTPUT) || (inOut == ANALOG_OUTPUT))
		modeBit = 0;
	else
		modeBit = 1;
	
	unsigned int tempRegDir = readWord(REG_DIR_B);
	if (modeBit)	
		tempRegDir |= (1<<pin);
	else
		tempRegDir &= ~(1<<pin);
	
	writeWord(REG_DIR_B, tempRegDir);
	
	// If INPUT_PULLUP was called, set up the pullup too:
	if (inOut == INPUT_PULLUP)
		writePin(pin, HIGH);
	
	if (inOut == ANALOG_OUTPUT)
	{
		ledDriverInit(pin);
	}
}

void SX1509::pinMode(byte pin, byte inOut)
{
	pinDir(pin, inOut);
}

void SX1509::writePin(byte pin, byte highLow)
{
	unsigned int tempRegDir = readWord(REG_DIR_B);
	
	if ((0xFFFF^tempRegDir)&(1<<pin))	// If the pin is an output, write high/low
	{
		unsigned int tempRegData = readWord(REG_DATA_B);
		if (highLow)	tempRegData |= (1<<pin);
		else			tempRegData &= ~(1<<pin);
		writeWord(REG_DATA_B, tempRegData);
	}
	else	// Otherwise the pin is an input, pull-up/down
	{
		unsigned int tempPullUp = readWord(REG_PULL_UP_B);
		unsigned int tempPullDown = readWord(REG_PULL_DOWN_B);
		
		if (highLow)	// if HIGH, do pull-up, disable pull-down
		{
			tempPullUp |= (1<<pin);
			tempPullDown &= ~(1<<pin);
			writeWord(REG_PULL_UP_B, tempPullUp);
			writeWord(REG_PULL_DOWN_B, tempPullDown);
		}
		else	// If LOW do pull-down, disable pull-up
		{
			tempPullDown |= (1<<pin);
			tempPullUp &= ~(1<<pin);
			writeWord(REG_PULL_UP_B, tempPullUp);
			writeWord(REG_PULL_DOWN_B, tempPullDown);
		}
	}
}

void SX1509::digitalWrite(byte pin, byte highLow)
{
	writePin(pin, highLow);
}

byte SX1509::readPin(byte pin)
{
	unsigned int tempRegDir = readWord(REG_DIR_B);
	
	if (tempRegDir & (1<<pin))	// If the pin is an input
	{
		unsigned int tempRegData = readWord(REG_DATA_B);
		if (tempRegData & (1<<pin))
			return 1;
	}
	
	return 0;
}

byte SX1509::digitalRead(byte pin)
{
	return readPin(pin);
}

void SX1509::ledDriverInit(byte pin, byte freq /*= 1*/, bool log /*= false*/)
{
	unsigned int tempWord;
	byte tempByte;
	
	// Disable input buffer
	// Writing a 1 to the pin bit will disable that pins input buffer
	tempWord = readWord(REG_INPUT_DISABLE_B);
	tempWord |= (1<<pin);
	writeWord(REG_INPUT_DISABLE_B, tempWord);
	
	// Disable pull-up
	// Writing a 0 to the pin bit will disable that pull-up resistor
	tempWord = readWord(REG_PULL_UP_B);
	tempWord &= ~(1<<pin);
	writeWord(REG_PULL_UP_B, tempWord);
	
	// Set direction to output (REG_DIR_B)
	tempWord = readWord(REG_DIR_B);
	tempWord &= ~(1<<pin); // 0=output	
	writeWord(REG_DIR_B, tempWord);
	
	// Enable oscillator (REG_CLOCK)
	tempByte = readByte(REG_CLOCK);
	tempByte |= (1<<6);	// Internal 2MHz oscillator part 1 (set bit 6)
	tempByte &= ~(1<<5);	// Internal 2MHz oscillator part 2 (clear bit 5)
	writeByte(REG_CLOCK, tempByte);
	
	// Configure LED driver clock and mode (REG_MISC)
	tempByte = readByte(REG_MISC);
	if (log)
	{
		tempByte |= (1<<7);	// set logarithmic mode bank B
		tempByte |= (1<<3);	// set logarithmic mode bank A
	}
	else
	{
		tempByte &= ~(1<<7);	// set linear mode bank B
		tempByte &= ~(1<<3);	// set linear mode bank A
	}
	
	// Use configClock to setup the clock divder
	if (_clkX == 0) // Make clckX non-zero
	{
		_clkX = 2000000.0 / (1<<(1 - 1)); // Update private clock variable
		
		byte freq = (1 & 0x07) << 4;	// freq should only be 3 bits from 6:4
		tempByte |= freq;
	}
	writeByte(REG_MISC, tempByte);
	
	// Enable LED driver operation (REG_LED_DRIVER_ENABLE)
	tempWord = readWord(REG_LED_DRIVER_ENABLE_B);
	tempWord |= (1<<pin);
	writeWord(REG_LED_DRIVER_ENABLE_B, tempWord);
	
	// Set REG_DATA bit low ~ LED driver started
	tempWord = readWord(REG_DATA_B);
	tempWord &= ~(1<<pin);
	writeWord(REG_DATA_B, tempWord);
}

void SX1509::pwm(byte pin, byte iOn)
{
	// Write the on intensity of pin
	// Linear mode: Ion = iOn
	// Log mode: Ion = f(iOn)
	writeByte(REG_I_ON[pin], iOn);
}

void SX1509::analogWrite(byte pin, byte iOn)
{
	pwm(pin, iOn);
}

void SX1509::blink(byte pin, unsigned long tOn, unsigned long tOff, byte onIntensity, byte offIntensity)
{
	byte onReg = calculateLEDTRegister(tOn);
	byte offReg = calculateLEDTRegister(tOff);
	
	setupBlink(pin, onReg, offReg, onIntensity, offIntensity, 0, 0);
}

void SX1509::breathe(byte pin, unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, byte onInt, byte offInt, bool log)
{
	offInt = constrain(offInt, 0, 7);
	
	byte onReg = calculateLEDTRegister(tOn);
	byte offReg = calculateLEDTRegister(tOff);
	
	byte riseTime = calculateSlopeRegister(rise, onInt, offInt);
	byte fallTime = calculateSlopeRegister(fall, onInt, offInt);
	
	setupBlink(pin, onReg, offReg, onInt, offInt, riseTime, fallTime, log);
}

void SX1509::setupBlink(byte pin, byte tOn, byte tOff, byte onIntensity, byte offIntensity, byte tRise, byte tFall, bool log)
{
	ledDriverInit(pin, log);
	
	// Keep parameters within their limits:
	tOn &= 0x1F;	// tOn should be a 5-bit value
	tOff &= 0x1F;	// tOff should be a 5-bit value
	offIntensity &= 0x07;
	// Write the time on
	// 1-15:  TON = 64 * tOn * (255/ClkX)
	// 16-31: TON = 512 * tOn * (255/ClkX)
	writeByte(REG_T_ON[pin], tOn);
	
	// Write the time/intensity off register
	// 1-15:  TOFF = 64 * tOff * (255/ClkX)
	// 16-31: TOFF = 512 * tOff * (255/ClkX)
	// linear Mode - IOff = 4 * offIntensity
	// log mode - Ioff = f(4 * offIntensity)
	writeByte(REG_OFF[pin], (tOff<<3) | offIntensity);
	
	// Write the on intensity:
	writeByte(REG_I_ON[pin], onIntensity);
	
	// Prepare tRise and tFall
	tRise &= 0x1F;	// tRise is a 5-bit value
	tFall &= 0x1F;	// tFall is a 5-bit value
	
	// Write regTRise
	// 0: Off
	// 1-15:  TRise =      (regIOn - (4 * offIntensity)) * tRise * (255/ClkX)
	// 16-31: TRise = 16 * (regIOn - (4 * offIntensity)) * tRise * (255/ClkX)
	if (REG_T_RISE[pin] != 0xFF)
		writeByte(REG_T_RISE[pin], tRise);
	// Write regTFall
	// 0: off
	// 1-15:  TFall =      (regIOn - (4 * offIntensity)) * tFall * (255/ClkX)
	// 16-31: TFall = 16 * (regIOn - (4 * offIntensity)) * tFall * (255/ClkX)
	if (REG_T_FALL[pin] != 0xFF)
		writeByte(REG_T_FALL[pin], tFall);
}

void SX1509::keypad(byte rows, byte columns, unsigned int sleepTime, byte scanTime, byte debounceTime)
{
	unsigned int tempWord;
	byte tempByte;
	
	// If clock hasn't been set up, set it to internal 2MHz
	if (_clkX == 0)
		clock(INTERNAL_CLOCK_2MHZ);
	
	// Set regDir 0:7 outputs, 8:15 inputs:
	tempWord = readWord(REG_DIR_B);
	for (int i=0; i<rows; i++)
		tempWord &= ~(1<<i);
	for (int i=8; i<(columns * 2); i++)
		tempWord |= (1<<i);
	writeWord(REG_DIR_B, tempWord);
	
	// Set regOpenDrain on 0:7:
	tempByte = readByte(REG_OPEN_DRAIN_A);
	for (int i=0; i<rows; i++)
		tempByte |= (1<<i);
	writeByte(REG_OPEN_DRAIN_A, tempByte);
	
	// Set regPullUp on 8:15:
	tempByte = readByte(REG_PULL_UP_B);
	for (int i=0; i<columns; i++)
		tempByte |= (1<<i);
	writeByte(REG_PULL_UP_B, tempByte);
	
	// Debounce Time must be less than scan time
	debounceTime = constrain(debounceTime, 1, 64);
	scanTime = constrain(scanTime, 1, 128);
	if (debounceTime >= scanTime)
	{
		debounceTime = scanTime >> 1; // Force debounceTime to be less than scanTime
	}
	debounceKeypad(debounceTime, rows, columns);
	
	// Calculate scanTimeBits, based on scanTime
	byte scanTimeBits = 0;
	for (byte i=7; i>0; i--)
	{
		if (scanTime & (1<<i))
		{
			scanTimeBits = i;
			break;
		}
	}
	
	// Calculate sleepTimeBits, based on sleepTime
	byte sleepTimeBits = 0;
	if (sleepTime != 0)
	{
		for (byte i=7; i>0; i--)
		{
			if (sleepTime & ((unsigned int)1<<(i+6)))
			{
				sleepTimeBits = i;
				break;
			}
		}
		// If sleepTime was non-zero, but less than 128, 
		// assume we wanted to turn sleep on, set it to minimum:
		if (sleepTimeBits == 0)
			sleepTimeBits = 1;
	}
	
	// RegKeyConfig1 sets the auto sleep time and scan time per row
	sleepTimeBits = (sleepTimeBits & 0b111)<<4;	
	scanTimeBits &= 0b111;	// Scan time is bits 2:0
	tempByte = sleepTime | scanTimeBits;
	writeByte(REG_KEY_CONFIG_1, tempByte);
	
	// RegKeyConfig2 tells the SX1509 how many rows and columns we've got going
	rows = (rows - 1) & 0b111;	// 0 = off, 0b001 = 2 rows, 0b111 = 8 rows, etc.
	columns = (columns - 1) & 0b111;	// 0b000 = 1 column, ob111 = 8 columns, etc.
	writeByte(REG_KEY_CONFIG_2, (rows << 3) | columns);
}

unsigned int SX1509::readKeypad()
{
	return readKeyData();
}

unsigned int SX1509::readKeyData()
{
	return (0xFFFF ^ readWord(REG_KEY_DATA_1));
}

byte SX1509::getRow(unsigned int keyData)
{
	byte rowData = byte(keyData & 0x00FF);
	
	for (int i=0; i<8; i++)
	{
		if (rowData & (1<<i))
			return i;
	}
	return 0;
}

byte SX1509::getCol(unsigned int keyData)
{
	byte colData = byte((keyData & 0xFF00) >> 8);
	
	for (int i=0; i<8; i++)
	{
		if (colData & (1<<i))
			return i;
	}
	return 0;
	
}

void SX1509::sync(void)
{
	// First check if nReset functionality is set
	byte regMisc = readByte(REG_MISC);
	if (!(regMisc & 0x04))
	{
		regMisc |= (1<<2);
		writeByte(REG_MISC, regMisc);
	}
	
	// Toggle nReset pin to sync LED timers
	pinMode(pinReset, OUTPUT);	// set reset pin as output
	digitalWrite(pinReset, LOW);	// pull reset pin low
	delay(1);	// Wait for the pin to settle
	digitalWrite(pinReset, HIGH);	// pull reset pin back high	
	
	// Return nReset to POR functionality
	writeByte(REG_MISC, (regMisc & ~(1<<2)));
}

void SX1509::debounceConfig(byte configValue)
{
	// First make sure clock is configured
	byte tempByte = readByte(REG_MISC);
	if ((tempByte & 0x70) == 0)
	{
		tempByte |= (1<<4);	// Just default to no divider if not set
		writeByte(REG_MISC, tempByte);
	}
	tempByte = readByte(REG_CLOCK);
	if ((tempByte & 0x60) == 0)
	{
		tempByte |= (1<<6);	// default to internal osc.
		writeByte(REG_CLOCK, tempByte);
	}
	
	configValue &= 0b111;	// 3-bit value
	writeByte(REG_DEBOUNCE_CONFIG, configValue);
}

void SX1509::debounceTime(byte time)
{
	if (_clkX == 0) // If clock hasn't been set up.
		clock(INTERNAL_CLOCK_2MHZ, 1); // Set clock to 2MHz.
	
	// Debounce time-to-byte map: (assuming fOsc = 2MHz)
	// 0: 0.5ms		1: 1ms
	// 2: 2ms		3: 4ms
	// 4: 8ms		5: 16ms
	// 6: 32ms		7: 64ms
	// 2^(n-1)
	byte configValue = 0;
	// We'll check for the highest set bit position, 
	// and use that for debounceConfig
	for (int i=7; i>=0; i--)
	{
		if (time & (1<<i))
		{
			configValue = i + 1;
			break;
		}
	}
	configValue = constrain(configValue, 0, 7);
	
	debounceConfig(configValue);
}

void SX1509::debounceEnable(byte pin)
{
	unsigned int debounceEnable = readWord(REG_DEBOUNCE_ENABLE_B);
	debounceEnable |= (1<<pin);
	writeWord(REG_DEBOUNCE_ENABLE_B, debounceEnable);
}

void SX1509::debouncePin(byte pin)
{
	debounceEnable(pin);
}

void SX1509::debounceKeypad(byte time, byte numRows, byte numCols)
{
	// Set up debounce time:
	debounceTime(time);
	
	// Set up debounce pins:
	for (int i = 0; i < numRows; i++)
		debouncePin(i);
	for (int i = 0; i < (8 + numCols); i++)
		debouncePin(i);
}

void SX1509::enableInterrupt(byte pin, byte riseFall)
{
	// Set REG_INTERRUPT_MASK
	unsigned int tempWord = readWord(REG_INTERRUPT_MASK_B);
	tempWord &= ~(1<<pin);	// 0 = event on IO will trigger interrupt
	writeWord(REG_INTERRUPT_MASK_B, tempWord);
	
	byte sensitivity = 0;
	switch (riseFall)
	{
	case CHANGE:
		sensitivity = 0b11;
		break;
	case FALLING:
		sensitivity = 0b10;
		break;
	case RISING:
		sensitivity = 0b01;
		break;
	}
	
	// Set REG_SENSE_XXX
	// Sensitivity is set as follows:
	// 00: None
	// 01: Rising
	// 10: Falling
	// 11: Both
	byte pinMask = (pin & 0x07) * 2;
	byte senseRegister;
	
	// Need to select between two words. One for bank A, one for B.
	if (pin >= 8)	senseRegister = REG_SENSE_HIGH_B;
	else			senseRegister = REG_SENSE_HIGH_A;
	
	tempWord = readWord(senseRegister);
	tempWord &= ~(0b11<<pinMask);	// Mask out the bits we want to write
	tempWord |= (sensitivity<<pinMask);	// Add our new bits
	writeWord(senseRegister, tempWord);
}

unsigned int SX1509::interruptSource(bool clear /* =true*/)
{
	unsigned int intSource = readWord(REG_INTERRUPT_SOURCE_B);
	if (clear)
		writeWord(REG_INTERRUPT_SOURCE_B, 0xFFFF);	// Clear interrupts
	return intSource;
}

bool SX1509::checkInterrupt(int pin)
{
	if (interruptSource(false) & (1<<pin))
		return true;
	
	return false;
}

void SX1509::clock(byte oscSource, byte oscDivider, byte oscPinFunction, byte oscFreqOut)
{
	configClock(oscSource, oscPinFunction, oscFreqOut, oscDivider);
}

void SX1509::configClock(byte oscSource /*= 2*/, byte oscPinFunction /*= 0*/, byte oscFreqOut /*= 0*/, byte oscDivider /*= 1*/)
{
	// RegClock constructed as follows:
	//	6:5 - Oscillator frequency souce
	//		00: off, 01: external input, 10: internal 2MHz, 1: reserved
	//	4 - OSCIO pin function
	//		0: input, 1 ouptut
	//	3:0 - Frequency of oscout pin
	//		0: LOW, 0xF: high, else fOSCOUT = FoSC/(2^(RegClock[3:0]-1))
	oscSource = (oscSource & 0b11)<<5;		// 2-bit value, bits 6:5
	oscPinFunction = (oscPinFunction & 1)<<4;	// 1-bit value bit 4
	oscFreqOut = (oscFreqOut & 0b1111);	// 4-bit value, bits 3:0
	byte regClock = oscSource | oscPinFunction | oscFreqOut;
	writeByte(REG_CLOCK, regClock);
	
	// Config RegMisc[6:4] with oscDivider
	// 0: off, else ClkX = fOSC / (2^(RegMisc[6:4] -1))
	oscDivider = constrain(oscDivider, 1, 7);
	_clkX = 2000000.0 / (1<<(oscDivider - 1)); // Update private clock variable
	oscDivider = (oscDivider & 0b111)<<4;	// 3-bit value, bits 6:4
	
	byte regMisc = readByte(REG_MISC);
	regMisc &= ~(0b111<<4);
	regMisc |= oscDivider;
	writeByte(REG_MISC, regMisc);
}

byte SX1509::calculateLEDTRegister(int ms)
{
	int regOn1, regOn2;
	float timeOn1, timeOn2;

	if (_clkX == 0)
		return 0;
	
	regOn1 = (float) (ms / 1000.0) / (64.0 * 255.0 / (float) _clkX);
	regOn2 = regOn1 / 8;
	regOn1 = constrain(regOn1, 1, 15);
	regOn2 = constrain(regOn2, 16, 31);
	
	timeOn1 = 64.0 * regOn1 * 255.0 / _clkX * 1000.0;
	timeOn2 = 512.0 * regOn2 * 255.0 / _clkX * 1000.0;

	if (abs(timeOn1 - ms) < abs(timeOn2 - ms))
		return regOn1;
	else
		return regOn2;
}

byte SX1509::calculateSlopeRegister(int ms, byte onIntensity, byte offIntensity)
{
	int regSlope1, regSlope2;
	float regTime1, regTime2;

	if (_clkX == 0)
		return 0;
	
	float tFactor = ((float) onIntensity - (4.0 * (float)offIntensity)) * 255.0 / (float) _clkX;
	float timeS = float(ms) / 1000.0;
	
	regSlope1 = timeS / tFactor;
	regSlope2 = regSlope1 / 16;
	
	regSlope1 = constrain(regSlope1, 1, 15);
	regSlope2 = constrain(regSlope2, 16, 31);

	regTime1 = regSlope1 * tFactor * 1000.0;
	regTime2 = 16 * regTime1;

	if (abs(regTime1 - ms) < abs(regTime2 - ms))
		return regSlope1;
	else
		return regSlope2;
}

// readByte(byte registerAddress)
//	This function reads a single byte located at the registerAddress register.
//	- deviceAddress should already be set by the constructor.
//	- Return value is the byte read from registerAddress
//		- Currently returns 0 if communication has timed out
byte SX1509::readByte(byte registerAddress)
{
	byte readValue;
	unsigned int timeout = RECEIVE_TIMEOUT_VALUE;

	Wire.beginTransmission(deviceAddress);
	Wire.write(registerAddress);
	Wire.endTransmission();
	Wire.requestFrom(deviceAddress, (byte) 1);

	while ((Wire.available() < 1) && (timeout != 0))
		timeout--;
		
	if (timeout == 0)
		return 0;

	readValue = Wire.read();

	return readValue;
}

// readWord(byte registerAddress)
//	This function will read a two-byte word beginning at registerAddress
//	- A 16-bit unsigned int will be returned.
//		- The msb of the return value will contain the value read from registerAddress
//		- The lsb of the return value will contain the value read from registerAddress + 1
unsigned int SX1509::readWord(byte registerAddress)
{
	unsigned int readValue;
	unsigned int msb, lsb;
	unsigned int timeout = RECEIVE_TIMEOUT_VALUE * 2;

	Wire.beginTransmission(deviceAddress);
	Wire.write(registerAddress);
	Wire.endTransmission();
	Wire.requestFrom(deviceAddress, (byte) 2);

	while ((Wire.available() < 2) && (timeout != 0))
		timeout--;
		
	if (timeout == 0)
		return 0;
	
	msb = (Wire.read() & 0x00FF) << 8;
	lsb = (Wire.read() & 0x00FF);
	readValue = msb | lsb;

	return readValue;
}

// readBytes(byte firstRegisterAddress, byte * destination, byte length)
//	This function reads a series of bytes incrementing from a given address
//	- firstRegsiterAddress is the first address to be read
//	- destination is an array of bytes where the read values will be stored into
//	- length is the number of bytes to be read
//	- No return value.
void SX1509::readBytes(byte firstRegisterAddress, byte * destination, byte length)
{
	byte readValue;

	Wire.beginTransmission(deviceAddress);
	Wire.write(firstRegisterAddress);
	Wire.endTransmission();
	Wire.requestFrom(deviceAddress, length);
	
	while (Wire.available() < length)
		;
	
	for (int i=0; i<length; i++)
	{
		destination[i] = Wire.read();
	}
}

// writeByte(byte registerAddress, byte writeValue)
//	This function writes a single byte to a single register on the SX509.
//	- writeValue is written to registerAddress
//	- deviceAddres should already be set from the constructor
//	- No return value.
void SX1509::writeByte(byte registerAddress, byte writeValue)
{
	Wire.beginTransmission(deviceAddress);
	Wire.write(registerAddress);
	Wire.write(writeValue);
	Wire.endTransmission();
}

// writeWord(byte registerAddress, ungisnged int writeValue)
//	This function writes a two-byte word to registerAddress and registerAddress + 1
//	- the upper byte of writeValue is written to registerAddress
//		- the lower byte of writeValue is written to registerAddress + 1
//	- No return value.
void SX1509::writeWord(byte registerAddress, unsigned int writeValue)
{
	byte msb, lsb;
	msb = ((writeValue & 0xFF00) >> 8);
	lsb = (writeValue & 0x00FF);
	Wire.beginTransmission(deviceAddress);
	Wire.write(registerAddress);
	Wire.write(msb);
	Wire.write(lsb);
	Wire.endTransmission();	
}

// writeBytes(byte firstRegisterAddress, byte * writeArray, byte length)
//	This function writes an array of bytes, beggining at a specific adddress
//	- firstRegisterAddress is the initial register to be written.
//		- All writes following will be at incremental register addresses.
//	- writeArray should be an array of byte values to be written.
//	- length should be the number of bytes to be written.
//	- no return value.
void SX1509::writeBytes(byte firstRegisterAddress, byte * writeArray, byte length)
{
	Wire.beginTransmission(deviceAddress);
	Wire.write(firstRegisterAddress);
	for (int i=0; i<length; i++)
	{
		Wire.write(writeArray[i]);
	}
	Wire.endTransmission();
}