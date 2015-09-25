#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "debug.h"

#include "drv/drvUART.h"
#include "drv/drvRGB.h"
#include "drv/drvRadio.h"
#include "drv/drvSDCard.h"
#include "drv/drvDHT22.h"
#include "drv/drvMQ135.h"
#include "drv/drvWiFi.h"
#include "drv/drvDS18B20.h"

#include "Wiring/WVector.h"

#define LOCAL_TEMPHUMID_SENSOR_ID 0

#define INVALID_DEVICE_ID 0xffffffff

#define DEV_RADIO   0x0001
#define DEV_SDCARD  0x0002
#define DEV_RGB     0x0004
#define DEV_MQ135   0x0008
#define DEV_DHT22   0x0010
#define DEV_WIFI    0x0020
#define DEV_DSTEMP  0x0040
#define DEV_UART    0x0080

#define DISABLE 	0x01
#define ENABLE 		0x02
#define CONFIG 		0x04

#define DEV_ERR_OK 		0
#define DEV_OTHER_ERR 	1
#define DEV_DEVIO_ERR 	2
#define DEV_PARAM_ERR 	3

#define isDevEnabled(dev) ((dev) & gDevicesState)

void enableDev(unsigned short, uint8_t op);
void initDevices();

class CGenericDevice
{
public:

	void requestUpdateState();

	void triggerState(int reason, int state);

	uint32_t m_ID;
	String m_FriendlyName;
	uint32_t m_LastUpdateTimestamp;
};

enum eTriggers
{
	trigClapClap 	= 0x01,
	trigOnMovement 	= 0x02,
	trigOnTimer 	= 0x04, //Turn on in 5 minutes -> sends interval in seconds to light device
	trigOnManual	= 0x08,
};

enum eClapClapState
{
	clapOff,
	clapQuarterIntensity,
	clapHalfIntensity,
	clapFullIntensity,
};

class devLight : public CGenericDevice
{
public:
	int curIntensity = 0;
	int minIntensity = 0;
	int maxIntensity = 255;
	int dimSpeed = 5;

	uint32_t turnOnTriggers = trigClapClap | trigOnMovement;

	uint32_t capabilities = trigClapClap | trigOnMovement | trigOnTimer | trigOnManual;

	eClapClapState clapState = clapOff;

	int movement_keepOnTimeSeconds = 15;
	uint32_t movement_lastMovementSecondsAgo = ~0;


};

enum eHeaterTypes
{
	heater_gasHeater,
	heater_electricHeater
};

enum eHeaterTurOnTriggers
{
	heatTrig_Manual,
	heatTrig_TempSensor,
};


struct devHeater
{
	eHeaterTypes type;

	uint32_t linked_TempHumidSensorId = LOCAL_TEMPHUMID_SENSOR_ID;

	int gasLevel_lastReading;
	int gasLevel_LowWarningThres;
	int gasLevel_MedWarningThres;
	int gasLevel_HighWarningThres;

	bool isOn = false;
	bool isFault = false;

	uint32_t lastUpdateTimestamp;
};


struct devTempHumidSensor
{
	int lastTemp;
	int lastHumid;

	int tempSetpoint;

	//confort related stuff

	uint32_t lastUpdateTimestamp;
};

extern Vector<devLight> g_activeLights;
extern Vector<devHeater> g_activeHeaters;
extern Vector<devTempHumidSensor> g_activeTHs;


#endif /*__DEVICE_H_*/
