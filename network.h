#ifndef __NETWORK_H__
#define __NETWORK_H__


#include "common.h"
#include "log.h"



listenSocketContext *createListenSocket(configContext *pConfig, size_t *socketNum, logContext *pLogCtx);

void deinitNetWorkContext(listenSocketContext *pListenContext);

#endif
