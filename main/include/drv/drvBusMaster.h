#ifndef DRV_BUS_MASTER
#define DRV_BUS_MASTER

#include "driver.h"
#include <SPISoft.h>

#define BUS_WAIT_TIME 5 //ms

class CDrvBus;

extern SPISoft SysSPI;
extern CDrvBus BusSPI, BusI2C;

enum eBusDevices
{
	devBus_NoDevice,  //Bus is free
	devBus_SameDevice, //The same requester is already using the bus from a different thread
	devSPI_SDCard,
	devSPI_Radio,

	devI2C_BORDER,

	devI2C_IO,
	devI2C_Gesture,
	devI2C_THSensor,
	devI2C_ADC,
};

enum eBusType
{
	eBusSPI,
	eBusI2C
};

class CDrvBus : public CGenericDriver
{
public:
	CDrvBus():currentDeviceUsingBus(devBus_NoDevice){}
	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvBus(){setup(drvDisable);}

	/*return when no device is using SPI bus
	 * or return when timeout occurred
	 * If successful sets the SPI busy by requesting device.
	 * Returns current device holding the SPI busy*/
	eBusDevices getBus(eBusDevices who, uint32_t waitMs = BUS_WAIT_TIME);

	void releaseBus(){currentDeviceUsingBus = devBus_NoDevice;}
private:
	eBusDevices currentDeviceUsingBus;
};



class CBusAutoRelease
{
public:
	CBusAutoRelease(eBusDevices who, uint32_t waitMs = BUS_WAIT_TIME):
		m_requester(who), m_waitTime(waitMs), m_shouldRelease(false)
	{
		if(who < devI2C_BORDER)
		{
			m_type = eBusSPI;
		}
		else
		{
			m_type = eBusI2C;
		}
	}

	~CBusAutoRelease()
	{
		if(m_shouldRelease)
		{
			if(m_type == eBusI2C)
			{
				BusI2C.releaseBus();
			}
			else if(m_type == eBusSPI)
			{
				BusSPI.releaseBus();
			}
		}
	}

	bool getBus()
	{
		eBusDevices result;

		if(m_type == eBusI2C)
		{
			result = BusI2C.getBus(m_requester, m_waitTime);
		}
		else if(m_type == eBusSPI)
		{
			result = BusSPI.getBus(m_requester, m_waitTime);
		}

		if(result == m_requester)
		{
			m_shouldRelease = true;
		}

		return m_shouldRelease;
	}
private:
	eBusType m_type;
	eBusDevices m_requester;
	uint32_t m_waitTime;
	bool m_shouldRelease;
};

#endif /*DRV_BUS_MASTER*/
