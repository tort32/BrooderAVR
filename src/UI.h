#pragma once

#include "stdafx.h"
#include "LCD4Bit_mod.h"
#include "KeyPad.h"
#include "Alarm.h"
#include "DS1307.h"
#include "DS18B20.h"
#include "System.h"

//#define DEBUG_UI 1

static void onKeyPressed(byte key);

// User Interface management class
// It use LCD for text output and KeyPad for user input.
// Draws system monitor and menus.
// For menus switching State Machine approach is used (http://en.wikipedia.org/wiki/Finite-state_machine)
class UIManager
{
public:
  UIManager()
  {
    state = kMonitor;
    frame = 0;
    KEYS.set_handler(onKeyPressed);
  }

  /* ----------------------- Input -----------------------*/

  void updateInput()
  {
    KEYS.read();
  }

  // Input handling
  // We should switch by menus and change options
  void OnKeyChanged(byte key)
  {
#ifdef DEBUG_UI
    Serial.print("KEY = ");
    Serial.println(key, HEX);
#endif
    switch(state)
    {
    case kMonitor:
      if(key == VKEY_SELECT)
        return setState(kMenu_Main);
      break;
    case kMenu_Main:
      if(key == VKEY_SELECT || key == VKEY_LEFT)
        return setState(kMonitor);
      if(key == VKEY_DOWN)
        return moveNext();
      if(key == VKEY_UP)
        return movePrev();
      if(key == VKEY_RIGHT)
      {
        // enter submenu
        switch(menu_pos)
        {
        case kMain_Time: return setState(kMenu_Time);
        case kMain_Date: return setState(kMenu_Date);
        case kMain_System: return setState(kMenu_System);
        }
      }
      break;
    case kMenu_Time:
      if(key == VKEY_SELECT) return setState(kMonitor);
      if(key == VKEY_LEFT) return setState(kMenu_Main, kMain_Time);
      if(key == VKEY_RIGHT) return moveNext();
      if(key == VKEY_UP) return incValue(+1);
      if(key == VKEY_DOWN) return incValue(-1);
      break;
    case kMenu_Date:
      if(key == VKEY_SELECT) return setState(kMonitor);
      if(key == VKEY_LEFT) return setState(kMenu_Main, kMain_Date);
      if(key == VKEY_RIGHT) return moveNext();
      if(key == VKEY_UP) return incValue(+1);
      if(key == VKEY_DOWN) return incValue(-1);
      break;
    case kMenu_System:
      if(key == VKEY_SELECT) return setState(kMonitor);
      if(key == VKEY_LEFT) return setState(kMenu_Main, kMain_System);
      if(key == VKEY_RIGHT)
      {
        if(menu_pos == kSystem_Brooder)
          toggleBooleanItem(EEPROM.get().enableBrooder);
        if(menu_pos == kSystem_Day)
          return toggleDay();
        if(menu_pos == kSystem_Coop)
          toggleBooleanItem(EEPROM.get().enableCoop);
        if(menu_pos == kSystem_Temp)
          toggleBooleanItem(EEPROM.get().swapTemp);
      }
      if(key == VKEY_UP) return movePrev();
      if(key == VKEY_DOWN) return moveNext();
      break;
    }
  }

  /* ----------------------- Output -----------------------*/

  // Outputs text onto LCD coresponding current state
  void updateOutput()
  {
    frame++;
    LCD.setDisplay(1,0,0);

    if(state == kMonitor)
    {
      drawMonitor();
    }
    else
    {
      // menu title
      LCD.cursorTo(1,0);
      drawMenuTitle();

      // menu item
      LCD.cursorTo(2,0);
      switch(state)
      {
      case kMenu_Main: return drawMainMenu();
      case kMenu_Time: return drawTimeMenu();
      case kMenu_Date: return drawDateMenu();
      case kMenu_System:
        if(menu_pos == kSystem_Brooder)
          return drawBooleanItem("Brooder", EEPROM.get().enableBrooder);
        if(menu_pos == kSystem_Day)
          return drawDayMenu();
        if(menu_pos == kSystem_Coop)
          return drawBooleanItem("Coop", EEPROM.get().enableCoop);
        if(menu_pos == kSystem_Temp)
          return drawBooleanItem("TSwap", EEPROM.get().swapTemp);
      }
    }
  }

private:

  /* ----------------------- States and Options -----------------------*/

