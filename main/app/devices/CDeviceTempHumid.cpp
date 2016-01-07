#include "CDeviceTempHumid.h"


void FilterMovingAve::init(int32_t *thefilter, uint16_t size, uint16_t mod, uint16_t div)
{
	uint16_t i;

	fSize = size;
	isInvalid = size -1;
	fSize_mod = mod;
	fSize_div = div;

	filter = thefilter;

	for(i=0;i<fSize;i++)
	{
		filter[i] = 0;
	}
}

	float  FilterMovingAve::feed(float sample)
	{
		int32_t retVal = (int32_t) (sample * 10);
		//LOG_I("Add %d -> %d", retVal, nextIndex);
		accumulator += retVal;
		filter[nextIndex] = retVal;
		nextIndex = (nextIndex + 1) & fSize_mod;

		if(isInvalid)
		{
			--isInvalid;
			return 0;
		}

		retVal = (accumulator >> fSize_div);

		accumulator -= filter[nextIndex];

		//LOG_I("Remove %d, ret%d", nextIndex, retVal);

		return (retVal/10 + 0.1f*(retVal%10));
	}

void CDeviceTempHumid::onUpdateTimer()
{
	requestUpdateState();
	m_updateTimer.initializeMs(m_updateInterval, TimerDelegate(&CDeviceTempHumid::onUpdateTimer, this)).start(false);
}

