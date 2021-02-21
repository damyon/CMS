#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "project.h"
#include "malloc.h"
#include "config.h"
#include "errors.h"
#include "logging.h"
#include "strings.h"
#include "ipc.h"
#include "md5.h"
#include "env.h"
#include "request.h"
#include "xml.h"
#include "webdav.h"
#include <execinfo.h>

/* get REG_EIP from ucontext.h */
#ifndef __USE_GNU
#define __USE_GNU
#endif 
#include <ucontext.h>


int quitrequest;
int numtofork;
int numforked;

#define MAXFORKS 1024

/* This is passed to the signal handler function */
enum PROCESSTYPE {
  PARENT = 0,
  CHILD };

void setSignalHandlers(int parent);
void signalParent(int sig);

/************************************************************************
* printUsage...
*
* Prints the usage information.
************************************************************************/
void printUsage(char *progname) {
  fprintf(stdout, COPYRIGHT);
  fprintf(stdout, VERSIONINFO);
  fprintf(stdout, "Usage: %s \n", progname);
  fprintf(stdout, "          --help\n");
  fprintf(stdout, "            Prints this usage information.\n");
  fprintf(stdout, "          --prefix\n");
  fprintf(stdout, "            Specifies the installation directory.\n");
  fprintf(stdout, "\n");
}


/************************************************************************
* printStartupInformation...
*
* Log a message on startup.
************************************************************************/
void printStartupInformation() {
  logInfo("Server %s begin accepting requests on port (%d).\n", getServerName(), getPortNumber());
}

/************************************************************************
* printShutdownInformation...
*
* Log a message on shutdown.
************************************************************************/
void printShutdownInformation() {
  logInfo("Server %s is shutting down.\n", getServerName());
}

/************************************************************************
* loadCommandLineArgs...
*
* This parses the comand line arguments and reads the config file.
************************************************************************/
int loadCommandLineArgs(int argc, char **argv) {
  int i = 0, errnum = 0;

  setDefaultSettings();

  setServerName(argv[0]);

  for (i = 1; i < argc; i++) {
    if (strcasecmp(argv[i], "--help") == 0) {
      printUsage(argv[0]);
      exit(0);
    } else if (strcasecmp(argv[i], "--prefix") == 0) {
      if (i == argc - 1) {
        return COMMANDLINEERROR;
      }
      setPrefix(argv[i + 1]);      
    }
  }

  if ((errnum = loadConfigFile()) != E_OK) {
    return errnum;
  }

  printStartupInformation();

  return E_OK;
}

/* Struct to hold a linked list of pids. */
typedef struct _pidlist_ {
  int pid;
  struct _pidlist_ *next;
} PidList;

/************************************************************************
* appendToPidList...
*
* Add this Process ID to the linked list of pids.
************************************************************************/
void appendToPidList(int pid, PidList **thepidlist) {
  PidList *pidlist = *thepidlist,
          *current = pidlist,
          *newpidlist = NULL;
  if (pidlist == NULL) {
    pidlist = (PidList *) malloc(sizeof(PidList));
    pidlist->pid = pid;
    pidlist->next = NULL;
  } else {
    while (current->next != NULL) current = current->next;
    newpidlist = (PidList *) malloc(sizeof(PidList));
    current->next = newpidlist;
    newpidlist->pid = pid;
    newpidlist->next = NULL;
  }
  *thepidlist = pidlist;
}

/************************************************************************
* freePidList...
*
* Free this whole linked list of PIDS.
************************************************************************/
void freePidList(PidList **thepidlist) {
  PidList *pidlist = *thepidlist,
          *nextpid = NULL;
  while (pidlist != NULL) {
    nextpid = pidlist->next;
    n_free(pidlist);
    pidlist = nextpid;
  }
}

/************************************************************************
* processConnection...
*
* Read the data off the socket and process the request.
************************************************************************/
void processConnection(int sockfd) {
  int newsockfd = 0, errcode = 0, type = 0;
  if (acceptConnection(sockfd, &newsockfd) != E_OK) {
    signalParent(SIGUSR1);
    return;
  }
  signalParent(SIGUSR1);
  if (authenticateConnection(newsockfd, &type) == E_OK) {
    if (type == STANDARDREQUEST) {
      if ((errcode = handleRequest(newsockfd)) != E_OK) {
        logError("Standard Request Error: %s\n", getErrorMesg(errcode));
      }
    } else if (type == WEBDAVREQUEST) {
      logDebug("Handle webdav request\n");
      handleWebDavRequest(newsockfd);
    }
  } else {
    logError("Invalid Request Received\n");
  }
  closeConnection(newsockfd);
}

