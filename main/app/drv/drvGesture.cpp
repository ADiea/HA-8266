#include "drv/drvGesture.h"
#include <SmingCore/SmingCore.h>

SparkFun_APDS9960 *gestureChip;

uint8_t init_DEV_GEST(uint8_t operation)
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
				if(gestureChip)
				{
					delete gestureChip;
					gestureChip = NULL;
				}

				gestureChip = new SI7021();

				gestureChip->begin();
			}
		}
		else
		{
			//deinit GPIO
			if(gestureChip)
			{
				delete gestureChip;
				gestureChip = NULL;
			}
		}
	}
	while(0);

	return retVal;
}


