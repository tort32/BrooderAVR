#pragma once

// Defines Brooder program logic
// Implements two devices: brooder and coop
// Brooder use temp sensor and manage heat lamp power to stabilize temperature
// Coop use time to manage light LEDs
class System
{
public:
  enum // Brooder day constants
  {
    kDay_Min = 0, // Start day. Display as 1st
    kDay_Max = 27, // Max valid day. Display as 28th
    kDay_Invalid = 0xff,
  };
  enum // PWM constants
  {
    kPWM_Max = 0xff,
    kPWM_Middle = 0x7f,
    kPWM_Min = 0x00,
  };
  enum // Alarm signal mask
  {
    kAlarm_Brudder_Hi = 0x01,
    kAlarm_Brudder_Lo = 0x02,
    kAlarm_Brudder = kAlarm_Brudder_Hi | kAlarm_Brudder_Lo,
    kAlarm_Coop_Hi = 0x04,
    kAlarm_Coop_Lo = 0x08,
    kAlarm_Coop = kAlarm_Coop_Hi | kAlarm_Coop_Lo,
    kAlarm = kAlarm_Brudder | kAlarm_Coop,
  };

public:
  System();
  void init();
  void update();

  byte getBrooderDay();
  void setBrooderDay(byte newDay);

  byte getAlarm();
  byte getCoopLight();

private:
  byte calculateDay();

  void updateBrooder();
  void resetBrooder();

  void updateCoop();
  void resetCoop();

private:
  byte day; // brooder program day (starts from 0)
  byte pwmHeatLamp; // heat lamp PWM value
  byte pwmLight; // light PWM value
  byte light; // light enable flag
};