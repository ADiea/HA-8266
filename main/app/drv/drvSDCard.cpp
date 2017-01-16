#include "drv/drvSDCard.h"

CDrvSD DrvSD;

bool CDrvSD::DelegateCS(eSPIChipSelect op)
{
	bool bRet = false;
	eBusDevices result;

	do
	{
/*		result = BusSPI.getBus(devSPI_SDCard, RADIO_WAIT_SPI_BUS);

		if(devSPI_SDCard != result)
		{
			LOG_W("SD: SPI busy: %d", result);
			break;
		}
*/
		if(op == eSPIRelease)
		{
			bRet = DrvIO.setPin(IO_PIN_CS_SD, 1);
		}
		else //Select
		{
			bRet = DrvIO.setPin(IO_PIN_CS_SD, 0);
		}
/*
		if(!bRet || (op == eSPIRelease))
		{
			BusSPI.releaseBus();
		}
*/
	}while(0);

	return bRet;
}

virtual eDriverError CDrvSD::setup(eDriverOp op/* = drvEnable*/)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
			CBusAutoRelease bus(devSPI_SDCard, SD_WAIT_SPI_BUS);
			if(bus.getBus())
			{
				SDCardSPI = &SysSPI;

				SDCard_begin(0xFF, DelegateCS);

				retErr = drvErrOK;
				m_State = drvEnabled;
			}
			else
			{
				retErr = drvErrBus;
			}
		}
		else if (drvDisable == op)
		{
			/* nothing to do */

			retErr = drvErrOK;
			m_State = drvDisabled;
		}
	} while(0);

	return retErr;
}