  // Switch state machine
  void setState(byte new_state, byte new_menu_pos = 0)
  {
    state = new_state;
    if(new_menu_pos == 0)
      menu_pos = (new_state & 0xf0) | 0x01; // Go to first menu item
    else
      menu_pos = new_menu_pos;
#ifdef DEBUG_UI
    Serial.print("STATE = ");
    Serial.println(state, HEX);
    Serial.print("POS = ");
    Serial.println(menu_pos, HEX);
#endif
    LCD.clear();
  }

  // Switch sub state
  void moveNext()
  {
    byte max_item = (state & 0x0f);
    if((menu_pos & 0x0f) == max_item)
      menu_pos = (state & 0xf0) + 1; // move to first
    else
      menu_pos++;
#ifdef DEBUG_UI
    Serial.print("POS = ");
    Serial.println(menu_pos, HEX);
#endif
    LCD.clear();
  }

  // Switch sub state
  void movePrev()
  {
    if((menu_pos & 0x0f) == 1)
      menu_pos = state; // move to last
    else
      menu_pos--;
#ifdef DEBUG_UI
    Serial.print("POS = ");
    Serial.println(menu_pos, HEX);
#endif
    LCD.clear();
  }

  // Change option value
  void incValue(int8_t delta)
  {
    byte dight;
    int8_t min = 0, max;
    switch(state)
    {
    case kMenu_Time:
      switch(menu_pos)
      {
      case kTime_Hour_Hi:
      case kTime_Hour_Lo:
        dight = DS1307::HOUR; max = 23;
        break;
      case kTime_Min_Hi:
      case kTime_Min_Lo:
        dight = DS1307::MIN; max = 59;
        break;
      case kTime_Sec_Hi:
      case kTime_Sec_Lo:
        dight = DS1307::SEC; max = 59;
        break;
      }
      switch(menu_pos)
      {
      case kTime_Hour_Hi:
      case kTime_Min_Hi:
      case kTime_Sec_Hi:
        delta *= 10;
        break;
      }
      break;
    case kMenu_Date:
      switch(menu_pos)
      {
      case kDate_Year_Hi:
      case kDate_Year_Lo:
        dight = DS1307::YEAR; max = 59;
        break;
      case kDate_Month_Hi:
      case kDate_Month_Lo:
        dight = DS1307::MONTH; min = 1; max = 12;
        break;
      case kDate_Date_Hi:
      case kDate_Date_Lo:
        dight = DS1307::DATE; min = 1; max =
          DateTime::daysPerMonth(RTC.get(DS1307::MONTH), RTC.get(DS1307::YEAR));
        break;
      case kDate_Day:
        dight = DS1307::DOW; min = 1; max = 7;
        break;
      }
      switch(menu_pos)
      {
      case kDate_Year_Hi:
      case kDate_Month_Hi:
      case kDate_Date_Hi:
        delta *= 10;
        break;
      }
      break;
    }

    if(state == kMenu_Time || state == kMenu_Date)
    {
      int8_t val = (int8_t)RTC.get(dight) + delta;
      if(val > max) val = min;
      if(val < min) val = max;
      RTC.set(dight, (uint8_t)val);
    }
    else
    {
      // Not implemented
      ALARM.beep(1);
    }
  }

  // Change boolean option value
  void toggleBooleanItem(byte& value)
  {
    value = !value;
    EEPROM.save();
    LCD.clear();
  }

  // Change brooder day option value
  void toggleDay()
  {
      byte day = SYSTEM.getBrooderDay();
      if(day == SYSTEM.kDay_Invalid)
        day = 0;
      else
        ++day;
      SYSTEM.setBrooderDay(day);
      LCD.clear();
  }

  /* ----------------------- Menu drawing -----------------------*/

  void drawMenuTitle()
  {
    // menu title
    switch(state)
    {
    case kMenu_Main: LCD.printIn("Main "); break;
    case kMenu_Time: LCD.printIn("Set Time "); break;
    case kMenu_Date: LCD.printIn("Set Date "); break;
    case kMenu_System: LCD.printIn("System "); break;
    }
    // menu position
    LCD.printDigit(menu_pos & 0x0f); // current menu item index
    LCD.print('/');
    LCD.printDigit(state & 0x0f); // total menu items count
  }

