#ifndef UART_H
#define UART_H

#include <stdarg.h>

void initUart(void);

int debugf(const char *fmt, ...);

#endif /*UART_H*/