#include "drv/drvSDCard.h"
#include <Libraries/SDCard/SDCard.h>



uint8_t init_DEV_SDCARD(uint8_t operation)
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
				SDCardSPI = new SPISoft(PIN_CARD_DO, PIN_CARD_DI, PIN_CARD_CK, PIN_CARD_SS, 10);
				SDCard_begin(PIN_CARD_SS);
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




