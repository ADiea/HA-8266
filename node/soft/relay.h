#ifndef RELAY_H
#define RELAY_H

#include "main.h"

void relay_init(void);

void relay_setDim(uint8_t dim);

//return ast period in multiple of 32 us
uint16_t relay_getLastPeriod(void);

uint16_t relay_getNumCrosses(void);

uint16_t relay_getNumSwitches(void);

uint16_t relay_getIgnoredPulses(void);

#endif /*RELAY_H*/