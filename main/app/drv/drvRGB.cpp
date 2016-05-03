#include "drv/drvRGB.h"
#include <WS2812/WS2812.h>

#define LED_PIN 0 // GPIO0

tColor COLOR_RED = {0x33, 0, 0};
tColor COLOR_GREEN = {0, 0x33, 0};
tColor COLOR_BLUE = {0, 0, 0x33};
tColor COLOR_OFF = {0, 0, 0};

uint8_t init_DEV_RGB(uint8_t operation)
{
	uint8_t retVal = DEV_ERR_OK;
	do
	{
		if(operation & ENABLE)
		{
			//init GPIO, enable device
			
			//configure device
			if(operation & CONFIG)
			{
			
			}
		}
		else
		{
			//deinit GPIO
		}
	}
	while(0);

	return retVal;
}

void devRGB_setColor(tColor c)
{
	//deactivate IRQ 
	//activate OUTPUT
	//devRGB_init(ENABLE);

/*	COLOR_RED.buf[0] = 0xff;
	COLOR_RED.buf[1] = 0x00;
	COLOR_RED.buf[2] = 0x00;
*/
	ws2812_writergb(LED_PIN, (char*)c.buf, 3);
	
	//send RGB color
	
	//activate input
	//activate switch IRQ
	//devRGB_init(DISABLE);
}
