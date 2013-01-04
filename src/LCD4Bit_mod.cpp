/*
LCD4Bit v0.1 16/Oct/2006 neillzero http://abstractplain.net

What is this?
An arduino library for comms with HD44780-compatible LCD, in 4-bit mode (saves pins)

Sources:
- The original "LiquidCrystal" 8-bit library and tutorial
    http://www.arduino.cc/en/uploads/Tutorial/LiquidCrystal.zip
    http://www.arduino.cc/en/Tutorial/LCDLibrary
- DEM 16216 datasheet http://www.maplin.co.uk/Media/PDFs/N27AZ.pdf
- Massimo's suggested 4-bit code (I took initialization from here) http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1144924220/8
See also:
- glasspusher's code (probably more correct): http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1160586800/0#0

Tested only with a DEM 16216 (maplin "N27AZ" - http://www.maplin.co.uk/Search.aspx?criteria=N27AZ)
If you use this successfully, consider feeding back to the arduino wiki with a note of which LCD it worked on.

Usage:
see the examples folder of this library distribution.

*/
#include "LCD4Bit_mod.h"
#include "arduino.h"		//all things wiring / arduino

extern "C" {
  #include <stdio.h>		//not needed yet
  #include <string.h>		//needed for strlen()
  #include <inttypes.h>
}

#include "stdafx.h"

//command bytes for LCD
#define CMD_CLEAR     0x01 /* 0000.0001 */
#define CMD_HOME      0x02 /* 0000.0010 */
#define CMD_ENTRY     0x04 /* 0000.0100 */
#define CMD_DISPLAY   0x08 /* 0000.1000 */
#define CMD_SHIFT     0x10 /* 0001.0000 */
#define CMD_FUNCTION  0x20 /* 0010.0000 */
#define CMD_SETCGRAM  0x40 /* 0100.0000 */
#define CMD_SETDDRAM  0x80 /* 1000.0000 */

// flags for CMD_ENTRY
#define OPT_ENTRY_RIGHT 0x00
#define OPT_ENTRY_LEFT 0x02
#define OPT_ENTRY_SHIFTINCREMENT 0x01
#define OPT_ENTRY_SHIFTDECREMENT 0x00

// flags for LCD_DISPLAY
#define CMD_DISPLAY_DISPLAY_ON 0x04
#define CMD_DISPLAY_DISPLAY_OFF 0x00
#define CMD_DISPLAY_CURSOR_ON 0x02
#define CMD_DISPLAY_CURSOR_OFF 0x00
#define CMD_DISPLAY_BLINK_ON 0x01
#define CMD_DISPLAY_BLINK_OFF 0x00

// flags for CMD_SHIFT
#define CMD_SHIFT_CURSOR_RIGHT 0x0C
#define CMD_SHIFT_CURSOR_LEFT  0x08

// --------- PINS -------------------------------------
//is the RW pin of the LCD under our control?  If we're only ever going to write to the LCD, we can use one less microcontroller pin, and just tie the LCD pin to the necessary signal, high or low.
//this stops us sending signals to the RW pin if it isn't being used.
//#define USING_RW

//RS, RW and Enable can be set to whatever you like
int RS = 8;
int RW = 11;
int Enable = 9;
//DB should be an unseparated group of pins  - because of lazy coding in pushNibble()
int DB[] = {4, 5, 6, 7};  //wire these to DB4~7 on LCD.

#define DATA_PORT PORTD

#define DATA_PINS 0xF0
#define DATA_PINS_LO_NIBBLE(x) ((x & 0x0f) << 4)
#define DATA_PINS_HI_NIBBLE(x) (x & 0xf0)

#define CONTROL_PORT PORTB
#define CONTROL_RS _BV(PB0)
#define CONTROL_EN _BV(PB1)
#define CONTROL_RW _BV(PB4)

//--------------------------------------------------------

//how many lines has the LCD? (don't change here - specify on calling constructor)
byte g_num_lines = 2;

