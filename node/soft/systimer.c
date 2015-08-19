#include "main.h"

volatile unsigned long gTimer;

volatile unsigned long g_32us; //increments every 32 useconds

#define NUM_CALLBACKS 4
t_timerCbk timerCallbacks[NUM_CALLBACKS];

uint8_t timer_addCallback(void (*cbk)(void), unsigned long timer)
{
	uint8_t i, ret=0;
	
	if(!cbk)
		return 0;
	
	for( i=0; i < NUM_CALLBACKS; i++)
	{
		if(!timerCallbacks[i].active)
		{
			timerCallbacks[i].cbk = cbk;
			timerCallbacks[i].timer = timer;
			timerCallbacks[i].active = 1;
			ret = 1;
			break;
		}
	}
	
	return ret;
}

void initSysTimer(void)
{
	uint8_t i;
	
	gTimer = 0;
	g_32us = 0;
	
	TCCR0 = 1<<CS00;
	TIMSK = 1<<TOIE0;//ena overflow interrupt

	for( i=0; i < NUM_CALLBACKS; i++)
	{
		timerCallbacks[i].active = 0;
	}
}

volatile unsigned char whole = 0;
volatile unsigned char fraction = 0;


ISR(TIMER0_OVF_vect)
{
	//fcpu = 8Mhz prescaler1 =>1/256 => f=39062.5
	
//	uint8_t i = 0;
	
	++g_32us;

	++whole;

	if(whole > 30)
	{
		whole = 0;
		++gTimer;
		fraction = (fraction + 1) % 4;
		if (fraction == 0)
			++gTimer;
/*		
		//check callbacks
		for( i=0; i < NUM_CALLBACKS; i++)
		{
			if(timerCallbacks[i].active && gTimer > timerCallbacks[i].timer)
			{
				timerCallbacks[i].cbk();
				timerCallbacks[i].active = 0;
			}
		}
*/		
	}
}