#ifndef _CONN_QUEUE_H_
#define _CONN_QUEUE_H_

typedef struct connectionQueueEntry
{
    int sock_fd;
    int in_process;
} connectionQueueEntry;

typedef struct connectionQueue
{
    int entriesCount;
    connectionQueueEntry** entries;
} connectionQueue;

void initConnectionQueue(connectionQueue** ppQueue);

void connPopQueue(connectionQueue** queue); //pop FIFO queue
connectionQueueEntry* connPeekQueue(connectionQueue** queue);
void connAddToQueue(connectionQueue** queue, int sock_fd); //add word to the queue
void* processConnectionQueue(void* param);

#endif //_CONN_QUEUE_H_
