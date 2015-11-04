#ifndef C_DEVICE_TH
#define C_DEVICE_TH
#include "device.h"

#define SENSOR_TH_MEASURE_INTERVAL 4000
#define FILTER_1M_LENGTH 16
#define FILTER_1M_MOD 15
#define FILTER_1M_DIV 4

#define FILTER_8M_LENGTH 128
#define FILTER_8M_MOD 127
#define FILTER_8M_DIV 7



struct FilterMovingAve
{
	FilterMovingAve()
	{
		accumulator = 0;
		nextIndex = 0;
	}
	~FilterMovingAve()
	{
	}

	void init(int32_t *thefilter, uint16_t size, uint16_t mod, uint16_t div);


	float feed(float sample);


	int32_t accumulator;
	uint16_t nextIndex;
	uint16_t fSize, fSize_mod, fSize_div, isInvalid;
	int32_t *filter;

};

enum eSensorLocation
{
	locLocal,
	locRemote
};

struct tTempHumidState
{
	tTempHumidState(float setpoint, float setpointMin, float setpointMax):
		tempSetpoint(setpoint),
		tempSetpointMin(setpointMin),
		tempSetpointMax(setpointMax),
		bNeedHeating(false),
		bNeedCooling(false),
		bEnabled(true),
		bIsHeating(false),
		bIsCooling(false)
	{
		lastTH.temp = -99.0f;
		lastTH.humid = -99.0f;
	}

	tTempHumidState():
		tempSetpoint(22),
		tempSetpointMin(16),
		tempSetpointMax(27),
		bNeedHeating(false),
		bNeedCooling(false),
		bEnabled(true),
		bIsHeating(false),
		bIsCooling(false)
	{
	}

	~tTempHumidState()
	{

	}

	TempAndHumidity lastTH;
	float tempSetpoint, tempSetpointMin, tempSetpointMax;

	bool bEnabled;

	bool bNeedHeating, bNeedCooling;

	bool bIsHeating, bIsCooling;

	float fLastTemp_1m, fLastTemp_8m;
	float fLastRH_1m;

	FilterMovingAve fAverageRH_1m, fAverageTemp_1m, fAverageTemp_8m;
	int32_t filterRH[FILTER_1M_LENGTH], filterTemp1m[FILTER_1M_LENGTH], filterTemp8m[FILTER_8M_LENGTH];

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
						eSensorLocation location,
						int updateInterval = SENSOR_TH_MEASURE_INTERVAL)
	{
		m_ID = deviceID;
		m_FriendlyName = friendlyName;
		//m_state = state;
		m_location = location;

		m_updateInterval = updateInterval;

		m_tempThreshold = 0.1;

		m_state.fAverageRH_1m.init(m_state.filterRH, FILTER_1M_LENGTH, FILTER_1M_MOD, FILTER_1M_DIV);
		m_state.fAverageTemp_1m.init(m_state.filterTemp1m, FILTER_1M_LENGTH, FILTER_1M_MOD, FILTER_1M_DIV);
		m_state.fAverageTemp_8m.init(m_state.filterTemp8m, FILTER_8M_LENGTH, FILTER_8M_MOD, FILTER_8M_DIV);

		m_updateTimer.initializeMs(m_updateInterval, TimerDelegate(&CDeviceTempHumid::onUpdateTimer, this)).start(false);
	}

	virtual bool deserialize(const char **string);
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
