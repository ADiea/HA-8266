#include "main.h"

volatile unsigned long gTimer;

void initSysTimer(void)
{
	gTimer = 0;
	
	TCCR0 = 1<<CS02;
	TIMSK = 1<<TOIE0;//ena overflow interrupt

}

volatile unsigned char whole = 0;
volatile unsigned char fraction = 0;


ISR(TIMER0_OVF_vect)
{
	//fcpu = 8Mhz prescaler = 1/256 => f=31250

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