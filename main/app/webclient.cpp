#include "webclient.h"

WebsocketClient wsClient;

String ws_Url =  "ws://echo.websocket.org"; //"ws://192.168.1.2:8080/";
void wsDisconnected(bool success);

wsMode g_wsCliMode;

void wsConnected(wsMode Mode)
{
	g_wsCliMode = Mode;
	if (Mode == ws_Connected)
	{
		LOG_I("Connection with server successful");
		wsCliSendMessage(String("{op:0}"));
		wsCliSendMessage(String("test"));
		wsCliSendMessage(String("231e5r4fe_test"));
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
}

void wsDisconnected(bool success)
{
	if (success == true)
	{
		LOG_I("Websocket Client Disconnected Normally. End of program ..");
	}
	else
	{
		LOG_I("Websocket Client Disconnected. Reconnecting ..");
		//set timer 1s
	}
}


void wsCliSendMessage(String msg)
{
	LOG_I("wsCliSendMessage: %s", msg);
	wsClient.sendMessage(msg);
}

void wsCliStart()
{
	LOG_I("wsCliStart()");
    wsClient.setOnReceiveCallback(wsMessageReceived);
    wsClient.setOnDisconnectedCallback(wsDisconnected);
    wsClient.setOnConnectedCallback(wsConnected);
	wsClient.connect(ws_Url);
}
