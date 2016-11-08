#include "drv/drvSHT21.h"
#include <SmingCore/SmingCore.h>


SI7021 *thChip = NULL;

uint8_t init_DEV_SHT21(uint8_t operation)
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
				if(thChip)
				{
					delete thChip;
					thChip = NULL;
				}

				thChip = new SI7021();

				thChip->begin();
			}
		}
		else
		{
			//deinit GPIO
			if(thChip)
			{
				delete thChip;
				thChip = NULL;
			}
		}
	}
	while(0);

	return retVal;
}
/*
ErrorDHT devDHT22_getLastError()
{
	if(dht)
		return dht->getLastError();
	else
		return errDHT_Other;
}

uint8_t devDHT22_read(TempAndHumidity& dest)
{
	if(dht)
	{
		if (!dht->readTempAndHumidity(dest))
		{
			return DEV_DEVIO_ERR;
		}

		return DEV_ERR_OK;
	}
	return DEV_PARAM_ERR;
}

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
