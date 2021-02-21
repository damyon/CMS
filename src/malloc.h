/************************************************************************
* malloc.h
*
* Memory Management functions.
************************************************************************/
#ifndef _DHUMALLOC_H
#define _DHUMALLOC_H

#ifdef __cplusplus
extern "C" {
#endif


void *dhurealloc(void *ptr, unsigned int bytes);

void *dhumalloc(unsigned int bytes);

//void dhufree(void *ptr);

char *dhustrdup(char *bytes);

#define dhufree(x) if((x) != NULL){free((void *) x);x=NULL;}

#ifdef __cplusplus
}
#endif


#endif // _DHUMALLOX_H
