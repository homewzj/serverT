#ifndef __MSGQUEUE_H__
#define __MSGQUEUE_H__

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


Queue *initQueue(size_t queueSize);

void deinitQueue(Queue *pQueue);

int pushQueueItem(Queue *pQueue, QueueItem *pItem);

QueueItem * popQueueItem(Queue *pQueue);

bool isQueueEmpty(Queue *pQueue);

bool isQueueFull(Queue *pQueue);

void deinitQueueItem(QueueItem *pItem);

QueueItem *createQueueItem(void *arg);

size_t QueueTaskLen(Queue *pQueue);


#endif
