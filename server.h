#ifndef __SERVER_H__
#define __SERVER_H__

#include "config.h"
#include "log.h"
#include "acl.h"
#include "network.h"
#include "threadpool.h"


int parseCmdOption(int argc, char *argv[]);

webServerContext * initWebServerContext(configContext *pConfig);

void deinitWebServerContext(webServerContext *pWebContext);




#endif
