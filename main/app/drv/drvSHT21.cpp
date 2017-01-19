#include "drv/drvSHT21.h"

CDrvSHT21 DrvTempHumid;

eDriverError CDrvSHT21::setup(eDriverOp op/* = drvEnable*/)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
			if(m_theChip)
			{
				delete m_theChip;
				m_theChip = NULL;
			}

			m_theChip = new SI7021();

			if(m_theChip)
			{
				CBusAutoRelease bus(devI2C_THSensor);

				if(bus.getBus())
				{
					if(m_theChip->begin())
					{
						retErr = drvErrOK;
						m_State = drvEnabled;
					}
					else
					{
						retErr = drvErrIO;
					}
				}
				else
				{
					retErr = drvErrBus;
				}
			}
			else
			{
				retErr = drvErrMalloc;
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


eDriverError CDrvSHT21::readTempHumid(TempHumidity& dest)
{
	m_lastError = drvErrNullDevice;
	si7021_env result;

	if(m_theChip)
	{
		CBusAutoRelease bus(devI2C_THSensor);

		if(bus.getBus())
		{
			m_theChip->getHumidityAndTemperature();

			if(result.error_crc)
			{
				m_lastError = drvErrCRC;
			}
			else
			{
				dest.humid = result.humidityPercent / 100.0f;
				dest.temp  = result.temperature / 100.0f;
				m_lastError = drvErrOK;
			}
		}
		else
		{
			m_lastError = drvErrBus;
		}
	}

	return m_lastError;
}
/*
float devDHT22_heatIndex()
{
	if(dht)
	{
		return dht->getHeatIndex();
	}
	return 0;
}

float devDHT22_dewPoint()
{
	if(dht)
	{
		uint32_t tick1, tick2;
		tick1 = system_get_time();
		LOG_I( ",%.2f", dht->getDewPoint(DEW_ACCURATE));
		tick2 = system_get_time(); LOG_I( ",%lu", tick2 - tick1);tick1 = system_get_time();

		LOG_I( ",%.2f", dht->getDewPoint(DEW_ACCURATE_FAST));
		tick2 = system_get_time(); LOG_I( ",%lu", tick2 - tick1);tick1 = system_get_time();

		LOG_I( ",%.2f", dht->getDewPoint(DEW_FAST));
		tick2 = system_get_time(); LOG_I( ",%lu", tick2 - tick1);tick1 = system_get_time();

		LOG_I( ",%.2f", dht->getDewPoint(DEW_FASTEST));
		tick2 = system_get_time(); LOG_I( ",%lu", tick2 - tick1);tick1 = system_get_time();
	}
	return 0;
}

float devDHT22_comfortRatio()
{
	ComfortState cf;
	if(dht)
	{
		LOG_I( ",%.2f:", dht->getComfortRatio(cf));
		switch(cf)
		{
		case Comfort_OK:
			LOG_I( "Comfort_OK");
			break;
		case Comfort_TooHot:
			LOG_I( "Comfort_TooHot");
			break;
		case Comfort_TooCold:
			LOG_I( "Comfort_TooCold");
			break;
		case Comfort_TooDry:
			LOG_I( "Comfort_TooDry");
			break;
		case Comfort_TooHumid:
			LOG_I( "Comfort_TooHumid");
			break;
		case Comfort_HotAndHumid:
			LOG_I( "Comfort_HotAndHumid");
			break;
		case Comfort_HotAndDry:
			LOG_I( "Comfort_HotAndDry");
			break;
		case Comfort_ColdAndHumid:
			LOG_I( "Comfort_ColdAndHumid");
			break;
		case Comfort_ColdAndDry:
			LOG_I( "Comfort_ColdAndDry");
			break;
		default:
			LOG_I( "Unknown:%d", cf);
			break;
		}
	}
	return 0;
}
*/
