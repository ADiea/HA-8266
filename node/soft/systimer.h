#ifndef SYSTIMER_H
#define SYSTIMER_H

extern volatile unsigned long gTimer;

void initSysTimer(void);

#define millis() gTimer

#endif /*SYSTIMER_H*/