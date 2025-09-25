#ifndef TM1637_h
#define TM1637_h

#include "stm32f1xx_hal.h"  // change for your microcontroller

void tm1637_SetUsDelayFunction(void (*usDelayFunction)(uint16_t));
void tm1637_SetDefault();
void tm1637_SetModeAndValue(uint8_t mode, int16_t value, uint8_t showValue);
void tm1637_SetNumber(int16_t value);
void tm1637_SetWord(char* word, uint8_t size);
void tm1637_SetWordAndNum(char* word, uint8_t n_word, uint8 num)

#endif /*TM1637_h*/