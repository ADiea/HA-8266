#include "drv/drvUART.h"
#include "debug.h"
#include <SmingCore/SmingCore.h>

uint8_t devUART_init(uint8_t operation)
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

				// Configure the UART
				//Serial.begin(SERIAL_BAUD_RATE);
				//Serial.commandProcessing(false);
#if DEBUG_BUILD
				//Serial.systemDebugOutput(true); // Enable debug output to serial
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
