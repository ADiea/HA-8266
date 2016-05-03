#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <user_config.h>
#include <SmingCore/SmingCore.h>

extern "C"{
extern uint32_t phy_get_rand();
}

//TODO: CHANGE static const char secret[] PROGMEM = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
//TODO: make PR to remove spiffs_mount(); from appinit/user_main.cpp
#include "device.h"
#include "commWeb.h"
#include "webserver.h"
#include "webclient.h"
#include "login.h"

#include "systemSettings.h"

#define ONE_SECOND 1000000

#if DEBUG_BUILD
	void debugStart();
#endif /*DEBUG_BUILD*/

void restartSystem();

#endif /*APP_MAIN_H*/
