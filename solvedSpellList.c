#include <stdlib.h>
#include <string.h>

#include "globals.h"

void initSolvedList(solvedSpellList** ppList)
{
    int i;
    solvedSpellList* list = malloc(sizeof(solvedSpellList)); //create list with DEFAULT_SOLVED_LEN constant size
    list->entries = malloc(sizeof(solvedSpellListEntry*)*DEFAULT_SOLVED_LEN);
    
    for (i=0;i<DEFAULT_SOLVED_LEN;++i)
        list->entries[i] = NULL; //set everything to null so that we can add and remove at will
    
    list->num_entries = 0;
    *ppList = list;
}

int findSolvedWord(solvedSpellList** list, const char* word)
{
    int i;
    
    if ((*list)->num_entries == 0)
        return 0;
    
    for (i=0;i<DEFAULT_SOLVED_LEN;++i) //random access array. Do not use list->num_entries
        //to iterate through the list and find valid entries
    {
        if ((*list)->entries[i] == NULL) //bypass empty entries
            continue;
        
        if (strcmp(word, (*list)->entries[i]->word) == 0) //check if the word is there
            return 1;
    }
    
    return 0;
}

int getSolvedWordSuccess(solvedSpellList** list, const char* word)
{
    int i;
    
    if ((*list)->num_entries == 0)
        return 0;
    
    for (i=0;i<DEFAULT_SOLVED_LEN;++i) //random access array. Do not use list->num_entries
        //to iterate through the list and find valid entries
    {
        if ((*list)->entries[i] == NULL) //bypass empty entries
            continue;
        
        if (strcmp(word, (*list)->entries[i]->word) == 0) //check if the word is there
            return (*list)->entries[i]->correct;
    }
    
    return -1;
}

//remove a word from the list after we've processed it
void removeSolvedWord(solvedSpellList** list, const char* word)
{
    int i;
    
    if ((*list)->num_entries == 0)
        return;
    
    for (i=0;i<DEFAULT_SOLVED_LEN;++i) //random access array. Do not use list->num_entries
        //to iterate through the list and find valid entries
    {
        if ((*list)->entries[i] == NULL) //bypass empty entries
            continue;
        
        if (strcmp(word, (*list)->entries[i]->word) == 0) //check if the word is there
        {
            free((*list)->entries[i]); //free memory and set back to null so that this space can be reused
            (*list)->entries[i] = NULL;
            --((*list)->num_entries); //decrement number of entries
            break;
        }
    }
}

//Error we had: 'correct' was 2485, because we didn't add a break after finding an empty spot to
//add to the solved list; therefore, it added the same word DEFAULT_SOLVED_LEN times (1000)
void addSolvedWord(solvedSpellList** list, const char* word, int correct)
{
    int i;
    for (i=0;i<DEFAULT_SOLVED_LEN;++i)
    {
        if ((*list)->entries[i] == NULL) //find an empty spot
        {
            (*list)->entries[i] = malloc(sizeof(solvedSpellList));
            strcpy((*list)->entries[i]->word,word);
            (*list)->entries[i]->correct = correct;
            ++((*list)->num_entries);
            break;
        }
    }
}


void destroySolvedList(solvedSpellList** list)
{
    int i;
    for (i=0;i<DEFAULT_SOLVED_LEN;++i)
        if ((*list)->entries[i] != NULL)
            free((*list)->entries[i]);
    
    free((*list));
}
