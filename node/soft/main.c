/*
* Author: ADiea
*
* Implement Home Automation Node
*
*/

/*
//todo:

node: pus quartz de la radio ca sursa; schimbat timeoutul in intreruperea de ceas -> rgb led?
trimis led mai des. trimis secventa pachet; testat buton de schimbare culoare, scimbat culoare doar pe buton

esp: refacut bucla de rx. vazut loop() cand se apeleaza
printat RSSI

*/
#include "main.h"

#define NUM_COLORS 4

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

void sendColorIndex(uint8_t color)
{
	char* message;
	
	switch(color)
	{
		case 0:
			message = "LED   OFF";
			break;
		case 1:
			message = "LED GREEN";
			break;
		case 2:
			message = "LED   RED";
			break;			
		case 3:
			message = "LED  BLUE";
			break;		
		default:
			message = "LED   ???";
			break;
	}
	
	if(!radio_sendPacketSimple(9, (unsigned char*)message))
	{
		debugf("TxERR\n");
	}
	else
	{
		debugf("TxOK\n");
	}
}

#define CYCLE_PERIOD_MS 5000

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
	
	debugf(" System Init OK\n");
	
	ws2812_setleds((tRGB*)&gColorPallette[curColorIndex], 1);
	
	do
	{	
		
		if(millis() - curTime > CYCLE_PERIOD_MS)
		{
			debugf("CH_COL\n");
			curTime = millis();
			curColorIndex = (curColorIndex + 1) % NUM_COLORS;
			ws2812_setleds((tRGB*)&gColorPallette[curColorIndex], 1);
			
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
				sendColorIndex(curColorIndex);
			break;
			
			case LongPress:
				debugf("LongPress\n");
			break;
		}
		
		
		_delay_ms(loopDelay);
	}
	while(1);

}
