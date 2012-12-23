#pragma once

//#define DEBUG_KEYPAD 1

#define VKEY_RIGHT  0
#define VKEY_UP     1
#define VKEY_DOWN   2
#define VKEY_LEFT   3
#define VKEY_SELECT 4
#define VKEY_NUM    5
#define VKEY_NONE   0xFF


//           VCC
// SCHEME:    |        KEY    ADC LEVEL
//  ________ [2k]
//|-^RIGHT ^--+-->ADC  RIGHT =0(MIN)                                   =0
//  ________ [330]                                                      MID=72
//|-^  UP  ^--+        UP    =1023*(330)/(2k+330)                      =145
//  ________ [620]                                                      MID=237
//|-^ DOWN ^--+        DOWN  =1023*(330+620)/(2k+330+620)              =329
//  ________ [1k]                                                       MID=417
//|-^ LEFT ^--+        LEFT  =1023*(330+620+1k)/(2k+330+620+1k)        =505
//  ________ [3k3]                                                      MID=623
//|-^SELECT^--+        SELECT=1023*(330+620+1k+3k3)/(2k+330+620+1k+3k3)=741
//                                                                      MID=882
//                     NONE  =1023(MAX)                                =1023

const uint16_t adc_key_val[VKEY_NUM] =
  { 72, 237, 417, 623, 882 }; // ADC MIDDLE

class KeyPad
{
public:
  typedef void (*OnKeyPressedFP)(byte);

  KeyPad(uint8_t pin, OnKeyPressedFP pHandler = NULL)
    : analogPin(pin)
    , key(VKEY_NONE)
    , onKeyPressed(pHandler)
  {
  }

  // Reads keys status and call callback if changed
  // Returns active key
  inline byte read()
  {
    uint16_t adc_key_in = analogRead(analogPin);
#ifdef DEBUG_KEYPAD
    Serial.print("KeyPad APIN = ");
    Serial.println(adc_key_in, HEX);
#endif
    byte new_key = get_key(adc_key_in); // convert into key
    if(key != new_key)
    {
      key = new_key;
#ifdef DEBUG_KEYPAD
      Serial.print("KeyPad KEY = ");
      Serial.println(key, DEC);
#endif
      if(onKeyPressed != NULL)
      {
#ifdef DEBUG_KEYPAD
        Serial.println("KeyPad onKeyPressed");
#endif
        (*onKeyPressed)(key);
      }
    }
    return key;
  }

  // Set key pressed function callback
  void set_handler(OnKeyPressedFP pHandler)
  {
    onKeyPressed = pHandler;
  }
//private:

  // Convert ADC value to key number
  static byte get_key(uint16_t adc_key_in)
  {
    for (byte k = 0; k < VKEY_NUM; ++k)
    {
      if (adc_key_in < adc_key_val[k])
        return k;
    }

    return VKEY_NONE; // No valid key pressed
  }

private:
  const uint8_t analogPin;
  byte key;
  OnKeyPressedFP onKeyPressed;
};

extern KeyPad KEYS;
