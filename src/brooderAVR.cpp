#include "stdafx.h"

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
#define LED _BV(PB5) // D13
#define LED_PORT PORTB

#define ALARM_OUT _BV(PB2)
#define ALARM_OUT_PORT PORTB

/* ------------------------------------------------------------------------- */
#include "LCD4Bit_mod.h"
LCD4Bit_mod lcd(2); // 16x2 literal

#include "DS1307.h"

#include "OneWire.h"
OneWire oneWire(2); // D2

#include "DS18B20.h"
DS18B20 temp(oneWire);

#include "KeyPad.h"
void onKeyPressed(byte);
KeyPad keys(A0, onKeyPressed);

#include "HardwareSerial.h"

// OneWire ROMs for 1-wire temperature sensors
const byte rom_max = 2;
byte rom[rom_max][8];

byte tmp1 = 255;
byte tmp2 = 255;

/* ------------------------------------------------------------------------- */
volatile byte STATUS = 0x0; // this is 8 status bits

#define ERROR _BV(0)
#define TEMP_STARTED _BV(1)

inline bool is_error()
{
  return is_bits(STATUS, ERROR);
}

void beep(byte beep_cnt)
{
  for(; beep_cnt; --beep_cnt)
  {
    setbits(ALARM_OUT_PORT, ALARM_OUT);
    delay(200);
    clrbits(ALARM_OUT_PORT, ALARM_OUT);
    delay(200);
  }
}

void error(const char* msg)
{
  setbits(STATUS, ERROR); // rise up error status bit

  lcd.clear();
  lcd.printIn(msg);
  Serial.println(msg);
}

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

/* ------------------------------------------------------------------------- */

void onKeyPressed(byte key)
{
  Serial.print("KEY = ");
  Serial.println(key, DEC);
}

void setup(void)
{
  DDRB = ALARM_OUT | LED;

  lcd.init();
  lcd.clear();
  lcd.printIn("Hello");

  Serial.begin(9600);

  byte rom_found = temp.search(rom, rom_max);

  //delay(250);

  if(rom_found != rom_max)
  {
    error("Lost TEMP");

    lcd.cursorTo(2,0);
    lcd.printIn("Found ");
    lcd_print_dight(rom_found);
    lcd.print('/');
    lcd_print_dight(rom_max);

    beep(3);
    return;
  }

  lcd.clear();
}

void loop(void)
{
  tglbits(LED_PORT, LED);

  if(is_error())
    return;

  if(!is_bits(STATUS,TEMP_STARTED))
  {
    temp.start();
    setbits(STATUS,TEMP_STARTED);
  }

  byte key = keys.read();

  static byte rtc_cnt = 0;
  if(++rtc_cnt == 20)
  {
    RTC.read(); // bottleneck ~100ms
    rtc_cnt = 0;
  }

  if(temp.is_ready())
  {
    temp.read(rom[0], &tmp1);
    temp.read(rom[1], &tmp2);
    clrbits(STATUS,TEMP_STARTED);
  }

  // TIME
  lcd.cursorTo(1,0);
  lcd_print_dight(RTC.get(DS1307::HOUR_HI)); // 1
  lcd_print_dight(RTC.get(DS1307::HOUR_LO)); // 2
  lcd.print(':'); // 3
  lcd_print_dight(RTC.get(DS1307::MIN_HI)); // 4
  lcd_print_dight(RTC.get(DS1307::MIN_LO)); // 5
  lcd.print(':'); // 6
  lcd_print_dight(RTC.get(DS1307::SEC_HI)); // 7
  lcd_print_dight(RTC.get(DS1307::SEC_LO)); // 8

  lcd.print(' '); // 9
  lcd_print_dight2(tmp1); // 10-11

  // DATE
  lcd.cursorTo(2,0);
  lcd_print_dight(RTC.get(DS1307::DATE_HI)); // 1
  lcd_print_dight(RTC.get(DS1307::DATE_LO)); // 2
  lcd.print('/'); // 3
  lcd_print_dight(RTC.get(DS1307::MONTH_HI)); // 4
  lcd_print_dight(RTC.get(DS1307::MONTH_LO)); // 5
  lcd.print('/'); // 6
  lcd_print_dight(RTC.get(DS1307::YEAR_HI)); // 7
  lcd_print_dight(RTC.get(DS1307::YEAR_LO)); // 8

  lcd.print(' '); // 9
  lcd_print_dight2(tmp2); // 10-11
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
