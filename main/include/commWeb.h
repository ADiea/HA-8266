#ifndef COMM_WEB_H
#define COMM_WEB_H

#include <SmingCore/SmingCore.h>

enum eCommWebErrorCodes
{
	cwErrSuccess,
	cwNotImplemented,
	cwErrInvalidPacketID,
	cwErrFunctionNotImplemented,
	cwErrInvalidDeviceID,
	cwErrDeviceDoesNotAnswer,
	cwErrInvalidCommandParams,
};

enum eCommWebMsgTYpes
{
	cwReplyToCommand = 0,

	cwGetLights,
	cwReplyLights,
	cwSetLightParams,
	cwNotifyLightStatus,

	cwGetTHs,
	cwReplyTHs,
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

	//plant sensors
	//plantwatering

	cwMaxId,

};

bool cwReceivePacket(WebSocket& socket, const char* pkt);

#endif
