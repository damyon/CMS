/***********************************************************
* env.c
*
* This file contains the structs that are used to pass data between the servers.
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strings.h"
#include "ipc.h"
#include "env.h"
#include "errors.h"
#include "malloc.h"

#ifdef WIN32
#include "win32.h"
#endif

/****************************************************
* initFileObject...
* 
* Init a single FileObject struct.
****************************************************/
FileObject *initFileObject(char *name, char *contenttype, char *filename, char *data, int datalen) {
  FileObject *file = NULL;

  file = (FileObject *) dhumalloc(sizeof(FileObject));
  file->name = name;
  file->contenttype = contenttype;
  file->filename = filename;
  file->data = data;
  file->datalen = datalen;
  file->next = NULL;
  return file;
}

/****************************************************
* freeFileObjectList...
* 
* Free the linked list of files.
****************************************************/
void freeFileObjectList(FileObject *file) {
  FileObject *current = file,
             *next = NULL;

  while (current != NULL) {
    next = current->next;
    dhufree(current->name);
    dhufree(current->contenttype);
    dhufree(current->filename);
    dhufree(current->data);
    dhufree(current);
    current = next;
  }
}

/****************************************************
* fileObjectExists...
* 
* Look for a file with this name.
****************************************************/
int fileObjectExists(char *name, Env *env) {
  FileObject *current = NULL;
  
  current = env->filehead;
  while (current != NULL) {
    if (strcasecmp(current->name, name) == 0)
      return 1;
    current = current->next;
  }
  return 0;
}

/****************************************************
* getFileObject...
* 
* Get a file object out of the env.
****************************************************/
FileObject *getFileObject(char *name, Env *env) {
  FileObject *current = NULL;
  
  current = env->filehead;
  while (current != NULL) {
    if (strcasecmp(current->name, name) == 0)
      return current;
    current = current->next;
  }
  return NULL;
}

/****************************************************
* appendFileObjectToList...
* 
* Add this file object to the end of the list.
****************************************************/
void appendFileObjectToList(FileObject **head, FileObject **tail, char *name, char *contenttype, char *filename, char *data, int datalen) {
  if (*head == NULL) {
    *tail = initFileObject(name, contenttype, filename, data, datalen);
    *head = *tail;
  } else {
    (*tail)->next = initFileObject(name, contenttype, filename, data, datalen);
    *tail = (*tail)->next;
  }
}

/****************************************************
* initValueToken...
* 
* Init a single ValueToken struct.
****************************************************/
ValueToken *initValueToken(char *token, char *value) {
  ValueToken *valtok = NULL;

  valtok = (ValueToken *) dhumalloc(sizeof(ValueToken));
  valtok->token = token;
  valtok->value = value;
  valtok->next = NULL;
  return valtok;
}

/****************************************************
* countFiles...
*
* How many files are in the environment?
****************************************************/
int countFiles(Env *env) {
  FileObject *current = env->filehead;
  int numfiles = 0;
 
  while (current != NULL) {
    numfiles++;
    current = current->next;
  }
  return numfiles;
}

/****************************************************
* countTokenValues...
*
* How many tokens are in the environment?
****************************************************/
int countTokenValues(Env *env) {
  ValueToken *valtok = env->tokenhead;
  int numtokens = 0;
 
  while (valtok != NULL) {
    numtokens++;
    valtok = valtok->next;
  }
    
  return numtokens;
}

/****************************************************
* freeTokenValueList...
* 
* Free the linked list of value tokens.
****************************************************/
void freeTokenValueList(ValueToken *valtok) {
  ValueToken *current = valtok,
             *next = NULL;

  while (current != NULL) {
    next = current->next;
    dhufree(current->value);
    dhufree(current->token);
    dhufree(current);
    current = next;
  }
}

/****************************************************
* appendTokenValueToList...
* 
* Add this value token pair to the end of the list.
****************************************************/
void appendTokenValueToList(ValueToken **head, ValueToken **tail, char *token, char *value) {
  if (*head == NULL) {
    *tail = initValueToken(token, value);
    *head = *tail;
  } else {
    (*tail)->next = initValueToken(token, value);
    *tail = (*tail)->next;
  }
}

/****************************************************
* appendFileObjectToEnv...
* 
* Add this file to the end of the env.
****************************************************/
void appendFileObjectToEnv(char *name, char *contenttype, char *filename, char *data, int datalen, Env *env) {
  appendFileObjectToList(&(env->filehead), &(env->filetail), name, contenttype, filename, data, datalen);
}

