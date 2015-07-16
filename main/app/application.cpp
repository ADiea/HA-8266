#include <user_config.h>
#include <SmingCore/SmingCore.h>

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
//FTPServer ftp;

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
		return;
		//Serial.print("Local Time    : ");
		//Serial.print(SystemClock.getSystemTimeString());
		//Serial.print(" UTC Time: ");
		//Serial.print(SystemClock.getSystemTimeString(eTZ_UTC));

		LOG(INFO, "\nFree heap size=%ld\r\n", system_get_free_heap_size());

		//LOG(INFO, "\r\n");
	}
#endif /*DEBUG_BUILD*/


uint32_t totalActiveSockets=0;
	void onIndex(HttpRequest &request, HttpResponse &response)
	{
		TemplateFileStream *tmpl = new TemplateFileStream("index.html");
		auto &vars = tmpl->variables();
		response.sendTemplate(tmpl); // this template object will be deleted automatically

	}

	void onFile(HttpRequest &request, HttpResponse &response)
	{
		String file = request.getPath();
		if (file[0] == '/')
			file = file.substring(1);

		if (file[0] == '.')
			response.forbidden();
		else
		{
			response.setCache(86400, true); // It's important to use cache for better performance.
			response.sendFile(file);
		}
	}

	void wsConnected(WebSocket& socket)
	{
		totalActiveSockets++;

		// Notify everybody about new connection
		WebSocketsList &clients = server.getActiveWebSockets();
		for (int i = 0; i < clients.count(); i++)
			clients[i].sendString("New friend arrived! Total: " + String(totalActiveSockets));
	}

	void wsMessageReceived(WebSocket& socket, const String& message)
	{
		//Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
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
			clients[i].sendString("We lost our friend :( Total: " + String(totalActiveSockets));
	}

void a(int x)
{
	for(int i=0;i<255;i++)
	{
		if(x%2)x++;
		else
			x--;
	}

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

	//Serial.println("\r\n=== WEB SERVER STARTED ===");
	//Serial.println(WifiStation.getIP());
	//Serial.println("==============================\r\n");
}

/*void startFTP()
{
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}*/

// Will be called when WiFi station was connected to AP
void connectOk()
{
	//Serial.println("I'm CONNECTED\n");
	//startFTP();
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
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void initSystem()
{
	enableDev(DEV_UART, ENABLE | CONFIG);

	//setup SDCard and load custom system settings, then disable SDCard
	enableDev(DEV_SDCARD, ENABLE | CONFIG);
	enableDev(DEV_SDCARD, DISABLE);

	//DHT22 periodically enabled to read data
	enableDev(DEV_DHT22, ENABLE | CONFIG);

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
	tmrHeartBeat.initializeUs(HEART_BEAT, heartbeat_cb).start();
	LOG(INFO, "Chip id=%ld\r\n", system_get_chip_id());
	LOG(INFO, "Mem info:\r\n");
	system_print_meminfo();

	LOG(INFO, "pi=%f\n", 3.14f);
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

	devSDCard_benchmark();
	*/

	LOG(INFO,"Time,H,T,readTime(us),H_idx_C,DP_Acc,DP_Acc(us),DP_AccFast," \
			"DP_AccFast(us),DP_Fast,DP_Fast(us),DP_Fastest,DP_Fastest(us)," \
			"ComfortRatio,ComfortText\n");

	if (!fileExist("index.html"))
	{
		LOG(INFO,"NO index.html\n");

		fileSetContent("index.html",
				"<!DOCTYPE html>\n"
				"\n"
				"<meta charset=\"utf-8\" />\n"
				"\n"
				"<title>WebSocket Test</title>\n"
				"\n"
				"<script language=\"javascript\" type=\"text/javascript\">\n"
				"\n"
				"  var output;\n"
				"\n"
				"  function init()\n"
				"  {\n"
				"    output = document.getElementById(\"output\");\n"
				"    testWebSocket();\n"
				"  }\n"
				"\n"
				"  function testWebSocket()\n"
				"  {\n"
				"\tvar wsUri = \"ws://\" + location.host + \"/\";\n"
				"    websocket = new WebSocket(wsUri);\n"
				"    websocket.onopen = function(evt) { onOpen(evt) };\n"
				"    websocket.onclose = function(evt) { onClose(evt) };\n"
				"    websocket.onmessage = function(evt) { onMessage(evt) };\n"
				"    websocket.onerror = function(evt) { onError(evt) };\n"
				"  }\n"
				"\n"
				"  function onOpen(evt)\n"
				"  {\n"
				"    writeToScreen(\"CONNECTED\");\n"
				"    doSend(\"Sming love WebSockets\");\n"
				"  }\n"
				"\n"
				"  function onClose(evt)\n"
				"  {\n"
				"    writeToScreen(\"DISCONNECTED\");\n"
				"  }\n"
				"\n"
				"  function onMessage(evt)\n"
				"  {\n"
				"    writeToScreen('<span style=\"color: blue;\">RESPONSE: ' + evt.data+'</span>');\n"
				"    //websocket.close();\n"
				"  }\n"
				"\n"
				"  function onError(evt)\n"
				"  {\n"
				"    writeToScreen('<span style=\"color: red;\">ERROR:</span> ' + evt.data);\n"
				"  }\n"
				"\n"
				"  function doSend(message)\n"
				"  {\n"
				"    writeToScreen(\"SENT: \" + message); \n"
				"    websocket.send(message);\n"
				"  }\n"
				"\n"
				"  function writeToScreen(message)\n"
				"  {\n"
				"    var pre = document.createElement(\"p\");\n"
				"    pre.style.wordWrap = \"break-word\";\n"
				"    pre.innerHTML = message;\n"
				"    output.appendChild(pre);\n"
				"  }\n"
				"  \n"
				"  function doDisconnect()\n"
				"  {\n"
				"\tvar disconnect = document.getElementById(\"disconnect\");\n"
				"\tdisconnect.disabled = true;\n"
				"\twebsocket.close();\n"
				"  }\n"
				"\n"
				"  window.addEventListener(\"load\", init, false);\n"
				"\n"
				"</script>\n"
				"\n"
				"<h2>WebSocket Test <input type=\"button\" id=\"disconnect\" onclick=\"doDisconnect()\" value=\"X\" title=\"Disconnect WS\"/></h2>\n"
				"\n"
				"<div id=\"output\"></div>\n");
	}
	else LOG(INFO,"SUCCESS index.html\n");

}

static void mainLoop()
{
	devRGB_setColor(COLOR_RED);
	uint32_t tick1, tick2;

	LOG(INFO, SystemClock.getSystemTimeString().c_str());
	LOG(INFO, ",");

	tick1 = system_get_time();
	uchar errTemp = devDHT22_read(gLastTempHumid);
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
