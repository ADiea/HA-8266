#ifndef COMM_WEB_H
#define COMM_WEB_H

#include <SmingCore/SmingCore.h>

enum eCommWebErrorCodes
{
	cwErrSuccess,
	cwErrInvalidID,
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

bool cwReceivePacket(char* pkt);

#endif
