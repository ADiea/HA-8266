#include "drv/drvRadio.h"

CDrvRadio DrvRadio;

bool CDrvRadio::DelegateCS(eSPIChipSelect op)
{
	bool bRet = false;
	eBusDevices result;

	do
	{
/*		result = BusSPI.getBus(devSPI_Radio, RADIO_WAIT_SPI_BUS);

		if(devSPI_Radio != result)
		{
			LOG_W("Radio: SPI busy: %d", result);
			break;
		}
*/
		if(op == eSPIRelease)
		{
			bRet = DrvIO.setPin(IO_PIN_CS_RF, 1);
		}
		else //Select
		{
			bRet = DrvIO.setPin(IO_PIN_CS_RF, 0);
		}
/*
		if(!bRet || (op == eSPIRelease))
		{
			BusSPI.releaseBus();
		}
*/
	}while(0);

	return bRet;
}

uint8_t CDrvRadio::checkRadioChecksum(byte *pkg, uint8_t length)
{
	uint8_t i, chk=0;
	for(i=0; i<length-1; i++)
	{
		chk += pkg[i];
	}
	return chk == pkg[length-1];
}

eRadioError CDrvRadio::RadioSend(byte *pkg, uint8_t length, uint8_t *outLen, uint32_t waitMs)
{
	eRadioError result = err_RadioNotInit;
	
	if(m_theChip)
	{
		{//autorelease scope
			CBusAutoRelease bus(devSPI_Radio, waitMs);
			if(bus.getBus())
			{
				result = m_theChip->sendPacket(length, pkg, true, RADIO_WAIT_ACK_MS, outLen, pkg);
				if(result != err_NoError || *outLen > 64)
				{
					if(result == err_TXTimeout)
					{
						mRadioSendFaults++;
					}

					if(mRadioSendFaults > 3)
					{
						LOG_E("RADIO FAIL, REINIT");
						mRadioSendFaults = 0;
						initRadio();
					}
				}
				else
				{
					mRadioSendFaults = 0;
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
				LOG_I( "SPI busy");
				result = err_RadioBusy;
			}
		}//autorelease scope
	}
	else
	{
		LOG_I( "Radio not init");
	}
	return result;
}


eDriverError CDrvRadio::setup(eDriverOp op/* = drvEnable*/)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
			if(m_theChip)
			{
				delete m_theChip;
			}
			m_theChip = new Si4432(&SysSPI, 0, SPIDelegateCS(&CDrvRadio::DelegateCS, this));

			if(m_theChip)
			{
				if(initRadio())
				{
					retErr = drvErrOK;
					m_State = drvEnabled;
				}
				else
				{
					retErr = drvErrBus;
				}
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
	m_lastError = retErr;
	return retErr;
}

bool CDrvRadio::initRadio()
{
	if(m_theChip)
	{
		CBusAutoRelease bus(devSPI_Radio, RADIO_WAIT_SPI_BUS);
		if(bus.getBus())
		{
			//initialise radio with default settings
			m_theChip->init();

			//explicitly set baudrate and channel
			m_theChip->setBaudRateFast(eBaud_38k4);
			m_theChip->setChannel(0);

		#if DEBUG_SI4432
			//dump the register configuration to console
			m_theChip->readAll();
		#endif

			m_theChip->startListening();

			return true;
		}
		else
		{
			LOG_I( "SPI busy");
		}
	}
	return false;
}
