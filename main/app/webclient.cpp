#include "webclient.h"
#include "commWeb.h"
#include "util.h"
#include "device.h"


/* Debug SSL functions */
void displaySessionId(SSL *ssl)
{
    int i;
    const uint8_t *session_id = ssl_get_session_id(ssl);
    int sess_id_size = ssl_get_session_id_size(ssl);

    if (sess_id_size > 0)
    {
    	LOG_I("-----BEGIN SSL SESSION PARAMETERS-----");
        for (i = 0; i < sess_id_size; i++)
        {
        	m_printf("%02x", session_id[i]);
        }

        LOG_I("\n-----END SSL SESSION PARAMETERS-----");
    }
}

/**
 * Display what cipher we are using
 */
void displayCipher(SSL *ssl)
{
	LOG_I("CIPHER is ");
    switch (ssl_get_cipher_id(ssl))
    {
        case SSL_AES128_SHA:
        	LOG_I("AES128-SHA");
            break;

        case SSL_AES256_SHA:
        	LOG_I("AES256-SHA");
            break;

        case SSL_RC4_128_SHA:
        	LOG_I("RC4-SHA");
            break;

        case SSL_RC4_128_MD5:
        	LOG_I("RC4-MD5");
            break;

        default:
        	LOG_I("Unknown - %d", ssl_get_cipher_id(ssl));
            break;
    }

    LOG_I("\n");
}


WebsocketClient wsClient;

String ws_Url =  "wss://homea.herokuapp.com/";
void wsDisconnected(bool success);

WebWsProtocol_State g_wsCliConnStatus;
Timer gTmrStayConnected;
Timer gTmrReconnect;
Timer gTmrKeepAlive;

#define TMR_CHECK_CONN (1 * 60 * 1000000)
#define TMR_RECONN_ERR (5 * 1000000)
#define TMR_RECON_LATER (10 * 1000000)
#define TMR_PING (2 * 1000000)

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

void wsCliReconnect()
{
	wsClient.connect(ws_Url);
}

void wsCliPing()
{
	if(g_wsCliConnStatus == wsState_conn)
	{
		wsClient.sendPing();
	}
}

