#ifndef _SOLVED_SPELL_LIST_H_
#define _SOLVED_SPELL_LIST_H_

#define DEFAULT_SOLVED_LEN 1000
#define MAX_WORD_LEN 32

typedef struct solvedSpellListEntry
{
    char word[MAX_WORD_LEN]; //word searched
    int correct; //only valid after the word has been checked and returned.
    //-1 on creation of the struct instance. After the word is checked, 0 for incorrect, or 1 for correct
} solvedSpellListEntry;

typedef struct solvedSpellList
{
    solvedSpellListEntry** entries;
    int num_entries;
} solvedSpellList;

void initSolvedList(solvedSpellList** ppList);
int findSolvedWord(solvedSpellList** list, const char* word);
int getSolvedWordSuccess(solvedSpellList** list, const char* word);
void removeSolvedWord(solvedSpellList** list, const char* word);
void addSolvedWord(solvedSpellList** list, const char* word, int correct);

void destroySolvedList(solvedSpellList** list);

#endif //_SOLVED_SPELL_LIST_H_
