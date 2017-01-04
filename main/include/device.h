#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "driver.h"
#include "util.h"

#define LOCAL_TEMPHUMID_SENSOR_ID 0

#define INVALID_DEVICE_ID 0xffffffff

#define DEV_PATH_ON_DISK "/"

#define MAX_FRIENDLY_NAME 64

#define MAXDEVSZ 1472
extern char g_devScrapBuffer[MAXDEVSZ];

#define MIN_TIME_WRITE_TO_DISK 5000 //5s
#define ONE_MINUTE 60000

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
					char* buf, uint32_t size, int numEntries, bool &printHeader, int &entriesWritten, uint32_t &entriesRead);
bool deviceAppendLogEntry(uint32_t id, unsigned long timestamp, char* logEntry, eDeviceType devType);
bool deviceDeleteLog(uint32_t id);

#endif /*__DEVICE_H_*/
