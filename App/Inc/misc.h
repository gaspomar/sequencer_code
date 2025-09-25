#ifndef MISC_H
#define MISC_H

#include <stdint.h>
#include <stdbool.h>

#define SATURATE(a, max) ((a)>(max) ? (max) : (a))

#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

//typedef enum {false,true} bool;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

void DelayFuncUs(uint16_t time);

void CalculateSyncTimestamps(uint16 tempoBPM, volatile uint32* timeStampTable_100us);

static inline uint16 CalculateStepTime(uint16 BPM, uint16 syncEventsPerStep)
{
    return (uint16)((60000*syncEventsPerStep)/(BPM*24))-2;
}


static inline uint16 CalculateSyncEventsPerStep(uint16 resolution)
{
    return (uint16)((24*4)/resolution);
}


static inline uint16 CalculateGateTime(uint16 stepTime, uint8 gatePercent)
{
    return (stepTime * gatePercent) / 100;
}


/* UNUSED */
bool HasElapsedUs(uint16 counterPrevTime, uint16 deltaT);
void SaveCurrentTime(uint16* counter);

#endif /* MISC_H */