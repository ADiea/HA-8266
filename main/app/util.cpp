#include "appMain.h"
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
		LOG_D( "Float no. detected");
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

		LOG_E( "Bad terminal: %x, str: %s", c, *s);
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
			LOG_D("Parsed:%f", f);
		}
		else
		{
			LOG_E( "Int:%d, bad terminal: %x", intPart, **s);
			return false;
		}
	}
	else if(**s == ';')
	{
		LOG_I( "No decimals");
		*dest =  float(intPart);
		++(*s);
	}
	else
	{
		LOG_I( "Int:%d, unknown char: %x", intPart, **s);
		return false;
	}

	return true;
}

bool skipString(const char** s, char* dest, int destLen)
{
	if(**s == ';')
	{
		LOG_E( "Null string");
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
			LOG_E( "StringTooLong");
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
		LOG_E( "StringTooLong");
		dest[destLen-1] = 0;
		return false;
	}
	else dest[length] = 0;
	
	return true;
}

bool skipUint(const char **s, uint32_t *dest)
{
	uint32_t i = 0;
	char c;

	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';

	if(**s == '.')
	{
		LOG_D( "Float no. detected");
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

		LOG_E( "Bad terminal: %x, str: %s", c, *s);
		return false;
	}

	*dest =  i;
	return true;
}

uint32_t readFileFull(const char* path, char** buf, bool bForce/* = false*/)
{
	WDT.alive();

	if(!path || !buf)
	{
		LOG_E("Bad param");
		return 0;
	}

	FIL file;
	FRESULT fRes;
	FILINFO fno;
	uint32_t fActualSize = 0;
	CBusAutoRelease bus(devSPI_SDCard, 1000);
	if(bForce || bus.getBus())
	{
		do
		{
			fRes = f_stat(path, &fno);
			if(fRes != FR_OK)
			{
				LOG_E("Fstat err %d", (int)fRes);
				break;
			}

			if(fno.fsize == 0)
			{
				LOG_E("File %s has 0 bytes", path);
				break;
			}

			*buf = new char[fno.fsize + 1];

			if(!buf)
			{
				LOG_E("%s no heap", path);
				break;
			}

			LOG_I("ReadFileFull %s, %d bytes", path, fno.fsize);

			fRes = f_open(&file, path, FA_READ);

			if (fRes != FR_OK)
			{
				LOG_E("fopen: %d", (int)fRes);
				break;
			}

			f_read(&file, *buf, fno.fsize, &fActualSize);
			f_close(&file);

			if(fActualSize != fno.fsize)
			{
				LOG_E("only read %d", fActualSize);
			}

			LOG_D("read %d:%s", fActualSize, *buf);
			(*buf)[fActualSize] = 0;
		}
		while(0);

	}
	else
	{
		LOG_E( "SPI busy");
	}
	return fActualSize;
}

uint32_t writeFileFull(const char* path, char* buf, uint32_t len, bool bForce/* = false*/)
{
	WDT.alive();

	FIL file;
	FRESULT fRes;

	uint32_t actual = 0;

	CBusAutoRelease bus(devSPI_SDCard, 1000);
	if(bForce || bus.getBus())
	{
		do
		{
			fRes = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);

			if (fRes != FR_OK)
			{
				LOG_E("Fstat err %d", (int)fRes);
				break;
			}

			f_write(&file, buf, len, &actual);

			if (actual != len)
			{
				LOG_E("written %d of %d bytes", actual, len);
			}
			f_close(&file);
		}
		while(0);

		if(!bForce)
			releaseRadio();
	}
	else
	{
		LOG_E( "SPI busy");
	}

	return actual;
}
