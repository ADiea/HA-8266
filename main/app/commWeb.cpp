
#include "debug.h"
#include "device.h"
#include "commWeb.h"
#include "util.h"


#define PKG_BUF_SIZE 256



char scrapPackage[PKG_BUF_SIZE];



bool reply_cwReplyToCommand(WebSocket& socket, eCommWebErrorCodes err)
{
	int sizePkt = snprintf(scrapPackage, sizeof(scrapPackage), "%d;%d;", cwReplyToCommand, err);
	socket.send((const char*)scrapPackage, sizePkt);
}

bool handle_cwErrorHandler(WebSocket& socket, const char **pkt)
{
	LOG_E( "Invalid pktId RXed");
	reply_cwReplyToCommand(socket, cwErrInvalidPacketID);
}

bool handle_cwGetLights(WebSocket& socket, const char **pkt)
{

	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetLightParams(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwGetTHs(WebSocket& socket, const char **pkt)
{
	int i = 0, numDevs = 0, sizePkt = 0;
	CDeviceTempHumid *th;

	for(; i < g_activeDevices.count(); i++)
	{
		if(devTypeTH == g_activeDevices[i]->m_deviceType)
		{
			++numDevs;
		}
	}
	sizePkt = snprintf(scrapPackage, sizeof(scrapPackage),
				"%d;%d;", cwReplyTHs, numDevs);
	//
	for(i=0; i < g_activeDevices.count(); i++)
	{
		if(devTypeTH == g_activeDevices[i]->m_deviceType)
		{
			th = (CDeviceTempHumid*)g_activeDevices[i];
			sizePkt += snprintf(scrapPackage + sizePkt, sizeof(scrapPackage) - sizePkt,
							"%d;%s;%f;%f;%d;%d;%d;%d;%f;%f;%f;", th->m_ID,
							th->m_FriendlyName.c_str(),
							th->m_state.tempSetpoint, th->m_state.lastTH.temp, 1, th->m_state.bEnabled, th->m_state.bIsHeating, th->m_state.bIsCooling,
							th->m_state.tempSetpointMin, th->m_state.tempSetpointMax, th->m_state.lastTH.humid);
		}
	}

	socket.send((const char*)scrapPackage, sizePkt);
}

bool handle_cwSetTHParams(WebSocket& socket, const char **pkt)
{
	int i = 0, numDevs = 0, sizePkt = 0;
	CDeviceTempHumid *th;

	int thID;
	float setTemp;

	eCommWebErrorCodes retCode = cwErrSuccess;

	do
	{

		if(!skipInt(pkt, &thID))
		{
			retCode = cwErrInvalidDeviceID;
			break;
		}

		for(i=0; i < g_activeDevices.count(); i++)
		{
			if(thID == g_activeDevices[i]->m_ID)
			{
				th = (CDeviceTempHumid*)g_activeDevices[i];

				if(th && skipFloat(pkt, &setTemp))
				{
					th->m_state.tempSetpoint = setTemp;
				}
				else
				{
					retCode = cwErrInvalidCommandParams;
				}

				break;
			}
		}
	}
	while(false);


	reply_cwReplyToCommand(socket, retCode);
}

bool handle_cwGetConfortStatus(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwGetRadioFMs(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetRadioFMParams(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwGetMovements(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetMovementParams(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool (*gCWHandlers[cwMaxId])(WebSocket&, const char**) =
{
	handle_cwErrorHandler,

	handle_cwGetLights,
	handle_cwErrorHandler,
	handle_cwSetLightParams,
	handle_cwErrorHandler,

	handle_cwGetTHs,
	handle_cwErrorHandler,
	handle_cwSetTHParams,
	handle_cwGetConfortStatus,
	handle_cwErrorHandler,
	handle_cwErrorHandler,

	handle_cwGetRadioFMs,
	handle_cwErrorHandler,
	handle_cwSetRadioFMParams,

	handle_cwGetMovements,
	handle_cwErrorHandler,
	handle_cwSetMovementParams,
	handle_cwErrorHandler,

};

bool cwReceivePacket(WebSocket& socket, const char* pkt)
{
	bool retVal = false;

	int pktId;

	if (!skipInt(&pkt, &pktId))
	{
		LOG_E( "cwReceivePacket: Cannot get Pkt ID");
	}
	else
	{
		LOG_D( "Received Pkt ID: %d", pktId);

		if(pktId >=  cwMaxId)
		{
			LOG_E( "cwReceivePacket: Bad pkt ID rx: %d", pktId);
		}
		else
		{
			retVal = gCWHandlers[pktId](socket, &pkt);
		}
	}

	return retVal;
}

