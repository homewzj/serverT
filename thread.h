#ifndef __THREAD_H__
#define __THREAD_H__


#include "msgQueue.h"
#include "log.h"


ThreadContext *initThreadContext(char *pthreadName, logContext *pLogCtx);

void deinitThreadContext(ThreadContext *pthreadContext, logContext *pLogCtx);

void *workerThreadRun(void *arg);

bool checkWorkerThreadStautsIsBusy(ThreadContext *pCtx);


#endif
