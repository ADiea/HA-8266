#include "drv/drvSDCard.h"

uchar devSDCard_init(uchar operation)
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
			
			}
		}
		else
		{
		//Therefore to make MMC/SDC release DO signal, the master device must send a byte after CS signal is deasserted.
			//deinit GPIO
		}
	}
	while(0);

	return retVal;
}

uchar devSDCard_getSize(uint32_t *highDWord, uint32_t *lowDWord)
{


}