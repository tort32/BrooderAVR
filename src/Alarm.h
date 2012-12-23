#pragma once

#include "stdafx.h"
#include "LCD4Bit_mod.h"

#define LED _BV(PB5) // D13
#define LED_PORT PORTB

#define ALARM_OUT _BV(PB2)
#define ALARM_OUT_PORT PORTB

class Alarm
{
public:
  Alarm()
  {
    error_status = 0x0;
    alarm_status = 0x0;
  }

  void init()
  {
    DDRB = ALARM_OUT | LED;
  }

  bool is_error()
  {
    return (error_status != 0);
  }

  void error(const char* msg)
  {
    error_status = 1;

    LCD.clear();
    LCD.printIn(msg);
    Serial.println(msg);
  }

  bool is_alarm()
  {
    return (alarm_status != 0);
  }

  void set_alarm(bool alarm)
  {
    alarm_status = alarm;
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

  void update()
  {
    wrtbits(ALARM_OUT_PORT, alarm_status ? ALARM_OUT : 0, ALARM_OUT);
  }

private:
  volatile byte error_status;
  volatile byte alarm_status;
};

