#include "CDeviceHeater.h"

void CDeviceHeater::triggerState(int reason, void* state)
{
	bool bHeaterRequestOn = false;
	for(int i=0; i < m_devWatchersList.count(); i++)
	{
		CDeviceTempHumid* thDevice = (CDeviceTempHumid*)(getDevice(m_devWatchersList[i]));

		if(thDevice)
		{
			if(thDevice->m_state.bNeedHeating && thDevice->m_state.bEnabled)
			{
				//at least one th requests heating
				bHeaterRequestOn = true;
				//this th needs heating
				thDevice->m_state.bIsHeating = true;
			}
			else
			{
				//this TH does not need heating
				thDevice->m_state.bIsHeating = false;
			}
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

bool CDeviceHeater::deserialize(const char **devicesString)
{
	int devID, numWatchers;
	#define MAX_FRIENDLY_NAME 64
	char friendlyName[MAX_FRIENDLY_NAME];

	if(!skipInt(devicesString, &devID))return false;
	if(!skipString(devicesString, (char*)friendlyName, MAX_FRIENDLY_NAME))return false;

	LOG_I("HEATER device ID:%d NAME: %s", devID, friendlyName);

	int gasHighThres = 200;
	int gasLowThres = 50;
	int gasMedThres = 100;

	if(!skipInt(devicesString, &gasHighThres))return false;
	if(!skipInt(devicesString, &gasLowThres))return false;
	if(!skipInt(devicesString, &gasMedThres))return false;

	tHeaterState state(gasHighThres, gasLowThres, gasMedThres);
	String name(friendlyName);

	initHeater((uint32_t)devID, name, state);

	if(!skipInt(devicesString, &numWatchers))return false;

	while(numWatchers--)
	{
		if(!skipInt(devicesString, &devID))return false;
		LOG_I("Add watcher ID:%d", devID);
		addWatcherDevice(devID);
	}

	return true;
}

uint32_t CDeviceHeater::serialize(char* buffer, uint32_t size)
{

}
