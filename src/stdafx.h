#define __AVR_ATmega328P__
#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

// Override Arduino delays functions
#define delayMicroseconds(x) _delay_us(x)
#define delay(x) _delay_ms(x)

#define setbits(port,mask)	(port)|=(mask)
#define clrbits(port,mask)	(port)&=~(mask)
#define tglbits(port,mask)	(port)^=(mask)
#define wrtbits(port,bits,mask) (port)=((port)&(~(mask)))|((bits)&(mask))

#define getbits(port,mask) ((port)&(mask))
#define is_bits(port,mask) (((port)&(mask))!=0)

#define HI(wd) ((BYTE)((0xff00&wd)>>8))
#define LO(wd) ((BYTE)(0x00ff&wd))

#define CHK_BUT(pin,mask) ((~(pin))&(mask))

/* ARDUINO ENVIRONMENT */
#include "Arduino.h"
#include "HardwareSerial.h"

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

/* ----------------------- hardware I/O abstraction ------------------------ */

/*
// temperature 1-wire sensors
#define TEMP_1WIRE _BV(PD2) // D2
#define TEMP_1WIRE_PORT PORTD

// light sensor
#define LIGHT_IN _BV(PC1) // A1
#define LIGHT_PORT PORTC

// real time clock
#define TIME_SDA _BV(PC4) // A4
#define TIME_SCL _BV(PC5) // A5
#define TIME_PORT PORTC

// LCD data bus
#define LCD_DATA_D4 _BV(PD4) // D4
#define LCD_DATA_D5 _BV(PD5) // D5
#define LCD_DATA_D6 _BV(PD6) // D6
#define LCD_DATA_D7 _BV(PD7) // D7
#define LCD_DATA (LCD_DATA_D4|LCD_DATA_D5|LCD_DATA_D6|LCD_DATA_D7)
#define LCD_DATA_PORT PORTD

// LCD register control
#define LCD_REG_RS _BV(PB0) // D8
#define LCD_REG_E _BV(PB1) // D9
#define LCD_REG (LCD_REG_RS|LCD_REG_E)
#define LCD_REG_PORT PORTB

// LCD keys
#define LCD_BUT_IN _BV(PC0) // A0
#define LCD_BUT_IN_PORT PORTC

// alarm output
#define LED _BV(PB5) // D13
#define LED_PORT PORTB

#define ALARM_OUT _BV(PB2)
#define ALARM_OUT_PORT PORTB
*/