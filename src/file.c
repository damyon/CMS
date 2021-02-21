#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#include <direct.h>
#include <process.h>
#include <sys/locking.h>
#include "win32.h"
#else
#include <unistd.h>
#include <sys/file.h>
#define O_BINARY 0
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "strings.h"
#include "logging.h"
#include "errors.h"
#include "file.h"
#include "config.h"
#include "malloc.h"


#define DATAFILE ".data"
#define LOCKFILE ".lock"

/************************************************************************
* getFileSize...
*
************************************************************************/
int getFileSize(char *filepath) {
  struct stat filestats;
  int size = 0;

  if (!filepath)
    return 0;
  stat(filepath, &filestats);
 
  size = filestats.st_size;
  return size;
}

/************************************************************************
* getTmpFile...
*
* Generate the name of a tmp file.
************************************************************************/
char *getTmpFile(int seed) {
  char *path = NULL;
  char *repository = NULL;
  char *seedstr = NULL;
  char *pidstr = NULL;

  repository = getRepositoryPath();

  int2Str(getpid(), &pidstr);
  int2Str(seed, &seedstr);
  vstrdupcat(&path, getTmpDirPath(), "/tmpfile", seedstr, "-", pidstr, NULL);
  dhufree(pidstr);
  dhufree(seedstr);

  return path;
}

/************************************************************************
* deleteObjectFromDisk...
*
* Delete this file from the repository.
************************************************************************/
int deleteObjectFromDisk(int objid) {
  char *path = NULL;
  int errcode = 0, lock = 0, i = 0;
  char *scriptfile = NULL, *ptr = NULL;

  if ((errcode = generateFilePath(objid, &path)) != E_OK) {
    return errcode;
  }

  lock = lockDirectory(path);
  if (lock < 0) {
    logError("Could not get directory lock.");
    dhufree(path);
    return FILEWRITEERROR;
  }

  if ((errcode = unlink(path)) != E_OK) {
    logError("Could not unlink file.");
    unlockDirectory(lock);
    dhufree(path);
    return FILEWRITEERROR;
  }

  if (path == NULL)
    return -1;

  scriptfile = (char *) dhumalloc(sizeof(char) * (strlen(path) + 256));
  
  ptr = path + strlen(path) - 1; 

  while (*ptr != '/' && ptr != path) ptr--;
    *ptr = CNULL;
 
  do {
    sprintf(scriptfile, "%s/%d", path, i++);
  } while (unlink(scriptfile) == 0);

  unlockDirectory(lock);
  dhufree(scriptfile);
  dhufree(path);
  return E_OK;
}

/************************************************************************
* writeObjectToDisk...
*
* Write this file into the repository.
************************************************************************/
int writeObjectToDisk(char *data, int datalen, int objid) {
  char *path = NULL;
  int errcode = 0, lock = 0;

  if ((errcode = generateFilePath(objid, &path)) != E_OK) {
    return errcode;
  }

  lock = lockDirectory(path);
  if (lock < 0) {
    logError("Could not get directory lock.");
    dhufree(path);
    return LOCKFILETIMEOUT;
  }

  if ((errcode = writeFile(path, data, datalen)) != E_OK) {
    unlockDirectory(lock);
    dhufree(path);
    return errcode;
  }

  unlockDirectory(lock);
  dhufree(path);
  return E_OK;
}

/************************************************************************
* generateScriptPath...
*
* Work out the script path to the object referenced.
************************************************************************/
int generateScriptPath(int objectid, int node, char **filepath) {
  char *repository = NULL,
       *filename = NULL;
  int i1, i2, i3, i4, i5;

  i1 = (objectid / 100000000) % 100;
  i2 = (objectid /   1000000) % 100;
  i3 = (objectid /     10000) % 100; 
  i4 = (objectid /       100) % 100;
  i5 = objectid               % 100;

  repository = getRepositoryPath();

  filename = (char *) dhumalloc(sizeof(char) * ( strlen(repository) + 1024));
  sprintf(filename, "%s/%.2d/%.2d/%.2d/%.2d/%.2d/script%d.cmsc", repository, i1, i2, i3, i4, i5, node);

  *filepath = filename; 
  return E_OK;
}

/************************************************************************
* generateFilePath...
*
* Work out the file path to the object referenced.
************************************************************************/
int generateFilePath(int objectid, char **filepath) {
  char *repository = NULL,
       *filename = NULL;
  int i1, i2, i3, i4, i5;

  i1 = (objectid / 100000000) % 100;
  i2 = (objectid /   1000000) % 100;
  i3 = (objectid /     10000) % 100; 
  i4 = (objectid /       100) % 100;
  i5 = objectid               % 100;

  repository = getRepositoryPath();

  filename = (char *) dhumalloc(sizeof(char) * ( strlen(repository) + 1024));
  sprintf(filename, "%s/%.2d/%.2d/%.2d/%.2d/%.2d/%s", repository, i1, i2, i3, i4, i5, DATAFILE);

  *filepath = filename; 
  return E_OK;
}

/****************************
* fileExists...
*
* tests whether a file exists.
****************************/
int fileExists(char *fname) {
  struct stat fstat;
  return (stat(fname, &fstat) == 0);
}

/****************************
* dirExists...
*
* wrapper for file exists.
****************************/
int dirExists(char *dirname) {
  return fileExists(dirname);
}

