#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define DEFAULT_BUF_STRING_COUNT 10
#define MAX_SPELL_ENTRIES 50

#define DEFAULT_DICTIONARY "./dictionary.txt"
#define DEFAULT_PORT 1337

#define NUM_THREADS 5
#define MAX_CONNECTIONS 10

#include <pthread.h>
#include "logQueue.h"
#include "spellQueue.h"
#include "solvedSpellList.h"
#include "connectionQueue.h"

static char* GOAHEAD = "GOAHEAD";
static char* END = "END";
static char* OK = "OK";
static char* MISSPELLED = "MISSPELLED";

static pthread_cond_t connectionQueueOccupiedCondition = PTHREAD_COND_INITIALIZER;
static pthread_cond_t connectionQueueVacancyCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t connectionQueueVacancyMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct SQueues
{
    char* lpDictionary;
    
    logQueue* logQueueInst;
    spellQueue* spellCheckQueue;
    solvedSpellList* solvedSpellCheckList;
    connectionQueue* connectionQueueInst;
} SQueues;

static SQueues Queues;

#endif //_GLOBALS_H_
