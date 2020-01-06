#include "thread.h"
static bool getTaskFromMaster(ThreadPoolMangerContext *pContext, ThreadContext *pCtx, QueueItem **pTaskNode);

ThreadContext * initThreadContext(char *pthreadName, logContext *pLogCtx) {
    ThreadContext *pCtx = (ThreadContext *)malloc(sizeof(ThreadContext));
    if (!pCtx) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "s", "Init WorkerThread Context Occur Error!\n");
        return NULL;
    }
    pCtx->tid = -1;
    pCtx->bExitFlag = false;
    pCtx->pThreadName = strdup(pthreadName);
    pCtx->callback = workerThreadRun;
    pCtx->privdata = (void *)pCtx;
    pCtx->next = pCtx->prev = NULL;
    pCtx->lTaskHandleCount = 0;
    pCtx->next = NULL;
    pCtx->busy = 0;
    logRecord(pLogCtx, LOG_LEVEL_INFO, "sss", "Init WorkerThread Context:",pCtx->pThreadName,"successfully!");
    return pCtx;
}

void deinitThreadContext(ThreadContext  *pthreadContext, logContext *pLogCtx){
    assert(pthreadContext != NULL);
    if (pthreadContext->tid != -1) {
        pthread_join(pthreadContext->tid, NULL);
    }
    if (pthreadContext->pThreadName){
        free(pthreadContext->pThreadName);
        pthreadContext->pThreadName = NULL;
    }
    logRecord(pLogCtx, LOG_LEVEL_INFO, "s", "deinit Worker Thread Context successfully!");
    return;
}

void workerThreadSignalHandle(int signo) {
    switch(signo){
    case SIGINT:
    case SIGTERM:
        logRecord(gWebServerContext->pLogCtx, LOG_LEVEL_INFO, "sdsd", "Worker Thread:",(int)syscall(SYS_gettid),"Catch Signal:", signo);
        gWebServerContext->bExitFlag = true;
        break;
    default:
        break;
    }
}

static bool getTaskFromMaster(ThreadPoolMangerContext *pContext, ThreadContext *pCtx, QueueItem **pTaskNode) {
    pthread_mutex_lock(&(pContext->mtx));
    while(gWebServerContext->bExitFlag != true && isQueueEmpty(pContext->queue)) {            
        pthread_cond_wait(&(pContext->not_empty), &(pContext->mtx));
    }
    *pTaskNode = popQueueItem(pContext->queue);
    pthread_cond_signal(&(pContext->not_full));
    pthread_mutex_unlock(&(pContext->mtx));
    return (!(gWebServerContext->bExitFlag)&&(pCtx->bExitFlag != true));
}



void *workerThreadRun(void *arg) {
    QueueItem *pTaskNode = NULL;
    struct timeval taskStart = {0, 0};
    struct timeval taskEnd = {0, 0};
    ThreadContext *pCtx = (ThreadContext *)arg;
    signal(SIGINT, workerThreadSignalHandle);
    signal(SIGTERM, workerThreadSignalHandle);
    prctl(PR_SET_NAME, (unsigned long)(pCtx->pThreadName));
    ThreadPoolMangerContext *pContext = (ThreadPoolMangerContext *)(pCtx->pCtx);
    /*@TODO:sigmask some signal such as SIGINT, SIGTERMINL*/
    while(getTaskFromMaster(pContext, pCtx, &pTaskNode)){
        /*@TODO:*/
        if (pTaskNode){
            pCtx->busy = 1;
            gettimeofday(&taskStart, NULL);
            pTaskNode->taskHandle(pTaskNode->taskId);
            logRecord(gWebServerContext->pLogCtx, LOG_LEVEL_INFO, "sdsU", "workerThread:", (int)syscall(SYS_gettid), " handle TaskId:", pTaskNode->taskId);
            gettimeofday(&taskEnd, NULL);
            pCtx->lTaskHandleCount++;
            pCtx->busy = 0;
            pCtx->executeTaskTime += (taskEnd.tv_sec - taskStart.tv_sec)*1000+(taskEnd.tv_usec - taskStart.tv_usec)/1000;
            deinitQueueItem(pTaskNode);
        }
    }
    /*@TODO:*/
    logRecord(gWebServerContext->pLogCtx, LOG_LEVEL_INFO, "sdsU", "Worker Thread:",(int)syscall(SYS_gettid)," Exit....... handle TaskCount:", pCtx->lTaskHandleCount);
    printf("worker Thread Exit........handle TaskNum:%lld\n", pCtx->lTaskHandleCount);
    if ( gWebServerContext->bExitFlag ) { /*@TODO:grace exit!*/
        pthread_mutex_lock(&(pContext->mtx));
        pContext->currentNum--;
        pthread_cond_signal(&(pContext->cond));
        pthread_mutex_unlock(&(pContext->mtx));
    }
    /*@TODO: return pCtx;*/
    pthread_exit(0);
}


bool checkWorkerThreadStautsIsBusy(ThreadContext *pCtx){
    return (pCtx->busy == 1) ? true : false;
}
