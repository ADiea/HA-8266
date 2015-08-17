#ifndef SYSTIMER_H
#define SYSTIMER_H

extern volatile unsigned long gTimer;

extern volatile unsigned long g_32us;

void initSysTimer(void);

#define millis() gTimer
#define us_x32() g_32us

typedef struct _timerCbk
{
	void (*cbk)(void);
	unsigned long timer;
	uint8_t active;
} 	t_timerCbk;



#endif /*SYSTIMER_H*/