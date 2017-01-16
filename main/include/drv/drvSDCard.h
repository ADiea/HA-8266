#ifndef DRV_SDCARD
#define DRV_SDCARD

#include "driver.h"
#include <Libraries/SDCard/SDCard.h>

#define SD_WAIT_SPI_BUS 30

class CDrvSD : public CGenericDriver
{
public:
	CDrvSD(){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvSD(){setup(drvDisable);}

	bool DelegateCS(eSPIChipSelect op);

};

extern CDrvSD DrvSD;

#endif /*DRV_SDCARD*/
