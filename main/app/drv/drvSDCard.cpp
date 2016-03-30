#include "drv/drvSDCard.h"
#include <SmingCore/SmingCore.h>
#include <Libraries/SDCard/SDCard.h>


/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_CARD_DO 5	/* Master In Slave Out */
#define PIN_CARD_DI 4	/* Master Out Slave In */
#define PIN_CARD_CK 15	/* Serial Clock */
#define PIN_CARD_SS 12	/* Slave Select */

uint8_t devSDCard_init(uint8_t operation)
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




