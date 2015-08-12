#ifndef _KEY_H
#define _KEY_H

#define KEY_PIN    2
#define KEY_PORT   PORTD 
#define KEY_DDR    DDRD 
#define KEY_INP    PIND 

typedef enum _keyPress
{
	NotPressed = 0,
	ShortPress,
	LongPress
} eKeyPress;

void initKey(void);

eKeyPress keyPress();

#endif /*_KEY_H*/