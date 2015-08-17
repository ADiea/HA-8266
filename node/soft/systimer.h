#ifndef SYSTIMER_H
#define SYSTIMER_H

extern volatile unsigned long gTimer;

extern volatile unsigned long g_32us;

void initSysTimer(void);

#define millis() gTimer
#define us_x32() g_32us

#endif /*SYSTIMER_H*/