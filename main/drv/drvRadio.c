#include "drv/drvRadio.h"

uchar devRadio_init(uchar operation)
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