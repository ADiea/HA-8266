#include "CDeviceHeater.h"

bool CDeviceHeater::sendHeaterStatus(byte status)
{
	byte pkg[64] = {0};
	byte outLength;
	byte seq = RadioNextSeqID();
	bool bRet = false;
	byte retry = 0;
	eRadioError radioError = err_NoError;

	pkg[0] = m_ID;
	pkg[1] = GATEWAY_ID;
	pkg[2] = PKG_TYPE_HEATER;

	pkg[3] = status;

	pkg[4] = 0xFF & m_state.gasLevel_LowWarningThres;
	pkg[5] = m_state.gasLevel_LowWarningThres >> 8;

	pkg[6] = 0xFF & m_state.gasLevel_MedWarningThres;
	pkg[7] = m_state.gasLevel_MedWarningThres >> 8;

	pkg[8] = 0xFF & m_state.gasLevel_HighWarningThres;
	pkg[9] = m_state.gasLevel_HighWarningThres >> 8;

	pkg[0xa] = seq;
	pkg[0xb] = pkg[0] + pkg[1] + pkg[2] + pkg[3] + pkg[4] + pkg[5] +
				pkg[6] + pkg[7] + pkg[8] + pkg[9] + pkg[0xa];

	LOG_I("HEATER(%d) TX: %s", m_ID, (pkg[3]==HEATER_REQ_OFF)?"OFF":"ON");

	do
	{
		radioError = DrvRadio.RadioSend(pkg, PKG_HEATER_LEN, &outLength, 20);
		if(err_NoError == radioError)
		{
			if(PKG_HEATER_STATUS_LEN == outLength &&
				PKG_TYPE_HEATER_STATUS == pkg[2] &&
			   (seq) == pkg[0xd])
			{
				bRet = true;
				radioPktReceivedFromDevice((char*)pkg, outLength);
				break;
			}
		}
	}
	while(++retry < 4);

	if(!bRet)
	{
		LOG_I("HEATER(%d) TX FAIL err:%d", m_ID, radioError);
	}

	return bRet;
}

void CDeviceHeater::triggerState(int reason, void* state)
{
	bool bHeaterRequestOn = false;

	byte doSendPkg = 0,
		 status = (m_state.isOn)?HEATER_REQ_ON:HEATER_REQ_OFF;
	int dayInMo = SystemClock.now().Day;

	byte retry=0;

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
				if(m_state.isOn)
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
		if(!m_state.isOn)
		{
			status = HEATER_REQ_ON;
			doSendPkg = 1;
		}
	}
	else
	{
		if(m_state.isOn)
		{
			status = HEATER_REQ_OFF;
			doSendPkg = 1;
		}
	}

	if(!doSendPkg && !m_state.isStateSyncWithHardware)
	{
		if(m_state.isOn)
		{
			status = HEATER_REQ_ON;
		}
		else
		{
			status = HEATER_REQ_OFF;
		}
		doSendPkg = 1;
	}

	if(m_state.currentDayInMonth < 0 || (m_state.currentDayInMonth != dayInMo))
	{
		m_state.currentDayInMonth = dayInMo;
		m_state.timestampOn = (unsigned long)SystemClock.now(eTZ_Local).toUnixTime();
		m_state.onMinutesToday = 0;
	}

	if(system_get_time() / 1000 - m_LastUpdateTimestamp > 10000)
	{
		LOG_I("HEATER(%d) Force radio send", m_ID);
		doSendPkg = 1;
		m_LastUpdateTimestamp = system_get_time() / 1000;
	}

	LOG_I("HEATER(%d) trigState send:%d reqHeat:%d isSync:%d state:%d",
			m_ID, doSendPkg, bHeaterRequestOn, m_state.isStateSyncWithHardware, m_state.isOn);

	if(doSendPkg)
	{
		sendHeaterStatus(status);
	}
}

