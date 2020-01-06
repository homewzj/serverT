#include "threadpool.h"

static void addWorkerThreadToPool(char *pThreadName, ThreadPoolMangerContext **pContext, logContext *pLogCtx);
static void deleteWorkerThreadFromPool(ThreadPoolMangerContext **pContext, logContext *pLogCtx);
static int scanAllWorkerThreadStatus(ThreadPoolMangerContext *pContext, logContext *pLogCtx);
static void handleWorkerThreadScanResult(int resultCode, ThreadPoolMangerContext **pContext, logContext *pLogCtx);

static void handleWorkerThreadScanResult(int resultCode, ThreadPoolMangerContext **pContext, logContext *pLogCtx){
    assert(resultCode != -1);
    switch(resultCode) {
    case RESULT_CREATE_THREAD:
        addWorkerThreadToPool("newWorkerThread", pContext, pLogCtx);
        break;
    case RESULT_DELETE_THREAD:
        deleteWorkerThreadFromPool(pContext, pLogCtx);
        break;
    case RESULT_SAVE_CURRENT:
        break;
    default:
        fprintf(stderr, "unknown resultCode:%d\n", resultCode);
        break;
    }
}

static int scanAllWorkerThreadStatus(ThreadPoolMangerContext *pContext, logContext *pLogCtx) {
    assert(pContext != NULL);
    static size_t QueueTotalCount;
    static double executeTaskConsume;
    static size_t count = 1;
    size_t queueItemNum = 0;
    double averageTaskNum = 0;
    size_t averageExecuteTaskTime = 0;
    ThreadContext *pNode = pContext->threadPoolHead;
    while(pNode) {
        if (pNode->executeTaskTime != 0 && pNode->lTaskHandleCount != 0) {
            executeTaskConsume += pNode->executeTaskTime/pNode->lTaskHandleCount;
        }
        pNode = pNode->next;
    }
    queueItemNum = QueueTaskLen(pContext->queue);
    QueueTotalCount += (double)queueItemNum;
    count++;
    if ( count % 5 == 0 ){
        averageTaskNum = (double)QueueTotalCount / 5;
        averageExecuteTaskTime = (double )executeTaskConsume / 5;
        QueueTotalCount = 0, count = 1, executeTaskConsume = 0;
        averageTaskNum = averageTaskNum /(pContext->queue->queueSize);
        logRecord(pLogCtx, LOG_LEVEL_INFO, "su", "ThreadPool Manger Has Worker Thread Num:", pContext->currentNum);
        printf("averageTaskNum:%.2f  averageExecuteTaskTime:%ld\n", averageTaskNum, averageExecuteTaskTime);
        if ( averageTaskNum >= 0.5 && averageExecuteTaskTime > 150) {
            return RESULT_CREATE_THREAD;
        }else if ( averageTaskNum < 0.5 || averageExecuteTaskTime < 100) {
            return RESULT_DELETE_THREAD;
        }
        return RESULT_SAVE_CURRENT;
    }
    return RESULT_SAVE_CURRENT;
}

