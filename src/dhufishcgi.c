/***********************************************************
* dhufishcgi.c
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
#include <sys/reg.h>
#endif
#ifdef ISPVERSION
#include "structs.h"
#include "package.h"
#include "dbcalls.h"
#include "project.h"
#include "malloc.h"
#include "config.h"
#include "logging.h"
#include "parser.h"
#include "request.h"
#endif





/* This is the list of environment variables to load from the cgi */
#define CGIENVCOUNT 28
const char *CGIENV[] = {
                    "SERVER_SOFTWARE",
                    "SERVER_NAME",
                    "GATEWAY_INTERFACE",
                    "SERVER_PROTOCOL",
                    "SERVER_PORT",
                    "REQUEST_METHOD",
                    "PATH_INFO",
                    "PATH_TRANSLATED",
                    "SCRIPT_NAME",
                    "QUERY_STRING",
                    "REMOTE_HOST",
                    "REMOTE_ADDR",
                    "AUTH_TYPE",
                    "REMOTE_USER",
                    "REMOTE_IDENT",
                    "CONTENT_TYPE",
                    "CONTENT_LENGTH",
                    "HTTP_FROM",
                    "HTTP_ACCEPT",
                    "HTTP_ACCEPT_ENCODING",
                    "HTTP_ACCEPT_LANGUAGE",
                    "HTTP_USER_AGENT",
                    "HTTP_REFERER",
                    "HTTP_AUTHORIZATION",
                    "HTTP_CHARGE_TO",
                    "HTTP_IF_MODIFIED_SINCE",
                    "HTTP_PRAGMA",
                    "HTTP_COOKIE"
                 };


/****************************************************
* loadEnvironmentSettings...
* 
* Adds the list of environment settings to the
* value token list.
****************************************************/
void loadEnvironmentSettings(Env *env) {
  int i = 0;

  for (i = 0; i< CGIENVCOUNT; i++) {
    appendTokenValueToEnv(strdup(CGIENV[i]), strdup(getenv(CGIENV[i])?getenv(CGIENV[i]):""), env);
  }

}

/****************************************************
* isMultiPartForm...
* 
* Is this a multi part upload?
****************************************************/
int isMultiPartForm(Env *env) {
  char *contenttype = NULL;

  contenttype = getEnvValue("CONTENT_TYPE", env);
  if (strstr(contenttype?contenttype:"", "multipart/form-data") != NULL)
    return 1;
  return E_OK;
}

/**********************************************************
* mystrstrn...
*
* This does a search through the string for the token.
* This will search through binary data and doesn't
* assume a CNULL is the end of the line.
* It will search up to n characters for the next match.
**********************************************************/
char *mystrstrn(char *str, char *pat, int n) {
  char *ptr = str;
  unsigned int strlenpat = strlen(pat);
  int i = 0, max = n-strlenpat;

  for (i = 0; i< max; i++) {
    if (strncmp(pat, ptr, strlenpat) == 0)
      return ptr;
    else
      ptr++;
  }
  return NULL;
}

/**********************************************************
* mystrrstrn...
*
* This does a search through the string for the token.
* This will search through binary data and doesn't
* assume a CNULL is the end of the line.
* It will search up to n characters for the next match.
**********************************************************/
char *mystrrstrn(char *str, char *pat, int n) {
  unsigned int strlenpat = strlen(pat);
  int i = 0, max = n-strlenpat;
  char *ptr = str + max;

  for (i = max; i >= 0; i--) {
    if (strncmp(pat, ptr, strlenpat) == 0)
      return ptr;
    else
      ptr--;
  }
  return NULL;
}

/**********************************************************
* getDoubleNewLine...
*
* This function makes ptr point to either the next \n\n
* or the next \r\n\r\n.
**********************************************************/
void getDoubleNewLine(char *str, char **ptr) {
  char *lf = NULL, *cr = NULL;

  lf = strstr(str, "\n\n");
  cr = strstr(str, "\r\n\r\n");

  if (lf == NULL && cr == NULL) {
    *ptr = NULL;
  } else {
    if (lf == NULL) {
      *ptr =  cr;
    } else if (cr == NULL) {
      *ptr =  lf;
    } else if (lf < cr) {
      *ptr =  lf;
    } else {
      *ptr =  cr;
    }
  }
}

