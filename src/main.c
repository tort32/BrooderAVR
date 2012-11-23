//#define F_CPU   12000000L

#include "stdafx.h"
#include <util/delay.h>

/* ----------------------- hardware I/O abstraction ------------------------ */
#define TEMP_1WIRE _BV(PB0)

#define TIME_SCL _BV(PD0)
#define TIME_SDA _BV(PD1)

#define LCD_D4 _BV(PC0)
#define LCD_D5 _BV(PC1)
#define LCD_D6 _BV(PC2)
#define LCD_D7 _BV(PC3)
#define LCD_RS _BV(PC4)
#define LCD_E _BV(PC5)

#define TEMP_PORT PORTB
#define TIME_PORT PORTD
#define LCD_PORT PORTC

static void hardwareInit(void)
{

  /*DDRB=0;
  PORTB=PORTB_KEY;
  DDRD=PORTD_LED;
  PORTD=PORTD_LED|PORTD_KEY;*/

  /* configure timer 0 for a rate of 12M/(1024 * 256) = 45.78 Hz (~22ms) */
  TCCR0 = (1<<CS02)|(0<<CS01)|(1<<CS00); /* timer 0 prescaler: 1024 */
}

/* ------------------------------------------------------------------------- */

int main(void)
{
  //wdt_enable(WDTO_2S);
  hardwareInit();

  _delay_ms(250);
  //clrbits(PORTD,PORTD_LED);
  sei();

  for(;;) {
    //wdt_reset();

    // TODO
  }
  return 0;
}

/* ------------------------------------------------------------------------- */
