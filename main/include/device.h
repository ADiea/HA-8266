#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "types.h"
#include "debug.h"


#include "drv/drvUART.h"
#include "drv/drvRGB.h"
#include "drv/drvRadio.h"
#include "drv/drvSDCard.h"
#include "drv/drvDHT22.h"
#include "drv/drvMQ135.h"
#include "drv/drvWiFi.h"
#include "drv/drvDS18B20.h"

#define DEV_RADIO   0x01
#define DEV_SDCARD  0x02
#define DEV_RGB     0x04
#define DEV_MQ135   0x08
#define DEV_DHT22   0x10
#define DEV_WIFI    0x20
#define DEV_DSTEMP  0x40
#define DEV_UART    0x80

#define DISABLE 0x0
#define ENABLE 0x1
#define CONFIG 0x2

#define DEV_ERR_OK 0
#define DEV_OTHER_ERR 1

inline uchar isDevEnabled(uchar dev);
void enableDev(uchar dev, uchar op);

#endif /*__DEVICE_H_*/
