#include "util.h"
#include "debug_progmem.h"

#include <stdarg.h>

bool skipInt(const char **s, int *dest)
{
	int i = 0;
	char c;
	bool bSign = false;

	if(**s == '-')
	{
		bSign = true;
		++(*s);
	}

	//LOG_D( "skipInt: String is %s", *s);

	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';

	if(**s == '.')
	{
		LOG_D( "skipInt: Float no. detected");
	}
	else  if(**s == ';')
	{
		++(*s);
	}
	else
	{
		c = **s;
		while (';' != **s && 0 != **s)
					(*s)++;

		LOG_E( "skipInt: bad terminal: %x, str: %s", c, *s);
		return false;
	}

	*dest = bSign ? -i : i;
	return true;
}

bool skipFloat(const char **s, float *dest)
{
	int intPart;

	uint8_t level = 1, idx;

	float f = 0, digit;

	if(!skipInt(s, &intPart))
		return false;

	if(**s == '.')
	{
		f = intPart;
		++(*s);
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
			*dest = f;
			LOG_D("skipFloat: Parsed:%f", f);
		}
		else
		{
			LOG_E( "skipFloat: int:%d, bad terminal: %x", intPart, **s);
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
		LOG_I( "skipFloat: int:%d, unknown char: %x", intPart, **s);
		return false;
	}

	return true;
}

bool skipString(const char** s, char* dest, int destLen)
{
	if(**s == ';')
	{
		LOG_E( "skipString: null string");
		return false;
	}

	uint32_t length = 0;

	while ((**s != ';') && (**s != 0))
	{
		if(**s == '\\' && *(*s+1) == ';')
		{
			++(*s);
		}

		dest[length] = *((*s)++);

		if(length >= destLen)
		{
			LOG_E( "skipString: StringTooLong");
			dest[destLen-1] = 0;
			return false;
		}
		else ++length;
	}

	if(**s == ';')
	{
		++(*s);
	}

	if(length >= destLen)
	{
		LOG_E( "skipString: StringTooLong");
		dest[destLen-1] = 0;
		return false;
	}
	else dest[length] = 0;
	
	return true;
}
