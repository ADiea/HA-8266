#include "netpeer.h"
#include "commWeb.h"
#include "device.h"
#include "webclient.h"

ConnectedPeerList gConnectedPeers;

CAbstractPeer* findPeer(uint32_t id)
{
	CAbstractPeer *peer = NULL;
	uint32_t i=0;

	for(;i<gConnectedPeers.size();i++)
	{
		if(gConnectedPeers[i]->getId() == id)
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

void CWebPeer::onReceiveFromPeer(const char* message)
{
	lastRXTimestamp = system_get_time() / 1024;
	cwReceivePacket(*this, message);
}

void CWebPeer::sendToPeer(const char* msg, uint32_t size)
{
	char messageHeader[128];

	uint32_t szHeader = m_snprintf(messageHeader, sizeof(messageHeader),
						"{\"op\":%d,\"dest\":%d,\"type\":%d,\"msg\":\"",
						wsOP_msgRelay, id, wsValue_mobileApp);

	char *messageToSend = new char [size + szHeader + 2];

	if(messageToSend)
	{
		memcpy(messageToSend, messageHeader, szHeader);
		memcpy(messageToSend + szHeader, msg, size);
		memcpy(messageToSend + szHeader + size, "\"}", 2);

		wsCliSendMessage(messageToSend, size + szHeader + 2);

		delete messageToSend;
	}
}

/* =========== LAN peers =========== */

void CLanPeer::onReceiveFromPeer(const char* message)
{
	lastRXTimestamp = system_get_time() / 1024;
	cwReceivePacket(*this, message);
}

void CLanPeer::sendToPeer(const char* msg, uint32_t size)
{
	if(pSocket)
	{
		pSocket->send(msg, size);
	}
}


