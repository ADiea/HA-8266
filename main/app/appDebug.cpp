#include "appMain.h"

#ifdef MEMLEAK_DEBUG
//provided by linker
extern char _heap_start;
extern uint8_t gHeapOpFlushAfter;
#endif

#if DEBUG_BUILD
	#define HEART_BEAT (60 * ONE_SECOND)
	Timer tmrHeartBeat;

	static void heartbeat_cb(void)
	{
		LOG_I( "%s Heap: %ld",
				SystemClock.getSystemTimeString().c_str(),
				system_get_free_heap_size());
	}

void debugStart()
{
	tmrHeartBeat.initializeUs(HEART_BEAT, heartbeat_cb).start();
	#define CASE(x) case x: \
		LOG_I( #x); \
		break;

	LOG_II( "Reset: ");
	rst_info* rstInfo = system_get_rst_info();
	if(rstInfo)
	{
		switch(rstInfo->reason)
		{
			CASE(REASON_DEFAULT_RST)
			CASE(REASON_WDT_RST) 		// hardware watch dog reset
			CASE(REASON_EXCEPTION_RST)	// exception reset, GPIO status won’t change
			CASE(REASON_SOFT_WDT_RST)	// software watch dog reset, GPIO status won’t change
			CASE(REASON_SOFT_RESTART)	// software restart ,system_restart , GPIO status won’t change
			CASE(REASON_DEEP_SLEEP_AWAKE)
			default:
				LOG_I( "UNKNOWN (%d)", rstInfo->reason);
				break;
		}
	}
	LOG_I( "Chip id=%ld", system_get_chip_id());
	LOG_I( "Flash id=%ld", spi_flash_get_id());
	LOG_I( "Mem info:");
	system_print_meminfo();
#ifdef MEMLEAK_DEBUG
	LOG_I("\nhlog_param:{\"heap_start\":0x%x, \"heap_end\":0x3fffc000}", ((uint32_t)&_heap_start));
	gHeapOpFlushAfter = 32;
#endif
}
#endif /*DEBUG_BUILD*/

