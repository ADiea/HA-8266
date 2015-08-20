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
	
	relay_init();

	//enable and reset RBG led
	DDRD |= 1<<4;
	PORTD &= ~(1<<4);
	_delay_ms(1);
	
	//enable pullups, just to be sure
	SFIOR &= ~(1<<PUD);
	
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

void sendColorIndex(uint8_t color)
{
	char message[32], *s;

	s = ultoa(++pktSecv, message, 10);

	while(*s)
		++s;
	
	if(s - message < 32 - 9)
	{
		switch(color)
		{
			case 0:
				mcpy(s, "_LED   OFF", 10);
				break;
			case 1:
				mcpy(s, "_LED GREEN", 10);
				break;
			case 2:
				mcpy(s, "_LED   RED", 10);
				break;			
			case 3:
				mcpy(s, "_LED  BLUE", 10);
				break;		
			default:
				mcpy(s, "_LED   ???", 10);
				break;
		}
		
		s[10] = 0;
	}
	
	
	if(!radio_sendPacketSimple(slen(message), (unsigned char*)message))
	{
		debugf("TxERR\n");
	}
	else
	{
		debugf("TxOK: [ %s ]\n", message);
	}
}


int main(void)
{
	uint8_t loopDelay = 30;
	uint8_t curColorIndex = 1;
	uint8_t payLoad[64] = {0};
	uint8_t len = 0;
	uint8_t i;
	
	uint8_t dim = 30, dim_target=0;
	
	unsigned long curTime = millis();

	initHw();
	
	debugf(" System Init OK\n");
	
	ws2812_setleds((tRGB*)&gColorPallette[curColorIndex], 1);
	
	//preheat
	//relay_setDim(dim);
	//_delay_ms(1000);
	
	
	do
	{	
		
		/*++dim_hold;
		if(dim_hold > 5)
		{
			dim_hold = 0;

		}*/
		
		if(dim < dim_target)
		{
			++dim;
		}
		else if(dim > dim_target)
		{
			--dim;
		}
		relay_setDim(dim);
		
		if(millis() - curTime > CYCLE_PERIOD_MS)
		{
			debugf("Relay. num:%d switch:%d per:%d ignor:%d dim:%d\n", 
				relay_getNumCrosses(), relay_getNumSwitches(), 
				relay_getLastPeriod(), relay_getIgnoredPulses, dim);

			curTime = millis();
			//sendColorIndex(curColorIndex);

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
				debugf("%c", (char) payLoad[i]);
			}
			
			debugf("\n");
			
			if( len == 3 && payLoad[0] == 'I' && payLoad[1] == ':' )
			{
				i = payLoad[2]; 
				debugf("Ity:%u\n", i);
				dim_target = i;
			}
			
			debugf("REPLY: ");
			if(!radio_sendPacketSimple(2, (unsigned char*)"OK"))
			{
				debugf("ERR\n");
			}
			else
			{
				debugf("OK\n");
			}
		}
		
		switch(keyPress())
		{
			case NotPressed:
			break;
			
			case ShortPress:
				debugf("ShortPress\n");
				curColorIndex = (curColorIndex + 1) % NUM_COLORS;
				//ws2812_setleds((tRGB*)&gColorPallette[curColorIndex], 1);
			break;
			
			case LongPress:
				debugf("LongPress\n");
			break;
		}
		
		_delay_ms(loopDelay);
	}
	while(1);

}
