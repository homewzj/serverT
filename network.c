#include "network.h"
listenSocketContext * createListenSocketItem(char *pIpAddr, int port, logContext *pLogCtx);

listenSocketContext * createListenSocketItem(char *pIpAddr, int port, logContext *pLogCtx) {
    assert(pIpAddr != NULL && port != 0);
    int iRet = RET_ERROR;
    listenSocketContext *pNode = (listenSocketContext *)malloc(sizeof(listenSocketContext));
    if (!pNode) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "ss", "malloc memory occur error:", strerror(errno));
        return NULL;
    }
    int socketfd = RET_ERROR;
    memset(pNode, 0, sizeof(listenSocketContext));
    logRecord(pLogCtx, LOG_LEVEL_INFO, "sssd", "listen ipaddr:", pIpAddr, "port:", port);
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "ss", "call socket occur error:", strerror(errno));
        goto failure;
    }
    pNode->listenfd = socketfd;
    pNode->sockaddr.sin_family = AF_INET;
    pNode->sockaddr.sin_addr.s_addr = inet_addr(pIpAddr);
    pNode->sockaddr.sin_port = htons(port);
    int opt = 1;
    iRet = setsockopt(pNode->listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    if (iRet == RET_ERROR) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "ss", "call setsockopt occur error:", strerror(errno));
        goto failure;
    }

    iRet = bind(pNode->listenfd, (struct sockaddr *)&(pNode->sockaddr), sizeof(struct sockaddr));
    if (iRet == RET_ERROR) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "ss", "call bind occur error:", strerror(errno));
        goto failure;
    }

    iRet = listen(pNode->listenfd, 1024);
    if (iRet == RET_ERROR) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "ss", "call listen occur error:", strerror(errno));
        goto failure;
    }
    pNode->next = NULL;
    return pNode;
failure:
    deinitNetWorkContext(pNode);
    return NULL;
}

/*@TODO:*/
listenSocketContext *createListenSocket(configContext *pConfig, size_t *socketNum, logContext *pLogCtx) {
    assert(pConfig != NULL);
    char *pStr = NULL;
    size_t count = *socketNum;
    char *pTmp = NULL;
    char *pPort = NULL;
    listenSocketContext *pNode = NULL;
    listenSocketContext *pHead = NULL, *pTail = NULL;
    char *pIpAddr = pConfig->pIpAddr;
    while((pStr = strtok(pIpAddr, ";"))){
        printf("parse listen socketAddr:%s\n", pStr);
        pTmp = strstr(pStr, ":");
        if (!pTmp) {
            logRecord(pLogCtx, LOG_LEVEL_ERROR, "s", "listen socketaddr format error:", strerror(errno));
            goto failure;
        }
        pPort = pTmp + 1;
        *pTmp = '\0';
        pNode = createListenSocketItem(pStr,atoi(pPort), pLogCtx);
        if (!pNode) {
            logRecord(pLogCtx, LOG_LEVEL_ERROR, "s", "call createListenSocketItem occur error");
            goto failure;
        }
        if (pHead == NULL) {
            pHead = pTail = pNode;
        } else {
            pTail->next = pNode;
            pTail = pNode;
        }
        count++;
        pIpAddr = NULL;
    }
    *socketNum = count;
    return pHead;
failure:
    if( pHead ) deinitNetWorkContext(pHead);
    return NULL;
}


void deinitNetWorkContext(listenSocketContext *pListenContext) {
    assert(pListenContext != NULL);
    listenSocketContext *pHead = pListenContext;
    while(pHead) {
        if (pHead->listenfd > 0 ) {
            close(pHead->listenfd);
        }

        if (pHead) {
            free(pHead);
            pHead = NULL;
        }
        pHead = pHead->next;
    }
}


