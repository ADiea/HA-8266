#include "drv/drvHaptic.h"


CDrvHaptic DrvHaptic;

void CDrvHaptic::vibrate(uint16_t ms)
{
	if(DrvIO.setPin(IO_PIN_OUT_HAPTIC, 0))
	{
		vibrateTmr.initializeMs(ms, TimerDelegate(&CDrvHaptic::cancelVibrate, this));
		vibrateTmr.startOnce();
	}
}

void CDrvHaptic::cancelVibrate()
{
	DrvIO.setPin(IO_PIN_OUT_HAPTIC, 1);
}

eDriverError CDrvHaptic::setup(eDriverOp op)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
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
