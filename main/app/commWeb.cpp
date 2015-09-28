
#include "debug.h"
#include "device.h"
#include "commWeb.h"

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define PKG_BUF_SIZE 256

static bool skipInt(char **s, int *dest);
static bool skipFloat(char **s, float *dest);
static bool skipString(char** s, char* dest, uint32_t destLen);

char scrapPackage[PKG_BUF_SIZE];

int snprintf(char* buf, uint32_t length, const char *fmt, ...)
{
	char *p;
	va_list args;
	int n = 0;

	va_start(args, fmt);
	n = m_vsnprintf(buf, length, fmt, args);
	va_end(args);

	return n;
}

bool reply_cwReplyToCommand(WebSocket& socket, eCommWebErrorCodes err)
{
	int sizePkt = snprintf(scrapPackage, sizeof(scrapPackage), "%d;%d;", cwReplyToCommand, err);
	socket.send((const char*)scrapPackage, sizePkt);
}

bool handle_cwErrorHandler(WebSocket& socket, char **pkt)
{
	LOG_E( "Invalid pktId RXed");
	reply_cwReplyToCommand(socket, cwErrInvalidPacketID);
}

bool handle_cwGetLights(WebSocket& socket, char **pkt)
{

	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetLightParams(WebSocket& socket, char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwGetTHs(WebSocket& socket, char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetTHParams(WebSocket& socket, char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwGetConfortStatus(WebSocket& socket, char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwGetRadioFMs(WebSocket& socket, char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetRadioFMParams(WebSocket& socket, char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwGetMovements(WebSocket& socket, char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool handle_cwSetMovementParams(WebSocket& socket, char **pkt)
{
	reply_cwReplyToCommand(socket, cwErrFunctionNotImplemented);
}

bool (*gCWHandlers[cwMaxId])(WebSocket&, char**) =
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

	char *sPkt = (char*)pkt;

	int pktId;

	LOG_I( "Received Pkt ID: %d", pktId);

	if (!skipInt(&sPkt, &pktId))
	{
		LOG_E( "cwReceivePacket: Cannot get Pkt ID");
	}
	else
	{
		if(pktId >=  cwMaxId)
		{
			LOG_E( "cwReceivePacket: Bad pkt ID rx: %d", pktId);
		}
		else
		{
			retVal = gCWHandlers[pktId](socket, &sPkt);
		}
	}

	return retVal;
}

// static functions

static bool skipInt(char **s, int *dest)
{
	int i = 0;
	bool bSign = false;

	if(**s == '-')
	{
		bSign = true;
		++(*s);
	}

	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';

	if(**s == '.')
	{
		LOG_E( "skipInt: Float no. detected");
	}
	else if(**s == ';')
	{
		++(*s);
	}
	else
	{
		LOG_E( "skipInt: bad terminal: %x", **s);
		return false;
	}

	*dest = bSign ? -i : i;
	return true;
}

static bool skipFloat(char **s, float *dest)
{
	int intPart;

	uint8_t level = 1, idx;

	float f = 0, digit;

	if(!skipInt(s, &intPart))
		return false;

	if(**s == '.')
	{
		f = 0;

		while (is_digit(**s))
		{
			digit = *((*s)++) - '0';

			for(idx = 0; idx < level; idx++)
			{
				digit /= 10;
			}

			f += digit;

			level++;
		}

		if(**s == ';')
		{
			++(*s);
		}
		else
		{
			LOG_E( "skipFloat: bad terminal: %x", **s);
			return false;
		}

	}
	else if(**s == ';')
	{
		LOG_I( "skipFloat: no decimals");
		*dest =  float(intPart);
		++(*s);
	}
	else
	{
		LOG_I( "skipFloat: unknown char: %x", **s);
		return false;
	}

	return true;
}

static bool skipString(char** s, char* dest, uint32_t destLen)
{
	if(**s == ';')
	{
		LOG_E( "skipString: null string");

		return false;
	}

	uint32_t length = 0;

	while (**s != ';')
	{
		if(**s == '\\' && *(*s+1) == ';')
		{
			++(*s);
		}

		dest[length] = *((*s)++);

		if(length >= destLen)
		{
			LOG_E( "skipString: StringTooLong");
			return false;
		}
		else ++length;
	}

	return true;
}
