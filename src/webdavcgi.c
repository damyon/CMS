/***********************************************************
* webdavcgi.c
*
* This is the CGI that connects to the Simple Object Store Server.
* It forwards the request with all associated data and any uploaded
* files - then relays the response back to the browser.
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "config.h"
#include "logging.h"
#include "strings.h"
#include "ipc.h"
#include "env.h"
#include "errors.h"
#include "cgicommon.h"
#ifdef WIN32
#include "win32.h"
#include <process.h>
#else
#include <unistd.h>
#include <execinfo.h>
/* get REG_EIP from ucontext.h */
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <ucontext.h>
#endif

#ifdef ISPVERSION
#include "structs.h"
#include "package.h"
#include "dbcalls.h"
#include "project.h"
#include "malloc.h"
#include "config.h"
#include "logging.h"
#include "script.h"
#include "request.h"
#endif


/***********************************************************
* getCGIErrorMesg...
*
* Turn the error number into a message.
***********************************************************/
char *getCGIErrorMesg(int errnum) {
  switch(errnum) {
    case E_OK:
      return "OK";
    case CONFIGFILEREADERROR:
      return "An error occured while reading the config file. Please check this file is correct.";
    case CONNECTERROR:
      return "Could not connect to the server.";
    case SERVERFATALERROR:
      return "An internal error occurred and the server was not able to return the requested page.";
    default:
      return "Unknown error";
  }
  
}

/***********************************************************
* handleDavError...
*
* Return the error to the user.
***********************************************************/
void handleDavError(char *errmsg) {
  printf("Status: 500 %s\n", errmsg);
  printf("\n");

#ifdef ISPVERSION
  closeDatabase();
#endif
  exit(0);
}

/***********************************************************
* printConfigSettings...
*
* Print the config file settings.
***********************************************************/
void printConfigSettings() {
  printf("Content-Type:text/plain\n\n");
  printf("CMS CGI Initialised\n");
  printf("Server IP Address:%s\n", getIPAddress());
  printf("Server Port Number:%d\n", getPortNum());
  exit(0);
}

#ifndef WIN32
/************************************************************************
* segfaultSignalHandler...
*
* Handle shutdown events in the parent.
************************************************************************/
/*
void segfaultSignalHandler(int sig, siginfo_t *info,
                                   void *secret) {

  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;
  ucontext_t *uc = (ucontext_t *)secret;

  // Do something useful with siginfo_t 
  fprintf(stderr, "Got signal %d, faulty address is %p, from %p\n",
                   sig, info->si_addr, (void *) (uc->uc_mcontext.gregs[REG_EIP]));

  trace_size = backtrace(trace, 16);
  // overwrite sigaction with caller's address 
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];

  messages = backtrace_symbols(trace, trace_size);
  // skip first stack frame (points here) 
  fprintf(stderr, "[trace] Execution path:\n");
  for (i=1; i<trace_size; ++i)
        fprintf(stderr, "[trace] %s\n", messages[i]);

  exit(1);
}

*/

/************************************************************************
* installSegSignalHandler...
*
* Set this function as the signal handler for this signal.
************************************************************************/
/*
int installSegSignalHandler(int signum, void (* handler)(int, siginfo_t *, void *))
{
    struct sigaction newact;

    newact.sa_sigaction = handler;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = SA_RESTART | SA_SIGINFO; // restart interrupted OS calls 
    return sigaction(signum, &newact, NULL) ;
}
*/
#endif

void decodePath(char *path) {
    char *pos = path, *dest = path;

    while (*pos != '\0') {
	    if ((*pos == '\\') && (*(pos + 1) == 'x')) {
	      *dest = '%';
	      pos++;
	    } else {
	      *dest = *pos;
	    }
	    pos++;
	    dest++;
    }
    *dest = '\0';
}

