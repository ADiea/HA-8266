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
	FilterMovingAve(uint8_t size, uint8_t mod, uint8_t div):fSize(size), fSize_mod(mod), fSize_div(div)
	{
		accumulator = 0;
		nextIndex = 0;
		filter = NULL;
		LOG_I("fltr_0");
	}
	~FilterMovingAve()
	{
		LOG_I("fltr_~");
		if(filter)
			delete filter;
	}

	void init()
	{
		uint16_t i;
		LOG_I("fltr_init_0, size:%u", fSize);

		LOG_I( "%s Heap: %ld",
				SystemClock.getSystemTimeString().c_str(),
				system_get_free_heap_size());

		//if(fSize != 128)
		//filter = new int32_t(fSize);

		LOG_I( "%s Heap: %ld",
				SystemClock.getSystemTimeString().c_str(),
				system_get_free_heap_size());



if(!filter)
{
	LOG_I("fltr_init_0,heap!");
	}
else
		for(i=0;i<fSize;i++)
		{
			LOG_I("fltr_init_0, i:%u", i);
			filter[i] = 0;
		}

		LOG_I("fltr_init_1");
	}

	float feed(float sample)
	{
		int32_t retVal = (int32_t) (sample * 10);
		accumulator += retVal;
		filter[nextIndex] = retVal;
		nextIndex = (nextIndex + 1) & fSize_mod;
		accumulator -= filter[nextIndex];

		retVal = (accumulator >> fSize_div);

		return (retVal/10 + 0.1f*(retVal%10));
	}

	int32_t accumulator;
	uint16_t nextIndex;
	uint16_t fSize, fSize_mod, fSize_div;
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
		bIsCooling(false),
		fAverageRH_1m(NULL),
		fAverageTemp_1m(NULL),
		fAverageTemp_8m(NULL)
	{
		LOG_I("thst_1");
		lastTH.temp = -99.0f;
		lastTH.humid = -99.0f;
	}

	tTempHumidState()
	{
		LOG_I("thst_0");
		tTempHumidState(22.0f, 16.0f, 27.0f);
	}

	~tTempHumidState()
	{
		if(fAverageRH_1m)
			delete fAverageRH_1m;

		if(fAverageTemp_1m)
			delete fAverageTemp_1m;

		if(fAverageTemp_8m)
			delete fAverageTemp_8m;
	}

	TempAndHumidity lastTH;
	float tempSetpoint, tempSetpointMin, tempSetpointMax;

	bool bEnabled;

	bool bNeedHeating, bNeedCooling;

	bool bIsHeating, bIsCooling;

	float fLastTemp_1m, fLastTemp_8m;
	float fLastRH_1m;

	FilterMovingAve *fAverageRH_1m, *fAverageTemp_1m, *fAverageTemp_8m;

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
						//tTempHumidState& state,
						eSensorLocation location,
						int updateInterval = SENSOR_TH_MEASURE_INTERVAL)
	{
		m_ID = deviceID;
		m_FriendlyName = friendlyName;
		//m_state = state;
		m_location = location;

		m_updateInterval = updateInterval;

		m_tempThreshold = 0.1;

		m_state.fAverageRH_1m = new FilterMovingAve(FILTER_1M_LENGTH, FILTER_1M_MOD, FILTER_1M_DIV);
		m_state.fAverageTemp_1m = new FilterMovingAve(FILTER_1M_LENGTH, FILTER_1M_MOD, FILTER_1M_DIV);
		m_state.fAverageTemp_8m = new FilterMovingAve(FILTER_8M_LENGTH, FILTER_8M_MOD, FILTER_8M_DIV);

		m_state.fAverageRH_1m->init();
		m_state.fAverageTemp_1m->init();
		m_state.fAverageTemp_8m->init();

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
