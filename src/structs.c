/*******************************************************************************
* structs...
*
* This file contains the utility functions for some more common structures
* to be used in the server.
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "strings.h"
#include "structs.h"
#include "malloc.h"
#include "errors.h"

/****************************************************
* initPair...
*
* Create a pair of pointers.
****************************************************/
Pair * initPair(void *first, void *second) {
  Pair *p = NULL;

  p = (Pair *) dhumalloc(sizeof(Pair));
  if (p == NULL)
    return NULL;

  p->first = first;
  p->second = second;
  return p;
}

/****************************************************
* freePair...
*
* Free a pair (will free the pointers contained)
****************************************************/
void freePair(Pair *p) {
  if (p) {
    if (p->first)
      dhufree(p->first);
    if (p->second)
      dhufree(p->second);
    dhufree(p);
  }
}


/****************************************************
* initLeaf...
*
* Init a map struct.
****************************************************/
MapNode *initLeaf() {
  MapNode *node = NULL;

  node = (MapNode *) dhumalloc(sizeof(MapNode));
  if (node == NULL)
    return NULL;

  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->ele = NULL;
  node->colour = BLACKNODE;
  return node;
}

/****************************************************
* initMapNode...
*
* Init a map struct.
****************************************************/
MapNode *initMapNode(void *ele) {
  MapNode *map = NULL;

  map = (MapNode *) dhumalloc(sizeof(MapNode));
  if (map == NULL)
    return NULL;

  map->left = initLeaf();
  map->right = initLeaf();
  map->parent = NULL;
  map->ele = ele;
  map->colour = REDNODE;
  return map;
}

/****************************************************
* initMap...
*
* Init a map struct.
****************************************************/
Map *initMap(int (*compareFunc) (void *, void *), void (*freeFunc)(void *)) {
  Map *map = NULL;

  map = (Map *) dhumalloc(sizeof(Map));
  if (map == NULL)
    return NULL;

  map->root = NULL;
  map->freeFunc = freeFunc;
  map->compareFunc = compareFunc;
  map->size = 0;
  return map;
}

/****************************************************
* searchMapNode...
*
* Search a map struct.
****************************************************/
MapNode *searchMapNode(void *ele, MapNode *node, Map *m) {
  int cmp = 0;

  if (!m || m->root == NULL || node == NULL)
    return NULL;

  cmp = m->compareFunc(node->ele, ele);

  if (cmp > 0) {
    if (!node->left->left)
      return NULL;
    return searchMapNode(ele, node->left, m);
  } else if (cmp < 0) {
    if (!node->right->right)
      return NULL;
    return searchMapNode(ele, node->right, m);
  }
  return node;
}

/****************************************************
* getFirstMapNode...
*
* Get the first node in the map (by order)
****************************************************/
MapNode *getFirstMapNode(Map *m) {
  MapNode *node = NULL;

  if (!m || m->root == NULL)
    return NULL;

  node = m->root;

  while (node->left->left) node = node->left;

  return node;
}

/****************************************************
* getLastMapNode...
*
* Get the last node in the map (by order)
****************************************************/
MapNode *getLastMapNode(Map *m) {
  MapNode *node = NULL;

  if (!m || m->root == NULL)
    return NULL;

  node = m->root;

  while (node->right->right) node = node->right;

  return node;
}

/****************************************************
* getNextMapNode...
*
* Get the next node in the map (by order)
****************************************************/
MapNode *getNextMapNode(MapNode *n, Map *m) {
  MapNode *node = n;

  if (m == NULL || m->root == NULL || n == NULL)
    return NULL;

  if (node->right->right) {
    node = node->right;
    while (node->left->left) node = node->left;
  } else {
    // while we are the right node move up
    while ((node->parent != NULL) && (node == node->parent->right)) {
      node = node->parent;
    }
    node = node->parent;
  }

  return node;
}

/****************************************************
* getPreviousMapNode...
*
* Get the previous node in the map (by order)
****************************************************/
MapNode *getPreviousMapNode(MapNode *n, Map *m) {
  MapNode *node = n;

  if (m == NULL || m->root == NULL || n == NULL)
    return NULL;

  if (node->left->left) {
    node = node->left;
    while (node->right->right) node = node->right;
  } else {
    // while we are the left node move up
    while ((node->parent != NULL) && (node == node->parent->left)) {
      node = node->parent;
    }
    node = node->parent;
  }

  return node;
}

