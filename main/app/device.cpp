#include "device.h"

bool devicesLoadFromDisk();
bool deviceReadFromDisk(char* path);

char g_devScrapBuffer[MAXDEVSZ];

 Vector<CGenericDevice*> g_activeDevices;

unsigned short gDevicesState = 0x0000;

struct devCtl
{
	unsigned short dev;
	uint8_t (*initFunc)(uint8_t);
};

devCtl gDevices[] =
{
	{DEV_RADIO, devRadio_init},
	{DEV_SDCARD, devSDCard_init},
	{DEV_RGB, devRGB_init},
	{DEV_MQ135, devMQ135_init},
	{DEV_DHT22, devDHT22_init},
	{DEV_WIFI, devWiFi_init},
	{DEV_DSTEMP, devDSTemp_init},
	{DEV_UART, devUART_init}
};

#define NUM_DEVICES (sizeof(gDevices)/sizeof(gDevices[0]))

void initDevices()
{
	int retry = 10;

	enableDev(DEV_UART, ENABLE | CONFIG);

	//setup SDCard and load custom system settings
	enableDev(DEV_SDCARD, ENABLE | CONFIG);
	//enableDev(DEV_SDCARD, DISABLE);

	//DHT22 periodically enabled to read data
	enableDev(DEV_DHT22, ENABLE | CONFIG);

	enableDev(DEV_MQ135, ENABLE | CONFIG);

	//RGB periodically enabled to send data
	enableDev(DEV_RGB, DISABLE);

	//enable and config Radio
	enableDev(DEV_RADIO, ENABLE | CONFIG);

	//start listening for incoming packets
	if(Radio)
		Radio->startListening();

	//setup Wifi
	enableDev(DEV_WIFI, ENABLE | CONFIG);

	while(!devicesLoadFromDisk() && --retry);

}


/*generic local device logic */
//TODO: test if GPIO pins correspond to HW layout
void enableDev(unsigned short dev, uint8_t op)
{
	uint8_t i = 0;
	uint8_t retVal;
	do
	{
		if(op & DISABLE)
		{
			if(!isDevEnabled(dev))
			{
				LOG_I( "DEV %x is DIS\n", dev);
				break;
			}
		}
		else
		{
			if(isDevEnabled(dev))
			{
				LOG_I( "DEV %x is ENA\n", dev);
				break;
			}
		}

		for(; i<NUM_DEVICES; i++)
		{
			if(dev == gDevices[i].dev)
			{
				retVal = gDevices[i].initFunc(op);
				if(DEV_ERR_OK != retVal)
				{
					LOG_E( "DEV %x,%x FAIL: %d\n", dev, op, retVal);
				}
				break;
			}
		}

		if( NUM_DEVICES == i )
		{
			LOG_E( "DEV %x unknown\n", dev);
			break;
		}

		//All went well, update gDevicesState
		if(op & DISABLE)
		{
			gDevicesState &= ~dev;
		}
		else
		{
			gDevicesState |= dev;
		}
	}
	while(0);
}

bool devicesLoadFromDisk()
{
	bool bRet = false;

	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i;
	char *fn;   /* This function assumes non-Unicode configuration */

	if(getRadio(1000))
	{
		res = f_opendir(&dir, DEV_PATH_ON_DISK);                       /* Open the directory */
		if (res == FR_OK)
		{
			for (;;)
			{
				res = f_readdir(&dir, &fno);                   /* Read a directory item */
				if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
				if (fno.fname[0] == '.') continue;             /* Ignore dot entry */

				if(strstr(fno.fname, "DEV_") || strstr(fno.fname, "dev_"))
					deviceReadFromDisk(fno.fname);
			}
			f_closedir(&dir);
			bRet = true;
		}
		else
		{
			LOG_E( "devicesLoadFromDisk: err %d", (int)res);
		}

		releaseRadio();

	}
	else
	{
		LOG_E( "devicesLoadFromDisk: radio busy");
	}

	return bRet;
}

bool deviceWriteToDisk(CGenericDevice *dev)
{
	WDT.alive();

	bool bRet = false;
	FIL file;
	FRESULT fRes;

	uint32_t actual, size;

	char fname[128];

	if(!dev )
		return false;

	unsigned long now = millis();

	if( now - dev->m_LastWriteToDiskTimestamp < MIN_TIME_WRITE_TO_DISK)
	{
		LOG_E("devWriteDisk HOLD UPDATE for %d\n", dev->m_ID);
		return false;
	}

	m_snprintf(fname, sizeof(fname), "DEV_%d", dev->m_ID);

	if(getRadio(1000))
	{
		do
		{
			size = dev->serialize(g_devScrapBuffer, MAXDEVSZ);

			if(size == MAXDEVSZ)
			{
				LOG_E("devWriteDisk NOSPACE %d\n", dev->m_ID);
				f_close(&file);
				break;
			}

			fRes = f_open(&file, fname, FA_WRITE | FA_CREATE_ALWAYS);

			if (fRes != FR_OK)
			{
				LOG_E("devWriteDisk err %d", (int)fRes);
				break;
			}

			f_write(&file, g_devScrapBuffer, size, &actual);

			if (actual != size)
			{
				LOG_E("devWriteDisk written %d of %d bytes\n", actual, size);
			}
			else
			{
				LOG_E("devWriteDisk SAVED %d\n", dev->m_ID);
				bRet = true;
			}

			f_close(&file);
			dev->m_LastWriteToDiskTimestamp = now;
		}
		while(0);

		releaseRadio();
	}
	else
	{
		LOG_E( "devWriteDisk: radio busy");
	}

	return bRet;
}

