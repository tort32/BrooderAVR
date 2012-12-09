#ifndef LCD4Bit_mod_h
#define LCD4Bit_mod_h

#include <inttypes.h>

class LCD4Bit_mod {
public:
  LCD4Bit_mod(uint8_t num_lines);
  void commandWrite(uint8_t value);
  void init();
  void print(uint8_t value);
  void printIn(const char* msg);
  void clear();
  //non-core---------------
  void cursorTo(uint8_t line_num, uint8_t x);
  void leftScroll(uint8_t chars, uint8_t delay_time);
  //end of non-core--------

  //4bit only, therefore ideally private but may be needed by user
  void commandWriteNibble(uint8_t nibble);
private:
  void pulseEnablePin();
  void pushNibble(uint8_t nibble);
  void pushByte(uint8_t value);
};

#endif