  // Draws Main menu current item
  void drawMainMenu()
  {
    switch(menu_pos)
    {
    case kMain_Time: LCD.printIn("Time >"); break;
    case kMain_Date: LCD.printIn("Date >"); break;
    case kMain_System: LCD.printIn("System >"); break;
    }
  }

  // Draws Set Time sub-menu
  void drawTimeMenu()
  {
    LCD.printDigit(RTC.get(DS1307::HOUR_HI)); // 1
    LCD.printDigit(RTC.get(DS1307::HOUR_LO)); // 2
    LCD.print(':'); // 3
    LCD.printDigit(RTC.get(DS1307::MIN_HI)); // 4
    LCD.printDigit(RTC.get(DS1307::MIN_LO)); // 5
    LCD.print(':'); // 6
    LCD.printDigit(RTC.get(DS1307::SEC_HI)); // 7
    LCD.printDigit(RTC.get(DS1307::SEC_LO)); // 8
    switch(menu_pos)
    {
    case kTime_Hour_Hi: LCD.cursorTo(2,0); break;
    case kTime_Hour_Lo: LCD.cursorTo(2,1); break;
    case kTime_Min_Hi:  LCD.cursorTo(2,3); break;
    case kTime_Min_Lo:  LCD.cursorTo(2,4); break;
    case kTime_Sec_Hi:  LCD.cursorTo(2,6); break;
    case kTime_Sec_Lo:  LCD.cursorTo(2,7); break;
    }
    LCD.setDisplay(1,0,1);
  }

  // Draws Set Date sub-menu
  void drawDateMenu()
  {
    LCD.printDigit(RTC.get(DS1307::DATE_HI)); // 1
    LCD.printDigit(RTC.get(DS1307::DATE_LO)); // 2
    LCD.print('/'); // 3
    LCD.printDigit(RTC.get(DS1307::MONTH_HI)); // 4
    LCD.printDigit(RTC.get(DS1307::MONTH_LO)); // 5
    LCD.print('/'); // 6
    LCD.printDigit(RTC.get(DS1307::YEAR_HI)); // 7
    LCD.printDigit(RTC.get(DS1307::YEAR_LO)); // 8
    LCD.print(' '); // 9
    byte dow = RTC.get(DS1307::DOW);
    const char* sDOW = RTC.getDOWChars(dow); // 2 chars
    LCD.printIn(sDOW); // 10 - 11
    switch(menu_pos)
    {
    case kDate_Date_Hi:   LCD.cursorTo(2,0); break;
    case kDate_Date_Lo:   LCD.cursorTo(2,1); break;
    case kDate_Month_Hi:  LCD.cursorTo(2,3); break;
    case kDate_Month_Lo:  LCD.cursorTo(2,4); break;
    case kDate_Year_Hi:   LCD.cursorTo(2,6); break;
    case kDate_Year_Lo:   LCD.cursorTo(2,7); break;
    case kDate_Day:       LCD.cursorTo(2,10); break;
    }
    LCD.setDisplay(1,0,1);
  }

  // Draws boolean option item
  void drawBooleanItem(const char* name, const byte value)
  {
    LCD.printIn(name);
    LCD.printIn(" > ");
    LCD.printIn((value == 0) ? "Off" : "On");
  }

  // Draws brooder day option item
  void drawDayMenu()
  {
    LCD.printIn("Day > ");
    printDay();
  }

  /* ----------------------- Aux drawings -----------------------*/

  void printDay()
  {
    byte day = SYSTEM.getBrooderDay();
    if(day != SYSTEM.kDay_Invalid)
      LCD.printDigit2(day+1); // day 1-28
    else
      LCD.printIn("\xFD ");
  }

  void printTemp(int8_t tmp) // print 4 chars
  {
    if(tmp != TEMP.kTemp_Invalid)
    {
      if(tmp < 0)
      {
        tmp = -tmp;
        LCD.print('-');
      }
      else
      {
        LCD.print('+');
      }
      uint8_t celsius = (uint8_t)RAW_TO_CELSIUS(tmp);
      LCD.printDigit2(celsius);
      LCD.print('\xDF');
    }
    else
    {
      LCD.printIn(" N/A");
    }
  }

