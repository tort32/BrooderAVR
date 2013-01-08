#pragma once

#include "OneWire.h"
#include "stdafx.h"
#include "Alarm.h"

//#define DEBUG_DS18B20 1

// DS18B20 class routine
// 
class DS18B20
{
public:
  DS18B20(const OneWire& oneWire)
  {
    ow = const_cast<OneWire*>(&oneWire);
  }

  // rom is rom_max count arrays by 8 bytes each for returning device ROM's
  // Returns the number of found devices
  byte search(byte rom[][8], const byte rom_max)
  {
#ifdef DEBUG_DS18B20
    Serial.println("DS18B20 SEARCH");
#endif
    byte rom_cnt = 0;
    while (rom_cnt < rom_max && ow->search(rom[rom_cnt])) {
#ifdef DEBUG_DS18B20
      Serial.print("ROM = ");
      for(byte i = 0; i < 8; ++i)
        Serial.print(rom[rom_cnt][i], HEX);
#endif
      if (OneWire::crc8(rom[rom_cnt], 7) != rom[rom_cnt][7]) {
#ifdef DEBUG_DS18B20
        Serial.println(" CRC FAILED");
#endif
        continue;
      }
      if (rom[rom_cnt][0] == 0x28)
      {
#ifdef DEBUG_DS18B20
        Serial.println(" DS18B20");
#endif
        rom_cnt++;
      }
    }
    ow->reset_search();
    return rom_cnt;
  }

  // Read conversion ready status
  // This is used only just after DS18B20::start call
  // NOTE: if 1-wire reset was issued you have to make 44h command again before the reading time slot
  bool is_ready()
  {
#ifdef DEBUG_DS18B20
    Serial.print("DS18B20 CHECK ");
#endif
    byte ready = ow->read_bit();
#ifdef DEBUG_DS18B20
    Serial.print("R = ");
    Serial.println(ready, HEX);
#endif
    return ready;
  }

  // Request start the temperature conversion for all ROMs
  // You can call DS18B20::is_ready to check the conversion is done
  // This way ready status returns true only after all devices finished the conversion
  void start()
  {
#ifdef DEBUG_DS18B20
    Serial.println("DS18B20 START");
#endif
    ow->reset();
    ow->skip();
    ow->write(0x44); // CONVERT T
  }

  // Request start the temperature conversion for one device
  // You can call DS18B20::is_ready to check the conversion is done
  void start(byte rom[8])
  {
#ifdef DEBUG_DS18B20
    Serial.println("DS18B20 START");
#endif
    ow->reset();
    ow->select(rom);
    ow->write(0x44); // CONVERT T
  }

  // Reading temperature in celsius from device
  bool read(byte rom[8], volatile int8_t* temp)
  {
    int16_t temp_raw;
    if(!read(rom, &temp_raw))
      return false;

    int8_t celsius = (int8_t)(temp_raw / 16);

#ifdef DEBUG_DS18B20
    Serial.print("T = ");
    Serial.println(celsius, DEC);
#endif

    *temp = celsius;
    return true;
  }

