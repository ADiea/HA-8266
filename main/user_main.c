
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>

#include "types.h"
#include "debug.h"

#include "drv/drvUART.h"
#include "drv/drvRGB.h"
#include "drv/drvRadio.h"
#include "drv/drvSDCard.h"
#include "drv/drvDHT22.h"
#include "drv/drvMQ135.h"
#include "drv/drvWiFi.h"
#include "drv/drvDS18B20.h"

extern void wdt_feed(void);

unsigned char gDevicesState = 0x0;

#if DEBUG_BUILD

#define HEART_BEAT 5000 /* milliseconds */
LOCAL os_timer_t gInfo_timer;

LOCAL void ICACHE_FLASH_ATTR heartbeat_cb(void *arg)
{
	wdt_feed();
	LOG(INFO, "System Info\r\n");
	LOG(INFO, "Time=%ld\r\n", system_get_time());
	LOG(INFO, "Chip id=%ld\r\n", system_get_chip_id());
	LOG(INFO, "Free heap size=%ld\r\n", system_get_free_heap_size());
	LOG(INFO, "Mem info:\r\n");
	system_print_meminfo();
	LOG(INFO, "\r\n");
}

#endif /*DEBUG_BUILD*/


void startTimer(os_timer_t* pTimer, os_timer_func_t *pCallback, uint32_t ms, bool bRepeat)
{
	// os_timer_disarm(ETSTimer *ptimer)
	os_timer_disarm(pTimer);
	// os_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg)
	os_timer_setfn(pTimer, pCallback, (void *)0);
	// void os_timer_arm(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag)
	os_timer_arm(pTimer, ms, bRepeat);
}

inline void isDevEnabled(uchar dev)
{
	return (dev & gDevicesState);
}


//de testat pinii daca corespund 
void enableDev(uchar dev, uchar op)
{
	do
	{
		if(op & DISABLE)
		{
			if(!isDevEnabled(dev))
			{
				LOG(INFO, "Dev already disabled %d", dev);
				break;
			}
		}
		else
		{
			if(isDevEnabled(dev))
			{
				LOG(INFO, "Dev already enabled %d", dev);
				break;
			}
		}
		
		if( dev == DEV_RADIO && (ERR_OK != devRadio_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
					break;	
		}
		else if( dev == DEV_SDCARD && (ERR_OK != devSDCard_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}				
		else if( dev == DEV_RGB && (ERR_OK != devRGB_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_MQ135 && (ERR_OK != devMQ135_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_DHT22 && (ERR_OK != devDHT22_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_WIFI && (ERR_OK != devWiFi_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_DSTEMP && (ERR_OK != devDSTemp_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_UART && (ERR_OK != devUART_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
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



void initSystem()
{

#if DEBUG_BUILD
	enableDev(DEV_UART, ENABLE | CONFIG);
#endif	
	
	// enable some system messages
	system_set_os_print(1); 		
	
	//setup SDCard and load custom system settings, then disable SDCard
	enableDev(DEV_SDCARD, ENABLE | CONFIG);	
	enableDev(DEV_SDCARD, DISABLE);	

	//DHT22 periodically enabled to read data
	enableDev(DEV_DHT22, DISABLE);
	
	enableDev(DEV_MQ135, ENABLE | CONFIG);
	
	//RGB periodically enabled to send data
	enableDev(DEV_RGB, DISABLE);
	
	//enable and config Radio, then disable
	enableDev(DEV_RADIO, ENABLE | CONFIG);
	enableDev(DEV_RADIO, DISABLE);
	
	//setup Wifi
	enableDev(DEV_WIFI, ENABLE | CONFIG);
}

void startSystem()
{
#if DEBUG_BUILD
	startTimer(&gInfo_timer, (os_timer_func_t *)heartbeat_cb, HEART_BEAT)
#endif
	
}

void user_init()
{
	initSystem();
	
	setRGBColor(&COLOR_RED);	
	LOG(INFO, "System start.");
	
	startSystem();
	
	setRGBColor(&COLOR_GREEN);
	
	do
	{
		mainLoop();
	}
	while(1);
}

void mainLoop()
{


}