/**********************************************************
* strstri...
*
* case insensitive strstr
**********************************************************/
char *strstri(char *source, char *pattern) {
  char *ptr = source;
  while ((strncasecmp(ptr, pattern, strlen(pattern)) != 0) && (*ptr != CNULL)) ptr++;
  if (*ptr == CNULL)
    ptr = NULL;
  return ptr;
}

/**********************************************************
* getHeaderValue...
*
* this returns the value of the specified haeder (as a regex)
* or NULL if the header is not present.
**********************************************************/
char *getHeaderValue(char *header, char *value) {
  char *start = NULL, *end = NULL, c = '\0', *result = NULL;

  if (header == NULL || value == NULL)
    return NULL;

  if ((start = strstri(header, value)) == NULL) {
    return NULL;
  }

  end = strchr(start, ':');
  if (end == NULL) {
    return NULL;
  }

  start = end + 2;
  end = strpbrk(start, ";\r\n");
  if (end != NULL) {
    c = *end;
    *end = CNULL;
  }
  result = strdup(start);
  if (end != NULL)
    *end = c;
  return result;
}


/**********************************************************
* getMultiName...
*
* given a multi part field, will return the name
**********************************************************/
char *getMultiName(char *field) {
  char *start = NULL, *end = NULL, *result = NULL, c = '\0';

  if (field == NULL)
    return NULL;

  if ((start = strstr(field, " name=")) == NULL)
    return NULL;

  start += strlen(" name=");
  if (*start == '"')
    start++;
  end = strpbrk(start, "\"\r\n");
  if (end == NULL) {
    end = start + strlen(start);
  }

  c = *end;
  *end = CNULL;
  result = strdup(start);
  *end = c;

  return result;
}

/**********************************************************
* getMultiFileName...
*
* given a multi part field, will return the filename
**********************************************************/
char *getMultiFileName(char *field) {
  char *start = NULL, *end = NULL, *result = NULL, c = '\0';

  if (field == NULL)
    return NULL;

  if ((start = strstr(field, " filename=")) == NULL)
    return NULL;

  start += strlen(" filename=") +1;
  end = strchr(start, '"');
  if (end == NULL)
    return NULL;

  c = *end;
  *end = CNULL;
  result = strdup(start);
  *end = c;

  return result;
}

/**********************************************************
* fixMacFile...
*
* This takes a mac file (application/x-macbinary) and
* gets the real file type out of the header, then it
* stips out the data fork.
**********************************************************/
int fixMacFile(char *file, int length, char **datafork, char **filetype, char **filename, int *thedatalength) {
  int datalength = 0, bvalue = 0, df = 0;
  unsigned char filenamelength = 0;
  char macfilename[64], macfiletype[5];
  char *data = NULL;

  if (length < 128) {
    return 1;
  }

  filenamelength = file[1];

  if (filenamelength > 63 || filenamelength < 1) {
    return 1;
  }

  strncpy(macfilename, file+2, filenamelength);
  macfilename[filenamelength] = CNULL;

  strncpy(macfiletype, file + 65, 4);
  macfiletype[4] = CNULL;

  df = (int) file[83];
  df = df << 8;
  bvalue = (int) file[84];
  df = df | bvalue;
  df = df << 8;
  bvalue = (int) file[85];
  df = df | bvalue;
  df = df << 8;
  bvalue = (int) file[86];
  df = df | bvalue;
  datalength = (int) df;

  if (datalength < 0) {
    return 1;
  }

  data = (char *) malloc(sizeof(char)*datalength);
  if (data == NULL)
    return 1;

  memcpy(data, file + 128, (unsigned) datalength);
  *datafork = data;

  *filetype = (char *) strdup(macfiletype);
  if (*filetype == NULL) {
    n_free(data);
    return 1;
  }

  *filename = (char *) strdup(macfilename);
  if (*filename == NULL) {
    n_free(data);
    n_free(filetype);
    return 1;
  }
  *thedatalength = datalength;
  return 0;
}


