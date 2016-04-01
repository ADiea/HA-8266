#ifndef WEB_CLIENT_H
#define WEB_CLIENT_H

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Network/WebsocketClient.h>

#include "debug.h"
#include "commWeb.h"

extern WebsocketClient wsClient;

void wsCliStart();
void wsCliSendMessage(String msg);


#endif