#ifndef DRV_PORTEXPANDER
#define DRV_PORTEXPANDER

#include "driver.h"
#include <Libraries/PCF8574/PCF8574.h>

class CDrvIO : public CGenericDriver
{
public:
	CDrvIO():m_theChip(NULL){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvIO();
private:
	PCF8574 *m_theChip;
};

extern CDrvIO DrvIO;

#endif /*DRV_PORTEXPANDER*/
