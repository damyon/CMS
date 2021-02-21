/************************************************************************
* search.h
*
* Search and Indexing functions.
************************************************************************/
#ifndef _SEARCH_H
#define _SEARCH_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _SearchHit_ {
  int objectID, score;
} SearchHit;

/*********************************************************************
* indexObject...
*
* Take this text and add it to the index tables so it can be searched on.
*********************************************************************/
int indexObject(char *content, int objectid, void *sqlsock);

/*********************************************************************
* unIndexObject...
*
* Drop all references to this object from the index tables.
*********************************************************************/
int unIndexObject(int objectid, void *sqlsock);

/*********************************************************************
* searchContent...
*
* Search the database for documents.
*********************************************************************/
int searchContent(char *query, int timestamp, int **theids, int **thescores, int *thenumhits, void *sqlsock);

#ifdef __cplusplus
}
#endif


#endif // _SEARCH_H
