#ifndef DRV_HAPTIC
#define DRV_HAPTIC

#include "driver.h"

class CDrvHaptic : public CGenericDriver
{
public:
	CDrvHaptic(){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvHaptic(){setup(drvDisable);}

	void vibrate(uint16_t ms);
	void cancelVibrate();
private:
	Timer vibrateTmr;
};

extern CDrvHaptic DrvHaptic;


#endif /*DRV_HAPTIC*/
