#include "webclient.h"

WebsocketClient wsClient;

String ws_Url =  "ws://homea.herokuapp.com";
void wsDisconnected(bool success);

WebWsProtocol_State g_wsCliConnStatus;
Timer gTmrStayConnected;

#define MY_NODE_ID 122

int extractInt(char* msg)
{
	int i = 0;

	bool bSign = false;

	if(msg && *msg == '-')
	{
		bSign = true;
		++(msg);
	}

	//LOG_D( "skipInt: String is %s", *s);

	while (msg && *msg && is_digit(*msg))
		i = i * 10 + *(msg++) - '0';

	return bSign ? -i : i;
}

void terminateString(char* msg)
{
	while (msg && *msg)
	{
		if(*msg == '\"')
		{
			*msg = 0;
			break;
		}
		else ++msg;
	}
}

void wsCliOnTimerStayConnected();

void wsConnected(wsMode Mode)
{
	if (Mode == ws_Connected)
	{
		LOG_I("Connection with server successful");

		m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
							"{op:%d}", wsOP_cliHello);

		wsCliSendMessage(String(g_devScrapBuffer));
	}
	else
	{
		LOG_I("Connection with server not successful");
		wsClient.connect(ws_Url);
	}
}
void wsMessageReceived(String message)
{
    LOG_I("WebSocket message received: %s", message.c_str());

    message.getBytes(g_devScrapBuffer, sizeof(g_devScrapBuffer));

    CAbstractPeer* pRemoteClient = NULL;
    uint32_t index;
    uint32_t peerId;
    String relayedMsg;
    uint32_t op = 0;
    char *payloadMsg;

    //unpack
    index = message.indexOf(String("op:"));
    if(index < 0)
    {
    	return;
    }

    op = extractInt(g_devScrapBuffer + index + 3);
    LOG_I("WebSockCli RX Op is:%d", op);

    switch(op)
    {
    	case wsOP_servHello:
    		m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
    							"{op:%d,type:%d,id:%d}",
    							wsOP_cliLogin, wsValue_homeBase, MY_NODE_ID);

    		wsCliSendMessage(String(g_devScrapBuffer));
    		break;
	//wsOP_cliLogin=2,
    	case wsOP_msgRelay:

    	    index = message.indexOf(String("id:"));
    	    if(index < 0)
    	    {
    	    	return;
    	    }

    	    peerId = extractInt(g_devScrapBuffer + index + 3);
    	    LOG_I("WebSockCli RX peer id:%d", peerId);

    		pRemoteClient = findPeer(peerId);
    		if(!pRemoteClient)
    		{
    			pRemoteClient = new CWebPeer(id);
				if(!pRemoteClient)
				{
					LOG_E( "wsOP_msgRelay: No heap for peer\n");
					//reply nack?
					return;
				}

				gConnectedPeers.addElement(*pRemoteClient);
    		}

    		//extract message
    	    index = message.indexOf(String("msg:\""));
    	    if(index < 0)
    	    {
    	    	return;
    	    }

    	    peerId = terminateString(g_devScrapBuffer + index + 5);
    	    LOG_I("WebSockCli RX peer msg:%s", g_devScrapBuffer + index + 5);

    		//receive message
    		pRemoteClient->onReceiveFromPeer(String(g_devScrapBuffer + index + 5));

    		break;
    	case wsOP_msgSpecial:
    		break;
    	case wsOP_positiveAck:
    		break;
    	case wsOP_negativeAck:
    		break;

    };

}

void wsDisconnected(bool success)
{
	if (success == true)
	{
		LOG_I("WSCli disc by request");
	}
	else
	{
		LOG_I("Websocket Client Disconnected. Reconnecting ..");
		wsClient.connect(ws_Url);
	}
}


bool wsCliSendMessage(String msg)
{
	LOG_I("wsCliSendMessage: %s", msg.c_str());
	if(wsClient.getWSMode() != ws_Disconnected)
	{
		wsClient.sendMessage(msg);
	}
	else
	{
		wsClient.connect(ws_Url);
		return false;
	}
	return true;
}

void wsCliStart()
{
	LOG_I("wsCliStart()");
    wsClient.setOnReceiveCallback(wsMessageReceived);
    wsClient.setOnDisconnectedCallback(wsDisconnected);
    wsClient.setOnConnectedCallback(wsConnected);
    wsClient.connect(ws_Url);
	gTmrStayConnected.initializeUs(1 * 60 * 1000000, wsCliOnTimerStayConnected).start();
}

void wsCliOnTimerStayConnected()
{
	uint32_t i=0;
	bool bConnectionNeeded = false;
	
	if(wsClient.getWSMode() == ws_Disconnected)
	{
		wsClient.connect(ws_Url);
	}
	else
	{
		for(;i<gConnectedPeers.size();i++)
		{
			if(gConnectedPeers[i]->getType() == ePeerWEB)
			{
				if(gConnectedPeers[i]->isConnectionAlive())
				{
					bConnectionNeeded = true;
				}
				else
				{
					delete gConnectedPeers[i];
					gConnectedPeers.removeElementAt((unsigned int)i);
				}
			}
		}

		if (!bConnectionNeeded)
		{
			LOG_I("wsCliOnTimerStayConnected() disconnecting");
			wsClient.disconnect();
			g_wsCliConnStatus = wsState_inval;
		}
		else
		{
			LOG_I("wsCliOnTimerStayConnected() connection needed");
		}
	} //endif(wsClient.getWSMode() != ws_Disconnected)
}
