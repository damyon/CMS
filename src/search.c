/***********************************************************
* search.c
*
* This file contains the functions to index and search data.
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "env.h"
#include "strings.h"
#include "structs.h"
#include "package.h"
#include "objects.h"
#include "users.h"
#include "search.h"
#include "dbcalls.h"
#include "errors.h"
#include "malloc.h"
#include "logging.h"
#ifdef WIN32
#include "win32.h"
#endif

/***********************************************************
* The search tables look like this.
*
* Dictionary
* +----------+--------------+------+-----+---------+----------------+
* | Field    | Type         | Null | Key | Default | Extra          |
* +----------+--------------+------+-----+---------+----------------+
* | wordID   | int(11)      |      | PRI | NULL    | auto_increment |
* | wordStr  | varchar(255) |      | UNI |         |                |
* | wordMean | float        |      |     | 0       |                |
* | wordStd  | float        |      |     | 0       |                |
* +----------+--------------+------+-----+---------+----------------+
*
* Occurrence
* +----------+---------+------+-----+---------+-------+
* | Field    | Type    | Null | Key | Default | Extra |
* +----------+---------+------+-----+---------+-------+
* | objectID | int(11) |      | PRI | 0       |       |
* | wordID   | int(11) |      | PRI | 0       |       |
* | total    | int(8)  | YES  |     | 1       |       |
* +----------+---------+------+-----+---------+-------+
*
* All the indexer has to do is update the Occurrence table 
* and get mysql to recalculate the mean and stddev.
***********************************************************/

/*********************************************************************
* Word...
*
* This struct represents a single word in the contents of the doc.
*********************************************************************/
typedef struct _Word_ {
  char *word;
  int wordid, total;
} Word;

/*********************************************************************
* initWord...
*
* Allocate the memory for this struct.
*********************************************************************/
Word *initWord(char *w) {
  Word *wd = NULL;

  wd = (Word *) dhumalloc(sizeof(Word));
  if (wd == NULL)
    return NULL;

  wd->word = dhustrdup(w);
  wd->wordid = -1;
  wd->total = 1;
  return wd;
}

/*********************************************************************
* addWordToQueue...
*
* If this word is in the queue already just increment the total
* else add it to the queue.
*********************************************************************/
void addWordToQueue(Queue *q, char *word) {
  Word *w = NULL;
  int i = 0, size = 0;

  if (q == NULL || word == NULL)
    return;
  if (strlen(word) < 3)
    return;

  size = countQueue(q);
  for (i = 0; i < size; i++) {
    if (strcasecmp(((Word *) sniffNQueue(q, i))->word, word) == 0) {
      w = (Word *) sniffNQueue(q, i);
      w->total++;
      return;
    }
  }
  w = initWord(word);
  if (w == NULL)
    return;
  pushQueue(q, w); 
}


/*********************************************************************
* freeWord...
*
* Free the memory for this struct.
*********************************************************************/
void freeWord(Word *w) {
  if (w) {
    if (w->word) {
      dhufree(w->word);
    }
    dhufree(w);
  }
}

/*********************************************************************
* getNextWord...
*
* Parse the next word out of the stream.
*********************************************************************/
char *getNextWord(char **p) {
  char *data = *p, *eptr = NULL, c = '\0', *word = NULL;

  if (data == NULL)
    return NULL;

  while ((*data != CNULL) && (isspace(*data)))
    data++;

  if ((data == NULL) || (*data == CNULL))
    return NULL;

  eptr = data;
  while ((*eptr != CNULL) && (!isspace(*eptr)))
    eptr++;

  c = *eptr;
  *eptr = CNULL;

  word = dhustrdup(data?data:(char *) "");
  *eptr = c;
  data = eptr;
  *p = data;
  return word;
}

/*********************************************************************
* indexObject...
*
* Take this text and add it to the index tables so it can be searched on.
*********************************************************************/
int indexObject(char *content, int objectid, void *sqlsock) {
  Queue *words = NULL;
  char *ptr = NULL, *word = NULL;
  Word *w = NULL;

  words = initQueue();

  // CLEAR THE NON ALPHA CHARS
  ptr = content;
  while (*ptr != CNULL) {
    if (!isalpha(*ptr)) 
      *ptr = ' ';
    ptr++;
  }

  // COUNT THE WORDS
  ptr = content;
  while ((word = getNextWord(&ptr)) != NULL) {
    addWordToQueue(words, word); 
    dhufree(word);
  }
  
  while ((w = (Word *) popQueue(words)) != NULL) {
    // GET THE WORDIDS
    getWordID(w->word, &(w->wordid), sqlsock);
    // INSERT THE OCCURRENCE VALUES 
    increaseWordOccurrence(objectid, w->wordid, w->total, sqlsock);
    refreshWordStats(w->wordid, sqlsock);
    freeWord(w);
  }

  freeQueue(&words);

  return E_OK;  
}


