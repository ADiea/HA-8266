#include "led.h"

uint8_t gLedState = 1;

void led_init(void)
{
	DDRC |= 1<<1;
	PORTC |= (1<<1);
}


void led_set(uint8_t state)
{
	gLedState = state;
	
	if(state)
	{
		PORTC |= 1<<1;
	}
	else
	{
		PORTC &= ~(1<<1);
	}
}

void led_change(void)
{
	led_set(!gLedState);
}