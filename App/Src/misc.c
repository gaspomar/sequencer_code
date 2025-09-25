#include "main.h"
#include "misc.h"

// maps tempo knob percentage to deltaT step time in ms
uint16 tempoMapTable[100] = {
    999, 890, 803, 736, 683, 640, 603, 572, 545, 520,
    499, 479, 461, 444, 429, 414, 401, 388, 376, 365,
    355, 345, 335, 326, 318, 309, 302, 294, 287, 280,
    273, 267, 260, 255, 249, 243, 238, 233, 228, 223,
    218, 213, 209, 205, 201, 197, 193, 189, 185, 182,
    178, 175, 171, 168, 165, 162, 159, 156, 153, 150,
    148, 145, 143, 140, 138, 135, 133, 131, 128, 126,
    124, 122, 120, 118, 116, 114, 113, 111, 109, 107,
    106, 104, 102, 101, 99, 98, 96, 95, 94, 92,
    91, 90, 89, 87, 86, 85, 84, 83, 82, 81
};


/**
 * @brief Implements a delay in us.
 */
void DelayFuncUs(uint16_t time)
{
	uint16_t time0 = TIM2->CNT;
	uint16_t time1;
	uint16_t deltaTime = 0;
	do {
		time1 = TIM2->CNT;
		if(time0 <= time1)
		{
			deltaTime = time1 - time0;
		}
		else
		{
			deltaTime = (0xFFFF - time0) + time1;
		}
	} while (deltaTime < time);
}



void CalculateSyncTimestamps(uint16 tempoBPM, volatile uint32* timeStampTable_100us)
{
    uint32 deltaSync_10us = (uint32)(6000000/(tempoBPM*24));
    uint32 counter_10us = 0;
    for(int i=0; i<25; i++)
    {
        timeStampTable_100us[i] = counter_10us/10;
        counter_10us += deltaSync_10us;
    }
}


void SaveCurrentTime(uint16* counter)
{
    *counter = TIM2->CNT;
}


/**
 * @brief Tells wether the specified counter has reached the desired deltaT
 * @param counterPrevTime previous value of counter
 * @param deltaT desired time to wait
 */
bool HasElapsedUs(uint16 counterPrevTime, uint16 desiredDeltaT)
{
    uint16 currentTime = TIM2->CNT;
    uint16 deltaT;
    if(counterPrevTime < currentTime)
    {
        deltaT = currentTime - counterPrevTime;
    }
    else
    {
        deltaT = (0xFFFF - counterPrevTime) + currentTime;
    }

    if(desiredDeltaT < deltaT)
    {
        return true;
    }
    else
    {
        return false;
    }
}