/*********************************************************************
* scoreCompare...
*
* Compare function for the quicksort.
*********************************************************************/
int scoreCompare(const void *a, const void *b) {
  SearchHit *as = (SearchHit *) a,
            *bs = (SearchHit *) b;

  return bs->score - as->score;
}

/*********************************************************************
* mergeSearchHits...
*
* Merge the current search hits with the ones in the list.
*********************************************************************/
void mergeSearchHits(SearchHit *whits, int numwhits, SearchHit **hits, int *numhits, int mergeno) {
  SearchHit *h = NULL;
  int c = 0, i = 0, j = 0, d = 0;


  if (*numhits == 0) {
    c = numwhits;
    h = (SearchHit *) dhumalloc(sizeof(SearchHit) * c);    
    memcpy(h, whits, (sizeof(SearchHit) * c));
  } else {
    h = *hits;
    c = *numhits;
    for (i = 0; i < numwhits; i++) {
      d = 0;
      for (j = 0; j < c; j++) {
        if (whits[i].objectID == h[j].objectID) {
          h[j].score = (int)((double)h[j].score * (((double)mergeno - 1)/(double)mergeno) + ((double)whits[i].score/(double)mergeno));
          d = 1;
        }
      }
      if (!d) {
        h = (SearchHit *) dhurealloc(h, sizeof(SearchHit) * ((c) + 1));
        h[c].score = (int)((double)whits[i].score / (double)mergeno);
        h[c].objectID = whits[i].objectID;
        c++;
      }
    }
    for (i = 0; i < c; i++) {
      d = 0;
      for (j = 0; j < numwhits; j++) {
        if (h[i].objectID == whits[j].objectID) {
          d = 1;
        }
      }
      if (!d) {
        h[i].score = (int)((double)h[i].score * (double)(mergeno - 1) / (double)mergeno);
      }
    }
  }
  *hits = h;
  *numhits = c;
}

/*********************************************************************
* searchContent...
*
* Search the database for documents.
*********************************************************************/
int searchContent(char *query, int timestamp, int **theids, int **thescores, int *thenumhits, void *sqlsock) {
  Queue *words = NULL;
  char *ptr = NULL, *word = NULL;
  Word *w = NULL;
  SearchHit *hits, *whits;
  int numhits = 0, numwhits = 0, i = 0, mn = 0;
  int *ids = NULL, *scores = NULL;

  words = initQueue();

  // CLEAR THE NON ALPHA CHARS
  ptr = query;
  while (*ptr != CNULL) {
    if (!isalpha(*ptr)) 
      *ptr = ' ';
    ptr++;
  }

  // COUNT THE WORDS
  ptr = query;
  while ((word = getNextWord(&ptr)) != NULL) {
    addWordToQueue(words, word); 
    dhufree(word);
  }
  
  // GET THE WORDIDS
  for (i = 0; i < countQueue(words); i++) {
    w = ((Word *) sniffNQueue(words, i));
    getWordID(w->word, &(w->wordid), sqlsock);
    whits = getSearchHits(w->wordid, timestamp, &numwhits, sqlsock);
    mergeSearchHits(whits, numwhits, &hits, &numhits, ++mn); 
    dhufree(whits);
  }

  // SORT THE HITS
  if (numhits > 0) {
    qsort(hits, numhits, sizeof(SearchHit), scoreCompare);

    ids = (int *) dhumalloc(sizeof(int) * numhits);
    scores = (int *) dhumalloc(sizeof(int) * numhits);
    for (i = 0; i < numhits; i++) {
      ids[i] = hits[i].objectID;
      scores[i] = hits[i].score;
    }
    // Free the words
    dhufree(hits);
  }

  while ((w = (Word *) popQueue(words)) != NULL) {
    freeWord(w);
  }
  freeQueue(&words);
  
  *theids = ids;
  *thescores = scores;
  *thenumhits = numhits; 
  if (numhits <= 0)
    return NODATAFOUND;

  return E_OK;
}

/*********************************************************************
* unIndexObject...
*
* Drop all references to this object from the index tables.
*********************************************************************/
int unIndexObject(int objectid, void *sqlsock) {
  return removeObjectFromSearchTables(objectid, sqlsock);  
}

