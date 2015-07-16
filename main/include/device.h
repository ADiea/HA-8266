#ifndef __DEVICE_H_
#define __DEVICE_H_


#include "debug.h"


#include "drv/drvUART.h"
#include "drv/drvRGB.h"
#include "drv/drvRadio.h"
#include "drv/drvSDCard.h"
#include "drv/drvDHT22.h"
#include "drv/drvMQ135.h"
#include "drv/drvWiFi.h"
#include "drv/drvDS18B20.h"

#define DEV_RADIO   0x0001
#define DEV_SDCARD  0x0002
#define DEV_RGB     0x0004
#define DEV_MQ135   0x0008
#define DEV_DHT22   0x0010
#define DEV_WIFI    0x0020
#define DEV_DSTEMP  0x0040
#define DEV_UART    0x0080

#define DISABLE 	0x01
#define ENABLE 		0x02
#define CONFIG 		0x04

#define DEV_ERR_OK 		0
#define DEV_OTHER_ERR 	1
#define DEV_DEVIO_ERR 	2
#define DEV_PARAM_ERR 	3

#define isDevEnabled(dev) ((dev) & gDevicesState)

void enableDev(unsigned short, uint8_t op);

#endif /*__DEVICE_H_*/
