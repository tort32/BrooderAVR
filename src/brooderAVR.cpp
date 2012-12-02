#include "stdafx.h"
#include <util/delay.h>

/* ----------------------- hardware I/O abstraction ------------------------ */

/*
// temperature 1-wire sensors
#define TEMP_1WIRE _BV(PD2) // D2
#define TEMP_1WIRE_PORT PORTD

// light sensor
#define LIGHT_IN _BV(PC1) // A1
#define LIGHT_PORT PORTC

// real time clock
#define TIME_SDA _BV(PC4) // A4
#define TIME_SCL _BV(PC5) // A5
#define TIME_PORT PORTC

// LCD data bus
#define LCD_DATA_D4 _BV(PD4) // D4
#define LCD_DATA_D5 _BV(PD5) // D5
#define LCD_DATA_D6 _BV(PD6) // D6
#define LCD_DATA_D7 _BV(PD7) // D7
#define LCD_DATA (LCD_DATA_D4|LCD_DATA_D5|LCD_DATA_D6|LCD_DATA_D7)
#define LCD_DATA_PORT PORTD

// LCD register control
#define LCD_REG_RS _BV(PB0) // D8
#define LCD_REG_E _BV(PB1) // D9
#define LCD_REG (LCD_REG_RS|LCD_REG_E)
#define LCD_REG_PORT PORTB

// LCD keys
#define LCD_BUT_IN _BV(PC0) // A0
#define LCD_BUT_IN_PORT PORTC
*/

// alarm output
#define ALARM_OUT _BV(PB5)// D13
#define ALARM_OUT_PORT PORTB

/* ------------------------------------------------------------------------- */
#include "LCD4Bit_mod.h"
LCD4Bit_mod lcd(2); // 16x2 literal

#include "DS1307.h"

#include "OneWire.h"
OneWire oneWire(2); // D2

const byte rom_max = 2;
byte rom[rom_max][8]; // OneWire ROMs

#include "HardwareSerial.h"
/* ------------------------------------------------------------------------- */

void lcd_print_dight(byte value)
{
  byte ch = value < 10 ? value + '0' : value - 10 + 'A';
  lcd.print(ch);
}

inline void lcd_print_hex(byte value)
{
  lcd_print_dight(value);
}

inline void lcd_print_hex2(byte value)
{
  byte h2 = value / 16;
  byte h1 = value % 16;
  lcd_print_hex(h2);
  lcd_print_hex(h1);
}

void lcd_print_dight2(byte value)
{
  byte d2 = value / 10;
  byte d1 = value % 10;
  lcd_print_dight(d2);
  lcd_print_dight(d1);
}

void lcd_print_dight3(byte value)
{
  byte d3 = value / 100;
  byte d2 = (value / 10) % 10;
  byte d1 = value % 10;
  lcd_print_dight(d3);
  lcd_print_dight(d2);
  lcd_print_dight(d1);
}

void temp_start(byte device)
{
  oneWire.reset();
  oneWire.select(rom[device]);
  oneWire.write(0x44,1);
}

bool temp_is_ready(byte device)
{
  
}

byte temp_read(byte device)
{
  byte present = oneWire.reset();
  oneWire.select(rom[device]);
  oneWire.write(0xBE); // Read Scratchpad

  byte data[9]; // we need 9 bytes
  for (byte i = 0; i < 9; i++) {
    data[i] = oneWire.read();
  }
  //Serial.print( OneWire::crc8( data, 8), HEX);
  //Serial.println();

  uint32_t raw = (data[1] << 8) | data[0];
  byte cfg = (data[4] & 0x60);
  if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
  // default is 12 bit resolution, 750 ms conversion time
  byte celsius = raw / 16;

  return celsius;
}

/* ------------------------------------------------------------------------- */

void setup(void)
{
  Serial.begin(9600);

  DDRB = ALARM_OUT;

  int rom_cnt = 0;
  while ( rom_cnt<rom_max && oneWire.search(rom[rom_cnt])) {
    if ( OneWire::crc8( rom[rom_cnt], 7) != rom[rom_cnt][7]) {
      Serial.println("ROM CRC is not valid!");
      return;
    }
    if(rom[rom_cnt][0] == 0x28)
    {
      rom_cnt++;
    }
  }

  oneWire.reset_search();
  //delay(250);

  if(rom_cnt != 2)
  {
    Serial.println("Can't find ROMs");
    return;
  }

  lcd.init();
  lcd.clear();
  //lcd.printIn("hello");
}

bool temp = true;

void loop(void)
{
  tglbits(ALARM_OUT_PORT, ALARM_OUT);

  temp_start(0);
  temp_start(1);

  RTC.read();

  delay(750);

  byte tmp1=temp_read(0);
  byte tmp2=temp_read(1);

  // TIME
  lcd.cursorTo(1,0);
  lcd_print_dight(RTC.get(DS1307::HOUR_HI));
  lcd_print_dight(RTC.get(DS1307::HOUR_LO));
  lcd.print(':');
  lcd_print_dight(RTC.get(DS1307::MIN_HI));
  lcd_print_dight(RTC.get(DS1307::MIN_LO));
  lcd.print(':');
  lcd_print_dight(RTC.get(DS1307::SEC_HI));
  lcd_print_dight(RTC.get(DS1307::SEC_LO));

  lcd.print(' ');
  lcd_print_dight2(tmp1);

  // DATE
  lcd.cursorTo(2,0);
  lcd_print_dight(RTC.get(DS1307::DATE_HI));
  lcd_print_dight(RTC.get(DS1307::DATE_LO));
  lcd.print('/');
  lcd_print_dight(RTC.get(DS1307::MONTH_HI));
  lcd_print_dight(RTC.get(DS1307::MONTH_LO));
  lcd.print('/');
  lcd_print_dight(RTC.get(DS1307::YEAR_HI));
  lcd_print_dight(RTC.get(DS1307::YEAR_LO));

  lcd.print(' ');
  lcd_print_dight2(tmp2);
}

/* ------------------------------------------------------------------------- */

int main(void)
{
  init();
  setup();

  for (;;) {
    loop();
    if (serialEventRun) serialEventRun();
  }

  return 0;
}
