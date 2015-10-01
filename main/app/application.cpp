#include <user_config.h>
#include <SmingCore/SmingCore.h>

//TODO: CHANGE static const char secret[] PROGMEM = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
//TODO: make PR to remove spiffs_mount(); from appinit/user_main.cpp

#include "debug.h"
#include "device.h"
#include "commWeb.h"



/*
 The following 2 defines are present in wifipass.h
 #define WIFI_SSID "PleaseEnterSSID"
 #define WIFI_PWD "PleaseEnterPass"
*/
#include "wifipass.h"

#define ONE_SECOND 1000000

//Globals
TempAndHumidity gLastTempHumid;
NtpClient *gNTPClient;
HttpServer server;

static inline unsigned get_ccount(void)
{
	unsigned r;
	asm volatile ("rsr %0, ccount" : "=r"(r));
	return r;
}

static void mainLoop(void);

Timer tmrMainLoop;
#define LOOP_TIME (3 * ONE_SECOND)

#if DEBUG_BUILD
	#define HEART_BEAT (60 * ONE_SECOND)
	Timer tmrHeartBeat;

	static void heartbeat_cb(void)
	{
		LOG_I( "\n%s Heap: %ld\r\n",
				SystemClock.getSystemTimeString().c_str(),
				system_get_free_heap_size());
	}
#endif /*DEBUG_BUILD*/

	uint32_t totalActiveSockets=0;
	void onIndex(HttpRequest &request, HttpResponse &response)
	{
		response.sendString("Bad Request");
	}

	void onFile(HttpRequest &request, HttpResponse &response)
	{
		response.sendString("Bad Request");
	}

	void wsConnected(WebSocket& socket)
	{
		totalActiveSockets++;

		// Notify everybody about new connection
		WebSocketsList &clients = server.getActiveWebSockets();
		for (int i = 0; i < clients.count(); i++)
			clients[i].sendString("New ws conn! Total: " + String(totalActiveSockets));
	}

	bool parseWsPacket(const String& message)
	{
		const char* msg =  message.c_str();

		static uint8_t sequence = 0;

		byte pkg[64] = {0};

		bool retVal = false;



		int intensity = 0;
		int ontime_min = 0;
		int ontime_sec = 0;
		int dimspeed = 0;
		int minValue = 0;
		int manual = 0;

		

		LOG_I( "WS intensity:%u min:%u s:%u speed:%u manual:%u min:%u\n",
				intensity, ontime_min, ontime_sec, dimspeed, manual, minValue);

		

		#define DIMMER_ID 0x01
		#define MY_ID 0xFF

		/*PKG intensity
		[address 1B]
		[pgk_type==PKG_TYPE_INTENSITY 1B]
		[intensity 1B]
		[on duration(s)= min 4b + sec*4 4b 1B]
		[flags 4b fadeSpeed 4b]
		[minValue 1B]
		[sequence 1B]
		[checksum 1B]
		*/
		#define PKG_INTENSITY_LEN 0x08

		#define PKG_TYPE_INVALID 0x00
		#define PKG_TYPE_ACK 0x01
		#define PKG_TYPE_INTENSITY 0x02

		#define PKG_MANUAL_FLAG 0x80

		pkg[0] = DIMMER_ID;
		pkg[1] = PKG_TYPE_INTENSITY;
		pkg[2] = intensity;
		pkg[3] = ((ontime_min << 4) & 0xF0) | (0xF & (ontime_sec >> 2));
		pkg[4] = dimspeed & 0xF;
		if(manual)
			pkg[4] |= PKG_MANUAL_FLAG;
		pkg[5] = minValue;
		pkg[6] = ++sequence;

		pkg[7] = (pkg[0] + pkg[1] + pkg[2] + pkg[3] + pkg[4] + pkg[5]+ pkg[6]) & 0xFF;

		retVal = RadioSend(pkg, PKG_INTENSITY_LEN);
				
		return retVal;
	}


	void wsMessageReceived(WebSocket& socket, const String& message)
	{
		LOG_I( "WS message received:%s\n", message.c_str());

		cwReceivePacket(socket, message.c_str());
	}

	void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
	{
		//Serial.printf("Websocket binary data recieved, size: %d\r\n", size);
	}

	void wsDisconnected(WebSocket& socket)
	{
		totalActiveSockets--;

		// Notify everybody about lost connection
		WebSocketsList &clients = server.getActiveWebSockets();
		for (int i = 0; i < clients.count(); i++)
			clients[i].sendString("WS client disconnected Total: " + String(totalActiveSockets));
	}

void startWebServer()
{
	////Serial.print(3);
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onFile);

	// Web Sockets configuration
	server.enableWebSockets(true);
	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

	debugf("\r\n=== WEB SERVER STARTED ===\n %s ",
			WifiStation.getIP().toString().c_str());

}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	LOG_I( "AP CONNECT\n");

	startWebServer();

	gNTPClient = new NtpClient("pool.ntp.org", 10*60); //every 10 minutes
	if(gNTPClient)
	{
		gNTPClient->setAutoQueryInterval(10*60);
		gNTPClient->setAutoQuery(true);
		gNTPClient->setAutoUpdateSystemClock(true);
		gNTPClient->requestTime(); // Request to update time now.
	}
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	debugf("FAIL CONNECT");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void initSystem()
{
	initDevices();
}



void startSystem()
{

#if DEBUG_BUILD
	tmrHeartBeat.initializeUs(HEART_BEAT, heartbeat_cb).start();

	#define CASE(x) case x: \
		LOG_I( #x); \
		break;

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

	LOG_I("Time,H,T,readTime(us),H_idx_C,DP_Acc,DP_Acc(us),DP_AccFast," \
			"DP_AccFast(us),DP_Fast,DP_Fast(us),DP_Fastest,DP_Fastest(us)," \
			"ComfortRatio,ComfortText\n");

}

static void mainLoop()
{
	devRGB_setColor(COLOR_RED);
	uint32_t tick1, tick2;

	byte pkg[64] = {0};
	byte len = 0;

	uint16_t i;

	//
	//LOG_I( ",");

	if(Radio && !isRadioBusy())
	{
		if(Radio->isPacketReceived())
		{
			Radio->getPacketReceived(&len, pkg);

			LOG_I("ASYNC RX (%d):", len);

			for (i = 0; i < len; ++i)
			{
				LOG_I( "%x ", pkg[i]);
			}

			uint8_t senderID = pkg[0];

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
			devRGB_setColor(COLOR_GREEN);
		}
		else
		{
			devRGB_setColor(COLOR_RED);
		}
	}


	WDT.alive();
}


void init()
{
	initSystem();

	LOG_E("System start.\n");

	WDT.enable(false);

	startSystem();

	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(3);

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk, 30, connectFail);
}