int sendDavHeaders(int sockfd, int *contentlength, int *ishead) {
    char *method = NULL, *path = NULL, *value = NULL, *qstr = NULL;

    method = getenv("REQUEST_METHOD");
    if (method == NULL) {
	    method = "OPTIONS";
    }
    path = getenv("REDIRECT_URL");
    if (path == NULL) {
	    path = "/";
    }
    decodePath(path);
    qstr = getenv("REDIRECT_QUERY_STRING");
    if (qstr == NULL) {
	    qstr = "";
    }
    	
    if (strcmp(method, "HEAD") == 0) {
      *ishead = 1;
    }
    sendData(method, strlen(method), sockfd);
    sendData(" ", 1, sockfd);
    sendData(path, strlen(path), sockfd);
    sendData(" HTTP/1.0\n", 10, sockfd);

    value = getenv("CONTENT_LENGTH");
    if (value != NULL) {
        *contentlength = strtol(value, NULL, 10);
        sendData("CONTENT_LENGTH: ", 16, sockfd);
	sendData(value, strlen(value), sockfd);
        sendData("\n", 1, sockfd);
    }

    value = getenv("REDIRECT_AUTHORIZATION");
    if (value != NULL) {
        sendData("AUTHORIZATION: ", 15, sockfd);
	sendData(value, strlen(value), sockfd);
        sendData("\n", 1, sockfd);
    }

    value = getenv("HTTP_USER_AGENT");
    if (value != NULL) {
        sendData("USER_AGENT: ", 12, sockfd);
	sendData(value, strlen(value), sockfd);
        sendData("\n", 1, sockfd);
    }

    value = getenv("HTTP_DEPTH");
    if (value != NULL) {
        sendData("Depth: ", 7, sockfd);
	sendData(value, strlen(value), sockfd);
        sendData("\n", 1, sockfd);
    }
    
    value = getenv("HTTP_OVERWRITE");
    if (value != NULL) {
        sendData("Overwrite: ", 11, sockfd);
	sendData(value, strlen(value), sockfd);
        sendData("\n", 1, sockfd);
    }
    
    value = getenv("HTTP_DESTINATION");
    if (value != NULL) {
        sendData("Destination: ", 13, sockfd);
	sendData(value, strlen(value), sockfd);
        sendData("\n", 1, sockfd);
    }
    // end of headers
    sendData("\n", 1, sockfd);
    return E_OK;
}

int min(int a, int b) {
	if (a > b)
		return a;
	return b;
}

int sendDavBody(int sockfd, int length) {
    // we will stream this...
    char *buffer = NULL;
    int bytesread = 0, readsize = 0;

    if (length <= 0) {
      return E_OK;
    }
    buffer = (char *) malloc(sizeof(char) * length);
    memset(buffer, 0, sizeof(char) * length);
    
    do {
      readsize = fread(buffer, sizeof(char), length - bytesread, stdin);
      if (readsize > 0) {
        sendData(buffer, readsize, sockfd);
        bytesread += readsize;
      }
    } while (bytesread < length);
    free(buffer);

    return E_OK;
}

int readNextLine(int sockfd, char **line, int MAX) {
  char *current = NULL;
  int err = 0;

  current = *line;
  do {
    err = readDataStatic(current, sizeof(char), sockfd);
    if (err != E_OK) {
      return err;
    }
    current++;
  } while ((*(current-1) != '\n') && (((current-1) - (*line)) < MAX));

  return E_OK;
}

/*
 * readResponseHeaders.
 *
 * Read the response headers returned from the server and set them in the apache env.
 */
int readResponseHeaders(int sockfd, int *responselength) {
  char line[4096], *start = NULL;
  
  *responselength = 0;
  
  start = line;
  memset(line, 0, sizeof(char) * 4096);

  if (readNextLine(sockfd, &start, 4096) != E_OK) {
    return -1;
  }

  // convert the http status line to a status header
  start = strchr(start, ' ');
  if (start == NULL) {
    return -1;
  }
  start++;
  
  fwrite("Status: ", sizeof(char), 8, stdout);
  fwrite(start, sizeof(char), strlen(start), stdout);

  // add author via dav header
  fwrite("MS-Author-Via: DAV\r\n", sizeof(char), 20, stdout);

  start = line;
  memset(line, 0, sizeof(char) * 4096);

  while ((readNextLine(sockfd, &start, 4096) == E_OK) && (*start != '\r' && *start != '\n' && *start != '\0')) {
    if (strncmp(line, "Content-Length: ", 16) == 0) {
      *responselength = strtol(line + 16, NULL, 10);
    }

    fwrite(line, sizeof(char), strlen(line), stdout);
    memset(line, 0, sizeof(char) * 4096);
  }
  fwrite("\r\n", sizeof(char), 2, stdout);

  return E_OK;
}


