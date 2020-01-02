#include "log.h"


int main(int argc, char *argv[]) {
    logContext oldLogCtx;
    memset(&oldLogCtx, 0, sizeof(logContext));
    configContext pConfig;
    pConfig.pLogPath = strdup(".");
    pConfig.logLevel = LOG_LEVEL_DEBUG;
    pConfig.logOutBufSize = 1024;
    logContext *pLogCtx = logContextInit(&pConfig, &oldLogCtx);
    if (!pLogCtx) {
        return -1;
    }
    logRecord(pLogCtx, LOG_LEVEL_ERROR, "s", "error\n");
    logRecord(pLogCtx, LOG_LEVEL_WARN,  "s", "warn\n");
    logRecord(pLogCtx, LOG_LEVEL_DEBUG, "s", "debug\n");
    logRecord(pLogCtx, LOG_LEVEL_INFO, "s", "info\n");
    deinitLogContext(pLogCtx);
    return 0;
}
