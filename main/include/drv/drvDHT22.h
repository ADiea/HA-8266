#ifndef DRV_DHT22
#define DRV_DHT22

#include "types.h"
#include "device.h"

struct TempReading
{
	float temp;
	float humid;
};

float devDHT22_heatIndex(float temp, float humid);
float devDHT22_dewPoint(float temp, float humid);
uchar devDHT22_init(uchar operation);
uchar devDHT22_read(TempReading* dest);

#endif /*DRV_DHT22*/
