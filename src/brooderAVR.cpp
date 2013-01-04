#include "stdafx.h"
#include "Alarm.h"
#include "LCD4Bit_mod.h"
#include "HardwareSerial.h"
#include "UI.h"
#include "EEPROM.h"
#include "System.h"

/* ------------------------------------------------------------------------- */
void setup(void)
{
  Serial.begin(9600);

  ALARM.init();

  LCD.init();
  LCD.printIn("Hello");

  EEPROM.init();

  TEMP.init();

  SYSTEM.init();

  delay(250);

  LCD.clear();

  /*const byte smiley[8] = {
    B01110,
    B10101,
    B11111,
    B11111,
    B10001,
    B11011,
    B01110,
    B00000
  };

  for(byte i=0;i<8;++i)
    LCD.buildChar(i,smiley);

  LCD.cursorTo(1,0);
  char str[17];
  for(byte i = 0; i < 16; ++i)
    str[i] = i + 0x00;
  str[16] = '\0';
  LCD.printIn(str);

  LCD.cursorTo(2,0);
  LCD.printIn("0123456789ABCDEF");

  ALARM.set_error(1);*/
}

void loop(void)
{
  // This is a heartbeat pulse
  // Used for debugging cycle time
  //tglbits(LED_PORT, LED);

  if(ALARM.is_error())
    return;

  // Keys should be responsive
  // So update it every cycle
  setbits(LED_PORT, LED);
  UI.inputKeys(); // ~1.8ms
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
    TEMP.update(); // up to 21.6ms at read
    break;
  case 1:
    // Read time from RTC
    RTC.read(); // ~5.7ms
    break;
  case 2:
    // LCD update
    UI.outputLCD(); // ~5.4ms
    break;
  case 3:
    // Switch on alarm signal on alarm status
    ALARM.update();
    break;
  case 4:
    SYSTEM.update();
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
