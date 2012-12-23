#pragma once

// Defines Brooder program logic
class System
{
public:
  enum
  {
    kInvalidDay = 0xff,
    kDayMax = 30,
  };
public:
  System();
  void init();
  void update();
  byte getDay();
  void setDay(byte newDay);

private:
  byte calculateDay();

private:
  byte day;
};