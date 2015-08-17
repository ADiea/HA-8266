#include "relay.h"

volatile unsigned long g_lastCrossTime = 0;

volatile uint16_t g_lastPeriod = 0;


void relay_init(void)
{
	DDRD &= ~(1<<3);
	PORTD |= 1<<3;
	
	//int1 ena on falling edge
	MCUCR |= 1<<ISC11;
	GICR |= 1<<INT1;
}

//return ast period in multiple of 32 us
uint16_t relay_getLastPeriod(void)
{
	return g_lastPeriod;
}

ISR(INT1_vect)
{
	//g_lastPeriod = us_x32() - g_lastCrossTime;
	
	//g_lastCrossTime = us_x32();
	
	++g_lastPeriod;
}