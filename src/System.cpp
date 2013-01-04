#include "stdafx.h"
#include "System.h"
#include "EEPROM.h"
#include "DS1307.h"

System::System()
{
  day = kInvalidDay;
}

void System::init()
{
  day = calculateDay();
}

void System::update()
{
  bool enable = true;
  byte temp_min, temp_max;

  day = calculateDay();

  if(1 < day && day < 8) // 1-7
  {
    temp_min = 35;
    temp_max = 37;
  }
  else if(day < 15) // 8-14
  {
    temp_min = 30;
    temp_max = 32;
  }
  else if(day < 22) // 15-21
  {
    temp_min = 25;
    temp_max = 27;
  }
  else if(day < kDayMax) // 22-28
  {
    temp_min = 20;
    temp_max = 22;
  }
  else // out of range
  {
    enable = false;
  }
  if(enable)
  {
    // TODO: manage light power
  }
}

byte System::getDay()
{
  return day;
}

void System::setDay(byte newDay)
{
  if(newDay > kDayMax)
    return;

  const DateTime& now = RTC.getDateTime();
  Settings& settings = EEPROM.get();
  settings.startDayStamp = now.dayStamp() - (uint16_t)newDay;

  day = calculateDay();
}

byte System::calculateDay()
{
  // calculate day number
  const DateTime& now = RTC.getDateTime();
  const Settings& settings = EEPROM.get();
  uint16_t dayStamp = now.dayStamp() - settings.startDayStamp;
  if(dayStamp < 0 || dayStamp > kDayMax)
    return kInvalidDay;
  else
    return (byte)dayStamp;
}