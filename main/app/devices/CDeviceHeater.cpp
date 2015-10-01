#include "CDeviceHeater.h"

void CDeviceHeater::triggerState(int reason, void* state)
{
	bool bHeaterRequestOn = false;
	for(int i=0; i < m_devWatchersList.count(); i++)
	{
		CDeviceTempHumid* thDevice = (CDeviceTempHumid*)(getDevice(m_devWatchersList[i]));

		if(thDevice && thDevice->m_state.bNeedHeating)
		{
			bHeaterRequestOn = true;
			break;
		}
	}

	if(bHeaterRequestOn)
	{
		LOG_I("TurnOn: heater %d", m_ID);
		//send cmd to radio device
	}
	else
	{
		LOG_I("TurnOff: heater %d", m_ID);
		//send cmd to radio device
	}
}

bool CDeviceHeater::deserialize(const char *string)
{
}

uint32_t CDeviceHeater::serialize(char* buffer, uint32_t size)
{

}
