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
TempReading gLastTempHumid;

HttpServer server;
FTPServer ftp;

static inline unsigned get_ccount(void)
{
	unsigned r;
	asm volatile ("rsr %0, ccount" : "=r"(r));
	return r;
}

static void mainLoop(void);

	Timer tmrMainLoop;
#if DEBUG_BUILD
	#define HEART_BEAT (5*ONE_SECOND)
	Timer tmrHeartBeat;

	static void heartbeat_cb(void)
	{
		//wdt_feed();
		Serial.print("Local Time    : ");
		Serial.println(SystemClock.getSystemTimeString());
		Serial.print("UTC Time: ");
		Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));

		LOG(INFO, "Free heap size=%ld\r\n", system_get_free_heap_size());
		LOG(INFO, "Mem info:\r\n");
		system_print_meminfo();
		LOG(INFO, "\r\n");
	}
#endif /*DEBUG_BUILD*/



void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
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

void onAjaxInput(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["status"] = (bool)true;

	JsonObject& gpio = json.createNestedObject("gpio");
	//for (int i = 0; i < countInputs; i++)
	//	gpio[namesInput[i].c_str()] = digitalRead(inputs[i]);

	response.sendJsonObject(stream);
}

void onAjaxFrequency(HttpRequest &request, HttpResponse &response)
{
	int freq = request.getQueryParameter("value").toInt();
	System.setCpuFrequency((CpuFrequency)freq);

	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["status"] = (bool)true;
	json["value"] = (int)System.getCpuFrequency();

	response.sendJsonObject(stream);
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ajax/input", onAjaxInput);
	server.addPath("/ajax/frequency", onAjaxFrequency);
	server.setDefaultHandler(onFile);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}

void startFTP()
{
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED\n");
	startFTP();
	startWebServer();
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
#endif
	tmrMainLoop.initializeUs(HEART_BEAT, mainLoop).start(false);

	unsigned tick1;
	unsigned tick2;
	unsigned tickdiff, tickdiff2;

	/*Test timings 80Mhz -> tick=12.5ns -> 1us ~ 80 ticks*/
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
}

static void mainLoop()
{
	while(1)
	{
		//LOG(INFO, "main-loop\n");
		//wdt_feed();
		//devRGB_setColor(COLOR_RED);
		delayMicroseconds(.5*ONE_SECOND);
		//wdt_feed();
		//devRGB_setColor(COLOR_GREEN);
		delayMicroseconds(0.5*ONE_SECOND);
		//wdt_feed();
		//devRGB_setColor(COLOR_BLUE);
		delayMicroseconds(0.5*ONE_SECOND);
		//wdt_feed();

		uchar errTemp = devDHT22_read(&gLastTempHumid);
		if(DEV_ERR_OK != errTemp)
		{
			LOG(ERR, "DHT22 read FAIL:%d\n", errTemp);
		}
		else
		{
			//LOG(INFO, "%f H:%f T:%f\n", 3.14f, gLastTempHumid.humid, gLastTempHumid.temp);
			LOG(INFO, "H,");
			Serial.print(gLastTempHumid.humid);
			LOG(INFO, "T,");
			Serial.print(gLastTempHumid.temp);
			//LOG(INFO, "*C ");
			//LOG(INFO, " HIdx:");
			/*Serial.print(*/devDHT22_heatIndex(gLastTempHumid.temp, gLastTempHumid.humid)/*)*/;
			devDHT22_dewPoint(gLastTempHumid.temp, gLastTempHumid.humid);
			LOG(INFO, "\n");
		}
	}
}


void init()
{
	initSystem();

	LOG(INFO, "System start.\n");

	WDT.enable(false);

	startSystem();
/*
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);
*/
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);
}
