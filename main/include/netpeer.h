#ifndef NET_PEER_H
#define NET_PEER_H

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Network/WebsocketClient.h>

class CAbstractPeer;

typedef Vector<CAbstractPeer*> ConnectedPeerList;

extern ConnectedPeerList gConnectedPeers;

CAbstractPeer* findPeer(uint32_t id);

enum ePeerTypes
{
	ePeerLAN = 0, 
	ePeerWEB	
};

class CAbstractPeer
{
public:
	CAbstractPeer(ePeerTypes type, uint32_t _id):peerType(type), id(_id)
		{lastRXTimestamp = system_get_time() / 1024;}
	
	virtual void onReceiveFromPeer(const char* message) = 0;
	virtual void sendToPeer(const char* msg, uint32_t size) = 0;

	bool isConnectionAlive();
	ePeerTypes getType(){return peerType;}
	uint32_t getId(){return id;}

	virtual ~CAbstractPeer(){}
protected:

	uint32_t lastRXTimestamp;
	uint32_t id;
	ePeerTypes peerType;

};

class CWebPeer : public CAbstractPeer
{
public:
	CWebPeer(uint32_t _id):CAbstractPeer(ePeerWEB, _id){};
	
	virtual void onReceiveFromPeer(const char* message);
	virtual void sendToPeer(const char* msg, uint32_t size);
	
	virtual ~CWebPeer(){}
};

class CLanPeer : public CAbstractPeer
{
public:
	CLanPeer(uint32_t _id):CAbstractPeer(ePeerLAN, _id), pSocket(NULL){};
	
	virtual void onReceiveFromPeer(const char* message);
	virtual void sendToPeer(const char* msg, uint32_t size);
	
	virtual ~CLanPeer(){}
	
private:
	WebSocket *pSocket;
};

#endif
