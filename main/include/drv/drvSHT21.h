#ifndef DRV_SHT21
#define DRV_SHT21

#include "driver.h"
#include <Libraries/SI7021/SI7021.h>

struct TempHumidity
{
	float temp;
	float humid;
};


class CDrvSHT21 : public CGenericDriver
{
public:
	CDrvSHT21():m_theChip(nullptr){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvSHT21(){setup(drvDisable);}

	eDriverError readTempHumid(TempHumidity& dest);

private:
	SI7021 *m_theChip;
};

extern CDrvSHT21 DrvTempHumid;

#endif /*DRV_SHT21*/
