// Device macro for include proper headers
// Should be in consistence with Makefile constant
#define __AVR_ATmega328P__

// Device frequency
// Should be in consistence with Makefile constant
#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

// Override Arduino delays functions
#define delayMicroseconds(x) _delay_us(x)
#define delay(x) _delay_ms(x)

// Helper macros
#define setbits(port,mask)	(port)|=(mask)
#define clrbits(port,mask)	(port)&=~(mask)
#define tglbits(port,mask)	(port)^=(mask)
#define wrtbits(port,bits,mask) (port)=((port)&(~(mask)))|((bits)&(mask))

#define getbits(port,mask) ((port)&(mask))
#define is_bits(port,mask) (((port)&(mask))!=0)

#define HI(wd) ((BYTE)((0xff00&wd)>>8))
#define LO(wd) ((BYTE)(0x00ff&wd))

#define CHK_BUT(pin,mask) ((~(pin))&(mask))

// Auxiliary functions
uint8_t range_cycle(uint8_t min, uint8_t max, uint8_t val);

// Hardware managers
class Alarm;
class TempManager;
class LCD4Bit_mod;
class DS1307;
class UIManager;
class System;
class EpromManager;

extern Alarm ALARM;
extern TempManager TEMP;
extern LCD4Bit_mod LCD;
extern DS1307 RTC;
extern UIManager UI;
extern System SYSTEM;
extern EpromManager EEPROM;

#include "Arduino.h"
#include "HardwareSerial.h"