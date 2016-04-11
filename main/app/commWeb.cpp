#include "debug.h"
#include "device.h"
#include "commWeb.h"
#include "util.h"
#include "webserver.h"
#include "netpeer.h"

bool broadcastDeviceInfo(ConnectedPeerList &clients, CGenericDevice *device,
						CAbstractPeer* exceptPeer/* = NULL*/)
{
	if(!device) return false;
	size_t sizePkt = m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer),
					"%d;%d;%d;", cwReplyDevicesOfType, device->m_deviceType, 1);

	sizePkt += device->serialize(g_devScrapBuffer + sizePkt,
								(uint32_t)(sizeof(g_devScrapBuffer) - sizePkt));

	for (int i = 0; i < clients.count(); i++)
	{
		if(exceptPeer != clients[i])
			clients[i]->sendToPeer((const char*)g_devScrapBuffer, sizePkt);
	}
}

bool reply_cwReplyToCommand(CAbstractPeer& peer, eCommWebErrorCodes err, int lastCmdType = 0, int sequence = 0)
{
	int sizePkt = m_snprintf(g_devScrapBuffer, sizeof(g_devScrapBuffer), "%d;%d;%d;%d;", cwReplyToCommand, err, lastCmdType, sequence);
	peer.sendToPeer((const char*)g_devScrapBuffer, sizePkt);
}

bool handle_cwErrorHandler(CAbstractPeer& peer, const char **pkt)
{
	LOG_E( "Invalid pktId RXed");
	reply_cwReplyToCommand(peer, cwErrInvalidPacketID);
}

bool handle_cwGetLights(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwSetLightParams(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwGetDevicesOfType(CAbstractPeer& peer, const char **pkt)
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

	peer.sendToPeer((const char*)g_devScrapBuffer, sizePkt);
}

bool handle_cwSetTHParams(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwSetHeaterParams(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwSetGenericDeviceParams(CAbstractPeer& peer, const char **pkt)
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

	reply_cwReplyToCommand(peer, retCode, cwSetGenericDeviceParams, sequence);

	if(retCode == cwErrSuccess)
	{
		broadcastDeviceInfo(gConnectedPeers, g_activeDevices[i], &peer);
	}
}

bool handle_cwNotifyGenericDeviceStatus(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwGetConfortStatus(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwGetRadioFMs(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwSetRadioFMParams(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwGetMovements(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwSetMovementParams(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}

bool handle_cwGetGenericDeviceLogs(CAbstractPeer& peer, const char **pkt)
{
	int sizePkt = 0, devId, fromTime, decimation, numEntries, i, entriesWritten = 0;
	uint32_t entriesRead = 0;

	bool printHeader = true;

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

				do
				{
					sizePkt += deviceReadLog(devId, fromTime, decimation,
											 (char*)(g_devScrapBuffer + sizePkt), sizeof(g_devScrapBuffer) - sizePkt,
											 numEntries, printHeader, entriesWritten, entriesRead);

					fromTime = (fromTime/8192 + 1)*8192;
				}
				while( numEntries > entriesWritten && SystemClock.now(eTZ_UTC).toUnixTime() > fromTime);

				LOG_I("cwGetGenericDeviceLogs %s", g_devScrapBuffer);
				peer.sendToPeer((const char*)g_devScrapBuffer, sizePkt);
				break;
			}
		}
	}
}

bool handle_cwReplyGenericDeviceLogs(CAbstractPeer& peer, const char **pkt)
{
	reply_cwReplyToCommand(peer, cwErrFunctionNotImplemented);
}


bool handle_cwPrintDebugInformation(CAbstractPeer& peer, const char **pkt)
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
	reply_cwReplyToCommand(peer, retCode);
}

/*

soft radio sa ceara confirmarea la 10 secunde si sa clipeasca mai rapid ledul daca nu e ok
1 blink every 5s - all ok
2 blink every 5s comm error; dupa 10 comm error sa isi faca si mcu rese la radio; dupa 20 failures sa isi dea reset(?)

*/

bool handle_cwSpecialCommand(CAbstractPeer& peer, const char **pkt)
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
			LOG_I("DBG: RESTART");
			system_restart();
			break;

		case 1:
			if(!skipInt(pkt, &debugCommand))
			{
				retCode = cwErrInvalidCommandParams;
				break;
			}
			deviceDeleteLog(debugCommand);
			break;
		}
	}while(false);
	reply_cwReplyToCommand(peer, retCode);

}


bool (*gCWHandlers[cwMaxId])(CAbstractPeer& peer, const char**) =
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

bool cwReceivePacket(CAbstractPeer& peer, const char* pkt)
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
			retVal = gCWHandlers[pktId](peer, &pkt);
		}
	}

	return retVal;
}

