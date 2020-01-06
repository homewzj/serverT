#include "msgQueue.h"

Queue *initQueue(size_t queueSize) {
    assert(queueSize != 0);
    int index = 0;
    Queue *pQueue = (Queue *)malloc(sizeof(Queue));
    if (!pQueue) {
        return NULL;
    }
    pQueue->queue = (QueueItem **)malloc(sizeof(QueueItem *) * queueSize);
    if (!(pQueue->queue)) {
        goto failure;
    }
    for (; index < queueSize ; index++ ) {
        pQueue->queue[index] = NULL;
    }
    pQueue->queueSize = queueSize;
    pQueue->tail = pQueue->head = 0;
    return pQueue;
failure:
    deinitQueue(pQueue);
    return NULL;
}

void deinitQueue(Queue *pQueue) {
    assert(pQueue != NULL);
    if (pQueue->queue) {
        free(pQueue->queue);
        pQueue->queue = NULL;
    }
    free(pQueue);
    pQueue = NULL;
}

int pushQueueItem(Queue *pQueue, QueueItem *pItem) {
    assert(pQueue != NULL && pItem != NULL );
    pQueue->queue[pQueue->head] = pItem;
    pQueue->head++;
    if (pQueue->head >= pQueue->queueSize) {
        pQueue->head = 0;
    }
    return 0;
}

bool isQueueEmpty(Queue *pQueue) {
    return (pQueue->head - pQueue->tail == 0) ? true : false;
}

bool isQueueFull(Queue *pQueue) {
    return (((pQueue->head + 1)%(pQueue->queueSize)) == pQueue->tail) ? true : false;
}

QueueItem *popQueueItem(Queue *pQueue) {
    assert(pQueue != NULL && pQueue->queue != NULL);
    QueueItem *pItem = pQueue->queue[pQueue->tail];
    pQueue->queue[pQueue->tail] = NULL;
    pQueue->tail++;
    if ( pQueue->tail >= pQueue->queueSize ) {
        pQueue->tail = 0;
    }
    return pItem;
}

void taskPrintf(unsigned long long taskId) {
    printf("taskid:%llu\n", taskId);
    sleep(3);
    return;
}

QueueItem *createQueueItem(void *arg) {
    QueueItem *pNode = NULL;
    pNode = (QueueItem *)malloc(sizeof(QueueItem));
    if (!pNode) {
        return NULL;
    }
    pNode->taskId = *(unsigned long long *)arg; 
    pNode->taskHandle = taskPrintf;
    return pNode;
}

size_t QueueTaskLen(Queue *pQueue){
    if ( pQueue->head  >= pQueue->tail ) {
        return (pQueue->head  -  pQueue->tail);
    } else {
        return (pQueue->head + pQueue->queueSize - 1 - pQueue->tail);
    }
}

void deinitQueueItem(QueueItem *pItem) {
    assert(pItem != NULL);
    pItem->taskHandle = NULL;
    free(pItem);
    pItem = NULL;
}



