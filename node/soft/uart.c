#include "main.h"

#define BAUD 9600
#define BAUDVAL ((F_CPU)/(BAUD*16UL)-1)
  
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


  
void initUart(void)
{
	DDRD |= 1<<1;
	
    UBRRH = (BAUDVAL>>8);
    UBRRL = BAUDVAL;
    UCSRB = (1<<TXEN)|(1<<RXEN);
    UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
}

void printUart(char* str)
{
	while(*str)
	{
		while (!( UCSRA & (1<<UDRE)));
		UDR = *str++;                  
	}
}