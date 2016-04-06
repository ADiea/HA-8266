#include "netpeer.h"
#include "commWeb.h"

ConnectedPeerList gConnectedPeers;

/* =========== WEB peers =========== */

void CWebPeer::onReceiveFromPeer(String& message)
{
	//unpack
	
	//send
	cwReceivePacket(*this, message.c_str());
}

void CWebPeer::sendToPeer(const char* msg, uint32_t size)
{

}

/* =========== LAN peers =========== */

void CLanPeer::onReceiveFromPeer(String& message)
{
	cwReceivePacket(*this, message.c_str());
}

void CLanPeer::sendToPeer(const char* msg, uint32_t size)
{
	if(pSocket)
	{
		pSocket->send(msg, size);
	}
}


