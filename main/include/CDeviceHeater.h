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
		isOn(false), isFault(false), lastFault(HEATER_FAULT_NONE)
	{}

	tHeaterState()
	{
		tHeaterState(200);
	}

	uint16_t gasLevel_lastReading, gasLevel_LowWarningThres,
		gasLevel_MedWarningThres, gasLevel_HighWarningThres;

	byte lastFault;

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

	virtual bool deserialize(const char **string);
	virtual uint32_t serialize(char* buffer, uint32_t size);

	virtual void requestUpdateState()
	{
		//send state request from radio device
	}

	virtual void triggerState(int reason, void* state);

	virtual bool radioPktReceivedFromDevice(char* pktData, uint16_t pktLen);

	tHeaterState m_state;
};

#endif /*C_DEVICE_HEATER*/
