/*
* Author: ADiea
*
* Implement Home Automation Node
*
*/

#include "main.h"

#define NUM_COLORS 4

volatile tRGB gColorPallette[NUM_COLORS] = //g r b
{
	{0x00, 0x00, 0x00}, //black/off
	{0xFF, 0x00, 0x00}, //green
	{0x00, 0xFF, 0x00}, //red
	{0x00, 0x00, 255}	//blue
};


void initHw()
{
	initUart();
	
	initKey();
	
	spi_begin();
	
	radio_init();

	_delay_ms(1);
}



int main(void)
{
	uint8_t loopDelay = 10;
	uint8_t curColorIndex = 0;

	initHw();
	
	do
	{	
		switch(keyPress())
		{
			case NotPressed:
			break;
			
			case ShortPress:
				curColorIndex = (curColorIndex + 1) % NUM_COLORS;
				ws2812_setleds((tRGB*)&gColorPallette[curColorIndex], 1);
			break;
			
			case LongPress:
			break;
		}
		_delay_ms(loopDelay);
	}
	while(1);

}
