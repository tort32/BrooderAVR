#pragma once

#include "stdafx.h"
#include "LCD4Bit_mod.h"

// This pin used for alarm signal
#define ALARM_OUT _BV(PB5) // D13
#define ALARM_OUT_PORT PORTB
#define ALARM_OUT_DDR  DDRB

// Used for heartbeat pulse debugging
#define LED _BV(PC2) // A2
#define LED_PORT PORTC
#define LED_DDR  DDRC

// Alarm signal and debugging LED class
// setError/getError hangs loop cycle execution. Used for error notifications.
// setAlarm/getAlarm used for alarming signals. See setAlarm method for more details.
class Alarm
{
public:
  Alarm()
  {
    error_status = 0;
    alarm_status = 0;
  }

  void init()
  {
    // Configure pins as outputs
    setbits(ALARM_OUT_DDR, ALARM_OUT);
    setbits(LED_DDR, LED);
  }

  bool getError()
  {
    return (error_status != 0);
  }

  void setError(bool err)
  {
    error_status = err;
  }

  void error(const char* msg)
  {
    error_status = 1;

    LCD.clear();
    LCD.printIn(msg);
    Serial.println(msg);
  }

  byte getAlarm()
  {
    return alarm_status;
  }

  void setAlarm(byte alarm)
  {
    // 'alarm_status' has 8 bits
    // Each of them can be used for separate alarm signal
    // Alarm output will be set if any of this bits is set

    // alarm signals usage example:
    //   byte alarm = ALARM.getAlarm();
    //   alarm &= 1<<5; // set 5th alarm bit
    //   ALARM.setAlarm(alarm);

    alarm_status = alarm;
  }

  void beep(byte beep_cnt)
  {
    // Sequential beep in the buzzer
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
    // Switch alarm on if any bit is set
    wrtbits(ALARM_OUT_PORT, alarm_status ? ALARM_OUT : 0, ALARM_OUT);
  }

private:
  volatile byte error_status;
  volatile byte alarm_status;
};

