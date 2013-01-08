#pragma once

// Version control settings saved in EEPROM

// EEPROM data version
const byte version_v01 = 0x01;
const byte version_v02 = 0x02; // next version
const byte version_cur = version_v01;

// EEPROM data structure
struct Settings_v01
{
  byte swapTemp; // swap temperature sensors
  uint16_t startDayStamp; // brooder program start day
  byte enableBrooder; // enable brooder part
  byte enableCoop; // enable choop part
} __attribute__ ((aligned (1))); // should be packed to write EEPROM in block

/*
struct Settings_v02
{
  // next version
}
*/

typedef Settings_v01 Settings;

// EEPROM management class
// It loads/saves settings for the persistence
class EpromManager
{
public:
  EpromManager();
  void init();
  void reset();
  Settings& get();
  void save();

private:
  Settings data;
};
