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

/* ----------------------- hardware I/O abstraction ------------------------ */
// PIN      FUNCTION
// D0 (PD0) RX
// D1 (PD1) TX
// D2 (PD2) 1-wire
//~D3 (PD3) PWM for light LEDs
// D4 (PD4) LCD D4
//~D5 (PD5) LCD D5
//~D6 (PD6) LCD D6
// D7 (PD7) LCD D7
// D8 (PB0) LCD RS
//~D9 (PB1) LCD E
//~D10(PB2) LCD Backlight ctr
//~D11(PB3) PWM for heat lamp
// D12(PB4) Power ON sensor
// D13(PB5) Alarm buzzer
// A0 (PC0) Keys sensor
// A1 (PC1) Light sensor
// A2 (PC2) Debug LED
// A3 (PC3)
// A4 (PC4) RTC SDA
// A5 (PC5) RTC SCL

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