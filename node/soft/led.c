#include "led.h"

uint8_t gLedState = 1;

void led_init(void)
{
	DDRB |= 1<<1;
	PORTB |= (1<<1);
}


void led_set(uint8_t state)
{
	gLedState = state;
	
	if(state)
	{
		PORTB |= 1<<1;
	}
	else
	{
		PORTB &= ~(1<<1);
	}
}

void led_change(void)
{
	led_set(!gLedState);
}