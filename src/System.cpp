#include "stdafx.h"
#include "System.h"
#include "EEPROM.h"
#include "DS1307.h"
#include "DS18B20.h"

const uint8_t pinPWM1 = 11; /*PB3*/ // heat lamp
const uint8_t pinPWM2 = 3; /*PD3*/ // light lamp

System::System()
{
  day = kDay_Invalid;
  resetBrooder();
  resetCoop();
}

void System::init()
{
  day = calculateDay();
}

void System::update()
{
  day = calculateDay();

  const Settings& settings = EEPROM.get();
  if(settings.enableBrooder)
    updateBrooder();
  else
    resetBrooder();

  if(settings.enableCoop)
    updateCoop();
  else
    resetCoop();
}

// Execute Brooder program
void System::updateBrooder()
{
  byte enable = true; // heat lamp is on
  RawTemp temp_min, temp_max, temp_alarm_min, temp_alarm_max;
  if(day < 8) // 1-7 day
  {
    temp_alarm_min = CELSIUS_TO_RAW(33.0);
    temp_min = CELSIUS_TO_RAW(35.0);
    temp_max = CELSIUS_TO_RAW(37.0);
    temp_alarm_max = CELSIUS_TO_RAW(37.5);
  }
  else if(day < 15) // 8-14 day
  {
    temp_alarm_min = CELSIUS_TO_RAW(28.0);
    temp_min = CELSIUS_TO_RAW(30.0);
    temp_max = CELSIUS_TO_RAW(32.0);
    temp_alarm_max = CELSIUS_TO_RAW(34.0);
  }
  else if(day < 22) // 15-21 day
  {
    temp_alarm_min = CELSIUS_TO_RAW(21.0);
    temp_min = CELSIUS_TO_RAW(25.0);
    temp_max = CELSIUS_TO_RAW(27.0);
    temp_alarm_max = CELSIUS_TO_RAW(27.5);
  }
  else if(day <= kDay_Max) // 22-28 day
  {
    temp_alarm_min = CELSIUS_TO_RAW(19.0);
    temp_min = CELSIUS_TO_RAW(20.0);
    temp_max = CELSIUS_TO_RAW(22.0);
    temp_alarm_max = CELSIUS_TO_RAW(25.0);
  }
  else // kInvalidDay
  {
    enable = false;
  }

  if(!enable)
    return resetBrooder();

  // adjust heat power if temperature is out of range
  RawTemp temp_current = TEMP[0];
  if(temp_current != TEMP.kTemp_Invalid)
  {
    if(temp_current < temp_min && pwmHeatLamp!=kPWM_Max) pwmHeatLamp+=1;
    if(temp_current > temp_max && pwmHeatLamp!=kPWM_Min) pwmHeatLamp-=1;
  }
  analogWrite(pinPWM1, pwmHeatLamp);

  // set alarm if temperature is out of range
  byte alarm = ALARM.getAlarm();
  clrbits(alarm, kAlarm_Brudder);
  if(temp_current != TEMP.kTemp_Invalid)
  {
    if(temp_current > temp_alarm_max)
      setbits(alarm, kAlarm_Brudder_Hi);
    if(temp_current < temp_alarm_min)
      setbits(alarm, kAlarm_Brudder_Lo);
  }
  ALARM.setAlarm(alarm);
}

// Reset Brooder program execution
void System::resetBrooder()
{
  pwmHeatLamp = kPWM_Middle; // reset temp control value
  analogWrite(pinPWM1, kPWM_Min); // switch off heat lamp

  // Reset alarm
  byte alarm = ALARM.getAlarm();
  clrbits(alarm, kAlarm_Brudder_Hi);
  ALARM.setAlarm(alarm);
}

// Execute Coop program
void System::updateCoop()
{
  byte hour = RTC.get(DS1307::HOUR);
  if(hour >= 1 && hour < 8) // light is off from 1:00 to 8:00
    light = 0;
  else
    light = 1;
  if(!light)
    return resetCoop();

  analogWrite(pinPWM2, pwmLight);
}

// Reset Coop program execution
void System::resetCoop()
{
  light = 0;
  pwmLight = kPWM_Middle; // reset light control value
  analogWrite(pinPWM2, kPWM_Min); // switch off light lamp

  // Reset alarm
  byte alarm = ALARM.getAlarm();
  clrbits(alarm, kAlarm_Coop);
  ALARM.setAlarm(alarm);
}

// Return system alarm signals
byte System::getAlarm()
{
  return ALARM.getAlarm() & kAlarm;
}

// Returns true if Coop is lighted
byte System::getCoopLight()
{
  return light;
}

// Returns brooder program day (starting from 0)
// Or kDay_Invalid if out of range
byte System::getBrooderDay()
{
  return day;
}

// Saves brooder program day
void System::setBrooderDay(byte newDay)
{
  uint16_t startDayStamp = 0;
  if(newDay <= kDay_Max)
  {
    // calculate start day stamp
    const DateTime& now = RTC.getDateTime();
    startDayStamp = now.dayStamp() - (uint16_t)newDay;
  }

  // Save starting day into EEPROM for the persistence
  EEPROM.get().startDayStamp = startDayStamp;
  EEPROM.save();

  // update day according to new data
  day = calculateDay();
}

// Calculates bruder day number
byte System::calculateDay()
{
  // It use brooder start day stamp and current date
  // Date difference is the number of the day
  const DateTime& now = RTC.getDateTime();
  const Settings& settings = EEPROM.get();
  uint16_t dayStamp = now.dayStamp() - settings.startDayStamp;
  if(dayStamp < 0 || dayStamp > kDay_Max)
    return kDay_Invalid;
  else
    return (byte)dayStamp;
}