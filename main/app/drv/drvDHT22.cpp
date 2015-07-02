#include "drv/drvDHT22.h"
#include <SmingCore/SmingCore.h>
#include <Libraries/DHT/DHT.h>

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

				dht = new DHT(SENSOR_PIN, DHT22, true);

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

uchar devDHT22_read(TempReading* dest)
{
	if(dest && dht)
	{
		if (!dht->readTempAndHumidity(&(dest->temp), &(dest->humid)))
		{
			return DEV_DEVIO_ERR;
		}

		return DEV_ERR_OK;
	}
	return DEV_PARAM_ERR;
}

float devDHT22_heatIndex(float temp, float humid)
{
	if(dht)
	{
		LOG(INFO, "H_idx_F,");
		Serial.print(dht->computeHeatIndexF(dht->convertCtoF(temp), humid));
		LOG(INFO, " H_idx_C,");
		Serial.print(dht->computeHeatIndexC(temp, humid));
	}
	return 0;
}

float devDHT22_dewPoint(float temp, float humid)
{
	if(dht)
	{
		uint32_t tick1, tick2;
		tick1 = system_get_time();
		LOG(INFO, "DP_Acc,");
		Serial.print(dht->computeDewPoint(temp, humid, DEW_ACCURATE));
		tick2 = system_get_time(); LOG(INFO, ",%lu", tick2 - tick1);tick1 = system_get_time();

		LOG(INFO, "DP_AccFast,");
		Serial.print(dht->computeDewPoint(temp, humid, DEW_ACCURATE_FAST));
		tick2 = system_get_time(); LOG(INFO, ",%lu", tick2 - tick1);tick1 = system_get_time();

		LOG(INFO, "DP_Fast,");
		Serial.print(dht->computeDewPoint(temp, humid, DEW_FAST));
		tick2 = system_get_time(); LOG(INFO, ",%lu,", tick2 - tick1);tick1 = system_get_time();

		LOG(INFO, "DP_Fastest,");
		Serial.print(dht->computeDewPoint(temp, humid, DEW_FASTEST));
		tick2 = system_get_time(); LOG(INFO, ",%lu", tick2 - tick1);tick1 = system_get_time();
	}
	return 0;
}
