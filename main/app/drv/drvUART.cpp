#include "drv/drvUART.h"
#include <SmingCore/SmingCore.h>

uint8_t init_DEV_UART(uint8_t operation)
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
				uart_div_modify(0, UART_CLK_FREQ / (921600));

#if DEBUG_BUILD
				// enable some system messages
				//system_set_os_print(1);
#endif
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
