#ifndef _LOG_QUEUE_H_
#define _LOG_QUEUE_H_

#define MAX_PATH 255

#include <stdio.h>
//logQueue to handle logging tried word searches through multiple connections and threads

typedef struct logQueue
{
    char fileloc[MAX_PATH]; //file path of the log (MAX_PATH*sizeof char)
    FILE* fileHandle;
    int entriesCount;
    int currentMaxEntries; //handles max values so that we can realloc as needed
    char** buf; //data to log in queue
} logQueue;

void initLogQueue(logQueue** ppQueue, const char* path);
char* logPopQueue(logQueue** queue); //pop FIFO queue
void logAddToQueue(logQueue** queue, char* word); //add word to the queue

void* processLogQueue(void* param);

#endif //_LOG_QUEUE_H_
