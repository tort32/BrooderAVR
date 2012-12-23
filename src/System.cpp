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
  day = calculateDay();
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