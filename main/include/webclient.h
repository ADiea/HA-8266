#ifndef WEB_CLIENT_H
#define WEB_CLIENT_H

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Network/WebsocketClient.h>

#include "debug.h"


extern WebsocketClient wsClient;

void wsCliStart();
bool wsCliSendMessage(String msg);
bool wsCliSendMessage(const char* msg, uint32_t size);


#endif