//pulse the Enable pin high (for a microsecond).
//This clocks whatever command or data is in DB4~7 into the LCD controller.
void LCD4Bit_mod::pulseEnablePin(){
  //clrbits(CONTROL_PORT, CONTROL_EN); //digitalWrite(Enable,LOW);
  //delayMicroseconds(1);
  // send a pulse to enable
  setbits(CONTROL_PORT, CONTROL_EN); //digitalWrite(Enable,HIGH);
  delayMicroseconds(1); // enable pulse must be >450ns
  clrbits(CONTROL_PORT, CONTROL_EN); //digitalWrite(Enable,LOW);
  //delayMicroseconds(40); // commands need > 37us to settle
}

//push a nibble of data through the the LCD's DB4~7 pins, clocking with the Enable pin.
//We don't care what RS and RW are, here.
void LCD4Bit_mod::pushNibble(uint8_t value){
  wrtbits(DATA_PORT, DATA_PINS_LO_NIBBLE(value), DATA_PINS);
  pulseEnablePin();
}

//push a byte of data through the LCD's DB4~7 pins, in two steps, clocking each with the enable pin.
void LCD4Bit_mod::pushByte(uint8_t value){
  wrtbits(DATA_PORT, DATA_PINS_HI_NIBBLE(value), DATA_PINS);
  pulseEnablePin();
  wrtbits(DATA_PORT, DATA_PINS_LO_NIBBLE(value), DATA_PINS);
  pulseEnablePin();
}


//stuff the library user might call---------------------------------

//constructor. Use default num of lines
LCD4Bit_mod::LCD4Bit_mod() {}

//constructor.  num_lines must be 1 or 2, currently.
LCD4Bit_mod::LCD4Bit_mod(uint8_t num_lines) {
  g_num_lines = num_lines;
  if (g_num_lines < 1 || g_num_lines > 2)
  {
    g_num_lines = 1;
  }
}

void LCD4Bit_mod::commandWriteNibble(uint8_t nibble) {
  clrbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  pushNibble(nibble);
  delayMicroseconds(40); // commands need > 37us to settle
}


void LCD4Bit_mod::commandWrite(uint8_t value) {
  clrbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  pushByte(value);
  delayMicroseconds(40); // commands need > 37us to settle
}


//print the given character at the current cursor position. overwrites, doesn't insert.
void LCD4Bit_mod::print(uint8_t value) {
  //set the RS and RW pins to show we're writing data
  setbits(CONTROL_PORT, CONTROL_RS);//digitalWrite(RS, HIGH);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);//digitalWrite(RW, LOW);
#endif
  //let pushByte worry about the intricacies of Enable, nibble order.
  pushByte(value);
  delayMicroseconds(40); // commands need > 37us to settle
}


//print the given string to the LCD at the current cursor position.  overwrites, doesn't insert.
//While I don't understand why this was named printIn (PRINT IN?) in the original LiquidCrystal library, I've preserved it here to maintain the interchangeability of the two libraries.
void LCD4Bit_mod::printIn(const char* msg) {
  setbits(CONTROL_PORT, CONTROL_RS);//digitalWrite(RS, HIGH);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);//digitalWrite(RW, LOW);
#endif
  uint8_t i;  //fancy int.  avoids compiler warning when comparing i with strlen()'s uint8_t
  uint8_t len = strlen(msg);
  for (i=0;i < len ;i++){
    //print(msg[i]);
    //let pushByte worry about the intricacies of Enable, nibble order.
    pushByte(msg[i]);
    delayMicroseconds(40); // commands need > 37us to settle
  }
}


//send the clear screen command to the LCD
void LCD4Bit_mod::clear(){
  clrbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  pushByte(CMD_CLEAR);
  delayMicroseconds(1640); // 1.64ms
}

void LCD4Bit_mod::home(){
  clrbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  pushByte(CMD_HOME);
  delayMicroseconds(1640); // 1.64ms
}


