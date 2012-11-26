#define F_CPU   1000000L

#include "stdafx.h"
#include <util/delay.h>

/* ----------------------- hardware I/O abstraction ------------------------ */

// temperatire 1-wire sensors
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

// alarm output
#define ALARM_OUT _BV(PB5)// D13
#define ALARM_OUT_PORT PORTB

/* ------------------------------------------------------------------------- */
//OneWire temp(2);
#include "LCD4Bit_mod.h"
LCD4Bit_mod lcd(2); // 16x2 literal

//#include "DS1307.h"
// DS1307 RTC();
/* ------------------------------------------------------------------------- */

void setup(void)
{
  DDRB = ALARM_OUT;
  /*DDRD = LCD_DATA | TEMP_1WIRE
  DDRB = ALARM_OUT | LCD_REG;
  DDRC = 0x00;*/

  /* configure timer 0 for a rate of 12M/(1024 * 256) = 45.78 Hz (~22ms) */
  //TCCR0 = (1<<CS02)|(0<<CS01)|(1<<CS00); /* timer 0 prescaler: 1024 */

  lcd.init();
}

void loop(void)
{
  _delay_ms(100);
  static int8_t cnt = 0;
  tglbits(ALARM_OUT_PORT, ALARM_OUT);
  lcd.print(cnt++);
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
