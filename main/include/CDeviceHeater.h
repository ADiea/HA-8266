#ifndef C_DEVICE_HEATER
#define C_DEVICE_HEATER
#include "device.h"

enum eHeaterTurOnTriggers
{
	heatTrig_Manual 	= 0x01,
	heatTrig_TempSensor = 0x02,
};

struct tHeaterState
{
	tHeaterState(int highWarn,
				int lowWarn = 50,
				int medWarn = 100,
				int lastRead = 0):
		gasLevel_lastReading(lastRead),
		gasLevel_LowWarningThres(lowWarn),
		gasLevel_MedWarningThres(medWarn),
		gasLevel_HighWarningThres(highWarn),
		isOn(false), isFault(false)
	{}

	tHeaterState()
	{
		tHeaterState(200);
	}

	int gasLevel_lastReading, gasLevel_LowWarningThres,
		gasLevel_MedWarningThres, gasLevel_HighWarningThres;

	bool isOn, isFault;
};


class CDeviceHeater : public CGenericDevice
{
public:

	CDeviceHeater()
	{
		m_deviceType = devTypeHeater;
	}

	void initHeater(uint32_t heaterID, String& friendlyName, tHeaterState& state)
	{
		m_ID = heaterID;
		m_FriendlyName = friendlyName;
		m_state = state;
	}

	virtual bool deserialize(const char *string);
	virtual uint32_t serialize(char* buffer, uint32_t size);

	virtual void requestUpdateState()
	{
		//send state request from radio device
	}

	virtual void triggerState(int reason, void* state);

	virtual bool radioPktReceivedFromDevice(char* pktData, uint16_t pktLen)
	{
		//update state
	}

	tHeaterState m_state;
};

#endif /*C_DEVICE_HEATER*/