ThreadPoolMangerContext *initThreadPoolContext(webServerContext *pWebContext, size_t id, int pipefd) {
    assert(pWebContext!=NULL);
    size_t index = 0;
    ThreadPoolMangerContext *pContext = (ThreadPoolMangerContext *)malloc(sizeof(ThreadPoolMangerContext));
    if (!pContext) {
        logRecord(pWebContext->pLogCtx, LOG_LEVEL_ERROR, "s", "ThreadPoolManger Context alloc memory failure!\n");
        return NULL;
    }
    configContext *pConfig = pWebContext->pConfig;
    logContext *pLogCtx = pWebContext->pLogCtx;
    pContext->lastScan = time(NULL);
    pContext->scanTimeOut =  pConfig->scanTimeOut;
    pContext->threadInitNum = ( pConfig->workerThreadNum > 0 ) ? pConfig->workerThreadNum : THREADPOOL_DEFAULT_LEN;
    pContext->threadMaxNum = THREADPOOL_MAX_LEN;
    pContext->currentNum = pContext->threadInitNum;
    pContext->pipefd = pipefd;
    pthread_mutex_init(&(pContext->mtx), NULL);
    pthread_cond_init(&(pContext->not_full), NULL);
    pthread_cond_init(&(pContext->not_empty), NULL);
    pthread_cond_init(&(pContext->cond), NULL);
    pContext->queue = initQueue(MSGQUEUE_DEFAULT_LEN);
    if (!(pContext->queue)) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "s","ThreadPoolManger Context Init msgQueue failure\n");
        goto failure;
    }

    for (; index < pContext->threadInitNum; index++) {
        ThreadContext *pNode = initThreadContext("workerThread", pLogCtx);
        if (!pContext->threadPoolHead ) {
            pContext->threadPoolHead = pContext->threadPoolTail = pNode;
        }else{
            pContext->threadPoolTail->next = pNode;
            pNode->prev = pContext->threadPoolTail;
            pContext->threadPoolTail = pNode;
        }
        pNode->pCtx = (void *)pWebContext;
        pContext->index = pNode->id = id;
    }
    return pContext;
failure:
    deinitThreadPoolContext(pContext, pLogCtx);
    return NULL;
}


void deinitThreadPoolContext(ThreadPoolMangerContext * pContext, logContext *pLogCtx) {
    logRecord(pLogCtx, LOG_LEVEL_INFO, "s", "ThreadPoolManger Context Deinit start");
    assert(pContext != NULL);
    ThreadContext *pNode = pContext->threadPoolHead;
    while(pNode) {
        deinitThreadContext(pNode, pLogCtx);
        pNode = pNode->next;
    }
    
    if (pContext->queue) {
        deinitQueue(pContext->queue);
    }
    
    pthread_mutex_destroy(&(pContext->mtx));
    pthread_cond_destroy(&(pContext->not_full));
    pthread_cond_destroy(&(pContext->not_empty));
    pthread_cond_destroy(&(pContext->cond));
    return;
}

void threadPoolSignalHandler(int signo) {
    switch(signo) {
    case SIGINT:
    case SIGTERM:
        logRecord(gWebServerContext->pLogCtx, LOG_LEVEL_INFO, "sd", "ThreadPool Context catch signal:", signo);
        gWebServerContext->bExitFlag = true; 
        break;
    default:
        break;
    }
}

void ThreadPoolMangerRun(ThreadPoolMangerContext *pContext) {
    int resultCode = RESULT_SAVE_CURRENT;
    static unsigned long long taskId = 0;
    assert(pContext != NULL);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, threadPoolSignalHandler);
    signal(SIGTERM, threadPoolSignalHandler);
    /*@TODO:register signal handle to SIGINT, SIGTERMINAL*/
    while(!(gWebServerContext->bExitFlag)) {
        /*@step1:*/        
        if (time(NULL) - pContext->lastScan >= pContext->scanTimeOut) {
            resultCode = scanAllWorkerThreadStatus(pContext, gWebServerContext->pLogCtx);
            handleWorkerThreadScanResult(resultCode, &pContext, gWebServerContext->pLogCtx);
            pContext->lastScan = time(NULL);
        }
        /*@step2:*/
        pthread_mutex_lock(&(pContext->mtx));
        while( gWebServerContext->bExitFlag != true && isQueueFull(pContext->queue)){
            pthread_cond_wait(&(pContext->not_full), &(pContext->mtx));
        }
        QueueItem *pTaskNode = createQueueItem((void *)&taskId);
        if(pTaskNode) pushQueueItem(pContext->queue, pTaskNode);
        pthread_cond_signal(&(pContext->not_empty));
        pthread_mutex_unlock(&(pContext->mtx));
        usleep(500);
        taskId++;
    }

    pthread_cond_broadcast(&(pContext->not_empty));
    logRecord(gWebServerContext->pLogCtx, LOG_LEVEL_INFO, "susU", "ThreadPool Manger Context Exit!... worker Thread Num:", pContext->currentNum, "create taskNum:", taskId);
    printf("Thread Pool Manger Exit!....worker Thread Num:%u  create taskNum:%llu\n", pContext->currentNum, taskId);
    pthread_mutex_lock(&(pContext->mtx));
    while(pContext->currentNum > 0) {
        pthread_cond_wait(&(pContext->cond), &(pContext->mtx));
    }
    pthread_mutex_unlock(&(pContext->mtx));
    logRecord(gWebServerContext->pLogCtx, LOG_LEVEL_INFO, "s","ThreadPool Manger Context Exit Successfully!");
    return;
}

