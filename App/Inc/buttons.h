#ifndef BUTTONS_H
#define BUTTONS_H

#include "misc.h"


typedef enum
{
	BTN_PUSHED,
	BTN_RELEASED
} BtnEvent_e;


extern const GPIO_t btnSelect[4];
extern const GPIO_t btnDetect[8];
extern volatile uint8 btnDelay_ms[32];
extern volatile bool btnPressed[32];


void ButtonActivate(uint32 iBtn, bool shift, BtnEvent_e event);


#endif /*BUTTONS_H*/