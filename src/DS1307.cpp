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
#define HI_MTH  B00110000
#define HI_YR   B11110000

DS1307::DS1307()
{
  Wire.begin();
}

DS1307 RTC=DS1307();

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
    return rtc_bcd[DOW] & LO_DOW;
  case DATE:
    return get(DATE_HI) * 10 + get(DATE_LO);
  case MONTH:
    return get(MONTH_HI) * 10 + get(MONTH_LO);
  case YEAR:
    return get(YEAR_LO) * 10 + get(YEAR_HI);

  case SEC_LO:
    return rtc_bcd[SEC] & LO_BCD;
  case SEC_HI:
    return (rtc_bcd[SEC] & HI_SEC) >> 4;
  case MIN_LO:
    return rtc_bcd[MIN] & LO_BCD;
  case MIN_HI:
    return (rtc_bcd[MIN] & HI_MIN) >> 4;
  case HOUR_LO:
    return rtc_bcd[HOUR] & LO_BCD;
  case HOUR_HI:
    return (rtc_bcd[HOUR] & HI_HR) >> 4;
  case DATE_LO:
    return rtc_bcd[DATE] % 16;
  case DATE_HI:
    return rtc_bcd[DATE] / 16;
  case MONTH_LO:
    return rtc_bcd[MONTH] & LO_BCD;
  case MONTH_HI:
    return (rtc_bcd[MONTH] & HI_MTH) >> 4;
  case YEAR_LO:
    return rtc_bcd[YEAR] % 16;
  case YEAR_HI:
    return rtc_bcd[YEAR] / 16;
  } // end switch
  return 0;
}

void DS1307::set(byte c, byte v)  // Update buffer, then update the chip
{
  read(); //first read the buffer to preserve current time when writing modified values, as the function writes all values at the same time
  switch(c)
  {
  case SEC:
    if(v<60 && v>-1)
    {
      //preserve existing clock state (running/stopped)
      int state = rtc_bcd[SEC] & CLOCKHALT;
      rtc_bcd[SEC] = state | ((v / 10)<<4) + (v % 10);
    }
    break;
  case MIN:
    if(v<60 && v>-1)
      rtc_bcd[MIN] = ((v / 10)<<4) + (v % 10);
    break;
  case HOUR:
    // TODO : AM/PM  12HR/24HR
    if(v<24 && v>-1)
      rtc_bcd[HOUR] = ((v / 10)<<4) + (v % 10);
    break;
  case DOW:
    if(v<8 && v>-1)
      rtc_bcd[DOW] = v;
    break;
  case DATE:
    if(v<32 && v>-1)
      rtc_bcd[DATE] = ((v / 10)<<4) + (v % 10);
    break;
  case MONTH:
    if(v<13 && v>-1)
      rtc_bcd[MONTH] = ((v / 10)<<4) + (v % 10);
    break;
  case YEAR:
    if(v<50 && v>-1)
      rtc_bcd[YEAR] = ((v / 10)<<4) + (v % 10);
    break;
  } // end switch
  save();
}

void DS1307::stop(void)
{
  // set the ClockHalt bit high to stop the rtc
  // this bit is part of the seconds byte
  read(); 	//refresh buffer first to preserve existing time
  rtc_bcd[SEC] = rtc_bcd[SEC] | CLOCKHALT; //set the halt bit in the seconds value
  save(); //write register to the chip
}

void DS1307::start(void)
{
  // unset the ClockHalt bit to start the rtc
  read();				 //refresh buffer to get existing time
  rtc_bcd[SEC] -= CLOCKHALT; //unset the halt bit in the seconds value
  save(); //write register to the chip
}


