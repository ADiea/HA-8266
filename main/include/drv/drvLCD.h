#ifndef DRV_LCD
#define DRV_LCD

#include "driver.h"
#include <Libraries/Nextion/Nextion.h>

class CDrvLCD : public CGenericDriver
{
public:
	CDrvLCD():m_nexList(NULL){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvLCD(){setup(drvDisable);}
	void setNexListenList(NexTouch* nexList);
	void runLoop();
private:
	NexTouch *m_nexList;
};

extern CDrvLCD DrvLCD;

#endif /*DRV_LCD*/