/**********************************************************
* processUploadField...
*
* This processes a single (guaranteed) upload field in a HTML page.
* There are some issues.
* If the mime-type is mac-xbinary we
* need to kill the first 40 bytes. (stupido macs).
**********************************************************/
int processUploadField(char *data, char *name, char *contenttype, char *fname, int datalength, Env *env) {
  char *thedata = NULL, *thefilename = NULL, *thefiletype = NULL;
  int thedatalength = 0;

  if (strstr(contenttype, "application/x-macbinary") != NULL) {
    /* In this case we ignore it */
    if (fixMacFile(data, datalength, &thedata, &thefiletype, &thefilename, &thedatalength) != 0)
      return 1;
    appendFileObjectToEnv(name, thefiletype, thefilename, thedata, thedatalength, env);
    n_free(fname);
    n_free(data);
  } else if (strstr(contenttype, "image/x-citrix-pjpeg") != NULL) {
    appendFileObjectToEnv(name, strdup("image/jpeg"), fname, data, datalength, env);
  } else {
    appendFileObjectToEnv(name, contenttype, fname, data, datalength, env);
  }

  return 0;
}

/****************************************************
* isRestrictedToken...
* 
* Prevent malicious users escalating their privledges.
****************************************************/
int isRestrictedToken(char *token) {
  if (token == NULL)
    return 1;
  if ((strcasecmp(token, "ISSUPERUSER") == 0) || 
      (strcasecmp(token, "USERID") == 0)) 
    return 1;
  return 0;
}

/****************************************************
* parseMultiPartForm...
* 
* parses a multi-part form.
* it separates out the form variables and adds them to the
* user variables list. It separates out the upload objects
* and puts them in the data list.
****************************************************/
int parseMultiPartForm(char *data, char *boundary, int datalen, Env *env) {
  char *startfield = NULL,
       *endfield = NULL,
       *name = NULL,
       *fname = NULL,
       *value = NULL,
       *contenttype = NULL,
       *header = NULL,
       *footer = NULL,
       *thedata = NULL,
       *eod = NULL, /* end of data */
       c = '\0';
  int datalength = 0;

  if (data == NULL || boundary == NULL)
    return 1;

  startfield = strstr(data, boundary);
  while (startfield != NULL) {
    startfield += strlen(boundary);
    endfield = mystrstrn(startfield, boundary, datalen - (startfield-data));

    if (endfield != NULL) {
      eod = endfield-2;
      if (*(eod) == '\r')
        eod--;
      *(eod+1) = CNULL;
    }

    /* we have a pointer to this section in startfield */
    /* first split into header and footer */
    header = startfield;
    getDoubleNewLine(header, &footer);
    if (footer != NULL) {
      c = *footer;
      *footer = CNULL;
      if (c == '\r')
        footer += 4;
      else
        footer += 2;
      name = getMultiName(header);
      if (name == NULL)
        return 1;
      fname = getMultiFileName(header);

      value = strdup(footer);
      contenttype = getHeaderValue(header, "content-type");

      if (fname != NULL && *fname != CNULL) {
        if (endfield != NULL) {
          datalength = (eod+1) - footer;
        } else {
          eod = mystrrstrn(footer, boundary, datalen - (footer - data));
          if (eod == NULL) {
            datalength = datalen - (footer - data);
          } else {
            if (*(eod-2) == '\r')
              eod -= 2;
            else
              eod--;
            datalength = eod - footer;
          }
        }
        thedata = (char *) malloc(sizeof(char) * datalength);
        if (thedata == NULL) {
          n_free(name);
          n_free(value);
          n_free(contenttype);
          return 1;
        }
        memcpy(thedata, footer, (unsigned) datalength);
        /* This is an upload object (or a multi-multi object ?) */
        if (processUploadField(thedata, name, contenttype, fname, datalength, env) != 0) {
          n_free(thedata);
          n_free(name);
          n_free(value);
          n_free(contenttype);
          return 1;
        }
      } else {
        /* This is a normal form field */
        if (!isRestrictedToken(name))
          appendTokenValueToEnv(name, value, env);
      }
    }
    /* get the next section */
    if (endfield != NULL) {
      startfield = endfield;
    } else {
      startfield = NULL;
    }
  }

  return E_OK;
}

