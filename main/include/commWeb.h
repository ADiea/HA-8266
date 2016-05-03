#ifndef COMM_WEB_H
#define COMM_WEB_H

#include <SmingCore/SmingCore.h>
#include <netpeer.h>

class CGenericDevice;

enum eCommWebErrorCodes
{
	cwErrSuccess,
	cwErrNotImplemented,
	cwErrInvalidPacketID,
	cwErrFunctionNotImplemented,
	cwErrInvalidDeviceID,
	cwErrDeviceDoesNotAnswer,
	cwErrInvalidCommandParams,
	cwErrProtocolError,
	cwErrUnknown
};

enum eCommWebMsgTypes
{
	cwReplyToCommand = 0,

	cwGetLights,
	cwReplyLights,
	cwSetLightParams,
	cwNotifyLightStatus,

	cwGetDevicesOfType,
	cwReplyDevicesOfType,
	cwSetTHParams,
	cwGetConfortStatus,
	cwReplyConfortStatus,
	cwNotifyTHStatus,

	cwGetRadioFMs,
	cwReplyRadioFMs,
	cwSetRadioFMParams,

	cwGetMovements,
	cwReplyMovements,
	cwSetMovementParams,
	cwNotifyMovementStatus,

	cwSetHeaterParams,

	cwPrintDebugInformation,

	cwSpecialCommand,
	cwNotifyGenericDeviceStatus,
	cwSetGenericDeviceParams,
	cwGetGenericDeviceLogs,
	cwReplyGenericDeviceLogs,

	cwStartWiFiScan,
	cwReportWiFiList,
	cwSetWiFiParams,

//	cwGetDevices,
//	cwReplyDeviceList,

	//plant sensors
	//plantwatering

	cwMaxId,

};

//// web

enum WebWsProtocol_Ops
{
	//operation code
	wsOP_cliHello=0,
	wsOP_servHello=1,
	wsOP_cliLogin=2,
	wsOP_msgRelay=3,
	wsOP_msgSpecial=4,
	wsOP_positiveAck=5,
	wsOP_negativeAck=6,
	//wsOP_nop=7,
	//wsOP_remotePeerConnect=8,
	//wsOP_remotePeerDisconnect=9,
};

enum WebWsProtocol_SubOps
{
	//subopCodes
	wsOP_srvDebugConn=0,
};

enum WebWsProtocol_Keys
{
	//message keys
	wsKey_clientType=0,
};

enum WebWsProtocol_Vals
{
	//message values
	wsValue_homeBase=0,
	wsValue_webBrowser=1,
	wsValue_mobileApp=2,
	wsValue_unknown=3,
};

enum WebWsProtocol_State
{
	//connection states
	wsState_new=0,
	wsState_hello=1,
	wsState_conn=2,
	wsState_inval=3,
};


bool cwReceivePacket(CAbstractPeer& peer, const char* pkt);

bool broadcastDeviceInfo(ConnectedPeerList &clients, CGenericDevice *device,
						CAbstractPeer* exceptPeer = NULL);

void sendWiFiAPListToPeer(CAbstractPeer* peer, BssList& list);

#endif
