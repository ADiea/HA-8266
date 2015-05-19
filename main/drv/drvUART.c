#include "drv/drvUART.h"

uchar devUART_init(uchar operation)
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
				// Configure the UART
				uart_init(BIT_RATE_115200,0);
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