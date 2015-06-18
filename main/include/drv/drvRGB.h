#ifndef DRV_RGB
#define DRV_RGB

#include "types.h"

typedef struct _Color
{
	uchar r;
	uchar g;
	uchar b;
} tColor;

extern tColor COLOR_RED;
extern tColor COLOR_GREEN;
extern tColor COLOR_BLUE;

uchar devRGB_init(uchar operation);

void devRGB_setColor(tColor *c); 


#endif /*DRV_RGB*/
