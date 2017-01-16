#include "drv/drvBusMaster.h"

CDrvBus BusSPI, BusI2C;

#define PIN_SPI_DO 2	/* Master In Slave Out */
#define PIN_SPI_DI 4	/* Master Out Slave In */
#define PIN_SPI_CK 5	/* Serial Clock */

SPISoft SysSPI(PIN_SPI_DO, PIN_SPI_DI, PIN_SPI_CK);

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
	eBusDevices curDev = currentDeviceUsingBus;
	uint64_t exitMillis = millis() + waitMs;

	if(currentDeviceUsingBus == who)
	{
		curDev = devBus_SameDevice;
	}

	do
	{
		if(currentDeviceUsingBus == devBus_NoDevice)
		{
			currentDeviceUsingBus = who;
			curDev = who;
			break;
		}
		WDT.alive();
	}
	while (millis() < exitMillis) ;

	return curDev;
}
