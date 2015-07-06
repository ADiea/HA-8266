#ifndef DRV_DHT22
#define DRV_DHT22

#include "types.h"
#include "device.h"
#include <Libraries/DHT/DHT.h>

float devDHT22_heatIndex();
float devDHT22_dewPoint();
uchar devDHT22_init(uchar operation);
uchar devDHT22_read(TempAndHumidity& dest);
float devDHT22_comfortRatio();

#endif /*DRV_DHT22*/
