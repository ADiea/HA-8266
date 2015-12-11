#include "CDeviceHeater.h"

void CDeviceHeater::triggerState(int reason, void* state)
{
	bool bHeaterRequestOn = false;
	byte pkg[64] = {0};
	byte outLength;
	byte seq = RadioNextSeqID();
	byte doSendPkg = 0;



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
			pkg[3] = HEATER_REQ_ON;
			doSendPkg = 1;
		}
	}
	else
	{
		if(m_state.isOn)
		{
			pkg[3] = HEATER_REQ_OFF;
			doSendPkg = 1;
		}
	}

	if(doSendPkg || !m_state.isStateSyncWithHardware)
	{
		pkg[0] = m_ID;
		pkg[1] = GATEWAY_ID;
		pkg[2] = PKG_TYPE_HEATER;

		pkg[4] = 0xFF && m_state.gasLevel_LowWarningThres;
		pkg[5] = m_state.gasLevel_LowWarningThres >> 8;

		pkg[6] = 0xFF && m_state.gasLevel_MedWarningThres;
		pkg[7] = m_state.gasLevel_MedWarningThres >> 8;

		pkg[8] = 0xFF && m_state.gasLevel_HighWarningThres;
		pkg[9] = m_state.gasLevel_HighWarningThres >> 8;

		pkg[0xa] = seq;
		pkg[0xb] = pkg[0] + pkg[1] + pkg[2] + pkg[3] + pkg[4] + pkg[5] +
				pkg[6] + pkg[7] + pkg[8] + pkg[9] + pkg[0xa];

		if(RadioSend(pkg, PKG_HEATER_LEN, &outLength, 20))
		{
			if(PKG_HEATER_STATUS_LEN == outLength &&
				PKG_TYPE_HEATER_STATUS == pkg[2] &&
			   (seq) == pkg[0xd])
			{
				radioPktReceivedFromDevice((char*)pkg, outLength);
			}
		}
	}
}

bool CDeviceHeater::deserialize(const char **devicesString)
{
	int devID, numWatchers;

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

bool CDeviceHeater::radioPktReceivedFromDevice(char* pkg, uint16_t pktLen)
{
	uint16_t lowTh, medTh, highTh;

	if(PKG_HEATER_STATUS_LEN == pktLen &&
		pkg[1] == m_ID &&
		pkg[0] == GATEWAY_ID &&
		PKG_TYPE_HEATER_STATUS == pkg[2] &&
	   checkRadioChecksum((byte*)pkg, PKG_HEATER_STATUS_LEN))
	{
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
			LOG_E("Heater %d THRESHOLDS differ!", m_ID);
		}

		m_state.isStateSyncWithHardware = true;
	}
}

uint32_t CDeviceHeater::serialize(char* buffer, uint32_t size)
{
	int i;
	int sz = m_snprintf(buffer, size, "%d;%d;%s;%d;%d;%d;%d;", devTypeHeater, m_ID, m_FriendlyName.c_str(),
					m_state.gasLevel_HighWarningThres, m_state.gasLevel_LowWarningThres, m_state.gasLevel_MedWarningThres,
					m_devWatchersList.count());

	for(i = 0; i < m_devWatchersList.count(); i++)
	{
		sz += m_snprintf(buffer + sz, size - sz, "%d;", m_devWatchersList[i].id);
	}

	return sz;
}
