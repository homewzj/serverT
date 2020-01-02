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
    size_t index = 0;
    logContext oldLogCtx;
    pWebCtx->bExitFlag = false;
    pWebCtx->pConfig = pConfig;
    pWebCtx->pLogCtx = logContextInit(pWebCtx, &oldLogCtx);
    for(; index < pConfig->socketNum; index++) {
        pWebCtx->pThreadPoolContext[index] = initThreadPoolContext(pWebCtx, index);
    }
    return pWebCtx;
}

void deinitWebServerContext(webServerContext *pContext) {
    assert(pContext != NULL);
    size_t index = 0;
    for (; index < pContext->pConfig->socketNum; index++) {
        if (pContext->pThreadPoolContext[index]) {
            deinitThreadPoolContext(pContext->pThreadPoolContext[index], pContext->pLogCtx);
        }
    }
/*    if (pContext->pConfig) {
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