// initialize LCD after a short pause
//while there are hard-coded details here of lines, cursor and blink settings, you can override these original settings after calling .init()
void LCD4Bit_mod::init() {
  pinMode(Enable,OUTPUT);
  pinMode(RS,OUTPUT);
#ifdef USING_RW
  pinMode(RW,OUTPUT);
#endif
  pinMode(DB[0],OUTPUT);
  pinMode(DB[1],OUTPUT);
  pinMode(DB[2],OUTPUT);
  pinMode(DB[3],OUTPUT);

  delay(50);

  //The first 4 nibbles and timings are not in my DEM16217 SYH datasheet, but apparently are HD44780 standard...
  commandWriteNibble(0x03);
  delay(5);
  commandWriteNibble(0x03);
  delayMicroseconds(100);
  commandWriteNibble(0x03);
  delay(5);

  // needed by the LCDs controller
  //this being 2 sets up 4-bit mode.
  commandWriteNibble(0x02);
  commandWriteNibble(0x02);
  //todo: make configurable by the user of this library.
  //NFXX where
  //N = num lines (0=1 line or 1=2 lines).
  //F= format (number of dots (0=5x7 or 1=5x10)).
  //X=don't care

  uint8_t num_lines_ptn = g_num_lines - 1 << 3;
  uint8_t dot_format_ptn = 0x00;      //5x7 dots.  0x04 is 5x10

  commandWriteNibble(num_lines_ptn | dot_format_ptn);
  delayMicroseconds(60);

  //The rest of the init is not specific to 4-bit mode.
  //NOTE: we're writing full bytes now, not nibbles.

  // display control:
  // turn display on, cursor off, no blinking
  // 
  commandWrite(CMD_DISPLAY | CMD_DISPLAY_DISPLAY_ON/*0x0C*/);
  delayMicroseconds(60);

  //clear display
  commandWrite(CMD_CLEAR);
  delay(3);

  // entry mode set: 06
  // increment automatically, display shift, entire shift off

  commandWrite(0x06);
  delay(1);//TODO: remove unnecessary delays
}


//non-core stuff --------------------------------------
//move the cursor to the given absolute position.  line numbers start at 1.
//if this is not a 2-line LCD4Bit_mod instance, will always position on first line.
void LCD4Bit_mod::cursorTo(uint8_t line_num, uint8_t x){
  if (g_num_lines==1){
    line_num = 1;
  }
  //offset 40 chars in if second line requested
  if (line_num == 2){
    x += 0x40;
  }
  commandWrite(CMD_SETDDRAM | x);
}

//scroll whole display to left
void LCD4Bit_mod::leftScroll(uint8_t num_chars, uint8_t delay_time){
  for (uint8_t i=0; i<num_chars; ++i) {
    commandWrite(CMD_SHIFT | CMD_SHIFT_CURSOR_LEFT);
    delay(delay_time);
  }
}

void LCD4Bit_mod::setDisplay(bool display, bool cursor, bool blink)
{
  byte cmd = CMD_DISPLAY;
  if(display) cmd |= CMD_DISPLAY_DISPLAY_ON;
  if(cursor) cmd |= CMD_DISPLAY_CURSOR_ON;
  if(blink) cmd |= CMD_DISPLAY_BLINK_ON;
  commandWrite(cmd);
}

// Write char into CGRAM memory
void LCD4Bit_mod::buildChar(uint8_t loc, const uint8_t charmap[]) {
  // we only have 8 locations: 0-7
  commandWrite(CMD_SETCGRAM | (loc << 3));
  for(uint8_t i=0; i<8; ++i)
  {
    //commandWrite(CMD_SETCGRAM | (loc << 3) | i);
    print(charmap[i]);
  }
  commandWrite(CMD_SETDDRAM);
}

void LCD4Bit_mod::printDight(uint8_t value)
{
  uint8_t ch = value < 10 ? value + '0' : value - 10 + 'A';
  print(ch);
}

void LCD4Bit_mod::printHex(uint8_t value)
{
  printDight(value);
}

void LCD4Bit_mod::printHex2(uint8_t value)
{
  uint8_t h2 = value / 16;
  uint8_t h1 = value % 16;
  printHex(h2);
  printHex(h1);
}

void LCD4Bit_mod::printDight2(uint8_t value)
{
  uint8_t d2 = value / 10;
  uint8_t d1 = value % 10;
  printDight(d2);
  printDight(d1);
}

void LCD4Bit_mod::printDight3(uint8_t value)
{
  uint8_t d3 = value / 100;
  uint8_t d2 = (value / 10) % 10;
  uint8_t d1 = value % 10;
  printDight(d3);
  printDight(d2);
  printDight(d1);
}

//Improvements ------------------------------------------------
//Remove the unnecessary delays (e.g. from the end of pulseEnablePin()).
//Allow the user to pass the pins to be used by the LCD in the constructor, and store them as member variables of the class instance.
//-------------------------------------------------------------
