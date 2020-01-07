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
    size_t count = 0;
    logContext oldLogCtx;
    pWebCtx->bExitFlag = false;
    pWebCtx->pConfig = pConfig;
    pWebCtx->pLogCtx = logContextInit(pWebCtx, &oldLogCtx);
    pWebCtx->netWorkConext.head = createListenSocket(pConfig, &count, pWebCtx->pLogCtx);
    pWebCtx->netWorkConext.socketNum = count;
    pWebCtx->pAclCtx = aclctxInit(pWebCtx->pLogCtx);
    pWebCtx->pAclCtx = parseWhiteListAclRule(pWebCtx);
    pWebCtx->pThreadPoolContext = initThreadPoolContext(pWebCtx);
    return pWebCtx;
}

void deinitWebServerContext(webServerContext *pContext) {
    assert(pContext != NULL);
    deinitThreadPoolContext(pContext->pThreadPoolContext, pContext->pLogCtx);
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


