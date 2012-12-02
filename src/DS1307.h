/*
 *
 ********************************************************************************************************
 * 			DS1307.h 	 library for DS1307 I2C rtc clock				*
 ********************************************************************************************************
 *
 * Created by D. Sjunnesson 1scale1.com d.sjunnesson (at) 1scale1.com
 * Modified by bricofoy - bricofoy (at) free.fr
 *
 * Created with combined information from
 *  http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1180908809
 *  http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1191209057
 *
 * Big credit to  mattt (please contact me for a more correct name...) from the Arduino forum
 * which has written the main part of the library which I have modified
 *
 * Rev history :
 *
 * ??-??-??	- mattt & dsjunnesson 	- creation
 * 19-feb-2012	- bricofoy 		- added arduino 1.0 compatibility
 * 20-feb-2012	- bricofoy 		- bugfix : time was not preserved when setting or stopping clock
 * 21-feb-2012	- bricofoy 		- bugfix : preserve existing seconds when starting/stopping clock
 *
 *TODO: enable AM/PM
 *	enable square wave output
 *
 */


// ensure this library description is only included once
#ifndef DS1307_h
#define DS1307_h

// include types & constants of Wiring core API
// this "if" is for compatibility with both arduino 1.0 and previous versions
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// include types & constants of Wire ic2 lib
#include "Wire.h"

// library interface description
class DS1307
{
public:
  enum
  {
    SEC = 0,
    MIN = 1,
    HOUR = 2,
    DOW = 3,
    DATE = 4,
    MONTH = 5,
    YEAR = 6,
    BYTE_MAX = 7,

    SEC_LO,
    SEC_HI,
    MIN_LO,
    MIN_HI,
    HOUR_LO,
    HOUR_HI,
    DATE_LO,
    DATE_HI,
    MONTH_LO,
    MONTH_HI,
    YEAR_LO,
    YEAR_HI,
  };
  // user-accessible "public" interface
  public:
    DS1307();
    void getBuffer(byte*);
    byte get(byte);
    void set(byte, byte);
    void start(void);
    void stop(void);

    void read(void);
  // library-accessible "private" interface
  private:
    int rtc_bcd[BYTE_MAX]; // used prior to read/set ds1307 registers;
    void save(void);
};

extern DS1307 RTC;

#endif


