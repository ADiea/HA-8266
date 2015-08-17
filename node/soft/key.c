
#include "main.h"

#define PRESSED !(KEY_INP & (1 << KEY_PIN))

void initKey(void)
{
	KEY_DDR &= ~(1 << KEY_PIN);
	KEY_PORT |= 1 << KEY_PIN;
}

eKeyPress keyPress()
{
	static unsigned char btnTimer=0;
	
	eKeyPress retVal = NotPressed;
	uchar btnPressed = 0;

	if(PRESSED)
	{
		_delay_ms(5); //debounce
		if(PRESSED)
		{
			btnPressed = 1;
			
			if(btnTimer < 255)
				btnTimer++;
		}		
	}
	
	
	if(btnPressed)
	{
		if(btnTimer > 150)
		{	
			retVal = LongPress;
		}
	}
	else
	{
		if(btnTimer > 0 && btnTimer < 150)
		{
			retVal = ShortPress;
		}
		btnTimer = 0;
	}
	
	return retVal;
}