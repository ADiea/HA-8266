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

