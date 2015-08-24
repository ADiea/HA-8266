#ifndef LIGHT_H
#define LIGHT_H

#include "main.h"

void light_loop(void);

void light_processPkg(uint8_t* pkg, uint8_t len);

void light_init(void);

#endif //LIGHT_H