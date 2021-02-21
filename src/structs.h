/*******************************************************************************
* structs...
*
* This file contains the utility functions for some more common structures
* to be used in the server.
*******************************************************************************/

#ifndef _STRUCTS_H
#define _STRUCTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Pair.
*
* A pair of pointers
*******************************************************************************/
typedef struct _pair_ {
  void *first, *second;
} Pair;

/*******************************************************************************
* Stack.
*
* A generic stack
*******************************************************************************/
typedef struct _stack_ {
  void **ele;
  int size, nalloc;
} Stack;

/*******************************************************************************
* Queue.
*
* A generic queue.
*******************************************************************************/
typedef struct _queue_ {
  void **ele;
  int size, nalloc;
} Queue;

/*******************************************************************************
* Map.
*
* A generic map using a balanced binary tree.
*******************************************************************************/
typedef enum _nodecolour_ { REDNODE, BLACKNODE, LEAFNODE } NodeColour;

typedef struct _mapnode_ {
  void *ele;
  NodeColour colour;
  struct _mapnode_ *left, *right, *parent;
} MapNode;

typedef struct _map_ {
  struct _mapnode_ *root;
  void (*freeFunc) (void *);
  int (*compareFunc) (void *, void *);
  int size;
} Map;

/****************************************************
* initPair...
*
* Create a pair of pointers.
****************************************************/
Pair * initPair(void *first, void *second);

/****************************************************
* freePair...
*
* Free a pair (will free the pointers contained)
****************************************************/
void freePair(Pair *p);

/****************************************************
* getPreviousMapNode...
*
* Get the previous node in the map (by order)
****************************************************/
MapNode *getPreviousMapNode(MapNode *n, Map *m);

/****************************************************
* getNextMapNode...
*
* Get the next node in the map (by order)
****************************************************/
MapNode *getNextMapNode(MapNode *n, Map *m);

/****************************************************
* getFirstMapNode...
*
* Get the first node in the map (by order)
****************************************************/
MapNode *getFirstMapNode(Map *m);

/****************************************************
* getLastMapNode...
*
* Get the last node in the map (by order)
****************************************************/
MapNode *getLastMapNode(Map *m);

/****************************************************
* searchMap...
*
* Search a map struct.
****************************************************/
MapNode *searchMap(void *ele, Map *m);

/****************************************************
* printMapNode...
*
* Print the entire map below this node
****************************************************/
void printMapNode(MapNode *m, void (* printFunc)(void *e), int indent);

/****************************************************
* printMap...
*
* Print the entire map 
****************************************************/
void printMap(Map *m, void (* printFunc)(void *e));

/****************************************************
* initMapNode...
*
* Init a map struct.
****************************************************/
MapNode *initMapNode(void *ele);

/****************************************************
* initMap...
* 
* Init a map struct.
****************************************************/
Map *initMap(int(* compareFunc)(void *, void *), void (*freeFunc)(void *));

/****************************************************
* cmpString...
* 
* compare a string
****************************************************/
int cmpString(void *a, void *b);

/****************************************************
* freeString...
* 
* Free a char *
****************************************************/
void freeString(void *a);

/****************************************************
* freeMap...
* 
* Free a map struct.
****************************************************/
void freeMap(Map *map);

/****************************************************
* freeMapNode...
* 
* Free a mapnode struct.
****************************************************/
void freeMapNode(MapNode *map, void (*freeFunc)(void *));

/****************************************************
* insertMap...
*
* Insert a value into this map.
****************************************************/
int insertMapValue(MapNode *ele, Map *m);

/****************************************************
* removeMapValue...
*
* Remove a value from this map.
****************************************************/
int removeMapValue(MapNode *ele, Map *m);

/****************************************************
* initQueue...
* 
* Init a single queue struct.
****************************************************/
Queue *initQueue(void);

/****************************************************
* pushQueue...
* 
* Push a data element onto the queue.
****************************************************/
void pushQueue(Queue *queue, void *ptr);

/*******************************************************************************
* popNQueue.
*
* pop an item out of the middle of the queue.
*******************************************************************************/
void *popNQueue(Queue *queue, int n);

/*******************************************************************************
* popNStack.
*
* pop an item out of the middle of the stack.
*******************************************************************************/
void *popNStack(Stack *stack, int n);

/*******************************************************************************
* popQueue.
*
* pop an item off the bottom of the queue.
*******************************************************************************/
void *popQueue(Queue *queue);

/*******************************************************************************
* sniffQueue.
*
* look at the next item to come off the queue without removing it.
*******************************************************************************/
void *sniffQueue(Queue *queue);

/*******************************************************************************
* sniffNQueue.
*
* look at the next item to come off the queue without removing it.
*******************************************************************************/
void *sniffNQueue(Queue *queue, int i);

/*******************************************************************************
* countQueue...
* 
* How many elements are in the queue?
*******************************************************************************/
int countQueue(Queue *queue);

/*******************************************************************************
* freeQueue...
*
* release this memory
*******************************************************************************/
void freeQueue(Queue **queue);

/*******************************************************************************
* initStack.
*
* Initialise the stack.
*******************************************************************************/
Stack *initStack(void);

/*******************************************************************************
* pushStack.
*
* Push a pointer on to the stack.
*******************************************************************************/
void pushStack(Stack *stack, void *ptr);

/*******************************************************************************
* popStack.
*
* pop an item off the top of the stack.
*******************************************************************************/
void *popStack(Stack *stack);

/*******************************************************************************
* sniffStack.
*
* Look at the top of the stack without removing it.
*******************************************************************************/
void *sniffStack(Stack *stack);

/*******************************************************************************
* sniffNStack.
*
* Look at the element on the stack at n without removing it.
*******************************************************************************/
void *sniffNStack(Stack *stack, int n);

/*******************************************************************************
* countStack...
*
* How many elements are in the stack?
*******************************************************************************/
int countStack(Stack *stack);

/*******************************************************************************
* freeStack...
*
* release this memory
*******************************************************************************/
void freeStack(Stack **stack);

#ifdef __cplusplus
}
#endif


#endif // _STRUCTS_H
