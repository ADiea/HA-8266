#ifndef DRV_RGB
#define DRV_RGB

#include "types.h"

typedef struct _Color
{
	uchar r;
	uchar g;
	uchar b;
} tColor;

tColor COLOR_RED = {0xFF, 0, 0};
tColor COLOR_GREEN = {0, 0xFF, 0};
tColor COLOR_BLUE = {0, 0, 0xFF};

uchar devRGB_init(uchar operation);

void devRGB_setColor(tColor *c); 


#endif /*DRV_RGB*/
