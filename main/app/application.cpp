#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "types.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "drv/drvUART.h"
#include "drv/drvRGB.h"
#include "drv/drvRadio.h"
#include "drv/drvSDCard.h"
#include "drv/drvDHT22.h"
#include "drv/drvMQ135.h"
#include "drv/drvWiFi.h"
#include "drv/drvDS18B20.h"

#ifdef __cplusplus
}
#endif

#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"

unsigned char gDevicesState = 0x0;

#if DEBUG_BUILD

#define HEART_BEAT 5000 /* milliseconds */
LOCAL os_timer_t gInfo_timer;

LOCAL void ICACHE_FLASH_ATTR heartbeat_cb(void *arg)
{
	//wdt_feed();
	LOG(INFO, "System Info\r\n");
	LOG(INFO, "Time=%ld\r\n", system_get_time());
	LOG(INFO, "Chip id=%ld\r\n", system_get_chip_id());
	LOG(INFO, "Free heap size=%ld\r\n", system_get_free_heap_size());
	LOG(INFO, "Mem info:\r\n");
	system_print_meminfo();
	LOG(INFO, "\r\n");
}

#endif /*DEBUG_BUILD*/

HttpServer server;
FTPServer ftp;

int inputs[] = {0, 2}; // Set input GPIO pins here
Vector<String> namesInput;
const int countInputs = sizeof(inputs) /  sizeof(inputs[0]);


void startTimer(os_timer_t* pTimer, os_timer_func_t *pCallback, uint32_t ms, bool bRepeat)
{
	// os_timer_disarm(ETSTimer *ptimer)
	os_timer_disarm(pTimer);
	// os_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg)
	os_timer_setfn(pTimer, pCallback, (void *)0);
	// void os_timer_arm(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag)
	os_timer_arm(pTimer, ms, bRepeat);
}

inline uchar isDevEnabled(uchar dev)
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

		if( dev == DEV_RADIO && (DEV_ERR_OK != devRadio_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
					break;
		}
		else if( dev == DEV_SDCARD && (DEV_ERR_OK != devSDCard_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_RGB && (DEV_ERR_OK != devRGB_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_MQ135 && (DEV_ERR_OK != devMQ135_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_DHT22 && (DEV_ERR_OK != devDHT22_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_WIFI && (DEV_ERR_OK != devWiFi_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_DSTEMP && (DEV_ERR_OK != devDSTemp_init(op)))
		{
			LOG(ERR, "Dev [%x] init failed %x", dev, op);
			break;
		}
		else if( dev == DEV_UART && (DEV_ERR_OK != devUART_init(op)))
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
	for (int i = 0; i < countInputs; i++)
		gpio[namesInput[i].c_str()] = digitalRead(inputs[i]);

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
	Serial.println("I'm CONNECTED");

	startFTP();
	startWebServer();
	/*
	do
	{
		mainLoop();
	}
	while(1);*/
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
	startTimer(&gInfo_timer, (os_timer_func_t *)heartbeat_cb, HEART_BEAT, true);
#endif

}



void mainLoop()
{


}


void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	initSystem();

	//setRGBColor(&COLOR_RED);
	LOG(INFO, "System start.");

	startSystem();

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	for (int i = 0; i < countInputs; i++)
	{
		namesInput.add(String(inputs[i]));
		pinMode(inputs[i], INPUT);
	}

	//setRGBColor(&COLOR_GREEN);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);
}
