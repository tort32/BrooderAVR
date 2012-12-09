#pragma once

//#define DEBUG_VIA_SERIAL 1

#ifdef DEBUG_VIA_SERIAL
#include "HardwareSerial.h"
#endif

#define VKEY_RIGHT  0
#define VKEY_UP     1
#define VKEY_DOWN   2
#define VKEY_LEFT   3
#define VKEY_SELECT 4
#define VKEY_NUM    5
#define VKEY_NONE   0xFF

const int adc_key_val[VKEY_NUM] = {30, 150, 360, 535, 760};

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
    int adc_key_in = analogRead(analogPin);
#ifdef DEBUG_VIA_SERIAL
    Serial.print("KeyPad APIN = ");
    Serial.println(adc_key_in, HEX);
#endif
    byte new_key = get_key(adc_key_in); // convert into key
    if(key != new_key)
    {
      key = new_key;
#ifdef DEBUG_VIA_SERIAL
      Serial.print("KeyPad KEY = ");
      Serial.println(key, DEC);
#endif
      if(onKeyPressed != NULL)
      {
#ifdef DEBUG_VIA_SERIAL
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
  static byte get_key(int adc_key_in)
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

