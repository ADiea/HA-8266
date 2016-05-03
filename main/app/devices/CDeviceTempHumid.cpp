#include "CDeviceTempHumid.h"
#include "webserver.h"

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
	if(!isSavedToDisk)
	{
		isSavedToDisk  = deviceWriteToDisk(this);
	}

	requestUpdateState();
	m_updateTimer.initializeMs(m_updateInterval, TimerDelegate(&CDeviceTempHumid::onUpdateTimer, this)).start(false);
}

void CDeviceTempHumid::requestUpdateState()
{
	uint8_t errValue;
	char logEntry[64];

	float fTurnOnTemp, fTurnOffTemp;
	int i;
	if(locLocal == m_location)
	{
		errValue = devDHT22_read(m_state.lastTH);

		if(DEV_ERR_OK == errValue)
		{
			tTempHumidState oldState = m_state;

			m_LastUpdateTimestamp = system_get_time() / 1000;

			//todo: hack for current sensor
			m_state.lastTH.temp -= 3;

			m_state.fLastTemp_1m = m_state.fAverageTemp_1m.feed(m_state.lastTH.temp);
			m_state.fLastTemp_8m = m_state.fAverageTemp_8m.feed(m_state.lastTH.temp);
			m_state.fLastRH_1m = m_state.fAverageRH_1m.feed(m_state.lastTH.humid);
			m_state.fLastRH_8m = 0;

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
			DateTime now = SystemClock.now(eTZ_UTC);
			DateTime now_local = SystemClock.now(eTZ_Local);

			newAutopilotDay = now_local.DayofWeek- 1;
			if(newAutopilotDay < 0) newAutopilotDay = 6;

			h = now_local.Hour;
			m = now_local.Minute + h*60;

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

			if((m_state.m_autopilotDay != newAutopilotDay || m_state.m_autopilotIndex != newAutopilotIndex) &&
					newAutopilotIndex >= 0)
			{
				m_state.m_autopilotDay = newAutopilotDay;
				m_state.m_autopilotIndex = newAutopilotIndex;

				m_state.tempSetpoint = m_autoPrograms[newAutopilotDay][newAutopilotIndex].setTemp;

				LOG_I( "TH(%d) CHANGE autopilot idx=%d temp=%f\n", m_ID, m_state.m_autopilotIndex, m_state.tempSetpoint);
			}

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

			LOG_I("%02d:%02d:%02d %s H:%.2f(%.1f) T:%.2f(%.1f %.1f)/%.1f SetPt:%.2f needHeat:%d", now.Hour, now.Minute, now.Second, m_FriendlyName.c_str(),
					m_state.lastTH.humid, m_state.fLastRH_1m, m_state.lastTH.temp, m_state.fLastTemp_1m ,
					m_state.fLastTemp_8m, devDHT22_heatIndex(), m_state.tempSetpoint,
					m_state.bNeedHeating);

			for(i=0; i < m_devWatchersList.count(); i++)
			{
				CDeviceHeater* genDevice = (CDeviceHeater*)getDevice(m_devWatchersList[i]);

				if(genDevice)
				{
					genDevice->triggerState(0, NULL);
				}
			}

			if(	oldState.lastTH.humid != m_state.lastTH.humid ||
				oldState.lastTH.temp != m_state.lastTH.temp ||
				oldState.fLastRH_1m != m_state.fLastRH_1m ||
				oldState.fLastTemp_1m != m_state.fLastTemp_1m ||
				oldState.fLastTemp_8m != m_state.fLastTemp_8m ||
				oldState.m_autopilotDay != m_state.m_autopilotDay ||
				oldState.m_autopilotIndex != m_state.m_autopilotIndex )
			{
				LOG_I( "TH(%d) BROADCAST to %d clients\n", m_ID, gHttpServer.getActiveWebSockets().count());
				broadcastDeviceInfo(gConnectedPeers, (CGenericDevice*)this);
			}


			if(m_LastUpdateTimestamp - m_lastLogTimestamp > ONE_MINUTE &&
					m_state.fLastTemp_1m > 0 && m_state.fLastTemp_8m > 0 &&
					m_state.fLastRH_1m > 0)
			{
				m_lastLogTimestamp = m_LastUpdateTimestamp ;

				m_snprintf(logEntry, sizeof(logEntry), "|%u;%d;%.1f;%.1f;%.1f;",
							(unsigned int)now.toUnixTime(),
							m_state.bIsHeating?(int)m_state.fLastTemp_8m:(int)(m_state.fLastTemp_8m-1),
							m_state.fLastTemp_1m,
							m_state.fLastTemp_8m,
							m_state.fLastRH_1m);
				if(deviceAppendLogEntry(m_ID, now.toUnixTime(), logEntry, devTypeTH))
					LOG_I( "TH(%d) saved to log: %s", m_ID, logEntry);
			}
			else LOG_D( "TH(%d) not logging: log:%u cur:%u", m_ID, m_lastLogTimestamp, m_LastUpdateTimestamp);
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
	int numWatchers, devID;
	float dummyFloat;
	char friendlyName[MAX_FRIENDLY_NAME];

	LOG_I("TH device: %s", *devicesString);

	if(!skipInt(devicesString, &devID))return false;
	m_ID = devID;

	if(!skipString(devicesString, (char*)friendlyName, MAX_FRIENDLY_NAME))return false;

	LOG_I("TH device ID:%d NAME: %s", m_ID, friendlyName);

	if(!skipFloat(devicesString, &(m_state.tempSetpoint)))return false;

	m_FriendlyName = friendlyName;

	if(!skipFloat(devicesString, &(m_state.tempSetpointMin)))return false;

	if(!skipFloat(devicesString, &(m_state.tempSetpointMax)))return false;

	int isLocal = 0;
	if(!skipInt(devicesString, &isLocal))return false;

	m_location = (eSensorLocation)isLocal;

	int isEnabled;
	if(!skipInt(devicesString, &isEnabled))return false;
	m_state.bEnabled = isEnabled;

	if(!skipInt(devicesString, &numWatchers))return false;

	while(numWatchers--)
	{
		if(!skipInt(devicesString, &devID))return false;
		LOG_I("Add watcher ID:%d", devID);
		addWatcherDevice(devID);
	}

	int j, k, len;
	autoPilotSlot programSlot;

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

				if(k < MAX_PROGRAMS_PER_DAY)
					m_autoPrograms[j].addElement(programSlot);
				else
					LOG_E("TH[%d] ignore prog %d for day %d", m_ID, k, j);

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

	if(strlen(*devicesString) > 0) /*web will not send this*/
	{
		if(!skipInt(devicesString, &m_state.m_autopilotIndex))
			return false;
		if(!skipInt(devicesString, &m_state.m_autopilotDay))
			return false;
		if(!skipInt(devicesString, &isEnabled)) //dummy is heating
			return false;
		if(!skipInt(devicesString, &isEnabled)) //dummy is cooling
			return false;
		if(!skipFloat(devicesString, &dummyFloat)) //dummy cur temp
			return false;
		if(!skipFloat(devicesString, &dummyFloat)) //dummy cur humid
			return false;
		if(!skipFloat(devicesString, &dummyFloat)) //dummy cur temp 1min
			return false;
		if(!skipFloat(devicesString, &dummyFloat)) //dummy cur temp 8m
			return false;
		if(!skipFloat(devicesString, &dummyFloat)) //dummy cur humid 1m
			return false;
		if(!skipFloat(devicesString, &dummyFloat)) //dummy cur humid 8m
			return false;
	}
	else
	{
		m_state.m_autopilotIndex = -1;
		m_state.m_autopilotDay = -1;
	}

	return true;
}

uint32_t CDeviceTempHumid::serialize(char* buffer, uint32_t size)
{
	int i, j, k, len;
	int sz = m_snprintf(buffer, size, "%d;%d;%s;%.1f;%.1f;%.1f;%d;%d;%d;", devTypeTH, m_ID, m_FriendlyName.c_str(),
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

	sz += m_snprintf(buffer+sz, size-sz, "%d;%d;%d;%d;%.1f;%.1f;%.1f;%.1f;%.1f;%.1f;",
					m_state.m_autopilotIndex,
					m_state.m_autopilotDay,
					m_state.bIsHeating?1:0,
					m_state.bIsCooling?1:0,
					m_state.lastTH.temp,
					m_state.lastTH.humid,
					m_state.fLastTemp_1m,
					m_state.fLastTemp_8m,
					m_state.fLastRH_1m,
					m_state.fLastRH_8m);

	return sz;
}
