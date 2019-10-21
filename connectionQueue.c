#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "globals.h"

void initConnectionQueue(connectionQueue** ppQueue)
{
    connectionQueue* queue = malloc(sizeof(connectionQueue));
    int i;
    
    queue->entriesCount = 0;
    queue->entries = malloc(sizeof(connectionQueueEntry*)*MAX_CONNECTIONS);
    for (i=0;i<MAX_CONNECTIONS;++i)
        queue->entries[i] = NULL;
    *ppQueue = queue;
}

void connPopQueue(connectionQueue** queue) //pop FIFO queue
{
    pthread_mutex_lock(&connectionQueueVacancyMutex);
    while ((*queue)->entriesCount == 0) //no entries? no pop
        pthread_cond_wait(&connectionQueueOccupiedCondition, &connectionQueueVacancyMutex);
    
    //    return 0;
    int i; //loop var
    
    free((*queue)->entries[0]);
    
    for (i=1;i<MAX_CONNECTIONS;++i)
        (*queue)->entries[i-1] = (*queue)->entries[i];
    
    --((*queue)->entriesCount);
    
    if ((*queue)->entriesCount+1 == MAX_CONNECTIONS) //if we just popped a sock fd from a full queue
        pthread_cond_signal(&connectionQueueVacancyCondition); //signal that there is a vacancy in the connection queue to stop pthread_cond_wait'ing
    
    pthread_mutex_unlock(&connectionQueueVacancyMutex);
}

connectionQueueEntry* connPeekQueue(connectionQueue** queue)
{
    if ((*queue)->entriesCount > 0)
        return ((*queue)->entries[0]);
}

void connAddToQueue(connectionQueue** queue, int sock_fd) //add word to the queue
{
    pthread_mutex_lock(&connectionQueueVacancyMutex);
    //locks the connectionQueueVacancyMUTEX until connectionQueueVacancyCONDITION is signaled by pthread_cond_signal (in processConnectionQueue)
    while ((*queue)->entriesCount >= MAX_CONNECTIONS)
        pthread_cond_wait(&connectionQueueVacancyCondition, &connectionQueueVacancyMutex);
    
    connectionQueueEntry* entry = malloc(sizeof(connectionQueueEntry));
    entry->in_process = 0;
    entry->sock_fd = sock_fd;
    
    (*queue)->entries[(*queue)->entriesCount++] = entry;
    pthread_cond_signal(&connectionQueueOccupiedCondition);
    pthread_mutex_unlock(&connectionQueueVacancyMutex);
}

