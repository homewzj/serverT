#include "server.h"

int parseCmdOption(int argc, char *argv[]) {
    return RET_OK;
}

webServerContext * initWebServerContext(configContext *pConfig) {
    webServerContext * pWebCtx = (webServerContext *)malloc(sizeof(webServerContext));
    if (!pWebCtx) {
        /*@TODO:call logWriteFunc*/
        return NULL;
    }
    logContext oldLogCtx;
    pWebCtx->bExitFlag = false;
    pWebCtx->pConfig = pConfig;
    pWebCtx->pLogCtx = logContextInit(pWebCtx, &oldLogCtx);
    return pWebCtx;
}

void deinitWebServerContext(webServerContext *pContext) {
    assert(pContext != NULL);
    size_t index = 0;
    for (; index < pContext->pConfig->socketNum; index++) {
        deinitThreadPoolContext(pContext->pThreadPoolContext[index], pContext->pLogCtx);
    }
    /*if (pContext->pConfig) {
        deinitConfigContext(pContext->pConfig, pContext->pLogCtx);
    }*/
    if (pContext->pLogCtx) {
        deinitLogContext(pContext->pLogCtx);
    }

    if (pContext) {
        free(pContext);
        pContext = NULL;
    }
}


int processServerContext(webServerContext *pContext, configContext *pConfig) {
    assert(pContext != NULL && pConfig != NULL);
    size_t index = 0;
    int iRet = RET_ERROR;
    int pipe[2] = {0, 0};
    ThreadPoolMangerContext *pThreadPoolContext = NULL;

    for (; index < pConfig->socketNum; index++) {
        iRet = socketpair(AF_LOCAL,SOCK_STREAM, 0, pipe);
        if (iRet == RET_ERROR) {
            logRecord(pContext->pLogCtx, LOG_LEVEL_ERROR, "ss", "create socketpair occur:", strerror(errno));
            return iRet;
        }

        printf("create socketpair:%d %d\n", pipe[0], pipe[1]);
        logRecord(pContext->pLogCtx, LOG_LEVEL_ERROR, "sdd", "create socketpair:", pipe[0], pipe[1]);
        pThreadPoolContext = initThreadPoolContext(pContext, index, pipe[1]);
        pContext->pThreadPoolContext[index] = pThreadPoolContext;
        pContext->pipefd[index] = pipe[0];
        pid_t pid = fork();
        if (pid == 0 ) {
            iRet = childProcessCreateThread(pThreadPoolContext, pContext->pLogCtx);
            if ( iRet == RET_OK ) {
            //@TODO:init ThreadPool Context;
                 ThreadPoolMangerRun(pThreadPoolContext);
            } else {
                //@TODO:error;
            }
        }else if (pid < 0) {
            //error;
            logRecord(pContext->pLogCtx, LOG_LEVEL_ERROR, "ss", "create child process occur:", strerror(errno));
            return iRet;
        }
        printf("index:%ld  ThreadPoolManger:%p   pipefd:%d\n", index, pContext->pThreadPoolContext[index], pContext->pipefd[index]);
    }

    for(;;){

    }

    return RET_OK;
}
