#ifndef C_DEVICE_TH
#define C_DEVICE_TH
#include "device.h"

enum eSensorLocation
{
	locLocal,
	locRemote
};

struct tTempHumidState
{
	tTempHumidState(float setpoint):
		tempSetpoint(setpoint),
		bNeedHeating(false),
		bNeedCooling(false)
	{
		lastTH.temp = -99.0f;
		lastTH.humid = -99.0f;
	}

	tTempHumidState()
	{
		tTempHumidState(22.0f);
	}

	TempAndHumidity lastTH;
	float tempSetpoint;

	bool bNeedHeating, bNeedCooling;

	//confort related stuff
};

class CDeviceTempHumid : public CGenericDevice
{
public:

	CDeviceTempHumid()
	{
		m_deviceType = devTypeTH;
	}

	void initTempHumid(	uint32_t deviceID,
						String& friendlyName,
						tTempHumidState& state,
						eSensorLocation location,
						int updateInterval = 5000)
	{
		m_ID = deviceID;
		m_FriendlyName = friendlyName;
		m_state = state;
		m_location = location;

		m_updateInterval = updateInterval;

		m_tempThreshold = 0.1;

		m_updateTimer.initializeMs(m_updateInterval, TimerDelegate(&CDeviceTempHumid::onUpdateTimer, this)).start(false);
	}

	virtual bool deserialize(const char *string);
	virtual uint32_t serialize(char* buffer, uint32_t size);

	void onUpdateTimer();

	virtual void requestUpdateState();

	//no implementation required for sensors
	virtual void triggerState(int reason, void* state){};

	virtual bool radioPktReceivedFromDevice(char* pktData, uint16_t pktLen){}

	tTempHumidState m_state;

	eSensorLocation m_location;

	float m_tempThreshold;

};

#endif /*C_DEVICE_TH */
