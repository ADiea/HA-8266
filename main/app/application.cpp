#include <user_config.h>
#include <SmingCore/SmingCore.h>

//provided by linker
extern char _heap_start;

extern uint8_t gHeapOpFlushAfter;

//TODO: CHANGE static const char secret[] PROGMEM = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
//TODO: make PR to remove spiffs_mount(); from appinit/user_main.cpp

#include "debug.h"
#include "device.h"
#include "commWeb.h"
#include "webserver.h"
#include "webclient.h"

//#include

/*
 The following 2 defines are present in wifipass.h
 #define WIFI_SSID "PleaseEnterSSID"
 #define WIFI_PWD "PleaseEnterPass"
*/
#include "wifipass.h"

#define ONE_SECOND 1000000

static inline unsigned get_ccount(void)
{
	unsigned r;
	asm volatile ("rsr %0, ccount" : "=r"(r));
	return r;
}

static void mainLoop(void);

Timer tmrMainLoop;
#define LOOP_TIME (1 * ONE_SECOND)

#if DEBUG_BUILD
	#define HEART_BEAT (60 * ONE_SECOND)
	Timer tmrHeartBeat;

	static void heartbeat_cb(void)
	{
		LOG_I( "%s Heap: %ld",
				SystemClock.getSystemTimeString().c_str(),
				system_get_free_heap_size());
	}
#endif /*DEBUG_BUILD*/

