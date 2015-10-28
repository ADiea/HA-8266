#include "CDeviceTempHumid.h"


void CDeviceTempHumid::onUpdateTimer()
{
	requestUpdateState();
	m_updateTimer.initializeMs(m_updateInterval, TimerDelegate(&CDeviceTempHumid::onUpdateTimer, this)).start(false);
}

void CDeviceTempHumid::requestUpdateState()
{
	uint8_t errValue;
	int i;
	if(locLocal == m_location)
	{
		errValue = devDHT22_read(m_state.lastTH);

		if(DEV_ERR_OK == errValue)
		{
			m_LastUpdateTimestamp = system_get_time();

			LOG_I("%s H:%.2f T:%.2f SetPt:%.2f Time:%u", m_FriendlyName.c_str(),
					m_state.lastTH.humid, m_state.lastTH.temp, m_state.tempSetpoint,
					m_LastUpdateTimestamp);

			/*devDHT22_heatIndex();
			devDHT22_dewPoint();
			devDHT22_comfortRatio();
			LOG_I( "\n");*/

			if(m_state.tempSetpoint > m_tempThreshold + m_state.lastTH.temp)
			{
				m_state.bNeedHeating = true;
				m_state.bNeedCooling = false;
			}
			else if(m_state.tempSetpoint < m_state.lastTH.temp - m_tempThreshold)
			{
				m_state.bNeedHeating = false;
				m_state.bNeedCooling = true;
			}

		//	if(m_state.bNeedHeating && m_state.bEnabled)
		//	{
				for(i=0; i < m_devWatchersList.count(); i++)
				{
					CDeviceHeater* genDevice = (CDeviceHeater*)getDevice(m_devWatchersList[i]);

					if(genDevice)
					{
						genDevice->triggerState(0, NULL);
					}
				}
		//	}

		}
		else
		{
			LOG_E( "DHT22 read FAIL:%d (%d)\n", errValue, (int)devDHT22_getLastError());
		}
	}
	else //request by radio
	{

	}
}

bool CDeviceTempHumid::deserialize(const char **devicesString)
{
	int devID, numWatchers;
	#define MAX_FRIENDLY_NAME 64
	char friendlyName[MAX_FRIENDLY_NAME];

	if(!skipInt(devicesString, &devID))return false;
	if(!skipString(devicesString, (char*)friendlyName, MAX_FRIENDLY_NAME))return false;

	LOG_I("TH device ID:%d NAME: %s", devID, friendlyName);

	float tempSetPoint = 22.5f;
	if(!skipFloat(devicesString, &tempSetPoint))return false;

	tTempHumidState state(tempSetPoint, 16.0f, 27.0f);
	String name(friendlyName);

	if(!skipFloat(devicesString, &(state.tempSetpointMin)))return false;

	if(!skipFloat(devicesString, &(state.tempSetpointMax)))return false;

	int isLocal = 0;
	if(!skipInt(devicesString, &isLocal))return false;

	int iEnabled;
	if(!skipInt(devicesString, &(iEnabled)))return false;

	state.bEnabled = iEnabled;

	initTempHumid((uint32_t)devID, name, state, (eSensorLocation)isLocal);

	if(!skipInt(devicesString, &numWatchers))return false;

	while(numWatchers--)
	{
		if(!skipInt(devicesString, &devID))return false;
		LOG_I("Add watcher ID:%d", devID);
		addWatcherDevice(devID);
	}

	return true;
}

uint32_t CDeviceTempHumid::serialize(char* buffer, uint32_t size)
{

}
