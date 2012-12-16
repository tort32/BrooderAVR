#include "stdafx.h"
#include "LCD4Bit_mod.h"
#include "DS1307.h"
#include "OneWire.h"
#include "DS18B20.h"
#include "KeyPad.h"
#include "UI.h"

OneWire oneWire(2); // D2
TempManager temp(oneWire);

UIManager ui;
/* ------------------------------------------------------------------------- */
void setup(void)
{
  DDRB = ALARM_OUT | LED;

  LCD.init();
  LCD.printIn("Hello");

  Serial.begin(9600);

  temp.init();

  delay(1000);

  LCD.clear();
}

void loop(void)
{
  // This is a heartbeat pulse
  // Used for debugging cycle time
  //tglbits(LED_PORT, LED);

  if(is_error())
    return;

  // Keys should be responsive
  // So update it every cycle
  setbits(LED_PORT, LED);
  ui.inputKeys(); // ~1.8ms
  clrbits(LED_PORT, LED);

  // Some update parts take much time to perform
  // We use frame counter and schedule updates call for defined frames
  // 'frame_max' defines how mush cycles takes full update.
  // We can have empty update frames to
  // This value is determined empirical from real cycle speed.
  static byte frame = 0;
  const byte frame_max = 16;
  switch(frame)
  {
  case 0:
    // Start temperature conversion
    temp.update(); // up to 21.6ms at read
    break;
  case 1:
    // Read time from RTC
    RTC.read(); // ~5.7ms
    break;
  case 2:
    // LCD update
    ui.outputLCD(); // ~5.4ms
    break;
  case 3:
    // Switch on alarm signal on alarm status
    wrtbits(ALARM_OUT_PORT, is_alarm() ? ALARM_OUT : 0, ALARM_OUT);
    break;
  default:
    _delay_ms(20);
    break;
  }
  if(++frame == frame_max) frame = 0;
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
