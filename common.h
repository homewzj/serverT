#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <signal.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>


enum RETURN_STATUS{
    RET_ERROR = -1,
    RET_OK = 0
};

enum LOG_LEVEL{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO
};




#define  MSGQUEUE_DEFAULT_LEN  20
#define  DEFAULT_BUF_LEN  128

void taskPrintf(unsigned long long taskId);

typedef struct logContext_st{
    int   logfd;
    int (*log_callBack)(int fd, char *pLog, size_t logInfoLen);
    int   logLevel;
    char  *pLogBuf;
    size_t bufSize;
    size_t dataLen;
    char  *pLogFileName;
}logContext;

typedef struct configContext_st{
    size_t workerThreadNum; 
    time_t scanTimeOut;
    char   *pLogPath;
    char   *pAclFilePath;
    char   *pRootPath;
    int    logLevel;
    size_t logOutBufSize;
    char   *pConfigFilePath;
    size_t socketNum;
}configContext;

typedef struct queueItem_st {
    void (*taskHandle)(unsigned long long);
    unsigned long long taskId;
}QueueItem;

typedef struct queue_st{
    QueueItem **queue;
    size_t queueSize;
    volatile size_t head;
    volatile size_t tail;
}Queue;

typedef void * (*pThreadCallBack)(void *arg);

typedef struct threadContext{
    pthread_t tid;
    char *pThreadName;
    pThreadCallBack  callback;
    void *privdata;
    void *pCtx;
    size_t id;
    long long lTaskHandleCount;
    time_t executeTaskTime; 
    int  busy;
    struct threadContext *prev;
    struct threadContext *next;
    bool bExitFlag;
}ThreadContext;


#define THREADPOOL_DEFAULT_LEN  5
#define THREADPOOL_MAX_LEN 10

typedef struct threadPoolMangerSt {
    pthread_t tid;
    size_t threadInitNum; 
    size_t threadMaxNum;
    volatile unsigned int currentNum;
    pthread_mutex_t mtx;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    pthread_cond_t cond;
    ThreadContext *threadPoolHead;
    ThreadContext *threadPoolTail;
    Queue *queue;
    time_t lastScan; 
    time_t scanTimeOut;
}ThreadPoolMangerContext;


typedef struct webServerContext_st {
    logContext  *pLogCtx;
    ThreadPoolMangerContext *pThreadPoolContext[10];
    configContext  *pConfig;
    volatile bool bExitFlag;
}webServerContext;

extern webServerContext  *gWebServerContext;

#endif
