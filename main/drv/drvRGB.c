#include "drv/drvRGB.h"

uchar devRGB_init(uchar operation)
{
	uchar retVal = ERR_OK;
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

void devRGB_setColor(tColor *c)
{
	//deactivate IRQ 
	//activate OUTPUT
	devRGB_init(ENABLE);
	
	//send RGB color
	
	//activate input
	//activate switch IRQ
	devRGB_init(DISABLE);
}