#ifndef __LOG_H__
#define __LOG_H__


#include "common.h"



logContext * logContextInit(webServerContext *pWebContext, logContext *pOldLogCtx);
void deinitLogContext(logContext *pLogCtx);
void logRecord(logContext *pLogCtx, int level, const char *fmt, ...);
int logWriteCallBack(int fd, char *pLogInfo, size_t logSize);


#endif