void wsConnected(wsMode Mode)
{
	if (Mode == ws_Connected)
	{
		LOG_I("wscli: Connection with server successful");

		m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
							"{\"op\":%d}", wsOP_cliHello);

		wsCliSendMessage(String(g_devScrapBuffer));

		g_wsCliConnStatus = wsState_new;
		gTmrStayConnected.initializeUs(TMR_CHECK_CONN, wsCliOnTimerStayConnected).start();
//debug info
		SSL* ssl = wsClient.getSsl();
		if (ssl) {
			const char *common_name = ssl_get_cert_dn(ssl,SSL_X509_CERT_COMMON_NAME);
			if (common_name) {
				LOG_I("Common Name:\t%s\n", common_name);
			}
			displayCipher(ssl);
			//displaySessionId(ssl);
		}
	}
	else
	{
		LOG_I("wscli: Connection with server not successful");
		//wsClient.connect(ws_Url);
		gTmrReconnect.stop();
		gTmrReconnect.initializeUs(TMR_RECONN_ERR, wsCliReconnect).start(false);
	}
}
void wsMessageReceived(String message)
{
    LOG_I("wscli:WebSocket message received: %s", message.c_str());




    message.getBytes((unsigned char*)g_devScrapBuffer, sizeof(g_devScrapBuffer));

    CAbstractPeer* pRemoteClient = NULL;
    uint32_t index;
    uint32_t peerId;
    String relayedMsg;
    uint32_t op = 0;
    char *payloadMsg;

    //unpack
    index = message.indexOf(String("\"op\":"));
    if(index < 0)
    {
    	return;
    }

    op = extractInt(g_devScrapBuffer + index + 5);
    LOG_I("wscli:WebSockCli RX Op is:%d", op);

    switch(op)
    {
    	case wsOP_servHello:
    			LOG_I("wscli:WebSockCli State is %d SrvHello, login", g_wsCliConnStatus, op);
    			g_wsCliConnStatus = wsState_hello;
				m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
									"{\"op\":%d,\"type\":%d,\"id\":%d}",
									wsOP_cliLogin, wsValue_homeBase, MY_NODE_ID);

				wsCliSendMessage(String(g_devScrapBuffer));

    		break;

    	case wsOP_msgRelay:

    	    index = message.indexOf(String("\"from\":"));
    	    if(index < 0)
    	    {
    	    	return;
    	    }

    	    peerId = extractInt(g_devScrapBuffer + index + 7);
    	    LOG_I("wscli:WebSockCli RX peer id:%d", peerId);

    		pRemoteClient = findPeer(peerId);
    		if(!pRemoteClient)
    		{
    			LOG_I("wscli:WebSockCli RX new peer");
    			pRemoteClient = new CWebPeer(peerId);
				if(!pRemoteClient)
				{
					LOG_E( "wscli:wsOP_msgRelay: No heap for peer\n");
					//reply nack?
					return;
				}
				gConnectedPeers.addElement(pRemoteClient);
    		}

    		//extract message
    	    index = message.indexOf(String("\"msg\":\""));
    	    if(index < 0)
    	    {
    	    	return;
    	    }

    	    terminateString(g_devScrapBuffer + index + 7);

    	    LOG_I("wscli:WebSockCli RX peer msg:%s", g_devScrapBuffer + index + 7);

    		//receive message
    		pRemoteClient->onReceiveFromPeer((const char*)g_devScrapBuffer + index + 7);

    		break;
    	case wsOP_msgSpecial:
    		break;
    	case wsOP_positiveAck:
    		if(g_wsCliConnStatus == wsState_hello)
    		{
    			LOG_I("wscli:WebSockCli logged in", g_wsCliConnStatus, op);
    			g_wsCliConnStatus = wsState_conn;
    		}
    		break;
    	case wsOP_negativeAck:
    		if(g_wsCliConnStatus == wsState_hello)
    		{
    			LOG_I("wscli:WebSockCli server reject login", g_wsCliConnStatus, op);
    		}
    		break;
    };
}

void wsDisconnected(bool success)
{
	gTmrStayConnected.stop();
	gTmrReconnect.stop();

	if (success == true)
	{
		LOG_I("wscli:WsCli disc by request");
		gTmrReconnect.initializeUs(TMR_RECON_LATER, wsCliReconnect).start(false);
	}
	else
	{
		LOG_I("wscli:WsCli Disconnected. Reconnecting ..");
		gTmrReconnect.initializeUs(TMR_RECONN_ERR, wsCliReconnect).start(false);
	}
}

bool wsCliSendMessage(String msg)
{
	return wsCliSendMessage(msg.c_str(), msg.length());
}

bool wsCliSendMessage(const char* msg, uint32_t size)
{
	//LOG_I("wscli:wsCliSendMessage: %s", msg.c_str());
	if(wsClient.getWSMode() != ws_Disconnected)
	{
		wsClient.sendMessage(msg, size);
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
	LOG_I("wscli:wsCliStart()");
    wsClient.setOnReceiveCallback(wsMessageReceived);
    wsClient.setOnDisconnectedCallback(wsDisconnected);
    wsClient.setOnConnectedCallback(wsConnected);
    wsClient.addSslOptions(SSL_SERVER_VERIFY_LATER);
    wsClient.connect(ws_Url);

	gTmrKeepAlive.initializeUs(TMR_PING, wsCliPing).start();
}

void wsCliOnTimerStayConnected()
{
	uint32_t i=0;
	bool bConnectionNeeded = false;
	
	if(g_wsCliConnStatus != wsState_conn)
	{
		wsClient.disconnect();
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
			LOG_I("wscli:wsCliOnTimerStayConnected() disconnecting");
			wsClient.disconnect();
			g_wsCliConnStatus = wsState_inval;
		}
		else
		{
			LOG_I("wscli:wsCliOnTimerStayConnected() connection needed");
		}
	} //endif(wsClient.getWSMode() != ws_Disconnected)
}
