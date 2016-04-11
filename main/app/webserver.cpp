#include "webServer.h"
#include "device.h"

#define WEBSOCK_USERSLOTS 15

#define ALIVE_INIT_STATE 0
#define ALIVE_TEST_STATE 1
#define ALIVE_OK_STATE 2

	struct WebSockUserData
	{
		WebSockUserData():isInvalid(true), aliveState(ALIVE_INIT_STATE)
		{}

		void dataArrived()
		{
			aliveState = ALIVE_OK_STATE;
			lastDataTime = millis();
		}

		bool isAlive(uint32_t timeout)
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

			if(millis() - lastDataTime > timeout*1000)
			{
				LOG_I("WS: close inactive connection %x (%d)", webSock, millis() - lastDataTime);
				return false;
			}

			return true;
		}

		void invalidate()
		{
			isInvalid = true;

			gConnectedPeers.removeElement(connectedPeer);

			delete connectedPeer;
			connectedPeer = NULL;
		}

		WebSocket *webSock;
		CAbstractPeer *connectedPeer;
		bool isInvalid;
		uint32 aliveState;
		unsigned long lastDataTime;
	};

	WebSockUserData g_sockDataPool[WEBSOCK_USERSLOTS];

	NtpClient *gNTPClient;
	HttpServer gHttpServer;

	void onRequest(HttpRequest &request, HttpResponse &response)
	{
		response.sendString("Please use mobile App");
	}

	void wsConnected(WebSocket& socket)
	{
		uint32 i = 0, freeSlot = 0;
		for(;i<WEBSOCK_USERSLOTS;i++)
		{
			if(g_sockDataPool[i].isInvalid)
			{
				CAbstractPeer *pNewPeer = new CLanPeer(-1);
				if(!pNewPeer)
				{
					LOG_E( "WS Conn: No heap\n");
					return;
				}
				
				gConnectedPeers.addElement(pNewPeer);
				
				g_sockDataPool[i].isInvalid = false;
				g_sockDataPool[i].connectedPeer = pNewPeer;
				g_sockDataPool[i].aliveState = ALIVE_INIT_STATE;
				g_sockDataPool[i].webSock = &socket;
				
				freeSlot = 1;
				break;
			}
		}

		if(!freeSlot)
		{
			LOG_I( "WS Cannot accept new conn.\n");
		}
	}

	void wsMessageReceived(WebSocket& socket, const String& message)
	{
		uint32 i = 0;
		bool bFound = false;
		for(;i<WEBSOCK_USERSLOTS;i++)
		{
			if(!g_sockDataPool[i].isInvalid)
			{
				if(g_sockDataPool[i].webSock == &socket)
				{
					g_sockDataPool[i].dataArrived();
					bFound = true;
					break;
				}
			}
		}

		if(bFound && g_sockDataPool[i].connectedPeer)
		{
			g_sockDataPool[i].connectedPeer->onReceiveFromPeer(message.c_str());
		}
	}

	void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
	{
		LOG_I("WS bindata rxd, sz=%d", size);
	}

	void wsDisconnected(WebSocket& socket)
	{
		uint32 i = 0;
		for(;i<WEBSOCK_USERSLOTS;i++)
		{
			if(!g_sockDataPool[i].isInvalid)
			{
				if(g_sockDataPool[i].webSock == &socket)
				{
					g_sockDataPool[i].invalidate();
					break;
				}
			}
		}
	}

	void wsPruneConnections()
	{
		WebSockUserData *pData;
		uint32 i = 0, s = 0;
		WebSocketsList &clients = gHttpServer.getActiveWebSockets();
		for (s = 0; s < clients.count(); s++)
		{
			for(i = 0; i < WEBSOCK_USERSLOTS; i++)
			{
				if(!g_sockDataPool[i].isInvalid)
				{
					//todo: will comparison always work?
					if(g_sockDataPool[i].webSock == &(clients[s]))
					{
						if(!g_sockDataPool[i].isAlive((10*60)/clients.count()))
						{
							clients[s].close();
							g_sockDataPool[i].invalidate();
						}
						break;
					}
				}
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

			LOG_I("NTP client STARTED");
		}
	}