/****************************************************
* appendTokenValueToEnv...
* 
* Add this value token pair to the end of the env.
****************************************************/
void appendTokenValueToEnv(char *token, char *value, Env *env) {
  appendTokenValueToList(&(env->tokenhead), &(env->tokentail), token, value);
}

/****************************************************
* setTokenValue...
* 
* Set this value token pair in the env.
****************************************************/
void setTokenValue(char *token, char *value, Env *env) {
  ValueToken *current = env->tokenhead,
             *next = NULL;

  while (current != NULL) {
    next = current->next;
    if (strcmp(current->token, token) == 0) {
      dhufree(token);
      dhufree(current->value);
      current->value = value;
      return;
    }
    current = next;
  }
  appendTokenValueToEnv(token, value, env);
}

/****************************************************
* getTokenValue...
*
* Searches for this token and returns the value
****************************************************/
char *getTokenValue(ValueToken *head, char *token) {
  ValueToken *current = head;

  while (current != NULL) {
    if (strcasecmp(current->token, token) == 0) {
      return current->value;
    }
    current = current->next;
  }
  return NULL;
}

/****************************************************
* getEnvValue...
*
* Searches for this token and returns the value
****************************************************/
char *getEnvValue(char *token, Env *env) {
  return getTokenValue(env->tokenhead, token);
}

/****************************************************
* initEnv...
* 
* Init an Env struct.
****************************************************/
Env *initEnv(void) {
  Env *env = NULL;

  env = (Env *) dhumalloc(sizeof(Env));
  env->tokenhead = NULL;
  env->tokentail = NULL;
  env->filehead = NULL;
  env->filetail = NULL;
  return env;
}

/****************************************************
* freeEnv...
* 
* Free an Env struct.
****************************************************/
void freeEnv(Env *env) {
  freeTokenValueList(env->tokenhead);
  freeFileObjectList(env->filehead);
  dhufree(env);
}

/****************************************************
* printEnv...
* 
* Print an Env struct.
****************************************************/
void printEnv(Env *env) {
  ValueToken *valcurrent = NULL;
  FileObject *filecurrent = NULL;

  if (env == NULL) {
    fprintf(stderr, "Environment is NULL!\n");
    return;  
  }

  valcurrent = env->tokenhead;
  while (valcurrent != NULL) {
    fprintf(stderr, "TOKEN:%s VALUE:%s\n", valcurrent->token, valcurrent->value);
    valcurrent = valcurrent->next;
  }

  filecurrent = env->filehead;
  while (filecurrent != NULL) {
    fprintf(stderr, "NAME:%s CONTENTTYPE:%s FILENAME:%s FILESIZE:%d\n", filecurrent->name, filecurrent->contenttype, filecurrent->filename, filecurrent->datalen);
    filecurrent = filecurrent->next;
  }
}

/****************************************************
* writeEnvToStream...
* 
* Writes this env directly to the socket.
* This is more efficient than making a copy in memory.
****************************************************/
void writeEnvToStream(int sockfd, Env *env) {
  char numbuf[9];
  int tokvals = countTokenValues(env), i = 0,
      files = countFiles(env);
  ValueToken *current = env->tokenhead;
  FileObject *curfile = env->filehead;

  // send all the value tokens
  sprintf(numbuf, "%.8d", tokvals);
  sendData(numbuf, 8, sockfd);
  for (i = 0; i < tokvals; i++) {
    sprintf(numbuf, "%.8d", strlen(current->token?current->token:""));
    sendData(numbuf, 8, sockfd);
    sendData(current->token?current->token:(char *)"", strlen(current->token?current->token:""), sockfd);
    sprintf(numbuf, "%.8d", strlen(current->value?current->value:""));
    sendData(numbuf, 8, sockfd);
    sendData(current->value?current->value:(char *)"", strlen(current->value?current->value:""), sockfd);
    current = current->next;
  }
  // send all the data objects 
  sprintf(numbuf, "%.8d", files);
  sendData(numbuf, 8, sockfd);
  for (i = 0; i < files; i++) {
    sprintf(numbuf, "%.8d", strlen(curfile->name?curfile->name:""));
    sendData(numbuf, 8, sockfd);
    sendData(curfile->name?curfile->name:(char *)"", strlen(curfile->name?curfile->name:""), sockfd);
    sprintf(numbuf, "%.8d", strlen(curfile->contenttype?curfile->contenttype:""));
    sendData(numbuf, 8, sockfd);
    sendData(curfile->contenttype?curfile->contenttype:(char *)"", strlen(curfile->contenttype?curfile->contenttype:""), sockfd);
    sprintf(numbuf, "%.8d", strlen(curfile->filename?curfile->filename:""));
    sendData(numbuf, 8, sockfd);
    sendData(curfile->filename?curfile->filename:(char *)"", strlen(curfile->filename?curfile->filename:""), sockfd);
    sprintf(numbuf, "%.8d", curfile->datalen);
    sendData(numbuf, 8, sockfd);
    sendData(curfile->datalen > 0?curfile->data:(char *)"", curfile->datalen, sockfd);
    curfile = curfile->next;
  }
}

