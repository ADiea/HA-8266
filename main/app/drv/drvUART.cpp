#include "drv/drvUART.h"
#include "debug.h"
#include <SmingCore/SmingCore.h>

uchar devUART_init(uchar operation)
{
	uchar retVal = DEV_ERR_OK;
	do
	{
		if(operation & ENABLE)
		{
			//init GPIO, enable device
			
			//configure device
			if(operation & CONFIG)
			{
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
