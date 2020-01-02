#include "server.h"

webServerContext *gWebServerContext = NULL;

int main(int argc, char *argv[]) {
    int iRet = RET_ERROR;
    size_t index = 0;
    configContext pConfig;
    pConfig.pLogPath = strdup(".");
    pConfig.logLevel = LOG_LEVEL_DEBUG;
    pConfig.logOutBufSize = 1024;
    pConfig.workerThreadNum = 10;
    pConfig.scanTimeOut = 1000;
    pConfig.socketNum = 4;
    gWebServerContext = initWebServerContext(&pConfig);
    for( ; index < pConfig.socketNum; index++ ) {
        iRet = ThreadPoolMangerRun(gWebServerContext->pThreadPoolContext[index], gWebServerContext->pLogCtx);
    }
    deinitWebServerContext(gWebServerContext);
    return RET_OK;
}
