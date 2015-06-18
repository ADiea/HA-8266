#ifndef __TYPES__H
#define __TYPES__H

typedef unsigned char uchar;

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

#endif /*__TYPES__H*/