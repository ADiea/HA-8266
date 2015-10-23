#include "webServer.h"
#include "device.h"


NtpClient *gNTPClient;
HttpServer gHttpServer;

	uint32_t totalActiveSockets=0;
	void onRequest(HttpRequest &request, HttpResponse &response)
	{
		response.sendString("Bad Request");
	}


	void wsConnected(WebSocket& socket)
	{
		totalActiveSockets++;

		// Notify everybody about new connection
		WebSocketsList &clients = gHttpServer.getActiveWebSockets();
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
		LOG_I("WS bindata rxd, sz=%d", size);
	}

	void wsDisconnected(WebSocket& socket)
	{
		totalActiveSockets--;

		// Notify everybody about lost connection
		WebSocketsList &clients = gHttpServer.getActiveWebSockets();
		for (int i = 0; i < clients.count(); i++)
			clients[i].sendString("WS client disconnected Total: " + String(totalActiveSockets));
	}

	void startWebServers()
	{
		////Serial.print(3);
		gHttpServer.listen(80);
		gHttpServer.addPath("/", onRequest);
		gHttpServer.setDefaultHandler(onRequest);
	
		// Web Sockets configuration
		gHttpServer.enableWebSockets(true);
		gHttpServer.setWebSocketConnectionHandler(wsConnected);
		gHttpServer.setWebSocketMessageHandler(wsMessageReceived);
		gHttpServer.setWebSocketBinaryHandler(wsBinaryReceived);
		gHttpServer.setWebSocketDisconnectionHandler(wsDisconnected);
	
		LOG_I("HTTP server STARTED %s", WifiStation.getIP().toString().c_str());

		gNTPClient = new NtpClient("pool.ntp.org", 10*60); //every 10 minutes
		if(gNTPClient)
		{
			gNTPClient->setAutoQueryInterval(10*60);
			gNTPClient->setAutoQuery(true);
			gNTPClient->setAutoUpdateSystemClock(true);
			gNTPClient->requestTime(); // Request to update time now.

			LOG_I("NTP server STARTED");
		}
	}