/***********************************************************
* streamResponseBody...
*
* Stream the file from the socket onto stdout without reading
* it all into memory.
***********************************************************/
int streamResponseBody(int sockfd, int responselength) {
  char buf[1024];
  int read = 0, status = E_OK;

  memset(buf, 0, sizeof(buf));

  while (read < responselength && status == E_OK) {
    if ((status = readDataStatic(buf, responselength - read > 1024?1024:responselength - read, sockfd)) == E_OK) {
      fwrite(buf, sizeof(char), (responselength - read > 1024?1024:responselength - read), stdout);
      read += 1024;
    }
  }
  return E_OK;
}


/***********************************************************
* main...
*
* This is the main. This cgi is executed by the webserver
* every time a request is made to the server.
***********************************************************/
int main(int argc, char **argv) {
  int errnum = 0;
#ifndef ISPVERSION
  int retries = 0, connected = 0, ishead = 0;
#endif


#ifdef ISPVERSION
  if (initDatabase() != E_OK) {
    handleDavError(getCGIErrorMesg(errnum));
  }

  setPrefix("./");
  if ((errnum = loadConfigFile()) != E_OK) {
    handleDavError(getCGIErrorMesg(errnum));
  }
#else
  int sockfd = 0;

  if ((errnum = loadCGIConfigSettings()) != E_OK) {
    fprintf(stderr, "[%d] Could not load config settings\n", getpid());
    handleDavError(getCGIErrorMesg(errnum));
  }
#endif

#ifdef WIN32
  win32init();

  if ((errnum = loadConfigFile()) != E_OK) {
     fprintf(stderr, "[%d] Could not load config file\n", getpid());
     logError("Fatal Error:%s\n", getErrorMesg(errnum));
     exit(-1);
  }
   
#endif

#ifndef WIN32
  //installSegSignalHandler(SIGSEGV, segfaultSignalHandler);
#endif
  
#ifdef ISPVERSION
  errnum = validateLicenseKey();
  if (errnum != E_OK) {
    handleDavError("Invalid license key.");
  }
#endif

#ifdef ISPVERSION
  // build and handle webdav request here
#else
  // Connect to the server

  retries = 0;
 
  connected = 0;
  while (retries < 5 && !connected) {
    if ((openConnection(&sockfd, getIPAddress(), getPortNum()) == E_OK) &&
         (authenticateWebDavConnection(sockfd) == E_OK)) {
      connected = 1;
    }

    retries++;
  }

  if (!connected) {
    fprintf(stderr, "[%d] Could not connect to server\n", getpid());
    handleDavError(getCGIErrorMesg(CONNECTERROR));
  }
    
  int length = 0;
  if (sendDavHeaders(sockfd, &length, &ishead) != E_OK) {
    fprintf(stderr, "[%d] Could not read request headers.\n", getpid());
    fflush(stderr);
    handleDavError("Could not read request headers.\n");
  }

  if (length > 0) {
    if (sendDavBody(sockfd, length) != E_OK) {
      fprintf(stderr, "[%d] Could not send request body.\n", getpid());
      fflush(stderr);
      handleDavError("Could not send request body.\n");
    }
  }
  
  if (readResponseHeaders(sockfd, &length) != E_OK) {
    fprintf(stderr, "[%d] Could not read response from server.\n", getpid());
    fflush(stderr);
    handleDavError("Could not read response from server.\n");
  }

  if (ishead) {
    length = 0;
  }

  streamResponseBody(sockfd, length);

#endif

#ifdef ISPVERSION
  closeDatabase();
#endif

#ifdef WIN32
  win32shutdown();
#endif
  return E_OK;
}
