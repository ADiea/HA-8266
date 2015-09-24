
#include "debug.h"
#include "device.h"
#include "commWeb.h"

#define is_digit(c) ((c) >= '0' && (c) <= '9')

bool skipInt(char **s, int *dest)
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
		LOG(ERR, "skipInt: Float no. detected");
	}
	else if(**s == ';')
	{
		++(*s);
	}
	else
	{
		LOG(ERR, "skipInt: bad terminal: %x", **s);
		return false;
	}

	*dest = bSign ? -i : i;
	return true;
}

bool skipFloat(char **s, float *dest)
{
	int intPart = skipInt(s);

	uint8_t level = 1, idx;

	float f = 0, digit;

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
			LOG(ERR, "skipFloat: bad terminal: %x", **s);
			return false;
		}

	}
	else if(**s == ';')
	{
		LOG(INFO, "skipFloat: no decimals");
		*dest =  float(intPart);
		++(*s);
	}
	else
	{
		LOG(INFO, "skipFloat: unknown char: %x", **s);
		return false;
	}

	return true;
}

bool skipString(char** s, char* dest, uint32_t destLen)
{
	if(**s == ';')
	{
		LOG(ERR, "skipString: null string");
		*dest = NULL;
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
			LOG(ERR, "skipString: StringTooLong");
			return false;
		}
		else ++length;
	}

	return true;
}

bool handle_cwErrorHandler(char *pkt)
{
	LOG(ERR, "Invalid pktId RXed");
}

bool handle_cwGetLights(char *pkt)
{
}

bool handle_cwSetLightParams(char *pkt)
{
}

bool handle_cwGetTHs(char *pkt)
{
}

bool handle_cwSetTHParams(char *pkt)
{
}

bool handle_cwGetConfortStatus(char *pkt)
{
}

bool handle_cwGetRadioFMs(char *pkt)
{
}

bool handle_cwSetRadioFMParams(char *pkt)
{
}

bool handle_cwGetMovements(char *pkt)
{
}

bool handle_cwSetMovementParams(char *pkt)
{
}



bool (*gCWHandlers[cwMaxId])(char*) =
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

bool cwReceivePacket(char* pkt)
{
	bool retVal = false;
	uint32_t pktId;

	LOG(INFO, "Received Pkt ID: %d", pktId);

	if(pktId >=  cwMaxId)
	{
		LOG(ERR, "Bad pkt ID rx: %d", pktId);
	}
	else
	{
		retVal = gCWHandlers[gCWHandlers]();
	}

	return retVal;
}
