/************************************************************************
* file.h
*
* File Input output functions.
************************************************************************/
#ifndef _FILE_H
#define _FILE_H

#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************
* getFileSize...
*
* Get the size of this file.
************************************************************************/
int getFileSize(char *filepath);

/************************************************************************
* getTmpFile...
*
* Generate the name of a tmp file.
************************************************************************/
char *getTmpFile(int seed);

/************************************************************************
* deleteObjectFromDisk...
*
* Delete this file from the repository.
************************************************************************/
int deleteObjectFromDisk(int objid);

/************************************************************************
* writeObjectToDisk...
*
* Write this file into the repository.
************************************************************************/
int writeObjectToDisk(char *data, int datalen, int objid);

/************************************************************************
* generateFilePath...
*
* Work out the file path to the object referenced.
************************************************************************/
int generateFilePath(int objectid, char **filepath);

/************************************************************************
* lockDirectory...
*
* Get a file lock on the directory.
************************************************************************/
int lockDirectory(char *filename);

/************************************************************************
* unlockDirectory...
*
* Release a file lock on the directory.
************************************************************************/
void unlockDirectory(int lock);

/************************************************************************
* readFile...
*
* Read a file into memory
************************************************************************/
int readFile(char *filepath, char **filecontents, int *filelength);

/************************************************************************
* generateScriptPath...
*
* Work out the script path to the object referenced.
************************************************************************/
int generateScriptPath(int objectid, int node, char **filepath);

/************************************************************************
* writeFile...
*
* Write a file to disk
************************************************************************/
int writeFile(char *filepath, char *filecontents, int filelength);

/************************************************************************
* readFileData...
*
* Reads data from the file handle. (Assuming the memory is already allocated.)
************************************************************************/
int readFileData(void *thedata, int datalen, int sockfd);

/************************************************************************
* writeFileData...
*
* Writes the data down the file handle.
************************************************************************/
int writeFileData(void *data, int datalen, int sockfd);

#ifdef __cplusplus
}
#endif


#endif // _FILE_H
