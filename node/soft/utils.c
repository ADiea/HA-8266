#include "main.h"

#if HAS_RGBLED
#define NUM_COLORS 4
volatile tRGB gColorPallette[NUM_COLORS] = //g r b
{
	{0x00, 0x00, 0x00}, //black/off
	{0x20, 0x00, 0x00}, //green
	{0x00, 0x20, 0x00}, //red
	{0x00, 0x00, 0x20}	//blue
};
#endif /*HAS_RGBLED*/



void mcpy(char *d, char *s, unsigned int sz)
{
	for(; sz > 0; --sz)
	{
		*d++ = *s++;
	}
}


unsigned int slen(char *s)
{
	unsigned int sz = 0;
	while(*(s++)) ++sz;
	return sz;	
}