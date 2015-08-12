#include "main.h"

#define BAUD 38400
#define BAUDVAL ((F_CPU)/(BAUD*16UL)-1)

#define MPRINTF_BUF_SIZE 128

#define SIGN    	(1<<1)	/* Unsigned/signed long */

void initUart(void)
{
	DDRD |= 1<<1;
	PORTD |= 1<<0; //pullup on rx
	
    UBRRH = 0;//(BAUDVAL>>8);
    UBRRL = 207;//-> 2400 //BAUDVAL; 
    UCSRB = (1<<TXEN);//|(1<<RXEN);
    UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
}

void printCh(char ch)
{
	while (!( UCSRA & (1<<UDRE)));
	UDR = ch;                  
}

int m_vsnprintf(char *buf, uint32_t maxLen, const char *fmt, va_list args);

int debugf(const char *fmt, ...)
{
	char buf[MPRINTF_BUF_SIZE], *p;
	va_list args;
	int n = 0;

	va_start(args, fmt);
	m_vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	p = buf;
	while (*p)
	{
		printCh(*p);
		n++;
		p++;
	}

	return n;
}

int m_vsnprintf(char *buf, uint32_t maxLen, const char *fmt, va_list args)
{
	int base, flags;
	char *str;
	const char *s;


	const uint8_t overflowGuard = 10;

	char tempNum[24];

	for (str = buf; *fmt; fmt++)
	{
		if(maxLen - (str - buf) < overflowGuard)
		{
			*str++ = 'o';
			*str++ = 'v';
			*str++ = 'f';
			break;
		}

		if (*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}

		flags = 0;
		fmt++; // This skips first '%'


		//skip width and flags data - not supported
		while ((*fmt >= '0' && *fmt <= '9') || '+' == *fmt
				|| '-' == *fmt || '#' == *fmt || '*' == *fmt || '.' == *fmt)
			fmt++;


		// Default base
		base = 10;

		switch (*fmt)
		{
		case 'c':
			*str++ = (unsigned char) va_arg(args, int);
			continue;

		case 's':
			s = va_arg(args, char *);

			if (!s)
			{
				s = "<NULL>";
			}
			else
			{
				while (*s)
					*str++ = *s++;
			}

			continue;

		case 'x':
			*str++ = '0';
			*str++ = 'x';
			base = 16;
			break;

		case 'd':
			flags |= SIGN;
		case 'u':
			break;

		default:
			if (*fmt != '%')
				*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}


		if (flags & SIGN)
			s = ltoa(va_arg(args, int), tempNum, base);
		else
			s = ultoa(va_arg(args, unsigned int), tempNum, base);

		while (*s)
			*str++ = *s++;
	}

	*str = '\0';
	return str - buf;
}