/****************************************************
* searchMap...
*
* Search a map struct.
****************************************************/
MapNode *searchMap(void *ele, Map *m) {
  int cmp = 0;
  MapNode *node = NULL;

  if (!m || m->root == NULL)
    return NULL;

  node = m->root;

  cmp = m->compareFunc(node->ele, ele);

  if (cmp > 0) {
    if (!node->left->left)
      return NULL;
    return searchMapNode(ele, node->left, m);
  } else if (cmp < 0) {
    if (!node->right->right)
      return NULL;
    return searchMapNode(ele, node->right, m);
  }
  return node;
}

/****************************************************
* rightRotateMap...
*
* The purpose of the right rotate function is to dynamically
* balance the tree upon insertions.
****************************************************/
void rightRotateMap(Map *tree, MapNode *y) {
  MapNode *x;

  x = y->left;

  // Turn x's right sub tree into y's left sub tree 
  y->left = x->right;
  if (x->right != NULL)
    x->right->parent = y;

  x->parent = y->parent;

  if (y->parent == NULL) {
    tree->root = x;
  } else {
    if (y == (y->parent->right)) {
      // x was on the right
      y->parent->right = x;
    } else {
      // y was on the left
      y->parent->left = x;
    }
  }

  x->right = y;
  y->parent = x;
}

/****************************************************
* leftRotateMap...
*
* The purpose of the left rotate function is to dynamically
* balance the tree upon insertions.
****************************************************/
void leftRotateMap(Map *tree, MapNode *x) {
  MapNode *y;

  y = x->right;

  // Turn y's left sub tree into x's right sub tree 
  x->right = y->left;
  if (y->left != NULL)
    y->left->parent = x;

  y->parent = x->parent;

  if (x->parent == NULL) {
    tree->root = y;
  } else {
    if (x == (x->parent->left)) {
      // x was on the left
      x->parent->left = y;
    } else {
      // x was on the right
      x->parent->right = y;
    }
  }

  y->left = x;
  x->parent = y;
}

/****************************************************
* printMapNode...
*
* Print the entire map using the supplied print function.
****************************************************/
void printMapNode(MapNode *m, void (* printFunc)(void *e), int indent) {
  int i = 0;
  if (!m)
    return;

  printMapNode(m->left, printFunc, indent+1);
  for (i = 0; i < indent; i++) {
    printf(" ");
  }
  if (m->colour == REDNODE) {
    printf("[RED]  ");
    if (m->ele)
      printFunc(m->ele);
  } else if (m->colour == BLACKNODE) {
    printf("[BLACK]");
    if (m->ele)
      printFunc(m->ele);
  }
  printf("\n");
  printMapNode(m->right, printFunc, indent+1);
}

/****************************************************
* printMap...
*
* Print the entire map using the supplied print function.
****************************************************/
void printMap(Map *m, void (* printFunc)(void *) ) {
  printMapNode(m->root, printFunc, 0);
}

/****************************************************
* cmpString...
*
* Compare 2 strings.
****************************************************/
int cmpString(void *a, void *b) {
  return strcmp((char *)a, (char *)b);
}

/****************************************************
* freeString...
*
* Free a string.
****************************************************/
void freeString(void *a) {
  dhufree(a);
}

/****************************************************
* freeMapNode...
*
* Free a map struct.
****************************************************/
void freeMapNode(MapNode *map, void (*freeFunc)(void *)) {
  if (map) {
    if (map->left)
      freeMapNode(map->left, freeFunc);
    if (map->ele) 
      freeFunc(map->ele);
    if (map->right)
      freeMapNode(map->right, freeFunc);
    dhufree(map);
  }
}

/****************************************************
* insertMapNode...
*
* Insert a value into this map.
****************************************************/
int insertMapNode(MapNode *ele, MapNode *m, Map *tree) {
  MapNode *p = NULL;
  int cmp = 0;

  if (ele == NULL || m == NULL || tree == NULL) {
    return RESOURCEERROR;
  }

  p = m;

  cmp = tree->compareFunc(p->ele, ele->ele);
  if (cmp > 0) {
    if (p->left->left == NULL) {
      dhufree(p->left);
      ele->parent = p;
      p->left = ele;
      tree->size++;
    } else {
      return insertMapNode(ele, p->left, tree);
    }
  } else if (cmp < 0) {
    if (p->right->right == NULL) {
      dhufree(p->right);
      ele->parent = p;
      p->right = ele;
      tree->size++;
    } else {
      return insertMapNode(ele, p->right, tree);
    }

  } else {
    dhufree(ele->left);
    dhufree(ele->right);
    ele->left = p->left;
    ele->right = p->right;
    ele->parent = p->parent;
    if (p->parent->left == p)
      p->parent->left = ele;
    else
      p->parent->right = ele;

    p->left = NULL;
    p->right = NULL;
    p->parent = NULL;
    freeMapNode(p, tree->freeFunc);
  }
  
  return E_OK;
}

