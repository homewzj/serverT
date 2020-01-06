#ifndef __SERVER_H__
#define __SERVER_H__

#include "config.h"
#include "log.h"
#include "threadpool.h"

int parseCmdOption(int argc, char *argv[]);

webServerContext * initWebServerContext(configContext *pConfig);

void deinitWebServerContext(webServerContext *pWebContext);

int processServerContext(webServerContext *pContext, configContext *pConfig);


#endif
