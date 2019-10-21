#ifndef _SPELL_QUEUE_H_
#define _SPELL_QUEUE_H_

#define MAX_WORD_LEN 32

#include <pthread.h>

typedef struct spellQueueEntry
{
    char word[MAX_WORD_LEN]; //word searched
    pthread_cond_t* cond;
    pthread_mutex_t* mutx;
} spellQueueEntry;

typedef struct spellQueue
{
    spellQueueEntry** entries;
    int num_entries;
} spellQueue;

void initSpellQueue(spellQueue** queue);
spellQueueEntry* popSpellQueueEntry(spellQueue** queue); //pop entry from queue
void addSpellQueueEntry(spellQueue** queue, char* word, pthread_cond_t* cond_inst, pthread_mutex_t* mutx_inst); //add a new entry to the spell queue to process

void* processSpellQueue(void* param); //worker thread function for spelling checker queue

void destroySpellQueue(spellQueue** queue);

#endif //_SPELL_QUEUE_H_
