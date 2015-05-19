#ifndef _DEBUG__H
#define _DEBUG__H

#define DEBUG_BUILD 1

#define ERR 0	
#define WARN 1
#define INFO 2
#define DBG 3

#if DEBUG_BUILD
	#define VERBOSE_LEVEL DBG
	
	#define LOG(x, ...) do {if( x <=  VERBOSE_LEVEL) { os_printf(__VA_ARGS__) } }while(0)
#else
	#define LOG(...)
#endif	

#endif /*_DEBUG__H*/