/****************************************************
* parseCommandLine...
* 
* Read all user tokens from command line.
****************************************************/
void parseCommandLine(int argc, char **argv, Env *env) {
  int i = 0;
  
  for (i = 1; i < argc; i+=2) {
    if (!isRestrictedToken(argv[i])) {
      appendTokenValueToEnv(strdup(argv[i]), strdup(argv[i+1]), env);
    }
  }
}

/****************************************************
* parseQueryString...
* 
* Read all user tokens from query string.
****************************************************/
void parseQueryString(Env *env) {
  char *querystring = getEnvValue("QUERY_STRING", env), 
       *ptr = NULL, *token = NULL, *value = NULL;
  
  if (querystring == NULL)
    return;

  ptr = querystring;
  while (ptr != NULL && *ptr != CNULL) {
    token = ptr;
    value = strchr(token, '=');
    if (value == NULL)
      return;
    *value = CNULL;
    value++;
    if (*value == CNULL) {
      // INVALID QUERY STRING
      return;
    } 
    ptr = strchr(value, '&');
    if (ptr != NULL) {
      *ptr = CNULL;
      ptr++;
    }
    if (!isRestrictedToken(token))
      appendTokenValueToEnv(URLDecode(token), URLDecode(value), env);
  }
}


/****************************************************
* loadMultiPartValues...
* 
* Load all variables from a multi part form (may contain files).
****************************************************/
void loadMultiPartValues(Env *env) {
  char              *ptr = NULL,
                    *boundary = NULL,
                    *data = NULL,
                    *current = NULL,
                    *contenttype = NULL,
                    *contentlen = NULL;
  int          contentlength = 0,
                    cl = 0,
                    numread = 1; /* numread is 1 to start the while loop */
  
  parseQueryString(env);

  if ((contenttype = getEnvValue("CONTENT_TYPE", env)) == NULL || 
      (contentlen = getEnvValue("CONTENT_LENGTH", env)) == NULL)
    return;

  // get the boundary out of the contenttype
  if ((ptr = strchr(contenttype, '=')) == NULL)
    return;

  ptr += 1;

  boundary = (char *) malloc(sizeof(char)*(strlen(ptr) + 3));

  // Add a -- to the start of the boundary
  boundary[0] = '-';
  boundary[1] = '-';
  boundary[2] = CNULL;
  strcat(boundary, ptr);

  // now read in all the data from stdin
  contentlength = strtol(contentlen, NULL, 10);
  cl = contentlength;
  if (contentlength < 0) {
    n_free(boundary);
    return;
  }

  data = (char *) malloc(sizeof(char) * (contentlength +1));

  data[contentlength] = CNULL;
  current = data;
  while ((numread > 0) && (contentlength > 0)) {
    numread = fread(current, sizeof(char), (unsigned)contentlength, stdin);
    contentlength -= contentlength;
    current += numread;
  }

  if (parseMultiPartForm(data, boundary, cl, env) != 0) {
    n_free(data);
    n_free(boundary);
    return;
  }

  n_free(data);
  n_free(boundary);

}

