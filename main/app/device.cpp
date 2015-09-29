#include "device.h"

 Vector<CDeviceLight*> g_activeLights;
 Vector<CDeviceHeater*> g_activeHeaters;
 Vector<CDeviceTempHumid*> g_activeTHs;

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
	CDeviceTempHumid *localTHSensor = new CDeviceTempHumid();
	if(!localTHSensor)
	{
		LOG_E("Fatal: no heap to init devices");
		return;
	}

	enableDev(DEV_UART, ENABLE | CONFIG);

	//setup SDCard and load custom system settings, then disable SDCard
	enableDev(DEV_SDCARD, ENABLE | CONFIG);
	//enableDev(DEV_SDCARD, DISABLE);

	//DHT22 periodically enabled to read data
	enableDev(DEV_DHT22, ENABLE | CONFIG);

	tTempHumidState thState(22.1f);
	String thName("IndoorTemp");
	localTHSensor->initTempHumid(LOCAL_TEMPHUMID_SENSOR_ID, thName, thState, locLocal);

	g_activeTHs.addElement(localTHSensor);


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

}

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
