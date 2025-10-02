#ifndef LED_H
#define LED_H

#include "misc.h"

void LED_Set(bool steps[16], bool misc[6], bool pots[3], bool seq[4]);

void LED_SetAll();

void LED_ClearAll();

void LED_AnimationWelcome();

#endif /*LED_H*/