/************************************************************************
* yieldProcessor...
*
* Perform a nanosleep (in case we are threading).
************************************************************************/
void yieldProcessor() {
  struct timespec ts, rem;
  
  ts.tv_sec = 0;
  ts.tv_nsec = 1;

  nanosleep(&ts, &rem);
}

/************************************************************************
* preForkRequestHandlers...
*
* Fork an initial pool of handlers to take requests.
************************************************************************/
void preForkRequestHandlers(PidList **thepids, int sockfd) {
  PidList *pids = NULL;
  int numhandlers = getNumRequestHandlers(),
      i = 0, pid = 0;
   
  numtofork = numhandlers;
  for (i = 0; i < numhandlers; i++) {
    pid = fork();
    if (pid == 0) {
      // Child
      setSignalHandlers(CHILD);
      processConnection(sockfd); 
      _exit(0);
    } else if (pid > 0) {
      // Parent
      numforked = (numforked + 1) % MAXFORKS;
      appendToPidList(pid, &pids);
    } else {
      logError("Failed to fork request handler.");
    }
  } 
  *thepids = pids;
}

/*********************************************************************
* killAllChildren...
*
* Send a kill signal to all the child processes.
*********************************************************************/
void killAllChildren(PidList *pids) {
  PidList *current = NULL;

  current = pids;
  while (current != NULL) {
    if ((current->pid != -1) && (kill(current->pid, 0) == 0))
      kill(current->pid, SIGKILL);
    current = current->next;
  }
}

/************************************************************************
* maintainRequestHandlers...
*
* Keep a pool of handlers to take requests.
************************************************************************/
void maintainRequestHandlers(PidList **thepids, int sockfd) {
  int numhandlers = getNumRequestHandlers(),
      pid = 0, numalive = 0;
  PidList *pids = *thepids, *current = NULL, *prev = NULL;
  
  while ((numtofork - numforked) != 0) {
    pid = fork();
    if (pid == 0) {
      // Child
      setSignalHandlers(CHILD);
      processConnection(sockfd); 
      _exit(0);
    } else if (pid > 0) {
      // Parent
      numforked = (numforked + 1) % MAXFORKS;
      appendToPidList(pid, &pids);
    } else {
      logError("Failed to fork request handler.");
    }
  }

  current = pids;
  while (current != NULL) {
    if ((current->pid == -1) || (kill(current->pid, 0) != 0)) {
      if (prev == NULL) {
        current = current->next;
        n_free(pids);
        pids = current; 
      } else {
        prev->next = current->next;
        n_free(current);
        current = prev->next;
      } 
    } else {
      prev = current;
      current = current->next;
      numalive++;
    }
  }
  if (numalive < numhandlers)
    numtofork = (numtofork + 1) % MAXFORKS;
  
  *thepids = pids; 
}

/************************************************************************
* handleRequests...
*
* Handle all incoming requests.
************************************************************************/
int handleRequests() {
  PidList *pids = NULL;
  int sockfd = 0;

  if (openPort(&sockfd, getPortNumber()) != 0)
    return PORTNOTFREE;
  preForkRequestHandlers(&pids, sockfd);

  while (!quitrequest) {
    yieldProcessor();
    maintainRequestHandlers(&pids, sockfd);
  }

  killAllChildren(pids);

  freePidList(&pids);
  closePort(sockfd);
  return E_OK;
}

/************************************************************************
* installSegSignalHandler...
*
* Set this function as the signal handler for this signal.
************************************************************************/
int installSegSignalHandler(int signum, void (* handler)(int, siginfo_t *, void *))
{
    struct sigaction newact;

    newact.sa_sigaction = handler;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = SA_RESTART | SA_SIGINFO; /* restart interrupted OS calls */
    return sigaction(signum, &newact, NULL) ;                                 
}

