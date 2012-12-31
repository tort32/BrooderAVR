/*
 *
 ********************************************************************************************************
 * 			DS1307.cpp 	 library for DS1307 I2C rtc clock				*
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
#include "stdafx.h"
#include "DS1307.h"
#include "Wire.h"

#define CTRL_ID B1101000  //DS1307

// Define register bit masks
#define CLOCKHALT B10000000

#define LO_BCD  B00001111
#define HI_BCD  B11110000

#define HI_SEC  B01110000
#define HI_MIN  B01110000
#define HI_HR   B00110000
#define LO_DOW  B00000111
#define HI_DATE B00110000
#define HI_MTH  B00010000
#define HI_YR   B11110000

DS1307::DS1307()
{
  Wire.begin();
}

// PRIVATE FUNCTIONS

// Aquire data from the RTC chip in BCD format
// refresh the buffer
void DS1307::read(void)
{
  // use the Wire lib to connect to the rtc
  // reset the register pointer to zero
  Wire.beginTransmission(CTRL_ID);

  #if defined(ARDUINO) && ARDUINO >= 100
  Wire.write((byte)0x00);//workaround for issue #527
  #else
  Wire.send(0x00);
  #endif

  Wire.endTransmission();

  // request the 7 bytes of data    (secs, min, hr, dow, date. mth, yr)
  Wire.requestFrom(CTRL_ID, BYTE_MAX);
  for(int i=0; i<BYTE_MAX; i++)
  {
    // store data in raw bcd format
    #if defined(ARDUINO) && ARDUINO >= 100
    rtc_bcd[i]=Wire.read();
    #else
    rtc_bcd[i]=Wire.receive();
    #endif
  }
}

// update the data on the IC from the bcd formatted data in the buffer
void DS1307::save(void)
{
  Wire.beginTransmission(CTRL_ID);

  #if defined(ARDUINO) && ARDUINO >= 100
  Wire.write((byte)0x00);// reset register pointer
  #else
  Wire.send(0x00);
  #endif

  for(int i=0; i<BYTE_MAX; i++)
  {
    #if defined(ARDUINO) && ARDUINO >= 100
    Wire.write(rtc_bcd[i]);
    #else
    Wire.send(rtc_bcd[i]);
    #endif
  }
  Wire.endTransmission();
}

// PUBLIC FUNCTIONS
void DS1307::getBuffer(byte *rtc)   // Aquire data from buffer and convert to int
{
  for(byte i=0;i<BYTE_MAX;i++)  // cycle through each component, create array of data
  {
    rtc[i] = get(i);
  }
}

DateTime DS1307::getDateTime()
{
  return DateTime(get(YEAR),get(MONTH),get(DATE),get(HOUR),get(MIN),get(SEC));
}

byte DS1307::get(byte c)  // aquire individual RTC item from buffer, return as byte
{
  switch(c)
  {
  case SEC:
    return get(SEC_HI) * 10 + get(SEC_LO);
  case MIN:
    return get(MIN_HI) * 10 + get(MIN_LO);
  case HOUR:
    return get(HOUR_HI) * 10 + get(HOUR_LO);
  case DOW:
    return (rtc_bcd[DOW] & LO_DOW);
  case DATE:
    return get(DATE_HI) * 10 + get(DATE_LO);
  case MONTH:
    return get(MONTH_HI) * 10 + get(MONTH_LO);
  case YEAR:
    return get(YEAR_HI) * 10 + get(YEAR_LO);

  case SEC_LO:
    return (rtc_bcd[SEC] & LO_BCD);
  case SEC_HI:
    return (rtc_bcd[SEC] & HI_SEC) >> 4;
  case MIN_LO:
    return (rtc_bcd[MIN] & LO_BCD);
  case MIN_HI:
    return (rtc_bcd[MIN] & HI_MIN) >> 4;
  case HOUR_LO:
    return (rtc_bcd[HOUR] & LO_BCD);
  case HOUR_HI:
    return (rtc_bcd[HOUR] & HI_HR) >> 4;
  case DATE_LO:
    return (rtc_bcd[DATE] & LO_BCD);
  case DATE_HI:
    return (rtc_bcd[DATE] & HI_DATE) >> 4;
  case MONTH_LO:
    return (rtc_bcd[MONTH] & LO_BCD);
  case MONTH_HI:
    return (rtc_bcd[MONTH] & HI_MTH) >> 4;
  case YEAR_LO:
    return (rtc_bcd[YEAR] & LO_BCD);
  case YEAR_HI:
    return (rtc_bcd[YEAR] & HI_BCD) >> 4;
  } // end switch
  return 0;
}

static byte getNibbles(byte v)
{
  // tens go in HI nibble
  // units go in LO nibble
  return ((v / 10) << 4) + (v % 10);
}

void DS1307::set(byte c, byte v)  // Update buffer, then update the chip
{
  read(); //first read the buffer to preserve current time when writing modified values, as the function writes all values at the same time
  switch(c)
  {
  case SEC:
    wrtbits(rtc_bcd[SEC], getNibbles(v), HI_SEC | LO_BCD); break;
  case MIN:
    wrtbits(rtc_bcd[MIN], getNibbles(v), HI_MIN | LO_BCD); break;
  case HOUR:
    // TODO : AM/PM  12HR/24HR
    wrtbits(rtc_bcd[HOUR], getNibbles(v), HI_HR | LO_BCD); break;
  case DOW:
    wrtbits(rtc_bcd[DOW], v, LO_DOW); break;
  case DATE:
    wrtbits(rtc_bcd[DATE], getNibbles(v), HI_DATE | LO_BCD); break;
  case MONTH:
    wrtbits(rtc_bcd[MONTH], getNibbles(v), HI_MTH | LO_BCD); break;
  case YEAR:
    rtc_bcd[YEAR] = getNibbles(v); break;

  case SEC_HI:
    wrtbits(rtc_bcd[SEC], v << 4, HI_SEC); break;
  case SEC_LO:
    wrtbits(rtc_bcd[SEC], v, LO_BCD); break;
  case MIN_HI:
    wrtbits(rtc_bcd[MIN], v << 4, HI_MIN); break;
  case MIN_LO:
    wrtbits(rtc_bcd[MIN], v, LO_BCD); break;
  case HOUR_HI:
    wrtbits(rtc_bcd[HOUR], v << 4, HI_HR); break;
  case HOUR_LO:
    wrtbits(rtc_bcd[HOUR], v, LO_BCD); break;

  case DATE_HI:
    wrtbits(rtc_bcd[DATE], v << 4, HI_DATE); break;
  case DATE_LO:
    wrtbits(rtc_bcd[DATE], v, LO_BCD); break;
  case MONTH_HI:
    wrtbits(rtc_bcd[MONTH], v << 4, HI_MTH); break;
  case MONTH_LO:
    wrtbits(rtc_bcd[MONTH], v, LO_BCD); break;
  case YEAR_HI:
    wrtbits(rtc_bcd[YEAR], v << 4, HI_BCD); break;
  case YEAR_LO:
    wrtbits(rtc_bcd[YEAR], v, LO_BCD); break;
  } // end switch
  save();
}

void DS1307::stop(void)
{
  // set the ClockHalt bit high to stop the rtc
  // this bit is part of the seconds byte
  read(); //refresh buffer first to preserve existing time
  setbits(rtc_bcd[SEC], CLOCKHALT); //set the halt bit in the seconds value
  save(); //write register to the chip
}

void DS1307::start(void)
{
  // unset the ClockHalt bit to start the rtc
  read(); //refresh buffer to get existing time
  clrbits(rtc_bcd[SEC], CLOCKHALT); //unset the halt bit in the seconds value
  save(); //write register to the chip
}


// Source: https://github.com/adafruit/RTClib/blob/master/RTClib.cpp
////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second
const uint8_t daysInMonth[] PROGMEM = {
  31, // Jan
  28, // Feb (non-leap year)
  31, // Mar
  30, // Apr
  31, // May
  30, // Jun
  31, // Jul
  31, // Aug
  30, // Sep
  31, // Oct
  30, // Nov
  31  // Dec
};

#define SECONDS_FROM_1970_TO_2000 946684800

DateTime::DateTime(uint32_t t)
{
  t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970
  ss = t % 60;
  t /= 60;
  mm = t % 60;
  t /= 60;
  hh = t % 24;
  uint16_t days = t / 24;
  initDate(days);
}

DateTime::DateTime(uint16_t d)
{
  ss = mm = hh = 0;
  initDate(d);
}

// yearOff is a number of years from 2000
DateTime::DateTime(uint8_t yearOff, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
  yOff = yearOff;
  m = month;
  d = day;
  hh = hour;
  mm = min;
  ss = sec;
}

byte DateTime::daysPerMonth(byte month, byte year)
{
  byte leap = (year % 4  == 0);
  byte daysPerMonth = pgm_read_byte(daysInMonth + month - 1);
  if(leap && month == 2) ++daysPerMonth;
  return daysPerMonth;
}

void DateTime::initDate(uint16_t days)
{
  uint8_t leap;
  for (yOff = 0; ; ++yOff) {
    leap = yOff % 4 == 0;
    uint16_t daysPerYear = (uint16_t)365 + (uint16_t)leap;
    if (days < daysPerYear)
      break;
    days -= daysPerYear;
  }
  for (m = 1; ; ++m) {
    uint16_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
    if (leap && m == 2)
      ++daysPerMonth;
    if (days < daysPerMonth)
      break;
    days -= daysPerMonth;
  }
  d = days + 1;
}


// number of days since 1/1/2000, valid for 2001..2099
uint16_t DateTime::dayStamp() const
{
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if (m > 2 && yOff % 4 == 0)
    ++days; // add 29th of  day
  return days + 365 * yOff + (yOff + 3) / 4 - 1;
}

uint8_t DateTime::dayOfWeek() const
{
  uint16_t day = dayStamp();
  return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

uint32_t DateTime::unixtime() const
{
  uint16_t days = dayStamp();
  uint32_t t = ((days * 24L + hh) * 60 + mm) * 60 + ss;
  t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000
  return t;
}
