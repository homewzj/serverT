#include "server.h"

webServerContext *gWebServerContext = NULL;

int main(int argc, char *argv[]) {
    int iRet = RET_ERROR;
    configContext pConfig;
    pConfig.pLogPath = strdup(".");
    pConfig.logLevel = LOG_LEVEL_DEBUG;
    pConfig.logOutBufSize = 1024;
    pConfig.workerThreadNum = 2;
    pConfig.scanTimeOut = 5;
    pConfig.socketNum = 2;
    pConfig.pIpAddr =strdup("127.0.0.1:8080;127.0.0.1:8090");
    pConfig.pAclList = strdup("192.168.1.100/8,10.10.12.22/16, 192.168.100.100");
    gWebServerContext = initWebServerContext(&pConfig);
    iRet = createWorkerThread(gWebServerContext->pThreadPoolContext);
    if ( iRet != RET_OK ) {
        logRecord(gWebServerContext->pLogCtx, LOG_LEVEL_ERROR, "s", "create Worker Thread occur error:", strerror(errno));
        goto failure;
    }
    iRet = ThreadPoolMangerRun(gWebServerContext);
    if ( iRet != RET_OK ) {
        logRecord(gWebServerContext->pLogCtx, LOG_LEVEL_ERROR, "s", "Thread Pool Manger Exit occur error:", strerror(errno));
        goto failure;
    }
    deinitWebServerContext(gWebServerContext);
    return RET_OK;
failure:
    deinitWebServerContext(gWebServerContext);
    return RET_ERROR;
}
