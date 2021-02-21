#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#ifdef WIN32
#include "win32.h"
#include <process.h>
#else
#include <unistd.h>
#endif
#include "config.h"
#include "strings.h"
#include "malloc.h"
#include "logging.h"
#include <time.h>
#include <sys/stat.h>

void rotateFile(char *filename) {
  struct stat st;
  int maxlogsize = getMaxLogSize();
  char *tmp = NULL, numbuf[128], *newfilename = NULL;
  int nextnum = 1, foundfile = 0, i = 0;

  if (stat(filename, &st) == 0) {
    if (st.st_size >= maxlogsize) {
      while (!foundfile && (nextnum < getMaxLogFiles())) {
        sprintf(numbuf, "%d", nextnum);
        vstrdupcat(&newfilename, filename, ".", numbuf, NULL);
        if (stat(newfilename, &st) != 0) {
          foundfile = 1;
        }
        dhufree(newfilename);
        nextnum++;
      }
 
      i = nextnum - 1;
      while (i > 1) {
        sprintf(numbuf, "%d", i--);
        vstrdupcat(&newfilename, filename, ".", numbuf, NULL);
        sprintf(numbuf, "%d", i);
        vstrdupcat(&tmp, filename, ".", numbuf, NULL);
        rename(tmp, newfilename);
        dhufree(newfilename);
        dhufree(tmp);
      }
      sprintf(numbuf, "%d", 1);
      vstrdupcat(&tmp, filename, ".", numbuf, NULL);
      rename(filename, tmp);
      dhufree(tmp);
    } 
  }
}

/*******************************************************************************
* logDebug...
*
* Print a message to the debug log.
*******************************************************************************/
void logDebug(char *msg, ...) {
  char *logname = getDebugLogFile(), *date = NULL, *ptr = NULL;
  FILE *logfile = NULL;
  va_list fmt;
  time_t now = time(NULL);
  
  rotateFile(logname);

  date = ctime(&now);
  ptr = strpbrk(date, "\r\n");
  if (ptr)
    *ptr = CNULL;
  if (getDebugStatus() && logname) {
    /* Open the log file */
    logfile = fopen(logname, "ab");
    va_start(fmt, msg);       
    if (logfile) {
      /* Print a formatted string to the log file */
      fprintf(logfile, "[ %.6d ] [ %s ] [ DEBUG ] ", (int)getpid(), date); 
      vfprintf(logfile, msg, fmt); 
      fclose(logfile);
    }
    va_end(fmt);
    /* Done */
  }
}

/*******************************************************************************
* logInfo...
*
* Print a message to the info log.
*******************************************************************************/
void logInfo(char *msg, ...) {
  char *logname = getInfoLogFile(), *date = NULL, *ptr = NULL;
  FILE *logfile = NULL;
  va_list fmt;
  time_t now = time(NULL);

  rotateFile(logname);

  date = ctime(&now);
  ptr = strpbrk(date, "\r\n");
  if (ptr)
    *ptr = CNULL;

  /* Open the log file */
  if (getInfoStatus() && logname) {
    logfile = fopen(logname, "ab");
    va_start(fmt, msg);       
    if (logfile) {
      /* Print a formatted string to the log file */
      fprintf(logfile, "[ %.6d ] [ %s ] [ INFO ] ", (int)getpid(), date); 
      vfprintf(logfile, msg, fmt); 
      fclose(logfile);
    }
    va_end(fmt);
    /* Done */
  }
}

/*******************************************************************************
* logError...
*
* Print a message to the error log.
*******************************************************************************/
void logError(char *msg, ...) {
  char *logname = getErrorLogFile(), *date = NULL, *ptr = NULL;
  FILE *logfile = NULL;
  va_list fmt;
  time_t now = time(NULL);

  rotateFile(logname);

  date = ctime(&now);
  ptr = strpbrk(date, "\r\n");
  if (ptr)
    *ptr = CNULL;

  if (getErrorStatus() && logname) {
    /* Open the log file */
    logfile = fopen(logname, "ab");
    va_start(fmt, msg);       
    if (logfile) {
      /* Print a formatted string to the log file */
      fprintf(logfile, "[ %.6d ] [ %s ] [ ERROR ] ", (int)getpid(), date); 
      vfprintf(logfile, msg, fmt); 
      fclose(logfile);
    }

    va_end(fmt);
    /* Done */
  }
}