/************************************************************************
* installSignalHandler...
*
* Set this function as the signal handler for this signal.
************************************************************************/
int installSignalHandler(int signum, void (* handler)(int))
{
    struct sigaction newact;

    newact.sa_handler = handler;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    newact.sa_flags |= SA_RESTART; /* restart interrupted OS calls */
    return sigaction(signum, &newact, NULL) ;                                 
}

/************************************************************************
* childSignalHandler...
*
* Handle child died events in the parent.
************************************************************************/
void childSignalHandler(int sig) {
  sig = 0;
  numtofork = (numtofork + 1) % MAXFORKS;
}

/************************************************************************
* reloadSignalHandler...
*
* Reload the config file.
************************************************************************/
void reloadSignalHandler(int sig) {
  int errnum = 0;
  if ((errnum = loadConfigFile()) != E_OK) {
    quitrequest = 1;
  }
}

/************************************************************************
* shutdownSignalHandler...
*
* Handle shutdown events in the parent.
************************************************************************/
void shutdownSignalHandler(int sig) {
  sig = 0;
  quitrequest = 1;
}

/************************************************************************
* segfaultSignalHandler...
*
* Handle shutdown events in the parent.
************************************************************************/
void segfaultSignalHandler(int sig, siginfo_t *info,
				   void *secret) {

    /* This signal handling code is not portable
  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;
  ucontext_t *uc = (ucontext_t *)secret;

  //
  fprintf(stderr, "Got signal %d, faulty address is %p, from %p\n", 
                   sig, info->si_addr, (void *)(uc->uc_mcontext.gregs[REG_EIP]));
	
  trace_size = backtrace(trace, 16);
  //
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];

  messages = backtrace_symbols(trace, trace_size);
  //
  fprintf(stderr, "[trace] Execution path:\n");
  for (i=1; i<trace_size; ++i)
	fprintf(stderr, "[trace] %s\n", messages[i]);

    */

  fflush(stderr);
  exit(1);
}

/************************************************************************
* signalParent...
*
* Send a signal to the parent process.
************************************************************************/
void signalParent(int sig) {
  kill(getppid(), sig);
}

/************************************************************************
* setSignalHandlers...
*
* Specify the signal handlers.
************************************************************************/
void setSignalHandlers(int parent) {
  if (parent == PARENT) {
    installSignalHandler(SIGUSR1, childSignalHandler); 
    installSignalHandler(SIGCHLD, SIG_IGN);
    installSignalHandler(SIGTERM, shutdownSignalHandler);
    installSignalHandler(SIGKILL, shutdownSignalHandler);
    installSignalHandler(SIGABRT, shutdownSignalHandler);
    installSegSignalHandler(SIGSEGV, segfaultSignalHandler);
    installSignalHandler(SIGHUP,  reloadSignalHandler);
  } else {
    installSignalHandler(SIGCHLD, SIG_DFL);
    installSignalHandler(SIGHUP,  SIG_IGN);
    installSegSignalHandler(SIGSEGV, segfaultSignalHandler);
  }
}

/************************************************************************
* main...
*
* This is the entry point for the program.
************************************************************************/
int main(int argc, char **argv) {
  int errnum = E_OK;
  char *pidfile = NULL;
  pid_t tpid = 0;
  FILE *f = NULL;

  quitrequest = 0;

  if ((tpid = fork()) > 0) /* the parent just exits */
    exit(0);
  else if (tpid < 0)       /* Errors exit with error condition */
    exit(1);

  /* The child takes over from the parent, now detach */
  setsid();  /* and set process group to PID */

  setSignalHandlers(PARENT);

  errnum = loadCommandLineArgs(argc, argv);
  if (errnum != E_OK) {
    logError("Fatal Error:%s\n", getErrorMesg(errnum));
    printShutdownInformation();
    return errnum;
  }

  errnum = validateLicenseKey();
  if (errnum != E_OK) {
    logError("Fatal Error:%s\n", getErrorMesg(errnum));
    printShutdownInformation();
    return errnum;  
  }

  pidfile = getPidFile();
  f = fopen(pidfile, "wb");
  if (f != NULL) {
    fprintf(f, "%u", getpid());
    fclose(f);
  } else {
    logError("Could not write pid to file: %s\n", pidfile);
  }
  

  errnum = handleRequests();
  if (errnum != E_OK) {
    logError("Fatal Error:%s\n", getErrorMesg(errnum));
  }
  
  printShutdownInformation();
  return errnum;  
}
