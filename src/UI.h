#pragma once

#include "stdafx.h"
#include "LCD4Bit_mod.h"
#include "KeyPad.h"
#include "Alarm.h"
#include "DS1307.h"
#include "DS18B20.h"

//#define DEBUG_UI 1

static void onKeyPressed(byte key);

class UIManager
{
public:
  UIManager()
  {
    state = kMonitor;
    KEYS.set_handler(onKeyPressed);
  }

  void inputKeys()
  {
    KEYS.read();
  }

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
        case kMain_Temp: return setState(kMenu_Temp);
        }
      }
      break;
    case kMenu_Time:
      if(key == VKEY_SELECT) return setState(kMonitor);
      if(key == VKEY_LEFT) return setState(kMenu_Main, kMain_Time);
      if(key == VKEY_RIGHT) return moveNext();
      if(key == VKEY_UP) return incValue();
      if(key == VKEY_DOWN) return decValue();
      break;
    case kMenu_Date:
      if(key == VKEY_SELECT) return setState(kMonitor);
      if(key == VKEY_LEFT) return setState(kMenu_Main, kMain_Date);
      if(key == VKEY_RIGHT) return moveNext();
      if(key == VKEY_UP) return incValue();
      if(key == VKEY_DOWN) return decValue();
      break;
    case kMenu_Temp:
      if(key == VKEY_SELECT) return setState(kMonitor);
      if(key == VKEY_LEFT) return setState(kMenu_Main, kMain_Temp);
      if(key == VKEY_RIGHT) return toggleTempSwitch();
      break;
    }
  }

  void outputLCD()
  {
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
      if(state == kMenu_Main)
        return drawMainMenu();
      if(state == kMenu_Time)
        return drawTimeMenu();
      if(state == kMenu_Date)
        return drawDateMenu();
      if(state == kMenu_Temp)
        return drawTempMenu();
    }
  }

