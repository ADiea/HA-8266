#include "drv/drvRadio.h"


/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_RADIO_DO 5	/* Master In Slave Out */
#define PIN_RADIO_DI 4	/* Master Out Slave In */
#define PIN_RADIO_CK 15	/* Serial Clock */
#define PIN_RADIO_SS 13	/* Slave Select */

Si4432 *radio = NULL;
SPISoft *pRadioSPI = NULL;

uint8_t devRadio_init(uint8_t operation)
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
				pRadioSPI = new SPISoft(PIN_RADIO_DO, PIN_RADIO_DI, PIN_RADIO_CK, PIN_RADIO_SS);

					if(pRadioSPI)
					{
						radio = new Si4432(pRadioSPI);
					}

					if(radio)
					{
						delay(100);

						//initialise radio with default settings
						radio->init();

						//explicitly set baudrate and channel
						radio->setBaudRateFast(eBaud_38k4);
						radio->setChannel(0);

						//dump the register configuration to console
						radio->readAll();
					}
					else LOG_E( "Error not enough heap\n");


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