// Will be called when WiFi station was connected to AP
void connectOk()
{
	LOG_I( "AP CONNECT");
	startWebServers();
	wsCliStart();
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	LOG_E("FAIL CONNECT");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void startSystem()
{

#if DEBUG_BUILD
	tmrHeartBeat.initializeUs(HEART_BEAT, heartbeat_cb).start();

	#define CASE(x) case x: \
		LOG_I( #x); \
		break;

	/*
	 enum rst_reason {
REANSON_DEFAULT_RST = 0, // normal startup by power on
REANSON_WDT_RST = 1, // hardware watch dog reset
REANSON_EXCEPTION_RST = 2,// exception reset, GPIO status won’t change
REANSON_SOFT_WDT_RST = 3,// software watch dog reset, GPIO status won’t change
REANSON_SOFT_RESTART = 4,// software restart ,system_restart , GPIO status won’t change
REANSON_DEEP_SLEEP_AWAKE = 5, // wake up from deep-sleep
};

	 */


	LOG_I( "Reset: ");
	rst_info* rstInfo = system_get_rst_info();
	if(rstInfo)
	{
		switch(rstInfo->reason)
		{
			CASE(REASON_DEFAULT_RST)
			CASE(REASON_WDT_RST) 		// hardware watch dog reset
			CASE(REASON_EXCEPTION_RST)	// exception reset, GPIO status won’t change
			CASE(REASON_SOFT_WDT_RST)	// software watch dog reset, GPIO status won’t change
			CASE(REASON_SOFT_RESTART)	// software restart ,system_restart , GPIO status won’t change
			CASE(REASON_DEEP_SLEEP_AWAKE)
			default:
				LOG_I( "UNKNOWN (%d)", rstInfo->reason);
				break;
		}
	}

	LOG_I( "\nChip id=%ld\r\n", system_get_chip_id());
	LOG_I( "Flash id=%ld\r\n", spi_flash_get_id());

	LOG_I( "Mem info:\r\n");
	system_print_meminfo();

#endif
	tmrMainLoop.initializeUs(LOOP_TIME, mainLoop).start();

	/*Test timings 80Mhz -> tick=12.5ns -> 1us ~ 80 ticks*/
	/*
	unsigned tick1;
	unsigned tick2;
	unsigned tickdiff, tickdiff2;

	tick1 = get_ccount();
	tick2 = get_ccount();
	tickdiff = tick2 - tick1;
	LOG_I( "Tick diff %lu\r\n", tickdiff);

	tick1 = get_ccount();
	os_delay_us(1);
	tick2 = get_ccount();
	tickdiff2 = tick2 - tick1;
	LOG_I( "Tick diff 1us %lu corrected %lu\r\n", tickdiff2, tickdiff2 - tickdiff);

	tick1 = system_get_time();
	os_delay_us(10);
	tick2 = system_get_time();
	tickdiff = tick2 - tick1;
	LOG_I( "Tick diff 10us %lu\r\n", tickdiff);
	*/

	/*
	LOG_I("Time,H,T,readTime(us),H_idx_C,DP_Acc,DP_Acc(us),DP_AccFast," \
			"DP_AccFast(us),DP_Fast,DP_Fast(us),DP_Fastest,DP_Fastest(us)," \
			"ComfortRatio,ComfortText\n");
	*/
}

static void mainLoop()
{
	static uint32_t loopCount = 0;
	uint32_t tick1, tick2;
	byte pkg[64] = {0};
	byte len = 0;
	uint16_t i;

	/*
	if(loopCount % 2)
		devRGB_setColor(COLOR_BLUE);
	else
		devRGB_setColor(COLOR_OFF);
*/
	wsPruneConnections();

	if (system_get_free_heap_size() < 6500 &&
		gHttpServer.getActiveWebSockets().count() == 0)
	{
		debugf("LOW HEAP: %d\r\n", system_get_free_heap_size());
		//system_restart();
	}

	if(Radio && getRadio(5))
	{
		if(Radio->isPacketReceived())
		{
			Radio->getPacketReceived(&len, pkg);

			LOG_I("ASYNC RX (%d):", len);

			for (i = 0; i < len; ++i)
			{
				m_printf( "%x ", pkg[i]);
			}

			uint8_t senderID = pkg[1];

			for(i=0; i < g_activeDevices.count(); ++i)
			{
				if(senderID == g_activeDevices[i]->m_ID)
				{
					g_activeDevices[i]->radioPktReceivedFromDevice((char*)pkg, len);
				}
			}


/*
			if(len == 5)
			{
				if(pkg[1] == 3)
				{
					WebSocketsList &clients = server.getActiveWebSockets();
					if(pkg[3] == 0x01)
					{
						LOG_I("MOVEMENT ON");
						for (int i = 0; i < clients.count(); i++)
							clients[i].sendString("MOVEMENT ON");
					}
					else if(pkg[3] == 0x02)
					{
						LOG_I("MOVEMENT OFF");
						for (int i = 0; i < clients.count(); i++)
							clients[i].sendString("MOVEMENT OFF");
					}
				}
			}
*/

			//devRGB_setColor(COLOR_GREEN);
		}
		Radio->startListening();
		releaseRadio();
	}
	else LOG_I("MainLoop: no radio");
	WDT.alive();
	++loopCount;
}

static void initNetwork()
{
	LOG_I("initNetwork() OLD: TCP_WND=%d TCP_MAXRTX=%d TCP_SYNMAXRTX=%d",
		 TCP_WND, TCP_MAXRTX, TCP_SYNMAXRTX);

	  TCP_WND = (4 * TCP_MSS);
	  TCP_MAXRTX = 5;
	  TCP_SYNMAXRTX = 3;

	LOG_I("initNetwork() NEW: TCP_WND=%d TCP_MAXRTX=%d TCP_SYNMAXRTX=%d",
			 TCP_WND, TCP_MAXRTX, TCP_SYNMAXRTX);
}



extern void init()
{
	initNetwork();

	initDevices();

	WDT.enable(false);

	startSystem();

	// set timezone hourly difference to UTC
	//TODO: send from mobile phone & store on disk
	SystemClock.setTimeZone(2);

	LOG_I("\nhlog_param:{\"heap_start\":0x%x, \"heap_end\":0x3fffc000}", ((uint32_t)&_heap_start));

	gHeapOpFlushAfter = 32;

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk, 30, connectFail);
}