/****************************************************
* insertMap...
*
* Insert a value into this map.
****************************************************/
int insertMap(MapNode *ele, Map *m) {
  MapNode *p = NULL;
  int cmp = 0;

  if (ele == NULL || m == NULL) {
    return RESOURCEERROR;
  }

  p = m->root;
  // Empty tree
  if (p == NULL) {
    m->root = ele;
    return E_OK;
  }  

  cmp = m->compareFunc(p->ele, ele->ele);
  if (cmp > 0) {
    if (p->left->left == NULL) {
      dhufree(p->left);
      ele->parent = p;
      p->left = ele;
      m->size++;
    } else {
      return insertMapNode(ele, p->left, m);
    }
  } else if (cmp < 0) {
    if (p->right->right == NULL) {
      dhufree(p->right);
      ele->parent = p;
      p->right = ele;
      m->size++;
    } else {
      return insertMapNode(ele, p->right, m);
    }

  } else {
    dhufree(ele->left);
    dhufree(ele->right);
    ele->left = p->left;
    ele->right = p->right;
    ele->parent = p->parent;
    if (p->parent->left == p)
      p->parent->left = ele;
    else
      p->parent->right = ele;

    p->left = NULL;
    p->right = NULL;
    p->parent = NULL;
    freeMapNode(p, m->freeFunc);
  }
  
  return E_OK;
}

void printFuncDebug(void *a) {
  printf("[%s]", (char *) a);
}

/****************************************************
* insertMapValue...
*
* Insert a value into this map and balance the tree.
****************************************************/
int insertMapValue(MapNode *x, Map *tree) {
  int err = 0;
  MapNode *y = NULL;

  err = insertMap(x, tree);

  if (err == E_OK) {
    while (x->parent != NULL && x->parent->colour == REDNODE) {
      if (x->parent->parent->left == x->parent) {
        y = x->parent->parent->right;
        if (y && y->colour == REDNODE) {
          // uncle is red
          x->parent->colour = BLACKNODE;
          y->colour = BLACKNODE;
          x->parent->parent->colour = REDNODE;
          x = x->parent->parent;
        } else {
          // uncle is black
          if (x == x->parent->right) {
            x = x->parent;
            leftRotateMap(tree, x);
          }
 
          x->parent->colour = BLACKNODE;
          x->parent->parent->colour = REDNODE;
          rightRotateMap(tree, x->parent->parent);
        }
      } else {
        y = x->parent->parent->left;
        if (y && y->colour == REDNODE) {
          // uncle is red
          x->parent->colour = BLACKNODE;
          y->colour = BLACKNODE;
          x->parent->parent->colour = REDNODE;
          x = x->parent->parent;
        } else {
          // uncle is black
          if (x == x->parent->left) {
            x = x->parent;
            rightRotateMap(tree, x);
          }
 
          x->parent->colour = BLACKNODE;
          x->parent->parent->colour = REDNODE;
          leftRotateMap(tree, x->parent->parent);
        }
      }
    }
  }
  tree->root->colour = BLACKNODE;

  return err;
}

