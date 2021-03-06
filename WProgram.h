#ifndef WProgram_h
#define WProgram_h

#include <stdlib.h>
#include <string.h>
#include <math.h>

// some libraries and sketches depend on this
// AVR stuff, assuming Arduino.h or WProgram.h
// automatically includes it...
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "avr_functions.h"
#include "wiring.h"

#define DMAMEM __attribute__ ((section(".dmabuffers"), used))
#define FASTRUN __attribute__ ((section(".fastrun")))

#ifdef __cplusplus

//#include "WCharacter.h"
#include "WString.h"
#include "elapsedMillis.h"

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

// WMath prototypes
uint32_t random(void);
uint32_t random(uint32_t howbig);
int32_t random(int32_t howsmall, int32_t howbig);
void randomSeed(uint32_t newseed);
void srandom(uint32_t newseed);
long map(long, long, long, long, long);

#include "pins_arduino.h"

#endif // __cplusplus

#endif // WProgram_h
