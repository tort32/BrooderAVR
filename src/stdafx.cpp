#include "stdafx.h"

#include "LCD4Bit_mod.h"
#include "KeyPad.h"

KeyPad KEYS(A0);

//////////////////////////////////////////////////////////////////////////

volatile byte error_status = 0x0;
volatile byte alarm_status = 0x0;

bool is_error()
{
  return (error_status != 0);
}

void set_alarm(bool alarm)
{
  alarm_status = alarm;
}

bool is_alarm()
{
  return (alarm_status != 0);
}

void error(const char* msg)
{
  error_status = 1;

  LCD.clear();
  LCD.printIn(msg);
  Serial.println(msg);
}

void beep(byte beep_cnt)
{
  for(; beep_cnt; --beep_cnt)
  {
    setbits(ALARM_OUT_PORT, ALARM_OUT);
    delay(200);
    clrbits(ALARM_OUT_PORT, ALARM_OUT);
    delay(200);
  }
}