  void printTemp(int16_t tmp) // print 5 chars
  {
    if(tmp != TEMP.kTemp_Invalid)
    {
      if(tmp < 0)
      {
        tmp = -tmp;
        LCD.print('-');
      }
      else
      {
        LCD.print('+');
      }
      uint8_t celsius = (uint8_t)RAW_TO_CELSIUS(tmp);
      LCD.printDigit2(celsius);
      LCD.print('\xDF');
      uint8_t fr = (uint8_t)(tmp - CELSIUS_TO_RAW(celsius)); // fraction data
      uint8_t fr_dig = 0;
      if(fr & 8) fr_dig+=50; // 1000 = .5
      if(fr & 4) fr_dig+=25; // 0100 = .25
      if(fr & 2) fr_dig+=13; // 0010 = .125
      if(fr & 1) fr_dig+=06; // 0001 = .0625
      LCD.printDigit(fr_dig/10);
    }
    else
    {
      LCD.printIn("  N/A ");
    }
  }

  /* ----------------------- Draw system monitor -----------------------*/

  void drawMonitor()
  {
    // 0123456789ABCDEF
    // HH:MM B*01 +30°5
    // SU ** C ** +20°5

    const Settings& settings = EEPROM.get();
    byte alarm = SYSTEM.getAlarm();
    byte flash = frame % 2;

    LCD.cursorTo(1,0);
    LCD.printDigit(RTC.get(DS1307::HOUR_HI));
    LCD.printDigit(RTC.get(DS1307::HOUR_LO));
    LCD.print(':');
    LCD.printDigit(RTC.get(DS1307::MIN_HI));
    LCD.printDigit(RTC.get(DS1307::MIN_LO));
    LCD.print(' ');

    if(settings.enableBrooder)
    {
      LCD.printIn("B\xA5");
      printDay();

      if(alarm & flash & SYSTEM.kAlarm_Brudder_Hi)
        LCD.print('^');
      else if(alarm & flash & SYSTEM.kAlarm_Brudder_Lo)
        LCD.print('v');
      else
        LCD.print(' ');
    }
    else
    {
      LCD.printIn("B    ");
    }

    printTemp(TEMP[0]);

    LCD.cursorTo(2,0);
    byte dow = RTC.get(DS1307::DOW);
    const char* sDOW = RTC.getDOWChars(dow); // 2 chars
    LCD.printIn(sDOW);

    LCD.printIn("    ");
    if(settings.enableCoop)
    {
      LCD.printIn("C\xA5");
      if(SYSTEM.getCoopLight())
        LCD.print('*');
      else
        LCD.print(' ');
      LCD.print(' ');

      if(alarm & flash & SYSTEM.kAlarm_Coop_Hi)
        LCD.print('^');
      else if(alarm & flash & SYSTEM.kAlarm_Coop_Lo)
        LCD.print('v');
      else
        LCD.print(' ');
    }
    else
    {
      LCD.printIn("C    ");
    }

    printTemp(TEMP[1]);
  }

private:
  byte state; // state machine position
  enum eState
  {
    // Hi 4 bits - menu Id
    // Lo 4 bits - number of items in the menu
    kMonitor = 0x00,
    kMenu_Main = 0x13,
    kMenu_Time = 0x26,
    kMenu_Date = 0x37,
    kMenu_System = 0x44,
  };
  byte menu_pos; // sub state for menus
  enum eMenuPos
  {
    // Hi 4 bits - menu Id
    // Lo 4 bits - menu item index (starting 1)
    // Should be in consistent with eState constants
    kMain_Time = 0x11,
    kMain_Date = 0x12,
    kMain_System = 0x13,

    kTime_Hour_Hi  = 0x21,
    kTime_Hour_Lo  = 0x22,
    kTime_Min_Hi   = 0x23,
    kTime_Min_Lo   = 0x24,
    kTime_Sec_Hi   = 0x25,
    kTime_Sec_Lo   = 0x26,

    kDate_Date_Hi  = 0x31,
    kDate_Date_Lo  = 0x32,
    kDate_Month_Hi = 0x33,
    kDate_Month_Lo = 0x34,
    kDate_Year_Hi  = 0x35,
    kDate_Year_Lo  = 0x36,
    kDate_Day      = 0x37,

    kSystem_Brooder= 0x41,
    kSystem_Day    = 0x42,
    kSystem_Coop   = 0x43,
    kSystem_Temp   = 0x44,
  };
  byte frame; // frame counter. Used for flashing chars
};

static void onKeyPressed(byte key)
{
  UI.OnKeyChanged(key);
}