bool deviceReadFromDisk(char* path)
{
	WDT.alive();

	FIL file;
	FRESULT fRes;
	FILINFO fno;
	uint32_t fActualSize;
	CGenericDevice *device = NULL;

	bool bRet = false;
	char *devicesString = NULL, *originalDevString = NULL;
	int devType;

	do{
		fRes = f_stat(path, &fno);
		if(fRes != FR_OK)
		{
			LOG_E("devReadDisk err %d", (int)fRes);
			break;
		}

		if(fno.fsize == 0)
		{
			LOG_E("File %s has 0 bytes", path);
			break;
		}

		devicesString = new char[fno.fsize + 1];
		originalDevString = devicesString;
		if(!devicesString)
		{
			LOG_E("devReadDisk no heap");
			break;
		}

		LOG_I("Loading dev %s, %d bytes", path, fno.fsize);

		fRes = f_open(&file, path, FA_READ);

		if (fRes != FR_OK)
		{
			LOG_E("devReadDisk fopen: %d", (int)fRes);
			break;
		}

		f_read(&file, devicesString, fno.fsize, &fActualSize);
		f_close(&file);

		if(fActualSize != fno.fsize)
		{
			LOG_E("devReadDisk only read %d", fActualSize);
			break;
		}

		LOG_D("devReadDisk read %d:%s", fActualSize, devicesString);

		devicesString[fActualSize] = 0;

		if(!skipInt((const char**)&devicesString, &devType))
			break;

		switch(devType)
		{
			case devTypeLight:
			{
				LOG_I("Type: LIGHT");
				device = new CDeviceLight();
			}
			break;

			case devTypeTH:
			{
				LOG_I("Type: TH");
				device = new CDeviceTempHumid();
			}
			break;

			case devTypeHeater:
			{
				LOG_I("Type:HEATER");
				device = new CDeviceHeater();
			}
			break;

			default:
				LOG_I("UNKN device:%d", devType);
				break;
		};

		if(!device)
		{
			LOG_E("devReadDisk noheap or unkn");
			break;
		}
		else if( !device->deserialize((const char**)&devicesString))
		{
			LOG_E("devReadDisk !deserial");
			delete device;
			device = NULL;
			break;
		}

		if(!device->initDevice())
		{
			LOG_E("devReadDisk !init");
		}
	}
	while(0);

	if(originalDevString)
			delete[] originalDevString;

	if(device)
	{
		g_activeDevices.addElement(device);
		LOG_E("Added device.");
		bRet = true;
	}

	return bRet;
}

bool deviceAppendLogEntry(uint32_t id, char* logEntry)
{
	WDT.alive();

	bool bRet = false;
	FIL file;
	FRESULT fRes;

	uint32_t actual, size;

	char fname[128];

	m_snprintf(fname, sizeof(fname), "LOG_%d", id);

	if(getRadio(1000))
	{
		do
		{
			size = strlen(logEntry);

			fRes = f_open(&file, fname, FA_WRITE | FA_OPEN_ALWAYS);

			if (fRes != FR_OK)
			{
				LOG_E("deviceAppendLog err %d", (int)fRes);
				break;
			}

			LOG_E("deviceAppendLog ftell %d fsize %d", f_tell(&file), f_size(&file));

			if(f_size(&file) > 0)
			{
				f_lseek(&file, f_size(&file));
			}

			LOG_E("deviceAppendLog ftell %d ", f_tell(&file));

			///
			f_close(&file);
			fRes = f_open(&file, fname, FA_WRITE | FA_CREATE_ALWAYS);
			///

			f_write(&file, logEntry, size, &actual);

			if (actual != size)
			{
				LOG_E("deviceAppendLog written %d of %d bytes\n", actual, size);
			}
			else
			{
				bRet = true;
			}

			LOG_E("deviceAppendLog ftell %d fsize %d", f_tell(&file), f_size(&file));
			f_close(&file);
		}
		while(0);

		releaseRadio();
	}
	else
	{
		LOG_E( "deviceAppendLog: radio busy");
	}

	return bRet;
}


