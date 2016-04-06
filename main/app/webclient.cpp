#include "webclient.h"

WebsocketClient wsClient;

String ws_Url =  "ws://homea.herokuapp.com";
void wsDisconnected(bool success);

WebWsProtocol_State g_wsCliConnStatus;
Timer gTmrStayConnected;

bool g_bConnectionNeeded = true;

void wsCliConnect(bool bResetConnectionTimer);
void wsCliOnTimerStayConnected();

void wsConnected(wsMode Mode)
{
	if (Mode == ws_Connected)
	{
		LOG_I("Connection with server successful");
		wsCliSendMessage(String("{op:0}"));
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

    uint32_t op = 0;
    //unpack

    switch(op)
    {
    	case wsOP_servHello:
    		break;
	//wsOP_cliLogin=2,
    	case wsOP_msgRelay:
    		break;
    	case wsOP_msgSpecial:
    		break;
    	case wsOP_positiveAck:
    		break;
    	case wsOP_negativeAck:
    		break;
    	case wsOP_remotePeerConnect:

    		/*
    		 CAbstractPeer *pNewPeer = new CLanPeer(-1);
				if(!pNewPeer)
				{
					LOG_E( "WS Conn: No heap\n");
					return;
				}

				gConnectedPeers.addElement(*pNewPeer);
    		 */

    		break;
    	case wsOP_remotePeerDisconnect:

    		/*
    		 get remopte peer id
    		 connectedPeer = search peer

    		 gConnectedPeers.removeElement(connectedPeer);

			delete connectedPeer;


    		 */
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
		//set timer 1s
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
		wsCliConnect(false);
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
	wsCliConnect(true);
}

void wsCliConnect(bool bResetConnectionTimer)
{
	if(!bResetConnectionTimer)
	{
		gTmrStayConnected.initializeUs(5 * 60 * 1000000, wsCliOnTimerStayConnected).start();
	}
	
	wsClient.connect(ws_Url);
}

void wsCliOnTimerStayConnected()
{
	if (wsClient.getWSMode() != ws_Disconnected &&
		!g_bConnectionNeeded)
	{
		LOG_I("wsCliOnTimerStayConnected() disconnecting");
		wsClient.disconnect();
		g_wsCliConnStatus = wsState_inval;
	}
	else
	{
		LOG_I("wsCliOnTimerStayConnected() connection needed");
	}

}