/****************************************************
* parseStdin...
* 
* Read all user tokens from stdin.
****************************************************/
void parseStdin(Env *env) {
  char *contentlength = NULL, *querystring = NULL, *current = NULL, 
       *token = NULL, *value = NULL, *ptr = NULL;
  int length = 0, numread = 1;
 
  contentlength = getEnvValue("CONTENT_LENGTH", env); 

  if (contentlength == NULL || *contentlength == CNULL || strtol(contentlength, NULL, 10) <= 0)
    return;

  length = strtol(contentlength, NULL, 10);

  querystring = (char *) malloc(sizeof(char) * (length + 1)); 
  querystring[length] = CNULL;

  current = querystring;
  while ((numread > 0) && (length > 0)) {
    numread = fread(current, sizeof(char), length, stdin);
    length -= numread;
    current += numread;
  }
  ptr = querystring;
  while (ptr != NULL && &ptr != CNULL) {
    token = ptr;
    value = strchr(token, '=');
    if (value == NULL) {
      n_free(querystring);
      return;
    }
    *value = CNULL;
    value++;
    if (*value == CNULL) {
      // INVALID QUERY STRING
      return;
    }
    ptr = strchr(value, '&');
    if (ptr != NULL) {
      *ptr = CNULL;
      ptr++;
    }
    if (!isRestrictedToken(token))
      appendTokenValueToEnv(URLDecode(token), URLDecode(value), env);
  }

  n_free(querystring);
}

/****************************************************
* loadCookieData...
* 
* Adds the list of cookie variables to the environment.
****************************************************/
void loadCookieData(Env *env) {
  char *querystring = getEnvValue("HTTP_COOKIE", env), 
       *ptr = NULL, *token = NULL, *value = NULL;
  
  if (querystring == NULL)
    return;

  ptr = querystring;
  while (ptr != NULL && *ptr != CNULL) {
    token = ptr;
    value = strchr(token, '=');
    if (value == NULL) {
      return;
    }

    *value = CNULL;
    value++;
    if (*value == CNULL) {
      return;
    } 
    ptr = strchr(value, ';');
    if (ptr != NULL) {
      *ptr = CNULL;
      ptr++;
      while (isspace(*ptr) && *ptr != CNULL) ptr++;
    }
    appendTokenValueToEnv(URLDecode(token), URLDecode(value), env);
  }
}