void* processConnectionQueue(void* param)
{
    SQueues* queues = (SQueues*)param;
    connectionQueue* queue = (*queues).connectionQueueInst;
    spellQueue* spellCheckQueue = (*queues).spellCheckQueue;
    solvedSpellList* solvedSpellCheckList = (*queues).solvedSpellCheckList;
    logQueue* logQueueInst = (*queues).logQueueInst;
    
    while (1)
    {
        while (queue->entriesCount > 0)
        {
            connectionQueueEntry* usedEntry;
            if (queue->entries[0]->in_process) //Check if entry is in process already. If so, don't use it
            {
                int i = 1; //loop var
                int found_usable_entry = 0; //Found usable entry placeholder
                do
                {
                    if (queue->entries[i] != NULL && !queue->entries[i]->in_process)
                    {
                        usedEntry = queue->entries[i];
                        queue->entries[i]->in_process = 1;
                        found_usable_entry = 1; //if we find something, mark that we did
                        break;
                    }
                } while (++i < queue->entriesCount);
                if (!found_usable_entry) //we found nothing. Do not continue in the worker thread.
                    break;
                
            }
            else
            {
                usedEntry = queue->entries[0];
                queue->entries[0]->in_process = 1;
            }
            
            char wordBuf[MAX_WORD_LEN] = {0};
            int words_to_process = 0;
            int i = 0;
            
            //TODO:
            /*
             * Load dictionary file into memory - DONE
             * Receive number of words from the client - DONE
             * Send the "GOAHEAD" signal - DONE
             * Receive each word - DONE
             * Process them with the dictionary file (find the word in the dictionary file (reading line by line) - DONE
             * Send "OK" if you find anything or "MISSPELLED" if you don't - DONE
             *
             */
            int recval = recv(usedEntry->sock_fd, wordBuf, sizeof(int), 0);
            int multi_client = 0;
            if(recval <= 0) break;
            else wordBuf[recval] = '\0';         //Receive number of words from the client
            words_to_process = *(int*)wordBuf;   //take the first sizeof(int) bytes from wordBuf and translate them to an int pointer
            //(saying that our memory here is a pointer to a memory location holding an integer in byte form)
            //and then dereference said pointer into an integer
            //ex. wordBuf = [0x00, 0x00, 0x00, 0x10, ...] (or in binary - [0001][0000] | *(int*)wordBuf == 16
            if (words_to_process == 0)
            {
                multi_client = 1;
                words_to_process = 2;
            }
            //send
            send(usedEntry->sock_fd, GOAHEAD, sizeof(char)*(strlen(GOAHEAD)+1), 0);
            for (i=0;i<words_to_process;++i)
            {
                if (multi_client)
                    --i;
                
                int recval = recv(usedEntry->sock_fd, wordBuf, sizeof(char)*MAX_WORD_LEN, 0);
                if(recval <= 0) break;
                else wordBuf[recval] = '\0';
                printf("Client[%d] %s\n", usedEntry->sock_fd, wordBuf);
                
                if (strcmp(wordBuf, END) == 0) //escape character pressed in client. the client has ended our session early
                {
                    printf("Client[%d] - Early disconnect\n", usedEntry->sock_fd);
                    send(usedEntry->sock_fd, END, sizeof(char)*(strlen(END)+1), 0);
                    break;
                }
                
                //pthread condition variable for every word processed so we can have accurate
                //timing on when the word is question has been spellchecked
                pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
                pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;
                pthread_mutex_lock(&mutx);
                addSpellQueueEntry(&spellCheckQueue, wordBuf, &cond, &mutx); //add word to queue
                
                printf("Client[%d] - Waiting for word %s to be processed\n", usedEntry->sock_fd, wordBuf);
                while (findSolvedWord(&solvedSpellCheckList, wordBuf) == 0)
                    pthread_cond_wait(&cond,&mutx); //wait on word to be processed
                printf("Client[%d] - Word %s processing complete\n", usedEntry->sock_fd, wordBuf);
                pthread_mutex_unlock(&mutx);
                
                int success = getSolvedWordSuccess(&solvedSpellCheckList, wordBuf); //get whether it was in the dictionary
                removeSolvedWord(&solvedSpellCheckList, wordBuf); //remove from the list
                //send appropriate response
                char logEntry[255] = {0};
                strcat(logEntry,wordBuf);
                //Switched from a if to a case due to identical code structure... Why not..
                switch (success)
                {
                    case 0:
                        send(usedEntry->sock_fd, MISSPELLED, sizeof(char)*(strlen(MISSPELLED)+1), 0 );
                        logAddToQueue(&logQueueInst,strcat(logEntry, " - MISSPELLED"));
                        printf("Client[%d] - Word %s was not found. Sent %s\n", usedEntry->sock_fd, wordBuf, MISSPELLED);
                        printf("Client[%d] - Log append: %s\n", usedEntry->sock_fd, logEntry);
                        break;
                    case 1:
                        send(usedEntry->sock_fd, OK, sizeof(char)*(strlen(OK)+1), 0 );
                        logAddToQueue(&logQueueInst,strcat(logEntry, " - CORRECT"));
                        printf("Client[%d] - Word %s was found. Sent %s\n", usedEntry->sock_fd, wordBuf, OK);
                        printf("Client[%d] - Log append: %s\n", usedEntry->sock_fd, logEntry);
                        break;
                }
                
            }
            printf("Client[%d] - Popping connection from queue\n", usedEntry->sock_fd);
            connPopQueue(&queue);
        }
    }
    
    return NULL;
    
}
