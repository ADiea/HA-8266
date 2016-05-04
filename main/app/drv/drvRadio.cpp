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
		WDT.alive();
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

eRadioError RadioSend(byte *pkg, uint8_t length, uint8_t *outLen, uint32_t waitMs)
{
	eRadioError result = err_RadioNotInit;
	
	if(Radio)
	{
		if(getRadio(waitMs))
		{
			result = Radio->sendPacket(length, pkg, true, RADIO_WAIT_ACK_MS, outLen, pkg);
			releaseRadio();

			if(result != err_NoError || *outLen > 64)
			{
				if(result == err_TXTimeout)
				{
					gRadioSendFaults++;
				}

				if(gRadioSendFaults > 3)
				{
					LOG_E("RADIO FAIL, REINIT");
					gRadioSendFaults = 0;

					//initialise radio with default settings
					Radio->init();

					//explicitly set baudrate and channel
					Radio->setBaudRateFast(eBaud_38k4);
					Radio->setChannel(0);

#if DEBUG_SI4432
					//dump the register configuration to console
					Radio->readAll();
#endif
				}
			}
			else
			{
				gRadioSendFaults = 0;
				result = err_NoError;
#if DEBUG_SI4432
				LOG_II(" SENT! SYNC RX (%d):", *outLen);

				for (byte i = 0; i < *outLen; ++i)
				{
					m_printf( "%x ", pkg[i]);
				}

				LOG_II("\n");
#endif
			}
		}
		else
		{
			LOG_I( "Radio busy");
			result = err_RadioBusy;
		}
	}
	else
	{
		LOG_I( "Radio not init");
	}
	
	return result;
}

uint8_t init_DEV_RADIO(uint8_t operation)
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
				if(pRadioSPI)
					delete pRadioSPI;

				pRadioSPI = new SPISoft(PIN_RADIO_DO, PIN_RADIO_DI, PIN_RADIO_CK, PIN_RADIO_SS, 0);

				if(pRadioSPI)
				{
					if(Radio)
						delete Radio;
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
					//LOG_E("No heap %s:%d", __FUNCTION__, __LINE__);
					break;
				}
			}

			Radio->startListening();
		}
		else
		{
			//deinit GPIO
			//if(Radio) delete Radio;
			//if(pRadioSPI) delete pRadioSPI;
		}
	}
	while(0);

	return retVal;
}
