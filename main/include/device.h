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
#include <Libraries/DHT/DHT.h>
#include <SmingCore/SmingCore.h>

#include "util.h"

#define LOCAL_TEMPHUMID_SENSOR_ID 0

#define INVALID_DEVICE_ID 0xffffffff

#define DEV_PATH_ON_DISK "/"

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

#define MAX_FRIENDLY_NAME 64

#define MAXDEVSZ 4096
extern char g_devScrapBuffer[MAXDEVSZ];

#define MIN_TIME_WRITE_TO_DISK 5000 //5s
#define ONE_MINUTE 60000 //5s

#define isDevEnabled(dev) ((dev) & gDevicesState)

void enableDev(unsigned short, uint8_t op);
void initDevices();

class CGenericDevice;

extern Vector<CGenericDevice*> g_activeDevices;

struct Watcher
{
	uint32_t id;
	CGenericDevice *device;
};

enum eDeviceType
{
	devTypeLight,
	devTypeTH,
	devTypeHeater,
};

class CGenericDevice
{
public:
	CGenericDevice():m_ID(INVALID_DEVICE_ID), m_FriendlyName("notInit"),
					 m_LastUpdateTimestamp(~0), isSavedToDisk(true), m_lastLogTimestamp(0){};

	bool isInit()
		{return m_ID != INVALID_DEVICE_ID;}

	virtual void requestUpdateState() = 0;
	virtual void triggerState(int reason, void* state) = 0;

	virtual bool radioPktReceivedFromDevice(char* pktData, uint16_t pktLen) = 0;

	virtual bool deserialize(const char **string) = 0;
	virtual uint32_t serialize(char* buffer, uint32_t size) = 0;

	virtual bool initDevice()
	{
		return true;
	}

	CGenericDevice* findDevice(uint32_t deviceID)
	{
		int i = 0;
		for(; i < g_activeDevices.count(); i++)
		{
			if(deviceID == g_activeDevices[i]->m_ID)
			{
				return g_activeDevices[i];
			}
		}
		return NULL;
	}

	CGenericDevice* getDevice(Watcher& watcher)
	{
		if(watcher.device)
			return watcher.device;

		return findDevice(watcher.id);
	}

	void addWatcherDevice(uint32_t deviceID)
	{
		Watcher watcher;

		watcher.id = deviceID;
		watcher.device = findDevice(deviceID);

		m_devWatchersList.addElement(watcher);
	}

	virtual ~CGenericDevice(){}

	Vector<Watcher> m_devWatchersList;

	uint32_t m_ID;
	String m_FriendlyName;
	unsigned long m_LastUpdateTimestamp;
	unsigned long m_LastWriteToDiskTimestamp;
	eDeviceType m_deviceType;
	int m_updateInterval;
	Timer m_updateTimer;
	bool isSavedToDisk;
	uint32_t m_lastLogTimestamp;
};

#include "CDeviceHeater.h"
#include "CDeviceLight.h"
#include "CDeviceTempHumid.h"

bool deviceWriteToDisk(CGenericDevice *dev);

uint32_t deviceReadLog(uint32_t id, unsigned long fromTime, uint32_t decimation,
					char* buf, uint32_t size, int numEntries);
bool deviceAppendLogEntry(uint32_t id, unsigned long timestamp, char* logEntry);
bool deviceDeleteLog(uint32_t id);

#endif /*__DEVICE_H_*/
