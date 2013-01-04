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


// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime {
  // Source: https://github.com/adafruit/RTClib/blob/master/RTClib.h
public:
  DateTime(uint32_t t = 0);
  DateTime(uint16_t d = 0);
  DateTime(uint8_t yearOff, uint8_t month, uint8_t day,
    uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
  uint8_t year() const { return yOff; }
  uint8_t month() const { return m; }
  uint8_t day() const { return d; }
  uint8_t hour() const { return hh; }
  uint8_t minute() const { return mm; }
  uint8_t second() const { return ss; }

  uint8_t dayOfWeek() const;

  static byte daysPerMonth(byte month, byte year);

  uint16_t dayStamp() const; // 16-bit times as days since 1/1/2000
  uint32_t unixtime() const; // 32-bit times as seconds since 1/1/1970
private:
  void initDate(uint16_t days);
private:
  uint8_t yOff, m, d, hh, mm, ss;
};

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

    SEC_LO = 10,
    SEC_HI = 20,
    MIN_LO = 11,
    MIN_HI = 21,
    HOUR_LO = 12,
    HOUR_HI = 22,
    DATE_LO = 14,
    DATE_HI = 24,
    MONTH_LO = 15,
    MONTH_HI = 25,
    YEAR_LO = 16,
    YEAR_HI = 26,
  };
  // user-accessible "public" interface
  public:
    DS1307();
    void getBuffer(byte*);
    byte get(byte) const;
    DateTime getDateTime() const;

    const char* getDOWChars(byte dow) const;

    void set(byte, byte);
    void start(void);
    void stop(void);

    void read(void);
  // library-accessible "private" interface
  private:
    int rtc_bcd[BYTE_MAX]; // used prior to read/set ds1307 registers;
    void save(void);
};

#endif


