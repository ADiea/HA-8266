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

//	cwGetDevices,
//	cwReplyDeviceList,

	//plant sensors
	//plantwatering

	cwMaxId,

};

bool cwReceivePacket(CAbstractPeer& peer, const char* pkt);

bool broadcastDeviceInfo(ConnectedPeerList &clients, CGenericDevice *device,
						CAbstractPeer* exceptPeer = NULL);

#endif
