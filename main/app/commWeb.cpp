#include "debug.h"
#include "device.h"
#include "commWeb.h"
#include "util.h"
#include "webserver.h"

bool broadcastDeviceInfo(WebSocketsList &clients, CGenericDevice *device,
						WebSocket *exceptSock /* = NULL*/)
{
	if(!device) return false;
	size_t sizePkt = m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
					"%d;%d;%d;", cwReplyDevicesOfType, device->m_deviceType, 1);

	sizePkt += device->serialize(g_devScrapBuffer + sizePkt,
								(uint32_t)(sizeof(g_devScrapBuffer) - sizePkt));

	for (int i = 0; i < clients.count(); i++)
	{
		if(exceptSock != &(clients[i]))
			clients[i].send((const char*)g_devScrapBuffer, sizePkt);
	}
}

bool reply_cwReplyToCommand(WebSocket& socket, eCommWebErrorCodes err, int lastCmdType = 0, int sequence = 0)
{
	int sizePkt = m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer), "%d;%d;%d;%d;", cwReplyToCommand, err, lastCmdType, sequence);
	socket.send((const char*)g_devScrapBuffer, sizePkt);
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
	int i = 0, numDevs = 0, sizePkt = 0, j, len, k, devType;

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
	sizePkt = m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
				"%d;%d;%d;", cwReplyDevicesOfType, devType, numDevs);

	for(i=0; i < g_activeDevices.count(); i++)
	{
		if(devType == g_activeDevices[i]->m_deviceType)
		{
			sizePkt += g_activeDevices[i]->serialize(g_devScrapBuffer + sizePkt,
										sizeof(g_devScrapBuffer) - sizePkt);
		}
	}

	socket.send((const char*)g_devScrapBuffer, sizePkt);
}

bool handle_cwSetTHParams(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetHeaterParams(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetGenericDeviceParams(WebSocket& socket, const char **pkt)
{
	int i = 0, sequence = 0;
	int devID;
	bool deserialized = false;
	eCommWebErrorCodes retCode = cwErrInvalidDeviceID;

	do
	{
		if(!skipInt(pkt, &devID))
		{
			retCode = cwErrInvalidCommandParams;
			break;
		}

		for(i=0; i < g_activeDevices.count(); i++)
		{
			if(devID == g_activeDevices[i]->m_ID)
			{
				LOG_I("found dev\n");
				if(g_activeDevices[i]->deserialize(pkt))
				{
					LOG_I("deserialized\n");
					retCode = cwErrSuccess;

					if(!skipInt(pkt, &sequence))
						retCode = cwErrInvalidCommandParams;

					g_activeDevices[i]->isSavedToDisk  =
							deviceWriteToDisk(g_activeDevices[i]);
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

	reply_cwReplyToCommand(socket, retCode, cwSetGenericDeviceParams, sequence);

	if(retCode == cwErrSuccess)
	{
		broadcastDeviceInfo(gHttpServer.getActiveWebSockets(),
							g_activeDevices[i], &socket);
	}
}

bool handle_cwNotifyGenericDeviceStatus(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
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

bool handle_cwGetGenericDeviceLogs(WebSocket& socket, const char **pkt)
{
	int sizePkt = 0, devId, fromTime, decimation, numEntries, i;

	if (!skipInt(pkt, &devId) ||
		!skipInt(pkt, &fromTime) ||
		!skipInt(pkt, &decimation) ||
		!skipInt(pkt, &numEntries)
	)
	{
		LOG_E( "handle_cwGetGenericDeviceLogs: Invalid pkg");
	}
	else
	{
		for(i=0; i < g_activeDevices.count(); i++)
		{
			if(devId == g_activeDevices[i]->m_ID)
			{
				sizePkt = m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
							"%d;%d;%d;", cwReplyGenericDeviceLogs,
							g_activeDevices[i]->m_deviceType, devId);
				LOG_I("cwGetGenericDeviceLogs1 %s", g_devScrapBuffer);
				sizePkt += deviceReadLog(devId, fromTime, decimation,
						 (char*)(g_devScrapBuffer + sizePkt), sizeof(g_devScrapBuffer) - sizePkt, numEntries);

				LOG_I("cwGetGenericDeviceLogs2 %s", g_devScrapBuffer);
				socket.send((const char*)g_devScrapBuffer, sizePkt);
				break;
			}
		}
	}
}

bool handle_cwReplyGenericDeviceLogs(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}


bool handle_cwPrintDebugInformation(WebSocket& socket, const char **pkt)
{
	int debugCommand = 0;
	eCommWebErrorCodes retCode = cwErrSuccess;
	do
	{
		if(!skipInt(pkt, &debugCommand))
		{
			retCode = cwErrInvalidCommandParams;
			break;
		}

		switch(debugCommand)
		{
		case 0:
			LOG_I("DBG: heap %d", system_get_free_heap_size());
			break;

		case 1:
			NetUtils::debugPrintTcpList();
			break;
		}
	}while(false);
	reply_cwReplyToCommand(socket, retCode);
}

/*

soft radio sa ceara confirmarea la 10 secunde si sa clipeasca mai rapid ledul daca nu e ok
1 blink every 5s - all ok
2 blink every 5s comm error; dupa 10 comm error sa isi faca si mcu rese la radio; dupa 20 failures sa isi dea reset(?)

*/

bool handle_cwSpecialCommand(WebSocket& socket, const char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrSuccess);
	LOG_I("DBG: RESTART");
	system_restart();
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

	handle_cwPrintDebugInformation,

	handle_cwSpecialCommand,

	handle_cwNotifyGenericDeviceStatus,
	handle_cwSetGenericDeviceParams,

	handle_cwGetGenericDeviceLogs,
	handle_cwReplyGenericDeviceLogs,

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

