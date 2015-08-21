#include <user_config.h>
#include <SmingCore/SmingCore.h>

//TODO: CHANGE static const char secret[] PROGMEM = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
//TODO: make PR to remove spiffs_mount(); from appinit/user_main.cpp

#include "debug.h"
#include "device.h"

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

uint8_t gRadioBusy = 0;

static inline unsigned get_ccount(void)
{
	unsigned r;
	asm volatile ("rsr %0, ccount" : "=r"(r));
	return r;
}

static void mainLoop(void);

	Timer tmrMainLoop;
#define LOOP_TIME (3*ONE_SECOND)

#if DEBUG_BUILD
	#define HEART_BEAT (5*ONE_SECOND)
	Timer tmrHeartBeat;

	static void heartbeat_cb(void)
	{
		//Serial.print("Local Time    : ");
		//Serial.print(SystemClock.getSystemTimeString());
		//Serial.print(" UTC Time: ");
		//Serial.print(SystemClock.getSystemTimeString(eTZ_UTC));

		LOG(INFO, "\nHeap: %ld\r\n", system_get_free_heap_size());

		//LOG(INFO, "\r\n");
	}
#endif /*DEBUG_BUILD*/


	uint32_t totalActiveSockets=0;
	void onIndex(HttpRequest &request, HttpResponse &response)
	{
		response.sendString("Welcome");
		//response.forbidden();
	}

	void onFile(HttpRequest &request, HttpResponse &response)
	{
		response.sendString("Welcome");
		//response.notFound();
	}

	void wsConnected(WebSocket& socket)
	{
		totalActiveSockets++;

		// Notify everybody about new connection
		WebSocketsList &clients = server.getActiveWebSockets();
		for (int i = 0; i < clients.count(); i++)
			clients[i].sendString("New ws conn! Total: " + String(totalActiveSockets));
	}




	void wsMessageReceived(WebSocket& socket, const String& message)
	{
		char* msg =  (char*)message.c_str();

		LOG(INFO, "WS message received:%s\n", msg);
#if 0
		int intensity = 0;

		byte payLoad[64] = {0};
		byte len = 0;
		bool result;

		byte pkgIntensity[] = "I:#";

		if(strlen(msg) > 5 )
		{
			msg += 5;
			while(*msg >= '0' && *msg <='9')
			{
				intensity = intensity *10 + (*msg++) - '0';
			}

			LOG(INFO, "WS intensity:%u\n", intensity);

			/* radio send */
			if(radio)
			{
				if(gRadioBusy)
				{
					LOG(INFO, "Radio busy\n");
				}
				else
				{
					gRadioBusy = 1;

					pkgIntensity[2] = (byte)intensity;

					result = radio->sendPacket(3,
							pkgIntensity,
							true,
							RADIO_WAIT_ACK_MS,
							&len,
							payLoad);

					gRadioBusy = 0;

					if(!result || len > 64)
					{
						LOG(INFO," ERR!");
					}
					else
					{
						LOG(INFO," SENT! SYNC RX (%d):", len);

						for (byte i = 0; i < len; ++i)
						{
							LOG(INFO,"%c", ((char) payLoad[i]));
						}

						LOG(INFO,"\n");
					}

				}
			}
			else
			{
				LOG(INFO, "Radio not inited\n");
			}

		}
#endif
		String response = "Echo: " + message;
		socket.sendString(response);
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
	LOG(INFO, "AP CONNECT\n");

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
	enableDev(DEV_UART, ENABLE | CONFIG);

	//setup SDCard and load custom system settings, then disable SDCard
	enableDev(DEV_SDCARD, ENABLE | CONFIG);
	//enableDev(DEV_SDCARD, DISABLE);

	//DHT22 periodically enabled to read data
	enableDev(DEV_DHT22, ENABLE | CONFIG);

	enableDev(DEV_MQ135, ENABLE | CONFIG);

	//RGB periodically enabled to send data
	enableDev(DEV_RGB, DISABLE);

	//enable and config Radio, then disable
	enableDev(DEV_RADIO, ENABLE | CONFIG);
	//enableDev(DEV_RADIO, DISABLE);
/*
	//start listening for incoming packets
	radio->startListening();
*/

	//setup Wifi
	enableDev(DEV_WIFI, ENABLE | CONFIG);
}



void startSystem()
{

#if DEBUG_BUILD
	//tmrHeartBeat.initializeUs(HEART_BEAT, heartbeat_cb).start();

#define CASE(x) case x: \
					LOG(INFO, #x); \
					break;

	LOG(INFO, "Reset: ");
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
				LOG(INFO, "UNKNOWN (%d)", rstInfo->reason);
				break;
		}
	}

	LOG(INFO, "\nChip id=%ld\r\n", system_get_chip_id());
	LOG(INFO, "Flash id=%ld\r\n", spi_flash_get_id());

	LOG(INFO, "Mem info:\r\n");
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
	LOG(INFO, "Tick diff %lu\r\n", tickdiff);

	tick1 = get_ccount();
	os_delay_us(1);
	tick2 = get_ccount();
	tickdiff2 = tick2 - tick1;
	LOG(INFO, "Tick diff 1us %lu corrected %lu\r\n", tickdiff2, tickdiff2 - tickdiff);

	tick1 = system_get_time();
	os_delay_us(10);
	tick2 = system_get_time();
	tickdiff = tick2 - tick1;
	LOG(INFO, "Tick diff 10us %lu\r\n", tickdiff);
	*/

	LOG(INFO,"Time,H,T,readTime(us),H_idx_C,DP_Acc,DP_Acc(us),DP_AccFast," \
			"DP_AccFast(us),DP_Fast,DP_Fast(us),DP_Fastest,DP_Fastest(us)," \
			"ComfortRatio,ComfortText\n");

}

static void mainLoop()
{
	devRGB_setColor(COLOR_RED);
	uint32_t tick1, tick2;
/*
	LOG(INFO, SystemClock.getSystemTimeString().c_str());
	LOG(INFO, ",");

	tick1 = system_get_time();
	uint8_t errTemp = devDHT22_read(gLastTempHumid);
	tick2 = system_get_time();
	if(DEV_ERR_OK != errTemp)
	{
		LOG(ERR, "DHT22 read FAIL:%d\n", errTemp);
	}
	else
	{
		//LOG(INFO, "%f H:%f T:%f\n", 3.14f, gLastTempHumid.humid, gLastTempHumid.temp);
		//Serial.print(gLastTempHumid.humid);
		LOG(INFO, ",");

		//Serial.print(gLastTempHumid.temp);
		LOG(INFO, ",%lu", tick2 - tick1);

		devDHT22_heatIndex();
		devDHT22_dewPoint();
		devDHT22_comfortRatio();
		LOG(INFO, "\n");
	}
*/
	devRGB_setColor(COLOR_GREEN);
	WDT.alive();
}


void init()
{
	initSystem();

	LOG(INFO, "System start.\n");

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
