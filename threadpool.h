#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "thread.h"
#include "log.h"

enum SCAN_RESULT_STATUS{
    RESULT_CREATE_THREAD = 0, 
    RESULT_DELETE_THREAD,
    RESULT_SAVE_CURRENT 
};


ThreadPoolMangerContext *initThreadPoolContext(webServerContext *pWebContext);

void deinitThreadPoolContext(ThreadPoolMangerContext *pContext, logContext *pLogCtx);

int ThreadPoolMangerRun(ThreadPoolMangerContext *pContext);

int createWorkerThread(ThreadPoolMangerContext *pContext);

#endif
