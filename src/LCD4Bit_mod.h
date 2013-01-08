#ifndef LCD4Bit_mod_h
#define LCD4Bit_mod_h

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

#include <inttypes.h>

// HD04408 LCD module
class LCD4Bit_mod {
public:
  LCD4Bit_mod();
  LCD4Bit_mod(uint8_t num_lines);
  void commandWrite(uint8_t value);
  void init();
  void print(uint8_t value);
  void printIn(const char* msg);
  void clear();
  void home();
  //non-core---------------
  void cursorTo(uint8_t line_num, uint8_t x);
  void leftScroll(uint8_t chars, uint8_t delay_time);
  void setDisplay(bool display, bool cursor, bool blink);

  void printDigit(uint8_t value);
  void printDigit2(uint8_t value);
  void printDight3(uint8_t value);

  void buildChar(uint8_t location, const uint8_t charmap[]);
  //end of non-core--------

  //4bit only, therefore ideally private but may be needed by user
  void commandWriteNibble(uint8_t nibble);
private:
  void pulseEnablePin();
  void pushNibble(uint8_t nibble);
  void pushByte(uint8_t value);
};

#endif
