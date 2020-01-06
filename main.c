#include "server.h"

webServerContext *gWebServerContext = NULL;

int main(int argc, char *argv[]) {
    int iRet = RET_ERROR;
    configContext pConfig;
    pConfig.pLogPath = strdup(".");
    pConfig.logLevel = LOG_LEVEL_DEBUG;
    pConfig.logOutBufSize = 1024;
    pConfig.workerThreadNum = 2;
    pConfig.scanTimeOut = 2;
    pConfig.socketNum = 2;
    gWebServerContext = initWebServerContext(&pConfig);
    iRet = processServerContext(gWebServerContext, &pConfig); 
    deinitWebServerContext(gWebServerContext);
    return RET_OK;
}
