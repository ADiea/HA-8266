#include "device.h"

unsigned char gDevicesState = 0x0;

inline uchar isDevEnabled(uchar dev)
{
	return (dev & gDevicesState);
}

//TODO: test if GPIO pins correspond to HW layout
void enableDev(uchar dev, uchar op)
{
	do
	{
		if(op & DISABLE)
		{
			if(!isDevEnabled(dev))
			{
				LOG(INFO, "Dev alry DIS %d", dev);
				break;
			}
		}
		else
		{
			if(isDevEnabled(dev))
			{
				LOG(INFO, "Dev alry ENA %d", dev);
				break;
			}
		}

		if( dev == DEV_RADIO && (DEV_ERR_OK != devRadio_init(op)))
		{
			LOG(ERR, "Dev %x init FAIL %x", dev, op);
					break;
		}
		else if( dev == DEV_SDCARD && (DEV_ERR_OK != devSDCard_init(op)))
		{
			LOG(ERR, "Dev %x init FAIL %x", dev, op);
			break;
		}
		else if( dev == DEV_RGB && (DEV_ERR_OK != devRGB_init(op)))
		{
			LOG(ERR, "Dev %x init FAIL %x", dev, op);
			break;
		}
		else if( dev == DEV_MQ135 && (DEV_ERR_OK != devMQ135_init(op)))
		{
			LOG(ERR, "Dev %x init FAIL %x", dev, op);
			break;
		}
		else if( dev == DEV_DHT22 && (DEV_ERR_OK != devDHT22_init(op)))
		{
			LOG(ERR, "Dev %x init FAIL %x", dev, op);
			break;
		}
		else if( dev == DEV_WIFI && (DEV_ERR_OK != devWiFi_init(op)))
		{
			LOG(ERR, "Dev %x init FAIL %x", dev, op);
			break;
		}
		else if( dev == DEV_DSTEMP && (DEV_ERR_OK != devDSTemp_init(op)))
		{
			LOG(ERR, "Dev %x init FAIL %x", dev, op);
			break;
		}
		else if( dev == DEV_UART && (DEV_ERR_OK != devUART_init(op)))
		{
			LOG(ERR, "Dev %x init FAIL %x", dev, op);
			break;
		}
		else
		{
			LOG(ERR, "Unknown dev %x", dev);
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
