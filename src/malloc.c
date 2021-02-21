/***********************************************************
* malloc.c
*
* This file contains the memory allocation functions that allow
* a thread to clean up after itself.
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "errors.h"
#include "malloc.h"

void *dhurealloc(void *ptr, unsigned int bytes) {
  if (bytes == 0) {
    if (ptr != NULL)
      free(ptr);
    return NULL;
  }

  while ((ptr = realloc(ptr, bytes)) == NULL) {
    //logError("The system is running low on memory.\n");
  }

  return ptr;
}

void *dhumalloc(unsigned int bytes) {
  void *ptr = NULL;

  if (bytes == 0)
    return NULL;

  while ((ptr = malloc(bytes)) == NULL) {
    //logError("The system is running low on memory.\n");
  }

  return ptr;
}

char *dhustrdup(char *bytes) {
  char *ptr = NULL;

  if (bytes == NULL)
    return NULL;

  while ((ptr = strdup(bytes)) == NULL) {
    //logError("The system is running low on memory.\n");
  }

  return ptr;
}