  // Reading raw temperature from device
  bool read(byte rom[8], volatile int16_t* temp)
  {
#ifdef DEBUG_DS18B20
    Serial.print("DS18B20 READ ");
#endif
    ow->reset();
    ow->select(rom);
    ow->write(0xBE); // Read Scratchpad

    byte data[9]; // we need 9 bytes
    for (byte i = 0; i < 9; i++) {
      data[i] = ow->read();
    }

    ow->reset(); // stop reading

    byte crc = OneWire::crc8( data, 8);
    if(data[8] != crc)
    {
#ifdef DEBUG_DS18B20
      Serial.println("CRC FAILED");
#endif
      return false;
    }

    //    bits      7     6     5     4     3     2     1     0
    // data[0] = 2^+3, 2^+2, 2^+1, 2^+0, 2^-1, 2^-2, 2^-3, 2^-4
    // data[1] = SIGN, SIGN, SIGN, SIGN, SIGN, 2^+6, 2^+5, 2^+4

    // In low precision modes last low bits are undefined. So we should discard it.
    byte cfg = (data[4] & 0x60);
    byte raw_hi = data[1] & 0x87; // take a sign and 3 low data bits
    byte raw_lo;
    if (cfg == 0x00) // 9 bit resolution, 93.75 ms
      raw_lo = data[0] & 0xF8;
    else if (cfg == 0x20) // 10 bit res, 187.5 ms
      raw_lo = data[0] & 0xFC;
    else if (cfg == 0x40) // 11 bit res, 375 ms
      raw_lo = data[0] & 0xFE;
    else // default is 12 bit resolution, 750 ms conversion time
      raw_lo = data[0];

    uint16_t raw = (raw_hi << 8) | raw_lo;

#ifdef DEBUG_DS18B20
    Serial.print("CFG = ");
    Serial.println(cfg, HEX);
    Serial.print("RAW = ");
    Serial.println(raw, HEX);
#endif

    *temp = *((int16_t*)((uint16_t*)&raw));
    return true;
  }
private:
  OneWire* ow;
};

#include "LCD4Bit_mod.h"
#include "EEPROM.h"

// Temperature format using for TempManager
#define TEMP_RAW_INT 1
#ifdef TEMP_RAW_INT
typedef int16_t RawTemp;
#define CELSIUS_TO_RAW(t) ((int16_t)(t*16))
#define RAW_TO_CELSIUS(t) ((int8_t)(t/16))
#else
typedef int8_t RawTemp;
#define CELSIUS_TO_RAW(t) t
#define RAW_TO_CELSIUS(t) t
#endif

const byte rom_max = 2; // Number of sensors

// Temperature sensors management class
// Requests temperature sensors and read values
// Using settings to swap temperature sensors
class TempManager: private DS18B20
{
public:
  enum {
    kTemp_Invalid = (RawTemp)CELSIUS_TO_RAW(55), // default power-on reset value
  };

  TempManager(const OneWire& oneWire)
    : DS18B20(oneWire)
  {
    status = kInvalid;
    for(byte i=0;i<rom_max;++i)
      value[i]=kTemp_Invalid;
  }

  void init()
  {
    byte rom_found = search(rom, rom_max);

    if(rom_found != rom_max)
    {
      ALARM.error("Lost TEMP");

      LCD.cursorTo(2,0);
      LCD.printIn("Found ");
      LCD.printDigit(rom_found);
      LCD.print('/');
      LCD.printDigit(rom_max);

      ALARM.beep(3);
      return;
    }

    status = kIdle;
  }

  void update()
  {
    // 'status' used to cycle between steps:
    //   Idle -> Conversion -> Read 0 -> Read 1 -> Idle

    // When Idle we can start conversion
    if(status == kIdle)
    {
      start(); // ~2.9ms
      status = kConversion; // move to next step
      return;
    }

    // While Conversion we should wait all sensors is ready
    if(status == kConversion)
    {
      if(is_ready()) // ~100us
      {
        status = kRead; // move to next step
        return;
      }
    }

    // At Read we sequentially read sensors values
    if(status & kRead)
    {
      byte id = status & kReadMask;
      if(id < rom_max)
      {
        read(rom[id], &value[id]); // ~21.6ms
        status = kRead | (++id); // move to next sensor
        return;
      }
      else
      {
        status = kIdle; // move to next step
        return;
      }
    }
  }

  // Returns sensor value
  RawTemp operator [](byte device)
  {
    // Take into account swap sensors flag
    if(EEPROM.get().swapTemp == 0)
      return value[device]; // default order
    else
      return value[(rom_max - 1) - device]; // reverse order
  }

  private:
    enum
    {
      kIdle    = 0x0,
      kConversion = 0x1,
      kRead = 0x10,
      kReadMask = 0x0f,
      kInvalid = 0x80,
    };
    byte rom[rom_max][8]; // OneWire ROMs for 1-wire temperature sensors
    RawTemp value[rom_max]; // Sensors data array
    byte status; // function step counter
};