bool CDeviceHeater::radioPktReceivedFromDevice(char* pkg, uint16_t pktLen)
{
	uint16_t lowTh, medTh, highTh;

	if(PKG_HEATER_REQUEST_LEN == pktLen &&
		pkg[1] == m_ID &&
		pkg[0] == GATEWAY_ID &&
		PKG_TYPE_HEATER_REQUEST == pkg[2] &&
	   checkRadioChecksum((byte*)pkg, PKG_HEATER_REQUEST_LEN))
	{
		LOG_I("HEATER(%d) REQUEST", m_ID);
		sendHeaterStatus(m_state.isOn ? HEATER_REQ_ON : HEATER_REQ_OFF);
	}
	else if(PKG_HEATER_STATUS_LEN == pktLen &&
		pkg[1] == m_ID &&
		pkg[0] == GATEWAY_ID &&
		PKG_TYPE_HEATER_STATUS == pkg[2] &&
	   checkRadioChecksum((byte*)pkg, PKG_HEATER_STATUS_LEN))
	{
		if(!m_state.isOn && (pkg[3] & HEATER_STATUS_ON))//turning on
		{
			m_state.timestampOn = (unsigned long)SystemClock.now(eTZ_Local).toUnixTime();
		}
		else if(m_state.isOn && !(pkg[3] & HEATER_STATUS_ON)) //turning off
		{
			unsigned long min = ((unsigned long)SystemClock.now(eTZ_Local).toUnixTime() -
								m_state.timestampOn) / 60;
			if(min == 0) min = 1;

			m_state.onMinutesToday += min;
		}

		m_state.isOn = pkg[3] & HEATER_STATUS_ON;
		m_state.isFault = pkg[3] & HEATER_STATUS_FAULT;
		m_state.lastFault = pkg[4];

		m_state.gasLevel_lastReading = ((uint16_t)pkg[6])<<8 | pkg[5];

		lowTh = ((uint16_t)pkg[8])<<8 | pkg[7];
		medTh = ((uint16_t)pkg[0xa])<<8 | pkg[9];
		highTh = ((uint16_t)pkg[0xc])<<8 | pkg[0xb];

		LOG_I("Heater %d; on:%d fault:%d last:%u",
				m_ID, m_state.isOn, m_state.isFault, m_state.gasLevel_lastReading);

		if(lowTh != m_state.gasLevel_LowWarningThres ||
			medTh != m_state.gasLevel_MedWarningThres ||
			highTh != m_state.gasLevel_HighWarningThres )
		{
			LOG_E("Heater %d THRESHOLDS differ! %d %d %d", m_ID, lowTh, medTh, highTh);
		}

		m_state.isStateSyncWithHardware = true;
		m_LastUpdateTimestamp = system_get_time() / 1000;
	}
}

bool CDeviceHeater::deserialize(const char **devicesString)
{
	int devID, numWatchers;

	char friendlyName[MAX_FRIENDLY_NAME];

	if(!skipInt(devicesString, &devID))return false;
	m_ID = devID;

	if(!skipString(devicesString, (char*)friendlyName, MAX_FRIENDLY_NAME))return false;
	m_FriendlyName = friendlyName;

	LOG_I("HEATER device ID:%d NAME: %s", devID, friendlyName);

	if(!skipInt(devicesString, &devID))return false;
	m_state.gasLevel_HighWarningThres = devID;
	if(!skipInt(devicesString, &devID))return false;
	m_state.gasLevel_MedWarningThres = devID;
	if(!skipInt(devicesString, &devID))return false;
	m_state.gasLevel_LowWarningThres = devID;

	if(!skipInt(devicesString, &numWatchers))return false;

	while(numWatchers--)
	{
		if(!skipInt(devicesString, &devID))return false;
		LOG_I("Add watcher ID:%d", devID);
		addWatcherDevice(devID);
	}

	if(strlen(*devicesString) > 0)
	{
		if(!skipInt(devicesString, &devID))return false; //dev on dummy
		if(!skipInt(devicesString, &devID))return false; //is fault dummy
		if(!skipInt(devicesString, &devID))return false; //last reading dummy
		if(!skipInt(devicesString, &devID))return false; //last fault dummy
		if(!skipInt(devicesString, &devID))return false; //minutes on today dummy
	}

	return true;
}

void CDeviceHeater::onUpdateTimer()
{
	if(!isSavedToDisk)
	{
		isSavedToDisk  = deviceWriteToDisk(this);
	}



	m_updateTimer.initializeMs(5000, TimerDelegate(&CDeviceHeater::onUpdateTimer, this)).start(false);
}

uint32_t CDeviceHeater::serialize(char* buffer, uint32_t size)
{
	int i;
	unsigned long min = 0;

	int sz = m_snprintf(buffer, size, "%d;%d;%s;%d;%d;%d;%d;",
					devTypeHeater,
					m_ID, m_FriendlyName.c_str(),
					m_state.gasLevel_HighWarningThres,
					m_state.gasLevel_MedWarningThres,
					m_state.gasLevel_LowWarningThres,
					m_devWatchersList.count());

	for(i = 0; i < m_devWatchersList.count(); i++)
	{
		sz += m_snprintf(buffer + sz, size - sz, "%d;", m_devWatchersList[i].id);
	}

	if(m_state.isOn)
	{
		min = ((unsigned long)SystemClock.now(eTZ_Local).toUnixTime() -
							m_state.timestampOn) / 60;
		if(min == 0) min = 1;
	}

	min += m_state.onMinutesToday;

	sz += m_snprintf(buffer + sz, size - sz,
					"%d;%d;%d;%d;%d;",
					m_state.isOn?1:0,
					m_state.isFault?1:0,
					m_state.gasLevel_lastReading,
					m_state.lastFault,
					min);

	return sz;
}