static void addWorkerThreadToPool(char *pThreadName, ThreadPoolMangerContext **pContext, logContext *pLogCtx) {
    int iRet = RET_ERROR;
    ThreadContext *pNode = NULL;
    ThreadPoolMangerContext *pCtx = *pContext;
    assert(pCtx != NULL && pThreadName != NULL && pLogCtx != NULL);
    if ( pCtx->currentNum < pCtx->threadMaxNum) {
        pNode = initThreadContext(pThreadName, pLogCtx);
        pNode->id = pCtx->index;
        pCtx->threadPoolTail->next = pNode;
        pNode->prev = pCtx->threadPoolTail;
        pCtx->threadPoolTail = pNode;
        pCtx->currentNum++;
        pNode->pCtx = (void *)gWebServerContext;
        iRet = pthread_create(&(pNode->tid), NULL, pNode->callback, (void *)pNode);
        if ( iRet < 0 ) {
            logRecord(pLogCtx, LOG_LEVEL_ERROR, "s","ThreadPool Manger create New Worker Thread Failure!\n");
            goto failure;
        }
    }else{
        logRecord(pLogCtx, LOG_LEVEL_WARN, "su", "ThreadPool Manger Has reached Max Worker Thread Num:",pCtx->currentNum);
        printf("current ThreadPool has reach max thread Num:%u\n", pCtx->currentNum);
    }

    *pContext = pCtx;
    return;
failure:
    *pContext = pCtx;
    deinitThreadContext(pNode, pLogCtx);
    return;
}

static void deleteWorkerThreadFromPool(ThreadPoolMangerContext **pContext, logContext *pLogCtx) {
    ThreadPoolMangerContext *pCtx = *pContext;
    assert(pCtx != NULL );
    if (pCtx->currentNum == 1) {
        logRecord(pLogCtx, LOG_LEVEL_INFO, "s", "ThreadPool Manger Must Has One Worker Thread!\n");
        printf("current threadpool workerThread Num:%u\n", pCtx->currentNum);
        return;
    }
    ThreadContext *pNode = pCtx->threadPoolHead;
    while( pNode!= NULL && checkWorkerThreadStautsIsBusy(pNode)) {
        pNode = pNode->next;
    }
    if(pNode && !checkWorkerThreadStautsIsBusy(pNode)) {
        pNode->bExitFlag = true; /*@TODO:*/
        ThreadContext *next = pNode->next;
        ThreadContext *prev = pNode->prev;
        if(prev) prev->next = next;
        if(next) next->prev = prev;
        if(pNode == pCtx->threadPoolHead) {
            pCtx->threadPoolHead = next;
        }
        pCtx->currentNum--;
    }
    *pContext = pCtx;
    return;
}


int childProcessCreateThread(ThreadPoolMangerContext *pThreadContext, logContext *pLogCtx) {
    int iRet = RET_ERROR;
    ThreadContext *pNode = pThreadContext->threadPoolHead;
    while(pNode != NULL ) {
        iRet = pthread_create(&(pNode->tid), NULL, pNode->callback, (void *)pNode);
        if (iRet < 0) {
            logRecord(pLogCtx, LOG_LEVEL_ERROR, "s", "ThreadPoolManger Context Create WorkerThread Failure!\n");
            goto failure;
        }
        pNode = pNode->next;
    }
    return RET_OK;
failure:
    deinitThreadPoolContext(pThreadContext, pLogCtx);
    return RET_ERROR;
}

