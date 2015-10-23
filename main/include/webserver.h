#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "debug.h"
#include "commWeb.h"

extern NtpClient *gNTPClient;
extern HttpServer gHttpServer;

void startWebServers();

#endif
