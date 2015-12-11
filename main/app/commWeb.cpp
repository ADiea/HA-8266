
#include "debug.h"
#include "device.h"
#include "commWeb.h"
#include "util.h"


#define PKG_BUF_SIZE 256



char scrapPackage[PKG_BUF_SIZE];



bool reply_cwReplyToCommand(WebSocket& socket, eCommWebErrorCodes err)
{
	int sizePkt = m_snprintf(scrapPackage, sizeof(scrapPackage), "%d;%d;", cwReplyToCommand, err);
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

bool handle_cwGetDevicesOfType(WebSocket& socket, const char **pkt)
{
	int i = 0, numDevs = 0, sizePkt = 0;
	CDeviceTempHumid *th;
	CDeviceHeater *heat;

	int devType;

	if (!skipInt(pkt, &devType))
	{
		LOG_E( "handle_cwGetDevicesOfType: Cannot get Pkt Type");
	}

	for(; i < g_activeDevices.count(); i++)
	{
		if(devType == g_activeDevices[i]->m_deviceType)
		{
			++numDevs;
		}
	}
	sizePkt = m_snprintf(scrapPackage, sizeof(scrapPackage),
				"%d;%d;%d;", cwReplyDevicesOfType, devType, numDevs);
	//
	for(i=0; i < g_activeDevices.count(); i++)
	{
		if(devTypeTH == devType && devTypeTH == g_activeDevices[i]->m_deviceType)
		{
			th = (CDeviceTempHumid*)g_activeDevices[i];
			sizePkt += m_snprintf(scrapPackage + sizePkt, sizeof(scrapPackage) - sizePkt,
							"%d;%s;%.1f;%.1f;%d;%d;%d;%d;%.1f;%.1f;%.1f;%.1f;%.1f;", th->m_ID,
							th->m_FriendlyName.c_str(),
							th->m_state.tempSetpoint, th->m_state.lastTH.temp, 1, th->m_state.bEnabled, th->m_state.bIsHeating, th->m_state.bIsCooling,
							th->m_state.tempSetpointMin, th->m_state.tempSetpointMax, th->m_state.lastTH.humid,
							th->m_state.fLastTemp_1m, th->m_state.fLastTemp_8m);
		}
		else if(devTypeHeater == devType && devTypeHeater == g_activeDevices[i]->m_deviceType)
		{
			heat = (CDeviceHeater*)g_activeDevices[i];
			sizePkt += m_snprintf(scrapPackage + sizePkt, sizeof(scrapPackage) - sizePkt,
							"%d;%s;%d;%d;%d;%d;%d;%d;%d;", heat->m_ID,
							heat->m_FriendlyName.c_str(),
							heat->m_state.isOn?1:0, heat->m_state.isFault?1:0, heat->m_state.gasLevel_lastReading,
							heat->m_state.gasLevel_LowWarningThres , heat->m_state.gasLevel_MedWarningThres ,
							heat->m_state.gasLevel_HighWarningThres, heat->m_state.lastFault);
		}
	}

	socket.send((const char*)scrapPackage, sizePkt);
}

bool handle_cwSetTHParams(WebSocket& socket, const char **pkt)
{
	int i = 0, numDevs = 0, sizePkt = 0;
	CDeviceTempHumid *th;

	int thID;
	float setTemp, minTemp, maxTemp;

	char devName[MAX_FRIENDLY_NAME];

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

				if(th && skipFloat(pkt, &setTemp) &&
						skipString(pkt, (char*)devName, MAX_FRIENDLY_NAME) &&
						skipFloat(pkt, &minTemp) &&
						skipFloat(pkt, &maxTemp) )
				{
					th->m_state.tempSetpoint = setTemp;
					th->m_state.tempSetpointMin = minTemp;
					th->m_state.tempSetpointMax = maxTemp;

					th->m_FriendlyName = devName;
					deviceWriteToDisk((CGenericDevice*)th);
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

bool handle_cwSetHeaterParams(WebSocket& socket, const char **pkt)
{
	int i = 0, numDevs = 0, sizePkt = 0;
	CDeviceHeater *heat;

	char devName[MAX_FRIENDLY_NAME];

	int heatID;
	int lowT, medT, highT;

	eCommWebErrorCodes retCode = cwErrSuccess;

	do
	{
		if(!skipInt(pkt, &heatID))
		{
			retCode = cwErrInvalidDeviceID;
			break;
		}

		for(i=0; i < g_activeDevices.count(); i++)
		{
			if(heatID == g_activeDevices[i]->m_ID)
			{
				heat = (CDeviceHeater*)g_activeDevices[i];

				if(heat && skipString(pkt, (char*)devName, MAX_FRIENDLY_NAME))
				{
					heat->m_FriendlyName = devName;

					deviceWriteToDisk((CGenericDevice*)heat);
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
//,

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

	handle_cwGetDevicesOfType,
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

	handle_cwSetHeaterParams,

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

