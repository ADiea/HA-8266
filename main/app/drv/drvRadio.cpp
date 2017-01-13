#include "drv/drvRadio.h"

uint8_t CDrvRadio::checkRadioChecksum(byte *pkg, uint8_t length)
{
	uint8_t i, chk=0;
	for(i=0;i<length-1;i++)
		chk += pkg[i];

	return chk == pkg[length-1];
}

eRadioError CDrvRadio::RadioSend(byte *pkg, uint8_t length, uint8_t *outLen, uint32_t waitMs)
{
	eRadioError result = err_RadioNotInit;
	
	if(Radio)
	{
		{//autorelease scope
			CBusAutoRelease bus(devSPI_Radio, waitMs);
			if(bus.getBus())
			{
				result = Radio->sendPacket(length, pkg, true, RADIO_WAIT_ACK_MS, outLen, pkg);
			}
			else
			{
				LOG_I( "Radio busy");
				result = err_RadioBusy;
			}
		}

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
		LOG_I( "Radio not init");
	}
	
	return result;
}

virtual eDriverError CDrvRadio::setup(eDriverOp op/* = drvEnable*/)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
			if(pRadioSPI)
			{
				delete pRadioSPI;
			}
			
			pRadioSPI = new SPISoft(PIN_RADIO_DO, PIN_RADIO_DI, PIN_RADIO_CK, PIN_RADIO_SS, 0);

			if(pRadioSPI)
			{
				if(m_theChip)
				{
					delete m_theChip;
				}
				m_theChip = new Si4432(pRadioSPI);
			}

			if(m_theChip)
			{
				//SPIoI2C does auto request the SPI and i2c bus

				//initialise radio with default settings
				m_theChip->init();

				//explicitly set baudrate and channel
				m_theChip->setBaudRateFast(eBaud_38k4);
				m_theChip->setChannel(0);

				//dump the register configuration to console
				m_theChip->readAll();

				m_theChip->startListening();

				retErr = drvErrOK;
				m_State = drvEnabled;
			}
			else
			{
				retErr = drvErrMalloc;
				break;
			}
		}
		else if (drvDisable == op)
		{
			if(m_theChip)
			{
				delete m_theChip;
				m_theChip = NULL;
			}

			retErr = drvErrOK;
			m_State = drvDisabled;
		}
	} while(0);

	return retErr;
}
