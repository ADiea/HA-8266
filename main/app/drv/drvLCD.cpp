#include "drv/drvLCD.h"

extern CDrvLCD DrvLCD;

eDriverError CDrvLCD::setup(eDriverOp op)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
			//nexInit(serial!);

			retErr = drvErrOK;
			m_State = drvEnabled;
		}
		else if (drvDisable == op)
		{
			retErr = drvErrOK;
			m_State = drvDisabled;
		}
	} while(0);
	m_lastError = retErr;
	return retErr;
}

/*
void CDrvLCD::setNexListenList(NexTouch* nexList)
{
	if(m_nexList)
	{
		LOG_E("Nextion list not empty");
	}

	m_nexList = nexList;
}

void CDrvLCD::runLoop()
{
	nexLoop(m_nexList);
}
*/




