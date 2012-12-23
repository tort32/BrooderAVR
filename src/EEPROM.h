#pragma once

// eeprom data version should match to 0xf0 mask
const byte version_v01 = 0x01;
const byte version_cur = version_v01;

// eeprom data structure
struct Settings_v01
{
  byte temp_swtich; // swap temperature sensors
  uint16_t startDayStamp; // brooder program start day
} __attribute__ ((aligned (1))); // should be packed to write EEPROM in block

typedef Settings_v01 Settings;

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
