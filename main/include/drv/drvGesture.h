#ifndef DRV_GESTURE
#define DRV_GESTURE

#include "driver.h"
#include <Libraries/SparkFun_APDS9960/SparkFun_APDS9960.h>

class CDrvGesture : public CGenericDriver
{
public:
	CDrvGesture():m_theChip(NULL){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvGesture();
private:
	SparkFun_APDS9960 *m_theChip;
};

extern CDrvGesture DrvGest;

#endif /*DRV_GESTURE*/
