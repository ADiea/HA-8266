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

Timer tmrHalfSecond;
#define HALF_SECOND (ONE_SECOND >> 1)

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
		LOG_E("LOW HEAP: %d", system_get_free_heap_size());
		if( gHttpServer.getActiveWebSockets().count() == 0)
		{
			//disconnect all, deinit and restart
			//restartSystem();
		}
	}
	CBusAutoRelease bus(devSPI_Radio, 5);

	if(DrvRadio.isEnabled() && bus.getBus())
	{
		if(DrvRadio.get()->isPacketReceived())
		{
			DrvRadio.get()->getPacketReceived(&len, pkg);
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
		DrvRadio.get()->startListening();
	}
	else
		LOG_I("Radio busy(main)");

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
	m_printf("OUT OF HEAP req %d have %d", requested, system_get_free_heap_size());
	restartSystem();
}

void scanAPFinished(bool succeeded, BssList list);

void setupAPMode()
{
	LOG_E("WiFi AP not set, switch on SoftAP");
	//WifiStation.enable(false);
	//WifiAccessPoint.enable(true);

	//startWebServers();
/*
	softap_config config = {0};
	wifi_softap_get_config(&config);
	LOG_I("AP is set to ch%d maxc%d beacon%d", config.channel,
			config.max_connection, config.beacon_interval);

	WifiAccessPoint.config("Casa_1254", "", AUTH_OPEN, false, 8, 300);

	wifi_softap_get_config(&config);
	LOG_I("(2)AP is set to ch%d maxc%d beacon%d", config.channel,
			config.max_connection, config.beacon_interval);
*/
	debugf("AP. ip: %s mac: %s", WifiAccessPoint.getIP().toString().c_str(), WifiAccessPoint.getMAC().c_str());
	tmrHalfSecond.initializeUs(ONE_SECOND, startWebServers).start(false);
}

void startAPScan()
{
	LOG_I("start ap scan");
	WifiStation.startScan(scanAPFinished);
}

void scanAPFinished(bool succeeded, BssList list)
{
	if (!succeeded)
	{
		LOG_E("Failed to scan networks");
		startAPScan();
	}

	LOG_I("WiFi: found %d networks", list.count());
	for (int i = 0; i < list.count(); i++)
	{
		LOG_I("WiFi: %s, %s, %s ch:%d", list[i].ssid.c_str(), list[i].getAuthorizationMethodName(),
				list[i].hidden ? "(bcast)":"(hidden)", list[i].channel);
	}

	//wifi_station_set_reconnect_policy(false);
	//wifi_station_set_auto_connect(false);
	//wifi_station_disconnect();

	//Login.announceScanCompleted(list);
	tmrHalfSecond.initializeUs(ONE_SECOND, setupAPMode).start(false);
}

void probeRcved(int16_t rssi, uint8_t* mac)
{
	//LOG_I("probe %d", rssi);
}

extern void init()
{
	outOfMemoryCb = systemOutOfHeap;
	WDT.enable(false);

	initNetwork();

	initDevices();

	startSystem();
	
	//the ESP8266 station will not try to connect
	//to the router automatically when power on until wifi_station_connect is called.
	wifi_station_set_auto_connect(false);

	if(gSysCfg.wifiStationIsConfigured)
	{
		LOG_I("WiFi is configured, start STA");
		WifiAccessPoint.enable(false);
		WifiStation.enable(true);
		WifiStation.config(gSysCfg.wifiSSID, gSysCfg.wifiPwd);
		
		wifi_station_set_reconnect_policy(true); 

		// Run our method when station was connected to AP
		WifiStation.waitConnection(connectOk, 30, connectFail);
	}
	else
	{
		LOG_I("WiFi is _not_ configured");
		
		/*
		PHY_MODE_11B	= 1,
		PHY_MODE_11G	= 2,
		PHY_MODE_11N    = 3
		*/
		if(wifi_get_phy_mode() != PHY_MODE_11N)
		{
			wifi_set_phy_mode(PHY_MODE_11N);
			LOG_I("Set WiFi PHY mode to N");
		}

		WifiAccessPoint.enable(true);
		WifiAccessPoint.config("Casa_1254", "abcd1234", AUTH_WPA2_PSK, false, 8, 300);

		WifiStation.enable(true);
		wifi_station_set_reconnect_policy(false);
		WifiStation.disconnect();


		WifiAccessPoint.setIP(IPAddress(192, 168, 1, 1),
							  IPAddress(192, 168, 1, 3),
							  IPAddress(192, 168, 1, 6));

		// Set system ready callback method
		System.onReady(startAPScan);

		WifiEvents.onAccessPointProbeReqRecved(probeRcved);
	}
}
