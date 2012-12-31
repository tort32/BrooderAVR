#include "stdafx.h"

#include "LCD4Bit_mod.h"
#include "KeyPad.h"
#include "Alarm.h"
#include "OneWire.h"
#include "DS1307.h"
#include "DS18B20.h"
#include "UI.h"
#include "System.h"
#include "EEPROM.h"

OneWire oneWire = OneWire(2); // D2
DS1307 RTC = DS1307();
LCD4Bit_mod LCD = LCD4Bit_mod();
KeyPad KEYS = KeyPad(A0);
Alarm ALARM = Alarm();
TempManager TEMP = TempManager(oneWire);
UIManager UI = UIManager();
System SYSTEM = System();
EpromManager EEPROM = EpromManager();

//////////////////////////////////////////////////////////////////////////

byte range_cycle(byte min, byte max, byte val)
{
  if(val > max) return min;
  if(val < min) return max;
  return val;
}