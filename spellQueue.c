#include <string.h>
#include <stdlib.h>

#include "globals.h"

void initSpellQueue(spellQueue** ppQueue)
{
    int i = 0;
    
    spellQueue* queue = malloc(sizeof(spellQueue));
    spellQueueEntry** entries = malloc(sizeof(spellQueueEntry*)*MAX_SPELL_ENTRIES);
    
    queue->entries = entries;
    
    for (i = 0; i<MAX_SPELL_ENTRIES; ++i)
        queue->entries[i] = malloc(sizeof(spellQueueEntry));
    
    queue->num_entries = 0;
    *ppQueue = queue;
}

spellQueueEntry* popSpellQueueEntry(spellQueue** queue)
{
    if ((*queue)->num_entries == 0) //no entries? no pop
        return NULL;
    
    int i=0; //loop var
    
    if ((*queue)->entries[0] != NULL) //first entry in (*queue) that's not null
    {
        spellQueueEntry* retval = NULL; //return value. copy from input parameter
        retval = malloc(sizeof(spellQueueEntry));
        //copy values
        strcpy(retval->word,(*queue)->entries[0]->word);
        retval->mutx = (*queue)->entries[0]->mutx;
        retval->cond = (*queue)->entries[0]->cond;
        
        //free and pop entry from (*queue)
        free((*queue)->entries[0]);
        (*queue)->entries[0] = NULL;
        
        for (i=1; i<MAX_SPELL_ENTRIES-1; ++i)
            //reorder (*queue) so that next value is the first one. FIFO
            (*queue)->entries[i-1] = (*queue)->entries[i];
        
        --(*queue)->num_entries;
        
        //set all unused entries in array to NULL
        for (i=(*queue)->num_entries;i<MAX_SPELL_ENTRIES;++i)
            (*queue)->entries[i] = NULL;
        
        return retval;
    }
}

void addSpellQueueEntry(spellQueue** queue, char* word, pthread_cond_t* cond_inst, pthread_mutex_t* mutx_inst) //add a new entry to the spell queue to process
{
    spellQueueEntry* entry = NULL; //entry to add
    
    entry = malloc(sizeof(spellQueueEntry)); //initialize and copy values
    strcpy(entry->word,word);
    entry->cond = cond_inst;
    entry->mutx = mutx_inst;
    
    (*queue)->entries[(*queue)->num_entries++] = entry; //add to end of list as specified by num_entries, and increment the count
}

void* processSpellQueue(void* param) //worker thread function for spelling checker queue
{
    SQueues* queues = (SQueues*)param;
    spellQueue* queue = (*queues).spellCheckQueue;
    solvedSpellList* solvedQueue = (*queues).solvedSpellCheckList;
    char* lpDictionary = (*queues).lpDictionary;
    
    while (1)
    {
        while (queue->num_entries > 0)
        {
            //remove it and grab it
            spellQueueEntry* entry = popSpellQueueEntry(&queue);
            char* tok;
            char* tmpDict = malloc(sizeof(char)*strlen(lpDictionary)+1);
            int found = 0;
            //tmpDict for a temporary storage for the dictionary as to not mangle the file data with strtok()
            strcpy(tmpDict, lpDictionary);
            
            //go to our dictionary file and search for the word
            tok = strtok(tmpDict,"\r\n");
            
            while (tok != NULL)
            {
                if (strcmp(tok, entry->word) == 0) //did we find the word?
                {
                    found = 1;
                    break;
                }
                tok = strtok(NULL,"\r\n");
            }
            //add to solved list
            addSolvedWord(&solvedQueue, entry->word,found);
            //signal that entry's condition mutex that it's been processed
            pthread_mutex_lock(entry->mutx);
            pthread_cond_signal(entry->cond);
            pthread_mutex_unlock(entry->mutx);
            //free our data. strtok doesn't do it for us whether it's done or not
            free(tmpDict);
        }
    }
    
    return NULL;
}

void destroySpellQueue(spellQueue** queue)
{
    int i = 0;
    for (i=0;i<(*queue)->num_entries;++i)
        free((*queue)->entries[i]);
    free((*queue));
}
