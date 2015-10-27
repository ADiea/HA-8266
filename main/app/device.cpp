#include "device.h"




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

void loadSavedDevices()
{
	const char *devicesString = "3;1;0;IndoorTemp;29.2;16.0;26.5;0;1;1;1;2;1;Heater\\;;200;50;100;1;0;0;2;LightHall;";

	int numDevices, iDev = 0, devType, numWatchers;

	LOG_I("Loading saved devs\n");
	if(!skipInt(&devicesString, &numDevices))return;

	while(iDev++ < numDevices)
	{
		LOG_I("Loading dev %d of %d", iDev, numDevices);
		if(!skipInt(&devicesString, &devType))return;


		switch(devType)
		{
			case devTypeLight:
			{
				LOG_I("LIGHT");
				CDeviceLight *device = new CDeviceLight();
				if(!device)
				{
					LOG_E("Fatal: heap(%d)", iDev);
					return;
				}
				LOG_I("deserialize");
				if( device->deserialize(&devicesString))
				{
					g_activeDevices.addElement(device);
				}
				else
				{
					LOG_E("Fatal: deserial(%d)", iDev);
				}
			}
			break;

			case devTypeTH:
			{
				LOG_I("TH");
				CDeviceTempHumid *device = new CDeviceTempHumid();
				if(!device)
				{
					LOG_E("Fatal: heap(%d)", iDev);
					return;
				}

				if( device->deserialize(&devicesString))
				{
					g_activeDevices.addElement(device);
				}
				else
				{
					LOG_E("Fatal: deserial(%d)", iDev);
				}
			}
			break;

			case devTypeHeater:
			{
				LOG_I("HEATER");
				CDeviceHeater *device = new CDeviceHeater();
				if(!device)
				{
					LOG_E("Fatal: heap(%d)", iDev);
					return;
				}

				if( device->deserialize(&devicesString))
				{
					g_activeDevices.addElement(device);
				}
				else
				{
					LOG_E("Fatal: deserial(%d)", iDev);
				}
			}
			break;

			default:
				LOG_I("UNKN device:%d", devType);
				break;
		};
	}

	LOG_I("Done adding %d devices", numDevices);

}


void initDevices()
{
	enableDev(DEV_UART, ENABLE | CONFIG);

	//setup SDCard and load custom system settings, then disable SDCard
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

	loadSavedDevices();
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



