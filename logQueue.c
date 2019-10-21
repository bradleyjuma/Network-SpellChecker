#include <string.h>
#include <stdlib.h>

#include "globals.h"


void initLogQueue(logQueue** ppQueue, const char* path)
{
    int i=0; //loop var
    
    logQueue* queue = malloc(sizeof(logQueue));
    
    strcpy(queue->fileloc, path); //file location
    
    queue->entriesCount = 0;
    queue->currentMaxEntries = DEFAULT_BUF_STRING_COUNT;
    
    queue->buf = malloc(sizeof(char*)*DEFAULT_BUF_STRING_COUNT);
    for (i=0;i<DEFAULT_BUF_STRING_COUNT;++i)
    {
        queue->buf[i] = malloc(sizeof(char)*MAX_WORD_LEN);
        queue->buf[i][0] = '\0';
    }
    
    *ppQueue = queue;
}
char* logPopQueue(logQueue** queue) //pop FIFO queue
{
    if ((*queue)->entriesCount == 0) //no entries? no pop
        return NULL;
    
    int i=0; //loop var
    
    if (strlen((*queue)->buf[0]) > 0) //first entry in queue that's not null
    {
        //copy
        char* retval = malloc(sizeof(char)*MAX_WORD_LEN);
        strcpy(retval, (*queue)->buf[0]);
        
        //free
        free((*queue)->buf[0]);
        
        for (i=1;i<(*queue)->currentMaxEntries;++i)
            (*queue)->buf[i-1] = (*queue)->buf[i];
        
        --((*queue)->entriesCount);
        return retval;
    }
    return NULL;
}

void logAddToQueue(logQueue** queue, char* word) //add word to the queue
{
    if ((*queue)->entriesCount >= (*queue)->currentMaxEntries)
    {
        (*queue)->currentMaxEntries *= 2;
        (void)realloc((*queue)->buf, sizeof(char*)*(*queue)->currentMaxEntries);
    }
    strcpy((*queue)->buf[(*queue)->entriesCount++],word);
}

void* processLogQueue(void* param)
{
    SQueues* queues = (SQueues*)param;
    logQueue* queue = (*queues).logQueueInst;
    
    while (1)
    {
        while (queue->entriesCount > 0)
        {
            char* entry = logPopQueue(&queue);
            
            queue->fileHandle = fopen(queue->fileloc, "a+"); //open a file handle
            fwrite(entry,strlen(entry)*sizeof(char),1,queue->fileHandle);
            fwrite("\r\n",sizeof(char)*2,1,queue->fileHandle);
            fclose(queue->fileHandle);
        }
    }
    
    return NULL;
    
}
