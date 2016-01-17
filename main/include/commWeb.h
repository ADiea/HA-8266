#ifndef COMM_WEB_H
#define COMM_WEB_H

#include <SmingCore/SmingCore.h>

enum eCommWebErrorCodes
{
	cwErrSuccess,
	cwErrNotImplemented,
	cwErrInvalidPacketID,
	cwErrFunctionNotImplemented,
	cwErrInvalidDeviceID,
	cwErrDeviceDoesNotAnswer,
	cwErrInvalidCommandParams,
	cwErrProtocolError
};

enum eCommWebMsgTYpes
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

//	cwGetDevices,
//	cwReplyDeviceList,

	//plant sensors
	//plantwatering

	cwMaxId,

};

bool cwReceivePacket(WebSocket& socket, const char* pkt);

#endif
