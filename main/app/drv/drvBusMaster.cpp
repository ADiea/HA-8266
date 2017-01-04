#include "drv/drvBusMaster.h"

CDrvBus BusSPI, BusI2C;

virtual eDriverError CDrvBus::setup(eDriverOp op)
{
	m_State = (op == drvEnable) ? drvEnabled : drvDisabled;
	return drvErrOK;
}

/*return when no device is using SPI bus 
 * or return when timeout occurred 
 * If successful sets the SPI busy by requesting device. 
 * Returns current device holding the SPI busy*/
eBusDevices CDrvBus::getBus(eBusDevices who, uint32_t waitMs/* = BUS_WAIT_TIME*/)
{
	if(currentDeviceUsingBus != who)
	{
		uint64_t exitMillis = millis() + waitMs;

		do
		{
			if(currentDeviceUsingBus == devBus_NoDevice)
			{
				currentDeviceUsingBus = who;
				break;
			}
			WDT.alive();
		}
		while (millis() < exitMillis) ;
	}
	return currentDeviceUsingBus;
}
