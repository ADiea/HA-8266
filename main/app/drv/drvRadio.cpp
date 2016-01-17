#include "drv/drvRadio.h"


/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_RADIO_DO 5	/* Master In Slave Out */
#define PIN_RADIO_DI 4	/* Master Out Slave In */
#define PIN_RADIO_CK 15	/* Serial Clock */
#define PIN_RADIO_SS 13	/* Slave Select */

Si4432 *Radio = NULL;
SPISoft *pRadioSPI = NULL;

bool bRadioBusy = false;
bool bRadioHalt = false;

uint8_t gRadioSendFaults = 0;

uint8_t g_SeqID = 0;

uint8_t RadioNextSeqID()
{
	return g_SeqID++;
}

uint8_t checkRadioChecksum(byte *pkg, uint8_t length)
{
	uint8_t i, chk=0;
	for(i=0;i<length-1;i++)
		chk += pkg[i];

	return chk == pkg[length-1];
}

bool getRadio(uint32_t waitMs)
{
	uint64_t enterMillis = millis();

	do
	{
		if(!bRadioBusy)
		{
			bRadioBusy = true;
			//LOG_I("Take RADIO");
			return true;
		}
	}
	while (millis() - enterMillis < waitMs) ;

	return false;
}

bool _isRadioBusy()
{
	return bRadioBusy;
}

void releaseRadio()
{
	//LOG_I("Release RADIO");
	bRadioBusy = false;
}

/*
bool RadioMakePacket(byte *pkgBuffer, 
					uint8_t maxLen,
					uint8_t destAddress,
					uint8_t pkgType,
					
					)
{
	pkg[0] = destAddress;
	pkg[1] = pkgType;
}
*/

bool RadioSend(byte *pkg, uint8_t length, uint8_t *outLen, uint32_t waitMs)
{
	bool result = false;
	
	if(Radio)
	{
		if(getRadio(waitMs))
		{
			result = Radio->sendPacket(length, pkg, true, RADIO_WAIT_ACK_MS, outLen, pkg);
			releaseRadio();

			if(!result || *outLen > 64)
			{
				//LOG_I("Radio send err.");
				result = false;

				gRadioSendFaults++;
				if(gRadioSendFaults > 3)
				{
					gRadioSendFaults = 0;

					//initialise radio with default settings
					Radio->init();

					//explicitly set baudrate and channel
					Radio->setBaudRateFast(eBaud_38k4);
					Radio->setChannel(0);

					//dump the register configuration to console
					Radio->readAll();
				}
			}
			else
			{
				gRadioSendFaults = 0;
				LOG_I(" SENT! SYNC RX (%d):", *outLen);

				for (byte i = 0; i < *outLen; ++i)
				{
					m_printf( "%x ", pkg[i]);
				}

				LOG_I("\n");
				result = true;
			}
		}
		else
		{
			LOG_I( "Radio busy\n");
		}
	}
	else
	{
		LOG_I( "Radio not init\n");
	}
	
	return result;
}

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
					Radio = new Si4432(pRadioSPI);
				}

				if(Radio)
				{
					delay(100);

					//initialise radio with default settings
					Radio->init();

					//explicitly set baudrate and channel
					Radio->setBaudRateFast(eBaud_38k4);
					Radio->setChannel(0);

					//dump the register configuration to console
					Radio->readAll();
				}
				else
				{
					LOG_E("Error not enough heap\n");
				}
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
