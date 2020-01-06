#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "thread.h"
#include "log.h"

enum SCAN_RESULT_STATUS{
    RESULT_CREATE_THREAD = 0, 
    RESULT_DELETE_THREAD,
    RESULT_SAVE_CURRENT 
};


ThreadPoolMangerContext *initThreadPoolContext(webServerContext *pWebContext, size_t index, int pipefd);

void deinitThreadPoolContext(ThreadPoolMangerContext *pContext, logContext *pLogCtx);

void ThreadPoolMangerRun(ThreadPoolMangerContext *pContext);


int childProcessCreateThread(ThreadPoolMangerContext *pThreadContext, logContext *pLogCtx);

#endif
