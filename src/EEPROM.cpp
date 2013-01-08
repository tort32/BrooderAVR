#include "stdafx.h"
#include "EEPROM.h"
#include "avr/eeprom.h"
#include "System.h"

//#define DEBUG_EEPROM 1

// EEPROM section vars
byte eeVersion EEMEM;
byte eeLength EEMEM;
Settings eeData EEMEM;

EpromManager::EpromManager()
{
}

void EpromManager::init()
{
  // Load EEPROM data and check data version
  byte data_sizeof = sizeof(Settings);
#ifdef DEBUG_EEPROM
  Serial.println("EEPROM read");
#endif
  byte version = eeprom_read_byte(&eeVersion);
#ifdef DEBUG_EEPROM
  Serial.print("EEPROM version = ");
  Serial.println(version, HEX);
#endif
  byte dataLength = eeprom_read_byte(&eeLength);
#ifdef DEBUG_EEPROM
  Serial.print("EEPROM DataSize = ");
  Serial.println(dataLength, HEX);
#endif
  if(version_cur == version && data_sizeof == dataLength)
  {
    eeprom_read_block(&data, &eeData, (size_t)data_sizeof);
#ifdef DEBUG_EEPROM
    Serial.print("EEPROM Data = ");
    for(byte i = 0; i < data_sizeof; ++i)
    {
      Serial.print(((byte*)&data)[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
#endif
  }
  else
  {
    // TODO: convert old version settings
    reset();
    save();
  }
}

// Reset settings
void EpromManager::reset()
{
#ifdef DEBUG_EEPROM
  Serial.println("EEPROM reset");
#endif
  // reset settings
  data.swapTemp = 0;
  data.startDayStamp = SYSTEM.kDay_Invalid;
  data.enableBrooder = 0;
  data.enableCoop = 0;
}

// Returns settings for read/update
Settings& EpromManager::get()
{
  return data;
}

// Saves EEPROM data
void EpromManager::save()
{
#ifdef DEBUG_EEPROM
  Serial.println("EEPROM save");
#endif
  byte data_size = sizeof(Settings);
  eeprom_update_byte(&eeVersion, version_cur); // write version code
  eeprom_update_byte(&eeLength, data_size); // write data size
  eeprom_update_block(&data, &eeData, data_size); // write data
}