/****************************************************
* balanceDeleteMap...
*
* Rebalance the tree after a removal
****************************************************/
int balanceDeleteMap(MapNode *n, Map *tree) {
  MapNode *sibling = NULL;

  while (n->parent != NULL && n->colour == BLACKNODE) {
    if (n == n->parent->left) {
      // left case
      sibling = n->parent->right;
      
      //CASE 1: rotate left to convert into case 2, 3 or 4
      if (sibling->colour == REDNODE) {
        sibling->colour = BLACKNODE;
        n->parent->colour = REDNODE;
        leftRotateMap(tree, n->parent);
        sibling = n->parent->right;
      }

      //CASE 2: No rotation needed, simply recolor and move up the tree.
      if (sibling->right->colour == BLACKNODE && sibling->left->colour == BLACKNODE) {
        sibling->colour = REDNODE;
        n = n->parent;
      } else {
      
        //CASE: 3: rotate right to turn this into case 4
        if (sibling->right->colour == BLACKNODE) {
          sibling->left->colour = BLACKNODE;
          sibling->colour = REDNODE;
          rightRotateMap(tree, sibling);
          sibling = n->parent->right;
        }

        //CASE 4: We have to rotate left 
        sibling->colour = n->parent->colour;
        n->parent->colour = BLACKNODE;
        sibling->right->colour = BLACKNODE;
        leftRotateMap(tree, n->parent);
        n = tree->root; // to terminate the while loop
      }

    } else {
      // right case
      sibling = n->parent->left;
      
      //CASE 1: rotate right to convert into case 2, 3 or 4
      if (sibling->colour == REDNODE) {
        sibling->colour = BLACKNODE;
        n->parent->colour = REDNODE;
        rightRotateMap(tree, n->parent);
        sibling = n->parent->left;
      }

      //CASE 2: No rotation needed, simply recolor and move up the tree.
      if (sibling->left->colour == BLACKNODE && sibling->right->colour == BLACKNODE) {
        sibling->colour = REDNODE;
        n = n->parent;
      } else {
      
        //CASE: 3: rotate left to turn this into case 4
        if (sibling->left->colour == BLACKNODE) {
          sibling->right->colour = BLACKNODE;
          sibling->colour = REDNODE;
          leftRotateMap(tree, sibling);
          sibling = n->parent->left;
        }

        //CASE 4: We have to rotate right 
        sibling->colour = n->parent->colour;
        n->parent->colour = BLACKNODE;
        sibling->left->colour = BLACKNODE;
        rightRotateMap(tree, n->parent);
        n = tree->root; // to terminate the while loop
      }

    }
  }

  n->colour = BLACKNODE;

  return E_OK;
}

/****************************************************
* removeMap...
*
* Remove a value from this map.
****************************************************/
int removeMapValue(MapNode *ele, Map *tree) {
  MapNode *child = NULL, *cut = NULL;
  void *tmp = NULL;

  if (!ele || !tree) {
    return RESOURCEERROR;    
  }

  if ((ele->left->left == NULL) || (ele->right->right == NULL)) {
    cut = ele;
  } else {
    cut = ele->right;
    while (cut->left->left != NULL) cut = cut->left;
  }

  if (cut->left->left != NULL) {
    child = cut->left;
    cut->left = NULL;
  } else {
    child = cut->right;
    cut->right = NULL;
  }

  child->parent = cut->parent;
  
  if (cut->parent == NULL) {
    tree->root = child;
  } else {
    if (cut->parent->left == cut) {
      cut->parent->left = child;
    } else {
      cut->parent->right = child;
    }
  }

  if (cut != ele) {
    tmp = ele->ele;
    ele->ele = cut->ele;
    cut->ele = tmp;
  }

  if (cut->colour == BLACKNODE) {
    balanceDeleteMap(child, tree);
  }

  freeMapNode(cut, tree->freeFunc);
  
  return E_OK;
}


/****************************************************
* freeMap...
*
* Free a map struct.
****************************************************/
void freeMap(Map *map) {
  if (map) {
    if (map->root)
      freeMapNode(map->root, map->freeFunc);
    dhufree(map);
  }
}


/****************************************************
* initQueue...
* 
* Init a single queue struct.
****************************************************/
Queue *initQueue() {
  Queue *queue = NULL;
 
  queue = (Queue *) dhumalloc(sizeof(Queue));
  queue->size = 0;
  queue->nalloc = 4;
  queue->ele = (void **) dhumalloc(sizeof(void *) * queue->nalloc);

  return queue;
}

/****************************************************
* pushQueue...
* 
* Push a data element onto the queue.
****************************************************/
void pushQueue(Queue *queue, void *ptr) {
  if (queue == NULL) {
    return;
  }

  queue->size += 1;
  if (queue->size >= queue->nalloc) {
    queue->nalloc *= 2;
    queue->ele = (void **) dhurealloc(queue->ele, sizeof(void *) * (queue->nalloc));
  }
  queue->ele[queue->size - 1] = ptr;

}

/*******************************************************************************
* popNQueue...
*
* pop an item out of the middle of the queue.
*******************************************************************************/
void *popNQueue(Queue *queue, int n) {
  void *p = NULL;
  int i = 0;

  if (queue == NULL || queue->size == 0) {
    return NULL;
  }

  if (n < 0 || n >= queue->size) {
    return NULL;
  }
  p = queue->ele[n];
  queue->size --;
  for (i = n; i < queue->size; i++) {
    queue->ele[i] = queue->ele[i+1];
  }
  
  return p;
}

