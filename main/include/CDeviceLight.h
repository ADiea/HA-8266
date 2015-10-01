#ifndef C_DEVICE_LIGHT
#define C_DEVICE_LIGHT
#include "device.h"

enum eTriggers
{
	trigClapClap 	= 0x01,
	trigOnMovement 	= 0x02,
	trigOnTimer 	= 0x04, //Turn on in 5 minutes -> sends interval in seconds to light device
	trigOnManual	= 0x08,
};

enum eLightState
{
	lightOff,
	lightCustomIntensity,
	lightQuarterIntensity,
	lightHalfIntensity,
	lightFullIntensity,

	lightLastState,
};

struct tLightState
{

	tLightState(int minInt,
				int maxInt = 255,
				int dimSpd = 5,
				uint32_t trigger = trigClapClap | trigOnMovement,
				uint32_t capa = trigClapClap | trigOnMovement | trigOnTimer | trigOnManual,
				int onTime = 15):
			   curIntensity(minInt), minIntensity(minInt),
			   maxIntensity(maxInt), dimSpeed(dimSpd),
			   turnOnTriggers(trigger),
			   capabilities(capa),
			   lightState(lightOff), movement_keepOnTimeSeconds(onTime),
			   movement_lastMovementTimestamp(~0){};

	tLightState()
	{
		tLightState(50);
	}

	int curIntensity, minIntensity, maxIntensity,
		dimSpeed, movement_keepOnTimeSeconds;
	uint32_t turnOnTriggers, capabilities, movement_lastMovementTimestamp;
	eLightState lightState;
};

class CDeviceLight : public CGenericDevice
{
public:

	CDeviceLight()
	{
		m_deviceType = devTypeLight;
	}

	void initLight(uint32_t lightID, String& friendlyName, tLightState& state)
	{
		m_ID = lightID;
		m_FriendlyName = friendlyName;
		m_state = state;
	}

	virtual void requestUpdateState(){}
	virtual void triggerState(int reason, void* state){}
	virtual bool radioPktReceivedFromDevice(char* pktData, uint16_t pktLen){}

	virtual bool deserialize(const char *string);
	virtual uint32_t serialize(char* buffer, uint32_t size);

	tLightState m_state;
};

#endif /*C_DEVICE_LIGHT */
