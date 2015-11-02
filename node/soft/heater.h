#ifndef HEATER_H
#define HEATER_H

#include "main.h"

void heater_init(void);

void heater_processPkg(uint8_t* pkg, uint8_t len);

void heater_loop();

#endif /*RELAY_H*/