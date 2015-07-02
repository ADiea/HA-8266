#ifndef DRV_SDCARD
#define DRV_SDCARD

#include "types.h"
#include "device.h"
#include "fatfs/ff.h"

extern FATFS FatFs;		/* FatFs work area needed for each volume */

uchar devSDCard_init(uchar operation);
void devSDCard_benchmark();

#endif /*DRV_SDCARD*/
