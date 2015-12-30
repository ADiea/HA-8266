#include "webServer.h"
#include "device.h"

#define WEBSOCK_USERSLOTS 10

#define ALIVE_INIT_STATE 0
#define ALIVE_TEST_STATE 1
#define ALIVE_OK_STATE 2

struct WebSockUserData
{
	WebSockUserData():isInvalid(true), aliveState(ALIVE_INIT_STATE)
	{}

	void dataArrived()
	{
		LOG_I("WS Data arrived!");
		aliveState = ALIVE_OK_STATE;
	}

	bool isAlive()
	{
		if(aliveState == ALIVE_INIT_STATE)
		{
			aliveState = ALIVE_TEST_STATE;
			return true;
		}
		else if(aliveState == ALIVE_TEST_STATE)
		{
			return false;
		}
		return true;
	}

	bool isInvalid;
	uint32 aliveState;
};

WebSockUserData g_sockDataPool[WEBSOCK_USERSLOTS];

NtpClient *gNTPClient;
HttpServer gHttpServer;

	uint32 totalActiveSockets=0;
	void onRequest(HttpRequest &request, HttpResponse &response)
	{
		response.sendString("Bad Request");
	}

	void wsConnected(WebSocket& socket)
	{
		uint32 i = 0, freeSlot = 0;
		for(;i<WEBSOCK_USERSLOTS;i++)
		{
			if(g_sockDataPool[i].isInvalid)
			{
				g_sockDataPool[i].isInvalid = false;
				g_sockDataPool[i].aliveState = ALIVE_INIT_STATE;
				freeSlot = 1;
				break;
			}
		}

		if(freeSlot)
		{
			socket.setUserData((void*)(&g_sockDataPool[i]));

			totalActiveSockets++;

			// Notify everybody about new connection
			WebSocketsList &clients = gHttpServer.getActiveWebSockets();
			for (int i = 0; i < clients.count(); i++)
				clients[i].sendString("New ws conn! Total: " + String(totalActiveSockets));

		}
		else
		{
			LOG_I( "WS Cannot accept new conn.\n");
		}
	}

	void wsMessageReceived(WebSocket& socket, const String& message)
	{
		LOG_I( "WS RX:%s", message.c_str());

		WebSockUserData *pData = (WebSockUserData*) socket.getUserData();
		if(pData)
			pData->dataArrived();

		cwReceivePacket(socket, message.c_str());
	}

	void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
	{
		LOG_I("WS bindata rxd, sz=%d", size);
	}

	void wsDisconnected(WebSocket& socket)
	{
		WebSockUserData *pData = (WebSockUserData*) socket.getUserData();
		if(pData)
			pData->isInvalid = true;

		totalActiveSockets--;

		// Notify everybody about lost connection
		WebSocketsList &clients = gHttpServer.getActiveWebSockets();
		for (int i = 0; i < clients.count(); i++)
			clients[i].sendString("WS client disconnected Total: " + String(totalActiveSockets));
	}

	void wsPruneConnections()
	{
		WebSockUserData *pData;
		WebSocketsList &clients = gHttpServer.getActiveWebSockets();
		for (int i = 0; i < clients.count(); i++)
		{
			pData = (WebSockUserData*) clients[i].getUserData();
			if(pData && !pData->isAlive())
			{
				clients[i].disconnect();
				pData->isInvalid = true;
			}
		}
	}

	void startWebServers()
	{
		////Serial.print(3);
		gHttpServer.listen(80);
		//gHttpServer.setTimeOut(5);
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
