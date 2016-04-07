#include "netpeer.h"
#include "commWeb.h"

ConnectedPeerList gConnectedPeers;

CAbstractPeer* findPeer(uint32_t id)
{
	CAbstractPeer *peer = NULL;
	uint32_t i=0;

	for(;i<gConnectedPeers.size();i++)
	{
		if(gConnectedPeers[i]->id == id)
		{
			peer = gConnectedPeers[i];
			break;
		}
	}
	return peer;
}

bool CAbstractPeer::isConnectionAlive()
{
	return (system_get_time() / 1024) - lastRXTimestamp < 20000;
}

/* =========== WEB peers =========== */

void CWebPeer::onReceiveFromPeer(String& message)
{
	lastRXTimestamp = system_get_time() / 1024;
	cwReceivePacket(*this, message.c_str());
}

void CWebPeer::sendToPeer(const char* msg, uint32_t size)
{
	msg[size-1] = 0;
	m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
						"{op:%d,dest:%d,type:%d,msg:\"%s\"}",
						wsOP_msgRelay, id, wsValue_mobileApp, msg);

	wsCliSendMessage(String(g_devScrapBuffer));
}

/* =========== LAN peers =========== */

void CLanPeer::onReceiveFromPeer(String& message)
{
	lastRXTimestamp = system_get_time() / 1024;
	cwReceivePacket(*this, message.c_str());
}

void CLanPeer::sendToPeer(const char* msg, uint32_t size)
{
	if(pSocket)
	{
		pSocket->send(msg, size);
	}
}


