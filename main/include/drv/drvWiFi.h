#ifndef DRV_WIFI
#define DRV_WIFI

#include "driver.h"

class CDrvWiFi : public CGenericDriver
{
public:
	CDrvWiFi(){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvWiFi(){setup(drvDisable);}

	void startWiFi(SystemSettings&);
private:

};

extern CDrvWiFi DrvWiFi;

#endif /*DRV_WIFI*/