/****************************************************
* loadUserDefinedVariables...
* 
* Adds the list of user defined variables to the environment.
****************************************************/
void loadUserDefinedVariables(Env *env) {
  char *method = NULL;
  
  method = getEnvValue("REQUEST_METHOD", env); 

  if (strcasecmp(method?method:"", "POST") == 0) {
    // Method is POST - read from stdin
    parseQueryString(env);
    parseStdin(env);
  } else {
    // Method is GET - read from QUERY_STRING
    parseQueryString(env);
  }
}

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
* handleError...
*
* Return the error to the user.
***********************************************************/
void handleError(char *errmsg) {
  printf("Content-Type: text/html\n");
  printf("\n");
  printf("<html><title>Error.</title><body><p style=\"padding: 5em;\">%s</p></body></html>\n", errmsg);

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

/***********************************************************
* streamData...
*
* Stream the file from the socket onto stdout without reading
* it all into memory.
***********************************************************/
void streamData(int datalen, int sockfd) {
  char buf[1024];
  int read = 0, status = E_OK;
  
  memset(buf, 0, sizeof(buf));

  
  while (read < datalen && status == E_OK) {
    if ((status = readDataStatic(buf, datalen - read > 1024?1024:datalen - read, sockfd)) == E_OK) {
      fwrite(buf, sizeof(char), (datalen - read > 1024?1024:datalen - read), stdout);
      read += 1024;
    }
  }
  
}

char *strip(char *data) {
  char *ptr = NULL;

  ptr = strpbrk(data, "\r\n");
  if (ptr)
    *ptr = '\0';
  return data;
}

int dumpFile(char *data, int datalen) {
  int len = 0, total = 0;
  errno = 0; 
  
  while (total < datalen) {
    len = fwrite((char *) data + total, sizeof(char), datalen - total, stdout);
    if (len > 0) {
      total += len;
    } else {
      if (errno != EINTR) {
        return errno;
      }
    }
  }
  return E_OK;
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

/***********************************************************
* main...
*
* This is the main. This cgi is executed by the webserver
* every time a request is made to the server.
***********************************************************/
int main(int argc, char **argv) {
  int errnum = 0;
  Env *env = initEnv();
  FileObject *file = NULL;
  char *error = NULL, *cookie = NULL, *filename = NULL;
#ifndef ISPVERSION
  int retries = 0, connected = 0;
#endif



#ifdef ISPVERSION

  if (initDatabase() != E_OK) {
    handleError(getCGIErrorMesg(errnum));
  }

  setPrefix("./");
  if ((errnum = loadConfigFile()) != E_OK) {
    handleError(getCGIErrorMesg(errnum));
  }
#else
  int sockfd = 0;

  if ((errnum = loadCGIConfigSettings()) != E_OK) {
    fprintf(stderr, "[%d] Could not load config settings\n", getpid());
    handleError(getCGIErrorMesg(errnum));
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
    handleError("Invalid license key.");
  }
#endif

  // load environment
  loadEnvironmentSettings(env);

  // load command line args
  //
  parseCommandLine(argc, argv, env);


  // load user defined variables

  if (isMultiPartForm(env)) {
    loadMultiPartValues(env); 
  } else {
    loadUserDefinedVariables(env);
  }

  loadCookieData(env);

#ifdef ISPVERSION
  void *sqlsock = NULL;

  sqlsock = getDBConnection();

  processRequest(env, sqlsock);

  closeDBConnection(sqlsock);
#else
  // Connect to the server

  retries = 0;
 
  connected = 0;
  while (retries < 5 && !connected) {
    if ((openConnection(&sockfd, getIPAddress(), getPortNum()) == E_OK) &&
         (authenticateCGIConnection(sockfd) == E_OK)) {
      connected = 1;
    }

    retries++;
  }

  if (!connected) {
    fprintf(stderr, "[%d] Could not connect to server\n", getpid());
    handleError(getCGIErrorMesg(CONNECTERROR));
  }
    
  writeEnvToStream(sockfd, env);
  freeEnv(env);
  env = readEnvFromStreamNoFile(sockfd);
  if (env == NULL) {
    fprintf(stderr, "[%d] Could not read valid response %d\n", getpid(), sockfd);
    handleError(getCGIErrorMesg(SERVERFATALERROR));
  }
  
#endif

  if ((file = env->filehead) == NULL) {
    error = getEnvValue("ERRMSG", env);
    if (error == NULL) {
      fprintf(stderr, "[%d] No file and no error in response\n", getpid());
      handleError(getCGIErrorMesg(SERVERFATALERROR));
    } else {
      fprintf(stderr, "[%d] Server returned error response\n", getpid());
      handleError(error);
    }
  }

  cookie = getEnvValue(COOKIE_DATA, env);
  if (cookie) {
    fprintf(stdout, "Set-Cookie: %s; path=/;\r\n", cookie);
  }
  
  fprintf(stdout, "Content-Type:%s\r\n", file->contenttype);
  if (strncasecmp(file->contenttype, "text", 4) != 0) {
    time_t now;
    now = time(NULL) + 24*60*60;
    fprintf(stdout, "Expires: %s GMT\r\n", strip(ctime(&now)));
  }

  filename = getEnvValue(SAVEAS_FILENAME, env);
  if (filename) {
    fprintf(stdout, "Content-Disposition: attachment; filename=\"%s\"\r\n", filename);
    fprintf(stderr, "Content-Disposition: attachment; filename=\"%s\"\r\n", filename);
  }

  fprintf(stdout, "Content-Length:%d\r\n\r\n", file->datalen);

#ifdef ISPVERSION
  dumpFile(file->data, file->datalen);
#else
  // read in datalen bytes and write to stdout
  streamData(file->datalen, sockfd);

  closeConnection(sockfd);
#endif

 
  freeEnv(env);

#ifdef ISPVERSION
  closeDatabase();
#endif

#ifdef WIN32
  win32shutdown();
#endif
  return 0;
}
