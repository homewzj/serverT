#include "log.h"

void logRecord(logContext *pLogCtx, int level, const char *fmt, ...) {
    assert(pLogCtx != NULL);
    size_t bufSize = 0;
    time_t currentTime = time(NULL);
    const char *pLogLevelInfo[] = {
        "Debug",
        "Error",
        "Warn",
        "Info"
    };
    int id = 0;
    char *pInfo = NULL;
    char *pLogOut = NULL;
    size_t num = 0; 
    unsigned long long count = 0;
    char szTime[DEFAULT_BUF_LEN] =  {'\0'};
    char pLogBuf[DEFAULT_BUF_LEN] = {'\0'};
    struct tm *pLogRecordTime = localtime(&currentTime);
    strftime(szTime, DEFAULT_BUF_LEN, "%Y-%m-%d %H:%M:%S", pLogRecordTime);
    pLogCtx->dataLen = 0;
    pLogOut = (pLogCtx->pLogBuf != NULL ) ? pLogCtx->pLogBuf : pLogBuf; 
    bufSize = (pLogCtx->pLogBuf != NULL ) ? pLogCtx->bufSize : DEFAULT_BUF_LEN;
    pLogOut[0] = '\0';
    if (level >= pLogCtx->logLevel) {
        pLogCtx->dataLen += snprintf(pLogOut, bufSize, "%s|%s|", szTime, pLogLevelInfo[level]);
        va_list ap;
        va_start(ap, fmt);
        while(*fmt){
            switch(*fmt++){
            case 's':
                pInfo = va_arg(ap, char *);
                pLogCtx->dataLen += snprintf(pLogOut + pLogCtx->dataLen, bufSize - pLogCtx->dataLen, "%s", pInfo);
                break;
            case 'd':
                id = va_arg(ap, int);
                pLogCtx->dataLen += snprintf(pLogOut + pLogCtx->dataLen, bufSize - pLogCtx->dataLen, "%d", id);
                break;
            case 'u':
                num = va_arg(ap, size_t);
                pLogCtx->dataLen += snprintf(pLogOut + pLogCtx->dataLen, bufSize - pLogCtx->dataLen, "%ld", num);
                break;
            case 'U':
                count = va_arg(ap, unsigned long long);
                pLogCtx->dataLen += snprintf(pLogOut + pLogCtx->dataLen, bufSize - pLogCtx->dataLen, "%llu", count);
                break;
            default:
                fprintf(stderr, "unknown format is supported!\n");
                break;
            }
        }
        va_end(ap);
        pLogOut[pLogCtx->dataLen++] = '\n';
        pLogOut[pLogCtx->dataLen++] = '\0';
        if (pLogCtx->logfd != fileno(stdout)){
            pLogCtx->log_callBack(pLogCtx->logfd, pLogOut, pLogCtx->dataLen);
        } else {
            fprintf(stdout, pLogBuf);
            fflush(stdout);
        }
    }
}

logContext * logContextInit(webServerContext *pWebContext, logContext *pOldLogCtx){
    assert(pWebContext != NULL);
    size_t dataLen = 0;
#define LOG_PREFIX_MAX_LEN 100
    char pLogPath[LOG_PREFIX_MAX_LEN] = {'\0'};
    char pLogName[LOG_PREFIX_MAX_LEN] = {'\0'};
    time_t createTime = time(NULL);
    logContext *pLogCtx = (logContext *)malloc(sizeof(logContext));
    if (!pLogCtx) {
        logRecord(pOldLogCtx, LOG_LEVEL_ERROR,"s","alloc memory failure!\n");
        return NULL;
    }
    struct tm *logtime = localtime(&createTime);
    strftime(pLogName, LOG_PREFIX_MAX_LEN, "%Y%m%d%H%M%S", logtime);
    snprintf(pLogName + strlen(pLogName), LOG_PREFIX_MAX_LEN - strlen(pLogName), "%s", ".log");
    dataLen += snprintf(pLogPath + dataLen, LOG_PREFIX_MAX_LEN - dataLen, "%s/%s", pWebContext->pConfig->pLogPath, pLogName);
    pLogCtx->pLogFileName = strdup(pLogPath);
    pLogCtx->logfd = open(pLogCtx->pLogFileName, O_RDWR|O_APPEND|O_CREAT, 0644);
    if ( pLogCtx->logfd == RET_ERROR ) {
        logRecord(pOldLogCtx, LOG_LEVEL_ERROR,"s","loc memory failure!\n");
        goto failure;
    }
    pLogCtx->logLevel = pWebContext->pConfig->logLevel;
    pLogCtx->log_callBack = logWriteCallBack;
    pLogCtx->bufSize = pWebContext->pConfig->logOutBufSize;
    pLogCtx->pLogBuf = (char *)malloc(pLogCtx->bufSize);
    if (!(pLogCtx->pLogBuf)) {
        logRecord(pOldLogCtx, LOG_LEVEL_ERROR,"s",strerror(errno));
        goto failure;
    }
    pLogCtx->dataLen = 0;
    pLogCtx->logLevel =pWebContext->pConfig->logLevel;
    return pLogCtx;
failure:
    deinitLogContext(pLogCtx);
    return NULL;
}

void deinitLogContext(logContext *pLogCtx) {
    assert(pLogCtx != NULL);
    if(pLogCtx->logfd != 0) {
        close(pLogCtx->logfd);
    }

    if(pLogCtx->pLogFileName){
        free(pLogCtx->pLogFileName);
        pLogCtx->pLogFileName = NULL;
    }

    if(pLogCtx->pLogBuf) {
        free(pLogCtx->pLogBuf);
        pLogCtx->pLogBuf = NULL;
    }

    close(pLogCtx->logfd);
    if(pLogCtx) {
        free(pLogCtx);
        pLogCtx = NULL;
    }
}

int logWriteCallBack(int fd, char *pLogInfo, size_t logSize) {
    size_t written = 0;
    size_t ret = 0;
    for(;;) {
        ret = write(fd, pLogInfo + written, logSize - written);
        if ( ret == RET_ERROR ) {
            if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
                continue;
            }
            return RET_ERROR;
        } else if ( ret == RET_OK ) {
            break;
        }
        written += ret;
    }
    return RET_OK;
}