/****************************
* makeDirPath...
*
* Creates the complete directory.
****************************/
int makeDirPath(char *dirname) {
  char *dirsep = NULL;
  char *current = NULL;
  char sep = '/';
  current = dirname;
  int mode = 0755;

#ifdef ISPVERSION
  mode = 0777;
#endif
  while (strchr("/\\", *current) != NULL)
    current++;

  while ((dirsep = strpbrk(current, "/\\")) != NULL) {
    sep = *dirsep;
    *dirsep = CNULL;
    if (strlen(dirname) > 3) {
      if (!dirExists(dirname)) {
        if (mkdir(dirname, mode) != 0) {
          return -1;
        }
      }
    }
    *dirsep = sep;
    current = dirsep+1;
  }
  if (!dirExists(dirname))
    if (mkdir(dirname, mode) != 0)
      return -1;

  return 0;
}


/*******************************
* openLockFile...
*
* puts a lock file in this directory.
*******************************/
int openLockFile(char *dirname, int mode) {
  int lockfile = 0, result = 0;
  char *filename = NULL;

  if (!dirExists(dirname))
    makeDirPath(dirname);

  filename = (char *) dhumalloc(sizeof(char) * (strlen(dirname) +
                                          strlen(LOCKFILE) + 2));
  sprintf(filename, "%s/%s", dirname, LOCKFILE);

  if ((lockfile = open(filename, mode|O_CREAT|O_BINARY, S_IRWXU)) < 0)
    result = errno;
  dhufree(filename);

#ifdef WIN32
  _setmode( lockfile, _O_BINARY );
#endif

  if (lockfile < 0)
    errno = result;

  return lockfile;
}

/************************************************************************
* lockDirectory...
*
* Get a file lock on the directory.
************************************************************************/
int lockDirectory(char *filename) {
  int lock = 0;
  char *lockfile = NULL, *ptr = NULL;
  struct timeval tv, t;
  struct timespec ts, tr;
  int LOCKTIMEOUT = 10, err = 0;

  if (filename == NULL)
    return -1;

  lockfile = (char *) dhumalloc(sizeof(char) * (strlen(filename) + strlen(LOCKFILE) + 1));
  
  strcpy(lockfile, filename);
  ptr = lockfile + strlen(lockfile) - 1; 

  while (*ptr != '/' && *ptr != '\\' && ptr != lockfile) ptr--;
    *ptr = CNULL;
 
  lock = openLockFile(lockfile, O_WRONLY);
  if (lock == -1)
    perror("Directory Lock File Error:");

  // get flock
  gettimeofday(&tv, NULL);

  tv.tv_sec += LOCKTIMEOUT;

  gettimeofday(&t, NULL);

  // wait up to 10 seconds for a lock
  err = LOCKFILETIMEOUT;
  while (t.tv_sec < tv.tv_sec || ((t.tv_sec == tv.tv_sec) && (t.tv_usec < tv.tv_usec))) {
    if ( flock(lock, LOCK_EX | LOCK_NB ) == 0 ) {
      err = 0;
      break;
    }

    // random nanosleep
    //
    ts.tv_sec = 0;
    ts.tv_nsec = rand() % 10000000;
    nanosleep(&ts, &tr);
    gettimeofday(&t, NULL);
  }

  if (err == LOCKFILETIMEOUT) {
    logError("Lock file timeout: Could be recursive include.");
    dhufree(lockfile);
    return -1;
  }

  dhufree(lockfile);
  return lock;
}

/************************************************************************
* unlockDirectory...
*
* Release a file lock on the directory.
************************************************************************/
void unlockDirectory(int lock) {
  if ( flock(lock, LOCK_UN) != 0 )
    perror("Release Lock File Error:");

  if ( close(lock) != 0 )
    perror("Close Lock File Error:");
  return;
}

/************************************************************************
* readFileData...
*
* Reads data from the file handle.
************************************************************************/
int readFileData(void *thedata, int datalen, int sockfd) {
  int len = 0, total = 0;
  char *data = (char *) thedata;

  while (total < datalen) {
    errno = 0;
    len = read(sockfd, data + total, datalen - total);
    if (len > 0) {
      total += len;
    } else {
	  if (errno != EINTR) {
        return -1;
      }
	}
    
  }
  return E_OK;
} 

/************************************************************************
* writeFileData...
*
* Sends data down the file handle.
************************************************************************/
int writeFileData(void *thedata, int datalen, int sockfd) {
  int len = 0, total = 0;
  char *data = (char *) thedata;

  while (total < datalen) {
    errno = 0;
    len = write(sockfd, (char *)data + total, datalen - total);
    if (len > 0) {
      total += len;
    } else {
	  if (errno != EINTR) {
        return -1;
      }
	}
    
  }
  return E_OK;
} 


/************************************************************************
* readFile...
*
* Read a file into memory
************************************************************************/
int readFile(char *filepath, char **filecontents, int *filelength) {
  int  fd = 0;
  int len = 0;
  char *input = NULL;

  if ((fd = open(filepath, O_RDONLY|O_BINARY)) == -1)
    return FILEREADERROR;

#ifdef WIN32
  _setmode( fd, _O_BINARY );
#endif

  if ((input = readOpenFile(fd, &len)) == NULL) {
    close(fd);
    return FILEREADERROR;
  }

  close(fd);

  *filelength = len;
  *filecontents = input;
  return E_OK;
}

/************************************************************************
* writeFile...
*
* Write a file to disk
************************************************************************/
int writeFile(char *filepath, char *filecontents, int filelength) {
  FILE *fd = NULL;
  int written = 0, err = 0;

  if ((fd = fopen(filepath, "wb")) == NULL)
    return FILEWRITEERROR;

  while ((written < filelength) && (err == 0)) {
    err = fwrite(filecontents + written, sizeof(char), filelength - written, fd);
    if (err == 0) {
      err = ferror(fd);
    } else {
      written += err;
      err = 0;
    }
  }

  fclose(fd);
  if (err != 0)
    return FILEWRITEERROR;

  return E_OK;
}

