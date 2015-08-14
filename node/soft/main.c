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
	
	_delay_ms(1);

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

#define CYCLE_PERIOD_MS 1000

extern void output_grb(u8 * ptr, u16 count);



//palette[]

	u8 off[] = {0x00, 0x00, 0x00}; //black/off
	u8 green[] = {0x20, 0x00, 0x00}; //green
	u8 red[] = {0xFF, 0xFF, 0xFF}; //red
	u8 blue[] = {0x00, 0x00, 0x20};	//blue

	u8 *palette[] = {off, green, red, blue};

int main(void)
{
	uint8_t loopDelay = 100;
	uint8_t curColorIndex = 1;
	uint8_t payLoad[64] = {0};
	uint8_t len = 0;
	uint8_t i, match;
	
	const char *ping = "PING";
	
	unsigned long curTime = millis();

	initHw();
	
	DDRD |= 1<<4;
	
	debugf(" System Init OK\n");
	
	output_grb(red, 3);
	
	//ws2812_setleds((tRGB*)&gColorPallette[curColorIndex], 1);
	
	debugf(" Led color set\n");
	
	do
	{	
		
		if(millis() - curTime > CYCLE_PERIOD_MS)
		{
			debugf("T");
			curTime = millis();
			sendColorIndex(curColorIndex);
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
			
			match = 1;

			for ( i = 0; i < len; ++i) 
			{	
				if(i < 4)
				{
					if(ping[i] != payLoad[i])
						match = 0;
				}
				else match = 0;
				debugf("%c", (char) payLoad[i]);
			}
			
			debugf("\n");
			
			if(match)
			{
				debugf("TX PONG ");
				if(!radio_sendPacketSimple(2, (unsigned char*)"OK"))
				{
					debugf("ERR\n");
				}
				else
				{
					debugf("OK\n");
				}
			}
		}
		
		switch(keyPress())
		{
			case NotPressed:
			break;
			
			case ShortPress:
				debugf("ShortPress\n");
				curColorIndex = (curColorIndex + 1) % NUM_COLORS;
				ws2812_setleds((tRGB*)&gColorPallette[curColorIndex], 1);
			break;
			
			case LongPress:
				debugf("LongPress\n");
			break;
		}
		
		_delay_ms(loopDelay);
	}
	while(1);

}
