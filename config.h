#ifndef __CONFIG_H__
#define __CONFIG_H__ 

#include "common.h"
#include "log.h"


configContext *configContextInit(logContext *pLogCtx);
void deinitConfigContext(configContext *pConfigContext, logContext *pLogCtx );






#endif