/****************************************************
* readEnvFromStreamNoFile...
* 
* Read this env directly from the socket.
* This is more efficient than making a copy in memory.
* This doesn't read any files off the stream - it leaves
* them ready to be read.
****************************************************/
Env *readEnvFromStreamNoFile(int sockfd) {
  Env *env = NULL;
  char *buf = NULL, *token = NULL, *value = NULL, *name = NULL, *contenttype = NULL, *filename = NULL;
  int tokvals = 0, files = 0, i = 0, len = 0, datalen = 0;
   
  // find out how many token values there are
  if (readData(&buf, 8, sockfd) != E_OK) {
	//fprintf(stderr, "[%d] readEnv - Could not read header length\n", getpid());
    return NULL;
  }
  tokvals = strtol(buf, NULL, 10);
  dhufree(buf);

  env = initEnv();
  
  // read all the token values
  for (i = 0; i < tokvals; i++) {
    if (readData(&buf, 8, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read token length\n", getpid());
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&token, len, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read token name\n", getpid());
      freeEnv(env);
      return NULL;
    }
    if (readData(&buf, 8, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read value length\n", getpid());
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&value, len, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read token value\n", getpid());
      freeEnv(env);
      return NULL;
    }
    appendTokenValueToEnv(token, value, env); 
  }

  // find out how many files there are
  if (readData(&buf, 8, sockfd) != E_OK) {
    //fprintf(stderr, "[%d] readEnv - Could not file count\n", getpid());
    return NULL;
  }
  files = strtol(buf, NULL, 10);
  dhufree(buf);
  
  if (files > 0) {
    if (readData(&buf, 8, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read file name length\n", getpid());
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&name, len, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read file name\n", getpid());
      freeEnv(env);
      return NULL;
    }
    if (readData(&buf, 8, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read content type length\n", getpid());
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&contenttype, len, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read content type\n", getpid());
      freeEnv(env);
      return NULL;
    }
    if (readData(&buf, 8, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read filename length\n", getpid());
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&filename, len, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read filename\n", getpid());
      freeEnv(env);
      return NULL;
    }
    if (readData(&buf, 8, sockfd) != E_OK) {
      //fprintf(stderr, "[%d] readEnv - Could not read file length\n", getpid());
      freeEnv(env);
      return NULL;
    }
    datalen = strtol(buf, NULL, 10);
    dhufree(buf);
    appendFileObjectToEnv(name, contenttype, filename, NULL, datalen, env);
  }

  // SUCCESS
  return env;
}

/****************************************************
* readEnvFromStream...
* 
* Read this env directly from the socket.
* This is more efficient than making a copy in memory.
****************************************************/
Env *readEnvFromStream(int sockfd) {
  Env *env = NULL;
  char *buf = NULL, *token = NULL, *value = NULL, *name = NULL, *contenttype = NULL, *filename = NULL, *data = NULL;
  int tokvals = 0, files = 0, i = 0, len = 0, datalen = 0;
   
  // find out how many token values there are
  if (readData(&buf, 8, sockfd) != E_OK)
    return NULL;
  tokvals = strtol(buf, NULL, 10);
  dhufree(buf);

  env = initEnv();
  
  // read all the token values
  for (i = 0; i < tokvals; i++) {
    if (readData(&buf, 8, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&token, len, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    if (readData(&buf, 8, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&value, len, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    appendTokenValueToEnv(token, value, env); 
  }

  // find out how many files there are
  if (readData(&buf, 8, sockfd) != E_OK)
    return NULL;
  files = strtol(buf, NULL, 10);
  dhufree(buf);
  
  for (i = 0; i < files; i++) {
    if (readData(&buf, 8, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&name, len, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    if (readData(&buf, 8, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&contenttype, len, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    if (readData(&buf, 8, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    len = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&filename, len, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    if (readData(&buf, 8, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    datalen = strtol(buf, NULL, 10);
    dhufree(buf);
    if (readData(&data, datalen, sockfd) != E_OK) {
      freeEnv(env);
      return NULL;
    }
    appendFileObjectToEnv(name, contenttype, filename, data, datalen, env);
  }

  // SUCCESS
  return env;
}
