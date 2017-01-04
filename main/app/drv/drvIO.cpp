#include "drv/drvIO.h"

CDrvIO DrvIO;

virtual eDriverError CDrvIO::setup(eDriverOp op)
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

			m_theChip = new PCF8574();

			if(m_theChip)
			{
				CBusAutoRelease bus(devI2C_IO);

				if(bus.getBus())
				{
					if(m_theChip->begin())
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

	return retErr;
}





