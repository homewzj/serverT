#include "config.h"

configContext *configContextInit(logContext *pLogCtx) {
    configContext *pConfigContext = (configContext *)malloc(sizeof(configContext));
    if (!pConfigContext) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "alloc memory failure!\n");
        return NULL;
    }
    memset(pConfigContext, 0, sizeof(configContext));
    return pConfigContext;
}

void deinitConfigContext(configContext *pConfigContext, logContext *pLogCtx ) {
    logRecord(pLogCtx, LOG_LEVEL_INFO, "start to deinit config context!\n");
    assert(pConfigContext != NULL);
    if(pConfigContext->pLogPath) {
        free(pConfigContext->pLogPath);
        pConfigContext->pLogPath = NULL;
    }
    
    if(pConfigContext->pAclFilePath){
        free(pConfigContext->pAclFilePath);
        pConfigContext->pAclFilePath = NULL;
    }

    if(pConfigContext->pConfigFilePath){
        free(pConfigContext->pConfigFilePath);
        pConfigContext->pConfigFilePath = NULL;
    }
    if(pConfigContext){
        free(pConfigContext);
        pConfigContext = NULL;
    }
}

