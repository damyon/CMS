/***********************************************************
* env.h
*
* This file contains the structs that are used to pass data between the servers.
***********************************************************/

#ifndef _ENV_H
#define _ENV_H

#ifdef __cplusplus
extern "C" {
#endif


#define COOKIE_DATA "COOKIE"
#define SAVEAS_FILENAME "SAVEASFILENAME"

/****************************************************
* A struct to hold all the value token pairs passed in the environment. 
****************************************************/
typedef struct _ValueToken_ {
  char *value;
  char *token;
  struct _ValueToken_ *next;
} ValueToken;

/****************************************************
* A Struct to hold a list of files that have been uploaded.
****************************************************/
typedef struct _FileObject_ {
  char *name;
  char *data;
  char *contenttype;
  char *filename;
  int datalen;
  struct _FileObject_ *next;
} FileObject;

/****************************************************
* A Struct to hold the complete environment (files and tokens)
****************************************************/
typedef struct _Env_ {
  ValueToken *tokenhead, *tokentail;
  FileObject *filehead, *filetail;
} Env;


/****************************************************
* initFileObject...
* 
* Init a single FileObject struct.
****************************************************/
FileObject *initFileObject(char *name, char *contenttype, char *filename, char *data, int datalen);

/****************************************************
* freeFileObjectList...
* 
* Free the linked list of files.
****************************************************/
void freeFileObjectList(FileObject *file);

/****************************************************
* appendFileObjectToList...
* 
* Add this file object to the end of the list.
****************************************************/
void appendFileObjectToList(FileObject **head, FileObject **tail, char *name, char *contenttype, char *filename, char *data, int datalen);

/****************************************************
* initValueToken...
* 
* Init a single ValueToken struct.
****************************************************/
ValueToken *initValueToken(char *token, char *value);

/****************************************************
* freeTokenValueList...
* 
* Free the linked list of value tokens.
****************************************************/
void freeTokenValueList(ValueToken *valtok);

/**************************************************** 
* setTokenValue...
* 
* Set this value token pair in the env.
****************************************************/
void setTokenValue(char *token, char *value, Env *env);

/****************************************************
* appendTokenValueToList...
* 
* Add this value token pair to the end of the list.
****************************************************/
void appendTokenValueToList(ValueToken **head, ValueToken **tail, char *token, char *value);

/****************************************************
* appendFileObjectToEnv...
* 
* Add this file to the end of the env.
****************************************************/
void appendFileObjectToEnv(char *name, char *contenttype, char *filename, char *data, int datalen, Env *env);

/****************************************************
* appendTokenValueToEnv...
* 
* Add this value token pair to the end of the env.
****************************************************/
void appendTokenValueToEnv(char *token, char *value, Env *env);

/****************************************************
* getTokenValue...
*
* Searches for this token and returns the value
****************************************************/
char *getTokenValue(ValueToken *head, char *token);

/****************************************************
* getEnvValue...
*
* Searches for this token and returns the value
****************************************************/
char *getEnvValue(char *token, Env *env);

/****************************************************
* initEnv...
* 
* Init an Env struct.
****************************************************/
Env *initEnv(void);

/****************************************************
* freeEnv...
* 
* Free an Env struct.
****************************************************/
void freeEnv(Env *env);

/****************************************************
* fileObjectExists...
*
* Look for a file with this name.
****************************************************/
int fileObjectExists(char *name, Env *env);

/****************************************************
* getFileObject...
*
* Get a file object out of the env.
****************************************************/
FileObject *getFileObject(char *name, Env *env);

/****************************************************
* printEnv...
* 
* Print an Env struct.
****************************************************/
void printEnv(Env *env);

/****************************************************
* writeEnvToStream...
*     
* Writes this env directly to the socket.
* This is more efficient than making a copy in memory.
****************************************************/
void writeEnvToStream(int sockfd, Env *env);

/****************************************************
* readEnvFromStream...
* 
* Read this env directly from the socket.
* This is more efficient than making a copy in memory.
****************************************************/
Env *readEnvFromStream(int sockfd);

/****************************************************
* readEnvFromStreamNoFile...
*
* Read this env directly from the socket.
* This is more efficient than making a copy in memory.
* This doesn't read any files off the stream - it leaves
* them ready to be read.
****************************************************/
Env *readEnvFromStreamNoFile(int sockfd);

#ifdef __cplusplus
}
#endif


#endif
