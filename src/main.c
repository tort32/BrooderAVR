//#define F_CPU   12000000L

#include "stdafx.h"
#include <util/delay.h>

/* ----------------------- hardware I/O abstraction ------------------------ */
/*#define KEY1 _BV(PB0)
#define KEY2 _BV(PB1)
#define KEY3 _BV(PB2)
#define KEY4 _BV(PB3)
#define KEY5 _BV(PB4)
#define KEY6 _BV(PB5)
#define KEY7 _BV(PB6)
#define KEY8 _BV(PB7)

#define KEY9 _BV(0)
#define KEY10 _BV(1)
#define KEY11 _BV(2)
#define KEY12 _BV(3)
#define KEY13 _BV(4)
#define KEY14 _BV(5)
#define KEY15 _BV(6)
#define KEY16 _BV(7)

#define KEYA _BV(PD0)
#define KEYB _BV(PD6)

#define LED1 _BV(PD3)
#define LED2 _BV(PD4)
#define LED3 _BV(PD5)

#define PORTB_KEY (KEY1|KEY2|KEY3|KEY4|KEY5|KEY6|KEY7|KEY8)
#define PORTD_KEY (KEYA|KEYB)
#define PORTD_LED (LED1|LED2|LED3)*/

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
