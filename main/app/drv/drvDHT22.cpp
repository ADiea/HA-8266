#include "drv/drvDHT22.h"
#include <SmingCore/SmingCore.h>


#define SENSOR_PIN 14 //GPIO14

DHT *dht = NULL;

uchar devDHT22_init(uchar operation)
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
				if(dht)
				{
					delete dht;
					dht = NULL;
				}

				dht = new DHT(SENSOR_PIN);

				dht->begin();
			}
		}
		else
		{
			//deinit GPIO
			if(dht)
			{
				delete dht;
				dht = NULL;
			}
		}
	}
	while(0);

	return retVal;
}

uchar devDHT22_read(TempAndHumidity& dest)
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
		LOG(INFO, ",");
		//Serial.print(dht->getHeatIndex());
	}
	return 0;
}

float devDHT22_dewPoint()
{
	if(dht)
	{
		uint32_t tick1, tick2;
		tick1 = system_get_time();
		LOG(INFO, ",");
		//Serial.print(dht->getDewPoint(DEW_ACCURATE));
		tick2 = system_get_time(); LOG(INFO, ",%lu", tick2 - tick1);tick1 = system_get_time();

		LOG(INFO, ",");
		//Serial.print(dht->getDewPoint(DEW_ACCURATE_FAST));
		tick2 = system_get_time(); LOG(INFO, ",%lu", tick2 - tick1);tick1 = system_get_time();

		LOG(INFO, ",");
		//Serial.print(dht->getDewPoint(DEW_FAST));
		tick2 = system_get_time(); LOG(INFO, ",%lu", tick2 - tick1);tick1 = system_get_time();

		LOG(INFO, ",");
		//Serial.print(dht->getDewPoint(DEW_FASTEST));
		tick2 = system_get_time(); LOG(INFO, ",%lu", tick2 - tick1);tick1 = system_get_time();

	}
	return 0;
}

float devDHT22_comfortRatio()
{
	ComfortState cf;
	if(dht)
	{
		LOG(INFO, ",");
		//Serial.print(dht->getComfortRatio(cf));
		LOG(INFO, ",");
		switch(cf)
		{
		case Comfort_OK:
			LOG(INFO, "Comfort_OK");
			break;
		case Comfort_TooHot:
			LOG(INFO, "Comfort_TooHot");
			break;
		case Comfort_TooCold:
			LOG(INFO, "Comfort_TooCold");
			break;
		case Comfort_TooDry:
			LOG(INFO, "Comfort_TooDry");
			break;
		case Comfort_TooHumid:
			LOG(INFO, "Comfort_TooHumid");
			break;
		case Comfort_HotAndHumid:
			LOG(INFO, "Comfort_HotAndHumid");
			break;
		case Comfort_HotAndDry:
			LOG(INFO, "Comfort_HotAndDry");
			break;
		case Comfort_ColdAndHumid:
			LOG(INFO, "Comfort_ColdAndHumid");
			break;
		case Comfort_ColdAndDry:
			LOG(INFO, "Comfort_ColdAndDry");
			break;
		default:
			LOG(INFO, "Unknown:%d", cf);
			break;
		}
	}
	return 0;
}

