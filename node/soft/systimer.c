#include "main.h"

volatile unsigned long gTimer;

volatile unsigned long g_32us; //increments every 32 useconds

void initSysTimer(void)
{
	gTimer = 0;
	g_32us = 0;
	
	TCCR0 = 1<<CS00;
	TIMSK = 1<<TOIE0;//ena overflow interrupt

}

volatile unsigned char whole = 0;
volatile unsigned char fraction = 0;


ISR(TIMER0_OVF_vect)
{
	//fcpu = 8Mhz prescaler1 =>1/256 => f=39062.5
	
	++g_32us;

	++whole;

	if(whole > 30)
	{
		whole = 0;
		++gTimer;
		fraction = (fraction + 1) % 4;
		if (fraction == 0)
			++gTimer;
	}
}