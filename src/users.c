#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#ifdef WIN32
#include "win32.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#endif
#include <string.h>
#include <time.h>
#include "env.h"
#include "strings.h"
#include "config.h"
#include "errors.h"
#include "logging.h"
#include "structs.h"
#include "package.h"
#include "objects.h"
#include "users.h"
#include "dbcalls.h"
#include "malloc.h"

/*********************************************************************
* initUserDetails...
*
* dhumalloc the memory for this struct.
*********************************************************************/
UserDetails *initUserDetails() {
  UserDetails *user = NULL;

  user = (UserDetails *) dhumalloc(sizeof(UserDetails));
  if (user == NULL)
    return NULL;

  user->userID = NULL;
  user->userName = NULL;
  user->isSuperUser = NULL;
  user->isOnline = NULL;
  user->fullName = NULL;
  user->email = NULL;
  user->userType = NULL;

  return user;
}

/*********************************************************************
* freeUserDetails...
*
* free this struct
*********************************************************************/
void freeUserDetails(UserDetails *user) {
  if (user == NULL)
    return;
  dhufree(user->userID);
  dhufree(user->userName);
  dhufree(user->isSuperUser);
  dhufree(user->isOnline);
  dhufree(user->fullName);
  dhufree(user->email);
  dhufree(user->userType);
}

/*********************************************************************
* loadUserList...
*
* Load a filtered user list.
*********************************************************************/
int loadUserList(char *filter, int min, int max, void *sqlsock, Env *env, int **theusers, int *thenumusers) {
  int numusers = 0, *users = NULL, errcode = 0, i = 0, online = 0, count = 0, issuper = 1;
  char *super = NULL;

  errcode = loadUserListDB(filter, &users, &numusers, sqlsock);
  if (errcode != E_OK)
    return errcode;

  // ONLY SUPER USERS CAN CREATE USERS
  super = getEnvValue("ISSUPERUSER", env);
  
  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  *thenumusers = numusers;
  for (i = 0; i < numusers; i++) {
    if ((!issuper) &&
        ((isUserOnline(users[i], &online, sqlsock) != E_OK) || !online)) {
      users[i] = -1;
      (*thenumusers)--;
    }
  }

  *theusers = (int *) dhumalloc(sizeof(int) * (*thenumusers));
  *thenumusers = 0;
  count = 0;
  for (i = 0; i < numusers; i++) {
    if (users[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*thenumusers) + (min>0?min:0) < max || max < 0)) {
        (*theusers)[(*thenumusers)] = users[i];
        (*thenumusers)++;
      }
      count++;
    }
  }

  dhufree(users);
  if ((*thenumusers) == 0)
    return NODATAFOUND;
  return E_OK;
}


/*********************************************************************
* validateUser...
*
* Does this user exist and are they active and not deleted. 
* If so return their userid and super user status.
*********************************************************************/
int validateUser(char *username, char *password, int *uid, int *super, char **fullname, char **email, char **usertype, void *connection) {
  int errcode = 0;
  char *full = NULL, *mail = NULL, *type = NULL;

  if ((errcode = getUserLoginDetails(username, password, uid, super, &full, &mail, &type, 1, connection)) != E_OK) {
    return errcode;
  } 
  *fullname = full;
  *email = mail;
  *usertype = type;

  if ((errcode = isUserActive(*uid, connection)) != E_OK) {
    dhufree(full);
    return errcode;
  }  

  return E_OK;
}

/**********************************************************************
* execAuth...
*
* Execute the authentication program.
**********************************************************************/
int execAuth(char *username, char *password) {
  char *args[4];

  args[0] = getExtAuth();
  args[1] = username;
  args[2] = password;
  args[3] = NULL;

  logDebug("Authentication arguments: %s '%s' '%s'\n", args[0], args[1], args[2]);

  return execvp(getExtAuth(), args);
}

/**********************************************************************
* runAuth...
*
* Fork before we execute the authentication program.
**********************************************************************/
int validateExternalUser(char *username, char *password) {
  int pid = 0, status = RESOURCEERROR;

  pid = fork();
  if (pid == 0) {
    // child
    execAuth(username, password);

    _exit(0);
  } else if (pid > 0) {
    waitpid(pid, &status, 0);
  }

  if (WEXITSTATUS(status) != 0) {
    logDebug("Status is not 0. %d\n", status);
    return ACCESSDENIED;
  }

  return E_OK;
}

/*********************************************************************
* createValidSession...
*
* Add a session to the session manager and return the key.
*********************************************************************/
int createValidSession(char *username, char *password, int uid, int super, char *fullname, char *email, char *usertype, char **sess, void *sqlsock, Env *env) {
  char sessionkey[65], *uidstr = NULL;
  int i = 0;

#ifdef WIN32
  struct __timeb64 tstruct;
  _ftime64( &tstruct );
  srand((unsigned int)tstruct.millitm * getpid());

#else
  struct timeval random_seed;


  gettimeofday(&random_seed, NULL);
  srand((unsigned int)random_seed.tv_usec * getpid());
#endif

  memset(sessionkey, 0, sizeof(sessionkey));

  do {
    for (i = 0; i < 64; i++) {
      sessionkey[i] = 'a' + getRand(26);
    }
    sessionkey[i] = CNULL;
  } while (createSession(sessionkey, username, uid, super, fullname, usertype, sqlsock) != E_OK); 
  
  int2Str(uid, &uidstr);
  setTokenValue(dhustrdup("USERNAME"), dhustrdup(username), env);
  setTokenValue(dhustrdup("EMAIL"), dhustrdup(email), env);
  setTokenValue(dhustrdup("USERID"), uidstr, env);
  setTokenValue(dhustrdup("ISSUPERUSER"), dhustrdup(super?(char *) "y":(char *) "n"), env);
  setTokenValue(dhustrdup("FULLNAME"), dhustrdup(fullname), env);
  setTokenValue(dhustrdup("USERTYPE"), dhustrdup(fullname), env);
  
  *sess = dhustrdup(sessionkey);

  return E_OK;
}