uint32_t deviceReadLog(uint32_t id, unsigned long fromTime, uint32_t decimation,
					char* buf, uint32_t size, int numEntries)
{
	WDT.alive();

	FIL file;
	FRESULT fRes;

#define OVF_GUARD 24

	bool foundSemicolon;
	char *bufPtr, *startPtr;
	int remainingBytes = 0;
	char savedChar;
	uint32_t entriesRead = 0;
	uint32_t sizeWritten = 0, fActualSize;

	enum ParseLogState
	{
		elogStaWaitTimestamp,
		eLogStaWaitParamNumberInt,
		eLogStaWaitParamNumberFloat,
		eLogStaCountParamInt,
		eLogStaCountParamFloat,
	};

	ParseLogState logState = elogStaWaitTimestamp,
			logLastState = elogStaWaitTimestamp;
	int paramNoInt = 0, paramTotalInt = 0, paramNoFloat = 0, paramTotalFloat = 0;

	bool skipCurEntry = true;

	int token;
	float ftoken;
	char path[128];

	do{

		m_snprintf(path, sizeof(path), "LOG_%d", id);

		fRes = f_open(&file, path, FA_READ);

		if (fRes != FR_OK)
		{
			LOG_E("deviceReadLog fopen: %d", (int)fRes);
			break;
		}

		do
		{
			WDT.alive();

			f_read(&file, path + remainingBytes, 128 - remainingBytes - 1, &fActualSize);

			LOG_I("deviceReadLog read %d:%s", (int)fActualSize, path + remainingBytes);

			foundSemicolon = false;
			bufPtr = path + remainingBytes + fActualSize;
			while(!foundSemicolon && bufPtr > path)
			{
				if(*(--bufPtr) ==';' )
				{
					LOG_I("deviceReadLog last; %s", bufPtr);
					foundSemicolon = true;
					savedChar = *(bufPtr + 1);
					*(bufPtr + 1) = 0;
				}
			}

			if(foundSemicolon)
			{
				startPtr = path;
				do
				{
					if(eLogStaCountParamFloat == logState)
					{
						if (!skipFloat((const char**)&startPtr, &ftoken))
							return entriesRead;
					}
					else
					{
						if(!skipInt((const char**)&startPtr, &token))
							return entriesRead;
					}
					logLastState = logState;
					switch(logState)
					{
						case elogStaWaitTimestamp:
						{
							if((entriesRead++ % decimation == 0) && token > fromTime)
								skipCurEntry = false;
							else
								skipCurEntry = true;

							LOG_I("deviceReadLog Entry %d TS %u skip %d", entriesRead, token, skipCurEntry);

							logState = eLogStaWaitParamNumberInt;
							break;
						}
						case eLogStaWaitParamNumberInt:
						{
							paramNoInt = 0;
							paramTotalInt = token;
							//LOG_I("deviceReadLog eLogStaWaitParamNumberInt");

							logState = eLogStaWaitParamNumberInt;
						}
						break;
						case eLogStaWaitParamNumberFloat:
						{
							//LOG_I("deviceReadLog eLogStaWaitParamNumberFloat");
							paramNoFloat = 0;
							paramTotalFloat = token;

							if(paramTotalInt > 0)
								logState = eLogStaCountParamInt;
							else if(paramTotalFloat > 0)
								logState = eLogStaCountParamFloat;
							else
								logState = elogStaWaitTimestamp;
							break;
						}
						case eLogStaCountParamInt:
						{
							//LOG_I("deviceReadLog eLogStaCountParamInt");
							++paramNoInt;
							if(paramNoInt == paramTotalInt)
								logState = eLogStaCountParamFloat;
							break;
						}
						case eLogStaCountParamFloat:
						{
							//LOG_I("deviceReadLog eLogStaCountParamFloat");
							++paramNoFloat;
							if(paramNoFloat == paramTotalFloat)
								logState = elogStaWaitTimestamp;
							break;
						}
					};

					if(!skipCurEntry)
					{
						if(eLogStaCountParamFloat == logState ||
								eLogStaCountParamFloat == logLastState)
						{
							sizeWritten += m_snprintf(buf + sizeWritten, size - sizeWritten,
											"%.1f;", ftoken);
						}
						else
						{
							sizeWritten += m_snprintf(buf + sizeWritten, size - sizeWritten,
											"%d;", token);
						}
					}
				}while(strlen(startPtr) && numEntries > 0 && (size - sizeWritten > OVF_GUARD));

				path[0] = savedChar;
				remainingBytes = 0;
				while(*(++bufPtr) != 0)
					path[++remainingBytes] = *bufPtr;
			}
			else
			{
				break;
			}
		}while(fActualSize > 0 && numEntries > 0 && (size - sizeWritten > OVF_GUARD));

		f_close(&file);
	}
	while(0);
	LOG_I("deviceReadLog end");
	return sizeWritten;
}
