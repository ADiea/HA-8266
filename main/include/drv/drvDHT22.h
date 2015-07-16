#ifndef DRV_DHT22
#define DRV_DHT22


#include "device.h"
#include <Libraries/DHT/DHT.h>

float devDHT22_heatIndex();
float devDHT22_dewPoint();
uint8_t devDHT22_init(uint8_t operation);
uint8_t devDHT22_read(TempAndHumidity& dest);
float devDHT22_comfortRatio();

#endif /*DRV_DHT22*/
