#include "appMain.h"

/*
 The following 2 defines are present in wifipass.h
 #define WIFI_SSID "PleaseEnterSSID"
 #define WIFI_PWD "PleaseEnterPass"
*/
#include "wifipass.h"

static void mainLoop(void);

extern void (*outOfMemoryCb)(size_t);

Timer tmrMainLoop;
#define LOOP_TIME (1 * ONE_SECOND)

// Will be called when WiFi station was connected to AP
void connectOk()
{
	LOG_I( "AP connect OK");
	startWebServers();
	wsCliStart();
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	LOG_E("AP connect FAIL");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void restartSystem()
{
	saveSystemSettings();

	LOG_I("Reset now");
	((void (*)(void))0x40000080)();
	system_restart();
}

void startSystem()
{
	uint8_t retryCnt = 3;

	do{
		if(loadSystemSettings())
			break;
	}while(--retryCnt);

	if(!retryCnt)
	{
		LOG_E("Loading default settings");
		resetSystemSettings(resetSystemSettings());
		saveSystemSettings();
	}

	// set timezone hourly difference to UTC
	//TODO: send from mobile phone
	SystemClock.setTimeZone(gSysCfg.timeZone);
	SystemClock.setLastKnownTime(gSysCfg.lastKnownTimeStamp);

	//start main loop
	tmrMainLoop.initializeUs(LOOP_TIME, mainLoop).start();

#if DEBUG_BUILD
	debugStart();
#endif /*DEBUG_BUILD*/
}

static void mainLoop()
{
	static uint32_t loopCount = 0;
	uint32_t tick1, tick2;
	byte pkg[64] = {0};
	byte len = 0;
	uint16_t i;

	gSysCfg.lastKnownTimeStamp = RTC.getRtcSeconds();

	/*
	if(loopCount % 2)
		devRGB_setColor(COLOR_BLUE);
	else
		devRGB_setColor(COLOR_OFF);
	 */
	wsPruneConnections();

	if (system_get_free_heap_size() < 6500)
	{
		LOG_E("LOW HEAP: %d\r\n", system_get_free_heap_size());
		if( gHttpServer.getActiveWebSockets().count() == 0)
		{
			//disconnect all, deinit and restart
			//restartSystem();
		}
	}

	if(Radio && getRadio(5))
	{
		if(Radio->isPacketReceived())
		{
			Radio->getPacketReceived(&len, pkg);
			if(len > 0)
			{
				LOG_II("ASYNC RX (%d):", len);

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
			}

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
	LOG_I("OLD: TCP_WND=%d TCP_MAXRTX=%d TCP_SYNMAXRTX=%d",
		 TCP_WND, TCP_MAXRTX, TCP_SYNMAXRTX);

	  TCP_WND = (4 * TCP_MSS);
	  TCP_MAXRTX = 5;
	  TCP_SYNMAXRTX = 3;

	LOG_I("NEW: TCP_WND=%d TCP_MAXRTX=%d TCP_SYNMAXRTX=%d",
			 TCP_WND, TCP_MAXRTX, TCP_SYNMAXRTX);
}

void systemOutOfHeap(size_t requested)
{
	m_printf("NO HEAP req %d have %d", requested, system_get_free_heap_size());
	restartSystem();
}



void scanAPFinished(bool succeeded, BssList list)
{
	if (!succeeded)
	{
		LOG_E("Failed to scan networks");
		startAPScan();
	}

	for (int i = 0; i < list.count(); i++)
	{
		LOG_I("\tWiFi: %s, %s, %s", list[i].ssid, list[i].getAuthorizationMethodName(),
				list[i].hidden ? "(bcast)":"(hidden)");
	}

	//Login.announceScanCompleted(list);
	setupAPMode();
}

void setupAPMode()
{
	LOG_E("WiFi AP not set, switch on SoftAP");
	WifiStation.enable(false);
	WifiAccessPoint.enable(true);
	
	WifiAccessPoint.config("Casa_1254", "", AUTH_OPEN);

	startWebServers();
}

void startAPScan()
{
	WifiStation.startScan(scanAPFinished);
}


extern void init()
{
	outOfMemoryCb = systemOutOfHeap;
	WDT.enable(false);

	initNetwork();

	initDevices();

	startSystem();
	
	wifi_station_set_auto_connect(false); //the ESP8266 station will not try to connect to the router automatically when power on until wifi_station_connect is called.

	if(gSysCfg.wifiStationIsConfigured)
	{
		WifiAccessPoint.enable(false);
		WifiStation.enable(true);
		WifiStation.config(gSysCfg.wifiSSID, gSysCfg.wifiPwd);
		
		wifi_station_set_reconnect_policy(true); 

		// Run our method when station was connected to AP
		WifiStation.waitConnection(connectOk, 30, connectFail);
	}
	else
	{
		// Set system ready callback method
		System.onReady(startAPScan);
		
		WifiAccessPoint.enable(false);
		WifiStation.enable(true);
		
		wifi_station_set_reconnect_policy(false); 

		WifiStation.disconnect();
		
		//WifiAccessPoint.enable(true);
		WifiAccessPoint.setIP(IPAddress(192, 168, 1, 1));
	}
}
