#ifndef RELAY_H
#define RELAY_H

#include "main.h"

void relay_init(void);

//return ast period in multiple of 32 us
uint16_t relay_getLastPeriod(void);

#endif /*RELAY_H*/