#include "drv/drvGesture.h"

CDrvGesture DrvGest;

eDriverError CDrvGesture::setup(eDriverOp op)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
			if(m_theChip)
			{
				delete m_theChip;
				m_theChip = NULL;
			}

			m_theChip = new SparkFun_APDS9960();

			if(m_theChip)
			{
				CBusAutoRelease bus(devI2C_Gesture);

				if(bus.getBus())
				{
					if(m_theChip->init())
					{
						retErr = drvErrOK;
						m_State = drvEnabled;
					}
					else
					{
						retErr = drvErrIO;
					}
				}
				else
				{
					retErr = drvErrBus;
				}
			}
			else
			{
				retErr = drvErrMalloc;
			}
		}
		else if (drvDisable == op)
		{
			if(m_theChip)
			{
				delete m_theChip;
				m_theChip = NULL;
			}

			retErr = drvErrOK;
			m_State = drvDisabled;
		}
	} while(0);
	m_lastError = retErr;
	return retErr;
}