/*******************************************************************************
* popQueue...
*
* pop an item off the bottom of the queue.
*******************************************************************************/
void *popQueue(Queue *queue) {
  void *p = NULL;
  int i = 0;

  if (queue == NULL || queue->size == 0) {
    return NULL;
  }
  p = queue->ele[0];
  queue->size --;
  for (i = 0; i < queue->size; i++) {
    queue->ele[i] = queue->ele[i+1];
  }
  
  return p;
}

/*******************************************************************************
* sniffQueue...
*
* look at the next item to come off the queue without removing it.
*******************************************************************************/
void *sniffQueue(Queue *queue) {
  if (queue == NULL || queue->size == 0) {
    return NULL;
  }
  
  return queue->ele[0];
}

/*******************************************************************************
* sniffNQueue...
*
* look at the next item to come off the queue without removing it.
*******************************************************************************/
void *sniffNQueue(Queue *queue, int i) {
  if (queue == NULL || queue->size == 0) {
    return NULL;
  }
  if (i < 0 || i >= queue->size)
    return NULL;
  
  return queue->ele[i];
}

/*******************************************************************************
* freeQueue...
*
*******************************************************************************/
void freeQueue(Queue **queue) {
  Queue *q = *queue;
  if (q) {
    free(q->ele);
    free(q);
  }
  *queue = NULL;
}

/*******************************************************************************
* countQueue...
*
* How many elements are in the queue?
*******************************************************************************/
int countQueue(Queue *queue) {
  if (queue == NULL) {
    return 0;
  }
  return queue->size;
}

/*******************************************************************************
* initStack...
*
* Initialise the stack.
*******************************************************************************/
Stack *initStack() {
  Stack *stack = NULL;

  stack = (Stack *) dhumalloc(sizeof(Stack));
  stack->size = 0;
  stack->nalloc = 4;
  stack->ele = (void **) dhumalloc(sizeof(void *) * stack->nalloc);
  return stack;
}

/*******************************************************************************
* pushStack...
*
* Push a pointer on to the stack.
*******************************************************************************/
void pushStack(Stack *stack, void *ptr) {
  if (stack == NULL) {
    return;
  }

  stack->size += 1;
  while (stack->size >= stack->nalloc) {
    stack->nalloc *= 2;
    stack->ele = (void **) dhurealloc(stack->ele, sizeof(void *) * (stack->nalloc));
  }
  stack->ele[stack->size - 1] = ptr;
}

/*******************************************************************************
* popNStack...
*
* pop an item out of the middle of a stack
*******************************************************************************/
void *popNStack(Stack *stack, int index) {
  void *ele = NULL;
  int i = 0;

  if (stack == NULL || stack->size == 0 || index >= stack->size || index < 0) {
    return NULL;
  }

  ele = stack->ele[stack->size - 1 - index];
  for (i = stack->size - 1 - index; i < (stack->size - 1); i++) {
	  stack->ele[i] = stack->ele[i+1];
  }
  stack->size--;
  return ele;
}

/*******************************************************************************
* popStack...
*
* pop an item off the top of the stack.
*******************************************************************************/
void *popStack(Stack *stack) {
  if (stack == NULL || stack->size == 0) {
    return NULL;
  }
  stack->size--;
  return stack->ele[stack->size];
}

/*******************************************************************************
* getStackHead...
*
* Look at the top of the stack without removing it.
*******************************************************************************/
void *sniffStack(Stack *stack) {
  if (stack == NULL || stack->size == 0) {
    return NULL;
  }
  return stack->ele[stack->size - 1];
}

/*******************************************************************************
* sniffNStack...
*
* Look element on the stack at n without removing it.
*******************************************************************************/
void *sniffNStack(Stack *stack, int n) {
  
  if (stack == NULL || stack->size == 0) {
    return NULL;
  }
  if (n < 0 || n >= stack->size) {
    return NULL;
  }
  return stack->ele[stack->size - 1 - n];
}

/*******************************************************************************
* countStack...
*
* How many elements are in the stack?
*******************************************************************************/
int countStack(Stack *stack) {
  if (stack == NULL || stack->size <= 0) {
    return 0;
  }
  return stack->size;
}

/*******************************************************************************
* freeStack...
*
*******************************************************************************/
void freeStack(Stack **stack) {
  Stack *s = *stack;
  if (s) {
    free(s->ele);
    free(s);
  }
  *stack = NULL;
}
