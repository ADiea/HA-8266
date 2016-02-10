#ifndef C_DEVICE_HEATER
#define C_DEVICE_HEATER
#include "device.h"

enum eHeaterTurOnTriggers
{
	heatTrig_Manual 	= 0x01,
	heatTrig_TempSensor = 0x02,
};

/*
enum eHeaterRealState
{
	heatState_turningOn,
	heatState_On,

};
*/

struct tHeaterState
{
	tHeaterState(uint16_t highWarn,
			uint16_t lowWarn = 50,
			uint16_t medWarn = 100,
			uint16_t lastRead = 0):
		gasLevel_lastReading(lastRead),
		gasLevel_LowWarningThres(lowWarn),
		gasLevel_MedWarningThres(medWarn),
		gasLevel_HighWarningThres(highWarn),
		isOn(false), isFault(false), isStateSyncWithHardware(false), lastFault(HEATER_FAULT_NONE),
		onMinutesToday(0), currentDayInMonth(-1), timestampOn(0)
	{}

	tHeaterState()
	{
		tHeaterState(200);
	}

	uint16_t gasLevel_lastReading, gasLevel_LowWarningThres,
		gasLevel_MedWarningThres, gasLevel_HighWarningThres;

	byte lastFault;

	bool isOn, isFault, isStateSyncWithHardware;

	int onMinutesToday;
	int currentDayInMonth;
	unsigned long timestampOn;
};


class CDeviceHeater : public CGenericDevice
{
public:

	CDeviceHeater()
	{
		m_deviceType = devTypeHeater;
	}

	virtual bool initDevice()
	{
		m_updateTimer.initializeMs(5000, TimerDelegate(&CDeviceHeater::onUpdateTimer, this)).start(false);
		m_state.isOn = false;
		m_state.isFault = false;
		m_state.isStateSyncWithHardware = false;

		return true;
	}

	virtual bool deserialize(const char **string);
	virtual uint32_t serialize(char* buffer, uint32_t size);

	virtual void requestUpdateState()
	{
		//send state request from radio device
	}

	void onUpdateTimer();

	virtual void triggerState(int reason, void* state);

	virtual bool radioPktReceivedFromDevice(char* pktData, uint16_t pktLen);

	bool sendHeaterStatus(byte status);

	tHeaterState m_state;
};

#endif /*C_DEVICE_HEATER*/
