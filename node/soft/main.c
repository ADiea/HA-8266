/*
* Author: ADiea
*
* Implement Home Automation Node
*
*/

/*
//todo:
delay with pin low for rgb led

esp: refacut bucla de rx. vazut loop() cand se apeleaza
printat RSSI

*/
#include "main.h"

#define NUM_COLORS 4
#define CYCLE_PERIOD_MS 1000

#define LOOP_DELAY 30

typedef unsigned char u8;
typedef unsigned short u16;

volatile unsigned long int pktSecv = 0;

volatile tRGB gColorPallette[NUM_COLORS] = //g r b
{
	{0x00, 0x00, 0x00}, //black/off
	{0x20, 0x00, 0x00}, //green
	{0x00, 0x20, 0x00}, //red
	{0x00, 0x00, 0x20}	//blue
};


void initHw()
{
	initSysTimer();
	
	sei();

	initUart();
	
	initKey();
	
	spi_begin();
	
	radio_init();
	radio_readAll();
	
	led_init();

#if NODETYPE == NODE_LIGHT		
	relay_init();
	
	light_init();
#elif NODETYPE == NODE_HEATER
	heater_init();	
#endif	

#if HAS_RGBLED
	//enable and reset RBG led
	DDRD |= 1<<4;
	PORTD &= ~(1<<4);
	_delay_ms(1);
#endif
	
	//enable pullups, just to be sure
	MCUCR &= ~(1<<PUD);
}

void mcpy(char *d, char *s, unsigned int sz)
{
	for(; sz > 0; --sz)
	{
		*d++ = *s++;
	}
}

unsigned int slen(char *s)
{
	unsigned int sz = 0;
	while(*(s++)) ++sz;
	return sz;	
}

int main(void)
{
	uint8_t payLoad[64] = {0};
	uint8_t len = 0;
	uint8_t i;
	
	unsigned long curTime = millis();

	initHw();
	
	debugf(" System Init OK\n");
#if HAS_RGBLED	
	ws2812_setleds((tRGB*)&gColorPallette[1], 1);
#endif
	radio_startListening();
	
	do
	{	
#if NODETYPE == NODE_LIGHT	
		light_loop();
#elif NODETYPE == NODE_HEATER
		heater_loop();	
#endif	

		if(millis() - curTime > CYCLE_PERIOD_MS)
		{
			curTime = millis();
			led_change();
		}
		
		if(radio_isPacketReceived())
		{
			radio_getPacketReceived(&len, payLoad);
		}
		else
		{
			len = 0;
		}

		if(len != 0)
		{
			debugf("ARX(%d): ", len);

			for ( i = 0; i < len; ++i) 
			{	
				debugf("%x ", 0xFF & payLoad[i]);
			}
			debugf("\n");
			
#if NODETYPE == NODE_LIGHT	
			light_processPkg(payLoad, len);
#elif NODETYPE == NODE_HEATER
			heater_processPkg(payLoad, len);
#endif	
			radio_startListening();
		}
		_delay_ms(LOOP_DELAY);
	}
	while(1);
}
