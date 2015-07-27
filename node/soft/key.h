#ifndef _KEY_H
#define _KEY_H

#define KEY_PIN    0
#define KEY_PORT   PORTB 
#define KEY_DDR    DDRB 
#define KEY_INP    PINB 

typedef enum _keyPress
{
	NotPressed = 0,
	ShortPress,
	LongPress
} eKeyPress;

void initKey(void);

eKeyPress keyPress();

#endif /*_KEY_H*/