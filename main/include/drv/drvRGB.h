#ifndef DRV_RGB
#define DRV_RGB


#include "device.h"

typedef union _Color
{
	uint8_t buf[4]; //pad to 4 bytes
	struct c
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
} tColor;

extern tColor COLOR_RED;
extern tColor COLOR_GREEN;
extern tColor COLOR_BLUE;

uint8_t devRGB_init(uint8_t operation);

void devRGB_setColor(tColor c);


#endif /*DRV_RGB*/