void CDeviceTempHumid::requestUpdateState()
{
	uint8_t errValue;

	float fTurnOnTemp, fTurnOffTemp;
	int i;
	if(locLocal == m_location)
	{
		errValue = devDHT22_read(m_state.lastTH);

		if(DEV_ERR_OK == errValue)
		{
			m_LastUpdateTimestamp = system_get_time() / 1000;

			m_state.fLastTemp_1m = m_state.fAverageTemp_1m.feed(m_state.lastTH.temp);
			m_state.fLastTemp_8m = m_state.fAverageTemp_8m.feed(m_state.lastTH.temp);
			m_state.fLastRH_1m = m_state.fAverageRH_1m.feed(m_state.lastTH.humid);

			/*devDHT22_heatIndex();
			devDHT22_dewPoint();
			devDHT22_comfortRatio();
			LOG_I( "\n");*/

			fTurnOnTemp = m_state.fLastTemp_8m;

			if(fTurnOnTemp < m_tempThreshold)
				fTurnOnTemp = m_state.fLastTemp_1m;

			if(fTurnOnTemp < m_tempThreshold)
					fTurnOnTemp = m_state.lastTH.temp;

			fTurnOffTemp = m_state.fLastTemp_1m;

			if(fTurnOffTemp < m_tempThreshold)
					fTurnOffTemp = m_state.lastTH.temp;

			//Determine if autopilot program should be changed
			int newAutopilotIndex = -1, newAutopilotDay = -1, h, m;
			DateTime now = SystemClock.now(eTZ_Local);

			newAutopilotDay = now.DayofWeek- 1;
			if(newAutopilotDay < 0) newAutopilotDay = 7;

			h = now.Hour;
			m = now.Minute + h*60;

			for(i=0; i< m_autoPrograms[newAutopilotDay].count(); i++)
			{
				if(m >= m_autoPrograms[newAutopilotDay][i].startHour * 60 +
						m_autoPrograms[newAutopilotDay][i].startMinute)
				{
					if(m <= m_autoPrograms[newAutopilotDay][i].endHour * 60 +
							m_autoPrograms[newAutopilotDay][i].endMinute)
					{
						newAutopilotIndex = i;
						break;
					}
				}
			}

			if((m_autopilotDay != newAutopilotDay || m_autopilotIndex != newAutopilotIndex) &&
					newAutopilotIndex >= 0)
			{
				m_autopilotDay = newAutopilotDay;
				m_autopilotIndex = newAutopilotIndex;

				m_state.tempSetpoint = m_autoPrograms[newAutopilotDay][newAutopilotIndex].setTemp;

				LOG_I( "TH(%d) CHANGE autopilot idx=%d temp=%f\n", m_ID, m_autopilotIndex, m_state.tempSetpoint);
			}

			LOG_I("%2d:%2d %s H:%.2f(%.1f) T:%.2f(%.1f %.1f)/%.1f SetPt:%.2f Time:%u", now.Hour, now.Minute, m_FriendlyName.c_str(),
					m_state.lastTH.humid, m_state.fLastRH_1m, m_state.lastTH.temp, m_state.fLastTemp_1m ,
					m_state.fLastTemp_8m, devDHT22_heatIndex(), m_state.tempSetpoint,
					m_LastUpdateTimestamp);

			if(m_state.tempSetpoint > m_tempThreshold + fTurnOnTemp)
			{
				m_state.bNeedHeating = true;
				m_state.bNeedCooling = false;
			}
			else if(m_state.tempSetpoint < fTurnOffTemp - m_tempThreshold)
			{
				m_state.bNeedHeating = false;
				m_state.bNeedCooling = true;
			}

			for(i=0; i < m_devWatchersList.count(); i++)
			{
				CDeviceHeater* genDevice = (CDeviceHeater*)getDevice(m_devWatchersList[i]);

				if(genDevice)
				{
					genDevice->triggerState(0, NULL);
				}
			}
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

	char friendlyName[MAX_FRIENDLY_NAME];

	LOG_I("TH device: %s", *devicesString);

	if(!skipInt(devicesString, &devID))return false;
	if(!skipString(devicesString, (char*)friendlyName, MAX_FRIENDLY_NAME))return false;

	LOG_I("TH device ID:%d NAME: %s", devID, friendlyName);

	float tempSetPoint = 22.5f;
	if(!skipFloat(devicesString, &(m_state.tempSetpoint)))return false;

	m_state.tempSetpointMin = 16;
	m_state.tempSetpointMax = 27;

	String name(friendlyName);

	if(!skipFloat(devicesString, &(m_state.tempSetpointMin)))return false;

	if(!skipFloat(devicesString, &(m_state.tempSetpointMax)))return false;

	int isLocal = 0;
	if(!skipInt(devicesString, &isLocal))return false;

	int iEnabled;
	if(!skipInt(devicesString, &(iEnabled)))return false;

	m_state.bEnabled = iEnabled;

	initTempHumid((uint32_t)devID, name, /*state,*/ (eSensorLocation)isLocal);

	if(!skipInt(devicesString, &numWatchers))return false;

	while(numWatchers--)
	{
		if(!skipInt(devicesString, &devID))return false;
		LOG_I("Add watcher ID:%d", devID);
		addWatcherDevice(devID);
	}

	int j, k, len;
	autoPilotSlot programSlot;

	m_autopilotIndex = -1;
	m_autopilotDay = -1;

	if(strlen(*devicesString) > 0)
	{
		for(j=0; j<7; j++)
		{
			if(!skipInt(devicesString, &len))
				return false;

			m_autoPrograms[j].clear();

			for(k = 0; k < len; k++)
			{
				if(!skipFloat(devicesString, &(programSlot.setTemp)) ||
				   !skipInt(devicesString, &(programSlot.startHour)) ||
				   !skipInt(devicesString, &(programSlot.startMinute)) ||
				   !skipInt(devicesString, &(programSlot.endHour)) ||
				   !skipInt(devicesString, &(programSlot.endMinute)))
				{
					return false;
				}
				m_autoPrograms[j].addElement(programSlot);
			}
		}
	}
	else //set defaults
	{
		for(j=0; j<7; j++)
		{
			m_autoPrograms[j].clear();
		}
	}

	return true;
}

uint32_t CDeviceTempHumid::serialize(char* buffer, uint32_t size)
{
	int i, j, k, len;
	int sz = m_snprintf(buffer, size, "%d;%d;%s;%f;%f;%f;%d;%d;%d;", devTypeTH, m_ID, m_FriendlyName.c_str(),
					m_state.tempSetpoint, m_state.tempSetpointMin, m_state.tempSetpointMax, (int)m_location,
					m_state.bEnabled ? 1:0,
					m_devWatchersList.count());

	for(i = 0; i < m_devWatchersList.count(); i++)
	{
		sz += m_snprintf(buffer + sz, size - sz, "%d;", m_devWatchersList[i].id);
	}

	for(j=0; j<7; j++)
	{
		len = m_autoPrograms[j].count();
		sz += m_snprintf(buffer+sz, size-sz, "%d;", len);
		for(k = 0; k < len; k++)
		{
			sz += m_snprintf(buffer+sz, size-sz,
								"%.1f;%d;%d;%d;%d;",
								m_autoPrograms[j][k].setTemp,
								m_autoPrograms[j][k].startHour,
								m_autoPrograms[j][k].startMinute,
								m_autoPrograms[j][k].endHour,
								m_autoPrograms[j][k].endMinute);
		}
	}
	return sz;
}