private:

  void setState(byte new_state, byte new_menu_pos = 0)
  {
    LCD.clear();
    state = new_state;
    if(new_menu_pos == 0)
    {
      switch(new_state)
      {
      case kMenu_Main:
        menu_pos = kMain_Time;
        break;
      case kMenu_Time:
        menu_pos = kTime_Hour_Hi;
        break;
      case kMenu_Date:
        menu_pos = kDate_Date_Hi;
        break;
      case kMenu_Temp:
        menu_pos = kTemp_Switch;
        break;
      }
    }
    else
    {
      menu_pos = new_menu_pos;
    }
#ifdef DEBUG_UI
    Serial.print("STATE = ");
    Serial.println(state, HEX);
    Serial.print("POS = ");
    Serial.println(menu_pos, HEX);
#endif
  }

  void moveNext()
  {
    LCD.clear();
    byte max_item = (state & 0x0f);
    if((menu_pos & 0x0f) == max_item)
      menu_pos = (state & 0xf0) + 1; // move to first
    else
      menu_pos++;
#ifdef DEBUG_UI
    Serial.print("POS = ");
    Serial.println(menu_pos, HEX);
#endif
  }

  void movePrev()
  {
    LCD.clear();
    if((menu_pos & 0x0f) == 1)
      menu_pos = state; // move to last
    else
      menu_pos--;
#ifdef DEBUG_UI
    Serial.print("POS = ");
    Serial.println(menu_pos, HEX);
#endif
  }

  void incValue()
  {
    ALARM.beep(1);
  }

  void decValue()
  {
    ALARM.beep(1);
  }

  void toggleTempSwitch()
  {
    LCD.clear();
    Settings& settings = EEPROM.get();
    settings.temp_swtich = !settings.temp_swtich;
    EEPROM.save();
  }

  void drawMenuTitle()
  {
    // menu title
    switch(state)
    {
    case kMenu_Main: LCD.printIn("Main "); break;
    case kMenu_Time: LCD.printIn("Set Time "); break;
    case kMenu_Date: LCD.printIn("Set Date "); break;
    case kMenu_Temp: LCD.printIn("Temp options "); break;
    }
    LCD.printDight(menu_pos & 0x0f); // current menu item
    LCD.print('/');
    LCD.printDight(state & 0x0f); // menu items count
  }

  void drawMainMenu()
  {
    switch(menu_pos)
    {
    case kMain_Time: LCD.printIn("Time >"); break;
    case kMain_Date: LCD.printIn("Date >"); break;
    case kMain_Temp: LCD.printIn("Temperature >"); break;
    }
  }

  void drawTimeMenu()
  {
    LCD.printDight(RTC.get(DS1307::HOUR_HI)); // 1
    LCD.printDight(RTC.get(DS1307::HOUR_LO)); // 2
    LCD.print(':'); // 3
    LCD.printDight(RTC.get(DS1307::MIN_HI)); // 4
    LCD.printDight(RTC.get(DS1307::MIN_LO)); // 5
    LCD.print(':'); // 6
    LCD.printDight(RTC.get(DS1307::SEC_HI)); // 7
    LCD.printDight(RTC.get(DS1307::SEC_LO)); // 8
    switch(menu_pos)
    {
    case kTime_Hour_Hi: LCD.cursorTo(2,0); break;
    case kTime_Hour_Lo: LCD.cursorTo(2,1); break;
    case kTime_Min_Hi:  LCD.cursorTo(2,4); break;
    case kTime_Min_Lo:  LCD.cursorTo(2,5); break;
    case kTime_Sec_Hi:  LCD.cursorTo(2,7); break;
    case kTime_Sec_Lo:  LCD.cursorTo(2,8); break;
    }
  }

  void drawDateMenu()
  {
    LCD.printDight(RTC.get(DS1307::DATE_HI)); // 1
    LCD.printDight(RTC.get(DS1307::DATE_LO)); // 2
    LCD.print('/'); // 3
    LCD.printDight(RTC.get(DS1307::MONTH_HI)); // 4
    LCD.printDight(RTC.get(DS1307::MONTH_LO)); // 5
    LCD.print('/'); // 6
    LCD.printDight(RTC.get(DS1307::YEAR_HI)); // 7
    LCD.printDight(RTC.get(DS1307::YEAR_LO)); // 8
    switch(menu_pos)
    {
    case kDate_Date_Hi:   LCD.cursorTo(2,0); break;
    case kDate_Date_Lo:   LCD.cursorTo(2,1); break;
    case kDate_Month_Hi:  LCD.cursorTo(2,4); break;
    case kDate_Month_Lo:  LCD.cursorTo(2,5); break;
    case kDate_Year_Hi:   LCD.cursorTo(2,7); break;
    case kDate_Year_Lo:   LCD.cursorTo(2,8); break;
    }
  }

  void drawTempMenu()
  {
    const Settings& settings = EEPROM.get();
    LCD.printIn("Switch > ");
    LCD.printIn((settings.temp_swtich == 0) ? "Off" : "On");
  }

  void drawMonitor()
  {
    // TIME
    LCD.cursorTo(1,0);
    LCD.printDight(RTC.get(DS1307::HOUR_HI)); // 1
    LCD.printDight(RTC.get(DS1307::HOUR_LO)); // 2
    LCD.print(':'); // 3
    LCD.printDight(RTC.get(DS1307::MIN_HI)); // 4
    LCD.printDight(RTC.get(DS1307::MIN_LO)); // 5
    LCD.print(':'); // 6
    LCD.printDight(RTC.get(DS1307::SEC_HI)); // 7
    LCD.printDight(RTC.get(DS1307::SEC_LO)); // 8

    LCD.print(' '); // 9
    LCD.printDight2(TEMP[0]); // 10-11

    // DATE
    LCD.cursorTo(2,0);
    LCD.printDight(RTC.get(DS1307::DATE_HI)); // 1
    LCD.printDight(RTC.get(DS1307::DATE_LO)); // 2
    LCD.print('/'); // 3
    LCD.printDight(RTC.get(DS1307::MONTH_HI)); // 4
    LCD.printDight(RTC.get(DS1307::MONTH_LO)); // 5
    LCD.print('/'); // 6
    LCD.printDight(RTC.get(DS1307::YEAR_HI)); // 7
    LCD.printDight(RTC.get(DS1307::YEAR_LO)); // 8

    LCD.print(' '); // 9
    LCD.printDight2(TEMP[1]); // 10-11
  }

private:
  byte state;
  enum eState
  {
    kMonitor = 0x00,
    kMenu_Main = 0x13,
    kMenu_Time = 0x26,
    kMenu_Date = 0x36,
    kMenu_Temp = 0x41,
  };
  byte menu_pos;
  enum eMenuPos
  {
    kMain_Time = 0x11,
    kMain_Date = 0x12,
    kMain_Temp = 0x13,

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

    kTemp_Switch   = 0x41
  };
};

static void onKeyPressed(byte key)
{
  UI.OnKeyChanged(key);
}