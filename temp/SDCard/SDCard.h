/*
Author: ADiea
Project: Sming for ESP8266
License: MIT
Date: 16.07.2015
Descr: low level SDCard functions
*/
#ifndef _SD_CARD_
#define _SD_CARD_

#include "types.h"

#include "fatfs/ff.h"

extern FATFS FatFs;		/* FatFs work area needed for each volume */

class SDCardClass
{
public:
	SDCard():mSPI(NULL);
	void begin(uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t ss);
	inline SPISoft* getSPI(){return mSPI;}
private:
	SPISoft *mSPI;
}

#endif /*_SD_CARD_*/
