#pragma once

#include "OneWire.h"
#include "stdafx.h"

//#define DEBUG_VIA_SERIAL 1

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
#ifdef DEBUG_VIA_SERIAL
    Serial.println("DS18B20 SEARCH");
#endif
    byte rom_cnt = 0;
    while (rom_cnt < rom_max && ow->search(rom[rom_cnt])) {
#ifdef DEBUG_VIA_SERIAL
      Serial.print("ROM = ");
      for(byte i = 0; i < 8; ++i)
        Serial.print(rom[rom_cnt][i], HEX);
#endif
      if (OneWire::crc8(rom[rom_cnt], 7) != rom[rom_cnt][7]) {
#ifdef DEBUG_VIA_SERIAL
        Serial.println(" CRC FAILED");
#endif
        continue;
      }
      if (rom[rom_cnt][0] == 0x28)
      {
#ifdef DEBUG_VIA_SERIAL
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
#ifdef DEBUG_VIA_SERIAL
    Serial.print("DS18B20 CHECK ");
#endif
    byte ready = ow->read_bit();
#ifdef DEBUG_VIA_SERIAL
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
#ifdef DEBUG_VIA_SERIAL
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
#ifdef DEBUG_VIA_SERIAL
    Serial.println("DS18B20 START");
#endif
    ow->reset();
    ow->select(rom);
    ow->write(0x44); // CONVERT T
  }

  // Reading temperatire in celsius from device
  bool read(byte rom[8], volatile byte* temp)
  {
#ifdef DEBUG_VIA_SERIAL
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
#ifdef DEBUG_VIA_SERIAL
      Serial.println("CRC FAILED");
#endif
      return false;
    }

    uint32_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
    byte celsius = raw / 16;

#ifdef DEBUG_VIA_SERIAL
    Serial.print("T = ");
    Serial.println(celsius, DEC);
#endif

    *temp = celsius;
    return true;
  }
private:
  OneWire* ow;
};

#include "LCD4Bit_mod.h"

const byte rom_max = 2; // Number of sensors
class TempManager: private DS18B20
{
public:
  TempManager(const OneWire& oneWire)
    : DS18B20(oneWire)
  {
    status = kInvalid;
    for(byte i=0;i<rom_max;++i)
      value[i]=255;
  }

  void init()
  {
    byte rom_found = search(rom, rom_max);

    if(rom_found != rom_max)
    {
      error("Lost TEMP");

      LCD.cursorTo(2,0);
      LCD.printIn("Found ");
      LCD.printDight(rom_found);
      LCD.print('/');
      LCD.printDight(rom_max);

      beep(3);
      return;
    }

    status = kIdle;
  }

  void update()
  {
    if(status == kIdle)
    {
      start(); // ~2.9ms
      status = kConversion;
      return;
    }

    if(status == kConversion)
    {
      if(is_ready()) // ~100us
      {
        status = kRead;
        return;
      }
      clrbits(LED_PORT, LED);
    }

    if(status & kRead)
    {
      byte id = status & kReadMask;
      if(id < rom_max)
      {
        read(rom[id], &value[id]); // ~21.6ms
        status = kRead | (++id);
        return;
      }
      else
      {
        status = kIdle;
        return;
      }
    }
  }

  byte operator [](byte device)
  {
    return value[device];
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
    const DS18B20* ds;
    byte rom[rom_max][8]; // OneWire ROMs for 1-wire temperature sensors
    byte value[rom_max];
    byte status;
};