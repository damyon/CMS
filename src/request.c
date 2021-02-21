#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#include "win32.h"
#else
#include <unistd.h>
#endif
#include <ctype.h>
#include "ipc.h"
#include "env.h"
#include "strings.h"
#include "logging.h"
#include "errors.h"
#include "structs.h"
#include "package.h"
#include "objects.h"
#include "users.h"
#include "dbcalls.h"
#include "file.h"
#include "parser.h"
#include "request.h"
#include "api.h"
#include "malloc.h"
#include "image.h"
#include "config.h"

/*************************************************************
* authenticateConnection...
*
* Perform a handshake validation.
*************************************************************/
int authenticateConnection(int sockfd, int *type) {
  char *authstr = NULL; 
  
  sendData((char *) AUTHSTR, 8, sockfd);

  if (readData(&authstr, 8, sockfd) != 0)
    return INVALIDREQUEST;

  if (strcasecmp(authstr, STDSTR) == 0) {
    *type = STANDARDREQUEST;
  } else if (strcasecmp(authstr, DAVSTR) == 0) {
    *type = WEBDAVREQUEST;
  } else {
    logError("Invalid request.");
    return INVALIDREQUEST;
  }
  dhufree(authstr);

  return E_OK;
}

/*************************************************************
* sendErrorReply...
*
* Remove all tokens from the env and
* add the error mesg.
*************************************************************/
void sendErrorReply(char *mesg, Env *env) {
  logError("Request Error:%s\n", mesg);
  freeTokenValueList(env->tokenhead);
  env->tokenhead = NULL; 
  env->tokentail = NULL; 
  freeFileObjectList(env->filehead);
  env->filehead = NULL; 
  env->filetail = NULL; 

  appendTokenValueToEnv(dhustrdup("ERRMSG"), dhustrdup(mesg), env);
}

/*************************************************************
* getQueryStringValue...
*
* Parse this query string for the named value
* Note: This is not 100% correct as it assumes the token name
* will not be transformed if it is URL Encoded. (for speed)
*************************************************************/
char *getQueryStringValue(char *qstring, char *token) {
  char *current = qstring, *end = NULL, *result = NULL;

  current = strchr(qstring, '?');
  if (current) current++;

  while (current != NULL) {
    if ((strncmp(current, token, strlen(token)) == 0) &&
        (current[strlen(token)] == '=')) {
      end = strchr(current, '&');
      if (end) *end = '\0';
      result = URLDecode(current + strlen(token) + 1);
      if (end) *end = '&';
      break;
    }
    end = strchr(current, '&');
    if (end) 
      current = end + 1;
    else 
      current = NULL;
  }

  return result;
}

/*************************************************************
* isDeletedXRefValid...
*
* Is this a valid DeletedXRef?
*************************************************************/
int isDeletedXRefValid(char *xref, int timestamp, void *dbconn, Env *env, int *objectid) {
  char *pathsep = NULL, *current = NULL, *name = NULL, *tmp = NULL;
  Queue *queue = NULL;
  int parentid = -1, objid = 0, folder = 0;
  // first split the ref into its object names.  

  if (timestamp < 0)
    timestamp = time(NULL);

  queue = initQueue();

  current = xref;

  
  while ((pathsep = strchr(current, '/')) != NULL) {
    *pathsep = CNULL; 
    if ((strcmp(current, "..") == 0) && (countQueue(queue) > 0)) {
      tmp = (char *) popNQueue(queue, countQueue(queue) - 1);
      dhufree(tmp);
    } else if ((strcmp(current, ".") != 0) && (strlen(current) > 0)) {
      pushQueue(queue, dhustrdup(current)); 
    }
    *pathsep = '/';
    current = pathsep + 1;
  }
   
  if ((strcmp(current, "..") == 0) && (countQueue(queue) > 0)) {
    tmp = (char *) popNQueue(queue, countQueue(queue) - 1);
    dhufree(tmp);
  } else if ((strcmp(current, ".") != 0) && (strlen(current) > 0)) {
    pushQueue(queue, dhustrdup(current)); 
  }
  
  while ((name = (char *) popQueue(queue)) != NULL) {
    if ((countQueue(queue) > 0)) {
      if (getObjectID(parentid, name, timestamp, &objid, dbconn) != E_OK) {
        dhufree(name);
        while ((name = (char *) popQueue(queue)) != NULL) 
          dhufree(name);
        freeQueue(&queue);
        return INVALIDXPATH;
      } 
      if ((isObjectFolder(objid, &folder, dbconn) != E_OK) || (!folder)) {
        dhufree(name);
        while ((name = (char *) popQueue(queue)) != NULL) 
          dhufree(name);
        freeQueue(&queue);
        return INVALIDXPATH;
      }
    } else {
      if (getDeletedObjectID(parentid, name, timestamp, &objid, dbconn) != E_OK) {
        dhufree(name);
        while ((name = (char *) popQueue(queue)) != NULL) 
          dhufree(name);
        freeQueue(&queue);
        return INVALIDXPATH;
      } 
    }
    parentid = objid;
    *objectid = objid;
    dhufree(name);
  }
  freeQueue(&queue);
  
  return E_OK;
}

/*************************************************************
* isXRefValid...
*
* Is this a valid XRef?
*************************************************************/
int isXRefValid(char *xref, int timestamp, void *dbconn, Env *env, int *objectid) {
  char *pathsep = NULL, *current = NULL, *name = NULL, *tmp = NULL;
  Queue *queue = NULL;
  int parentid = -1, objid = 0, folder = 0;
  // first split the ref into its object names.  

  if (timestamp < 0)
    timestamp = time(NULL);

  queue = initQueue();

  current = xref;

  
  while ((pathsep = strchr(current, '/')) != NULL) {
    *pathsep = CNULL; 
    if ((strcmp(current, "..") == 0) && (countQueue(queue) > 0)) {
      tmp = (char *) popNQueue(queue, countQueue(queue) - 1);
      dhufree(tmp);
    } else if ((strcmp(current, ".") != 0) && (strlen(current) > 0)) {
      pushQueue(queue, dhustrdup(current)); 
    }
    *pathsep = '/';
    current = pathsep + 1;
  }
   
  if ((strcmp(current, "..") == 0) && (countQueue(queue) > 0)) {
    tmp = (char *) popNQueue(queue, countQueue(queue) - 1);
    dhufree(tmp);
  } else if ((strcmp(current, ".") != 0) && (strlen(current) > 0)) {
    pushQueue(queue, dhustrdup(current)); 
  }
  
  while ((name = (char *) popQueue(queue)) != NULL) {
    if (getObjectID(parentid, name, timestamp, &objid, dbconn) != E_OK) {
      dhufree(name);
      while ((name = (char *) popQueue(queue)) != NULL) 
        dhufree(name);
      freeQueue(&queue);
      return INVALIDXPATH;
    } 
    if ((countQueue(queue) > 0)) {
      if ((isObjectFolder(objid, &folder, dbconn) != E_OK) || (!folder)) {
        dhufree(name);
        while ((name = (char *) popQueue(queue)) != NULL) 
          dhufree(name);
        freeQueue(&queue);
        return INVALIDXPATH;
      }
    }
    if (userHasReadAccess(objid, env, dbconn) != E_OK) {
      dhufree(name);
      while ((name = (char *) popQueue(queue)) != NULL) 
        dhufree(name);
      freeQueue(&queue);
      return ACCESSDENIED;
    }
    parentid = objid;
    *objectid = objid;
    dhufree(name);
  }
  freeQueue(&queue);
  
  return E_OK;
}

/*************************************************************
* isXRefPublic...
*
* Is this a ispublic XRef?
*************************************************************/
int isXRefPublic(int objectid, void *dbconn) {
  int ispublic = 0;

  if (isObjectPublic(objectid, &ispublic, dbconn) != E_OK || !ispublic)
    return ACCESSDENIED;
  return E_OK;
}

/*************************************************************
* isXRefOnline...
*
* Is this XRef online?
*************************************************************/
int isXRefOnline(int objid, void *dbconn) {
  int online = 0;

  if (isObjectOnline(objid, &online, dbconn) != E_OK || !online)
    return NOTONLINE;
  return E_OK;
}

/*************************************************************
* userHasReadAccess...
*
* Perform a permission check.
*************************************************************/
int userHasReadAccess(int objectid, Env *env, void *dbconn) {
  // ARE THEY A SUPER USER ?
  int uid = -1, access = 0;
  char *super = NULL, *uidstr = NULL;

  super = getEnvValue("ISSUPERUSER", env);

  if (super != NULL && *super == 'y')
    return E_OK;

  uidstr = getEnvValue("USERID", env);
  uid = strtol(uidstr?uidstr:"-1", NULL, 10); 

  // Removed for version 3.0
  // You can see locked objects - just not edit them.
  // if ((isObjectLocked(objectid, dbconn) == E_OK) && (isObjectLockedByUser(objectid, uid, dbconn) != E_OK)) {
  //   return ACCESSDENIED;
  // }
  //

  if (isXRefOnline(objectid, dbconn) != E_OK) {
    if (userHasWriteAccess(objectid, env, dbconn) != E_OK) {
      return ACCESSDENIED;
    }
  }

  if ((accessCheck(objectid, uid, "r__", &access, dbconn) == E_OK) && (access)) {
    return E_OK;
  }
  
  return ACCESSDENIED;
}

/*************************************************************
* userHasWriteAccess...
*
* Perform a permission check.
*************************************************************/
int userHasWriteAccess(int objectid, Env *env, void *dbconn) {
  char *super = NULL, *uidstr = NULL;
  int uid = -1, access = 0;

  super = getEnvValue("ISSUPERUSER", env);

  // ARE THEY A SUPER USER ?
  if (super != NULL && *super == 'y')
    return E_OK;

  uidstr = getEnvValue("USERID", env);
  uid = strtol(uidstr?uidstr:"-1", NULL, 10);
  
  if ((isObjectLocked(objectid, dbconn) == E_OK) && (isObjectLockedByUser(objectid, uid, dbconn) != E_OK)) {
    return ACCESSDENIED;
  }

  if ((accessCheck(objectid, uid, "_w_", &access, dbconn) == E_OK) && (access)) {
    return E_OK;
  }
  
  return ACCESSDENIED;
}

/*************************************************************
* userHasExecuteAccess...
*
* Perform a permission check.
*************************************************************/
int userHasExecuteAccess(int objectid, Env *env, void *dbconn) {
  char *super = NULL, *uidstr = NULL;
  int uid = -1, access = 0;

  super = getEnvValue("ISSUPERUSER", env);

  // ARE THEY A SUPER USER ?
  if (super != NULL && *super == 'y')
    return E_OK;

  uidstr = getEnvValue("USERID", env);
  uid = strtol(uidstr?uidstr:"-1", NULL, 10);

  if ((isObjectLocked(objectid, dbconn) == E_OK) && (isObjectLockedByUser(objectid, uid, dbconn) != E_OK)) {
    return ACCESSDENIED;
  }

  if ((accessCheck(objectid, uid, "__x", &access, dbconn) == E_OK) && (access)) {
    return E_OK;
  }
  
  return ACCESSDENIED;
}

/*************************************************************
* loadXRef...
*
* Load the file from disk and parse it.
*************************************************************/
int loadXRef(int objectid, char **contents, int *len, ParserInfo *info) {
  char *filepath = NULL, *filecontents = NULL;
  char *parse = NULL;
  int lock = 0, errcode = 0;
  int filelength = 0;
  
  // Read the file
  if ((errcode = generateFilePath(objectid, &filepath)) != E_OK)
    return errcode; 
  
  lock = lockDirectory(filepath);
  if (lock < 0) {
    logError("Could not get directory lock.");
    dhufree(filepath);
    return LOCKFILETIMEOUT;
  }
  
  if ((errcode = readFile(filepath, &filecontents, &filelength)) != E_OK) {
    unlockDirectory(lock);
    dhufree(filepath);
    return errcode;
  } 

  parse = getEnvValue("parse", info->env);
  if (!(parse && (tolower(*parse) == 'n' || tolower(*parse) == 'f'))) {

    if ((errcode = parseScript(&filecontents, &filelength, objectid, info)) != E_OK) {
      dhufree(filecontents);
      return errcode;
    }
  }
    
  dhufree(filepath);
  unlockDirectory(lock);
  *contents = filecontents;
  *len = filelength;
  return E_OK;
}

/*************************************************************
* retrieveXRef...
*
* Load the xref from the disk, parse it for script
* and add it to the environment.
*************************************************************/
int retrieveXRef(int objectid, Env *env, void *dbconn) {
  char *filepath = NULL, *filecontents = NULL, *checkout = NULL;
  int lock = 0, errcode = 0, timestamp = 0, templateid = 0;
  int filelength = 0, imagedyn = 0;
  char *parse = NULL, *path = NULL;
  char filename[256], *ext = NULL;
  SymbolTable *symbols = NULL;
  ObjectDetails *details = NULL;
  ValueToken *search = NULL;
  ParserInfo info;
  Symbol *s = NULL;

  info.sqlsock = dbconn;
  info.env = env;
  info.maxInstructions = 100000;
  info.instructionCount = 0;

  details = initObjectDetails();
  errcode = getObjectDetails(objectid, &details, dbconn);
  if (errcode != E_OK) {
	  return errcode;
  }

  if (strcmp(details->type, "FOLDER") == 0) {
    freeObjectDetails(details);
    return ISAFOLDER;
  }
  
  // Read the file
  if ((errcode = generateFilePath(objectid, &filepath)) != E_OK) {
    freeObjectDetails(details);
    return errcode;
  }

  lock = lockDirectory(filepath); 
  if (lock < 0) {
    freeObjectDetails(details);
    dhufree(filepath);
    return errcode;
  }

  if ((errcode = readFile(filepath, &filecontents, &filelength)) != E_OK) {
    freeObjectDetails(details);
    unlockDirectory(lock);
    dhufree(filepath);
    return errcode;
  }
  
  dhufree(filepath);
  unlockDirectory(lock);

  symbols = initSymbolTable();
  info.symbols = symbols;

  parse = getEnvValue("parse", env);
  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, env), getEnvValue(CTIMETOK, env));

  if (!(parse && (tolower(*parse) == 'n' || tolower(*parse) == 'f'))) {
    // if the file has a tplt, load that instead.
    
    if (details->type != NULL && strcmp(details->type, "RESOURCE") != 0 && strcmp(details->type, "FOLDER") != 0 &&
        details->tplt != NULL && isXRefValid(details->tplt, timestamp, dbconn, env, &templateid) == E_OK) {

      // set the current file in the env and load the tplt instead.
      dhufree(filecontents);
      filecontents = NULL;
      filelength = 0;

      // if this is just content - set the content variable to point to the current page
      // else set the content variable to point to the extension and set the path to point
      // to the current page.

      if (strcmp(details->type, "CONTENT") == 0) {
        s = initVariableStringSymbol(strdup("CONTENT"), strdup(details->path));
        putSymbol(symbols, s);
        s = initVariableStringSymbol(strdup("PATH"), strdup(details->path));
        putSymbol(symbols, s);
      } else {
	vstrdupcat(&path, "epiction/extensions/", details->type, "/view.cms", NULL);
        
	s = initVariableStringSymbol(strdup("CONTENT"), path);
	putSymbol(symbols, s);
	
	s = initVariableStringSymbol(strdup("PATH"), strdup(details->path));
	putSymbol(symbols, s);
      }

      if ((errcode = loadXRef(templateid, &filecontents, &filelength, &info)) != E_OK) {
        freeObjectDetails(details);
        return errcode;
      }
    } else {

      // parse any script
      if (strncasecmp(details->mimeType?details->mimeType:"application/unknown", "text", 4) == 0) {
        if ((checkout = getEnvValue("checkout", env)) == NULL || !(((tolower(*checkout) == 'y' || tolower(*checkout) == 't') &&
                                                               (userHasWriteAccess(objectid, env, dbconn))))) { 
          if ((errcode = parseScript(&filecontents, &filelength, objectid, &info)) != E_OK) {
            dhufree(filecontents);
            freeObjectDetails(details);
            return errcode;
          }

        }
      }
    }
  }

  freeSymbolTable(symbols);

  search = env->tokenhead;
  imagedyn = 0;
  while ((search != NULL) && (search->token != NULL)) {
    if (strstr(search->token, "image-")) {
      imagedyn = 1;
      break;
    }    
    search = search->next;
  }
  logDebug("Resize image: %d\n", imagedyn);
  logDebug("Resize mimetype: %s\n", details->mimeType);

  if ((imagedyn) && (strncasecmp(details->mimeType?details->mimeType:"application/unknown", "image", 5) == 0)) {
    logDebug("Resize image: %d\n", imagedyn);
    if ((errcode = resizeImage(&filecontents, &filelength, env, dbconn)) != E_OK) {
      dhufree(filecontents);
      freeObjectDetails(details);
      return errcode;
    }
  }

  // Clear out the existing environment.
  freeFileObjectList(env->filehead);
  env->filehead = NULL; 
  env->filetail = NULL; 

  ext = details->mimeType;

  while ((ext != NULL) && ((*ext) != '\0') && ((*ext) != '/')) {
	  ext++;
  }

  if ((ext != NULL) && ((*ext) == '/')) {
	  ext++;
  }

  if ((ext == NULL) || ((*ext) == '\0') ) {
	  ext = "bin";
  }

  //vstrdupcat(&filename, details->objectName, ".", ext, NULL);
  snprintf(filename, 255, "%s.%s", details->objectName, ext);
  
  appendFileObjectToEnv(dhustrdup("file"), 
                          dhustrdup(details->mimeType), 
                          strdup(filename), 
                          filecontents, 
                          filelength, env);
  freeObjectDetails(details);

  return E_OK;
}

/*************************************************************
* processRequest...
*
* Get the tokens of of the env and use them to retrieve the document.
*************************************************************/
void processRequest(Env *env, void *dbconn) {
  char *xref = NULL, *session = NULL, *xmethod = NULL;
  int access = 0, errcode = 0, objid = 0, timestamp = 0;

  // printEnv(env);
  xref = getEnvValue(XREFTOK, env);
  xmethod = getEnvValue(XMETHODTOK, env);
  session = getEnvValue("CMSSESSION", env);
  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, env), getEnvValue(CTIMETOK, env));

  if (xref == NULL && xmethod == NULL) {
    sendErrorReply("No object/method reference in the request", env);
    return;
  }


  if (session) {
    loadSessionData(session, env, dbconn);
  } else {
    logInfo("No session provided. Proceding as ispublic user.\n");
  }

  if (xref != NULL) {
    // This is a retrieve request
    if (isXRefValid(xref, timestamp, dbconn, env, &objid) != E_OK) {
      logError("Could not find file: %s", xref);
      sendErrorReply("The document referenced does not exist", env);
      return;
    }
  
    if (userHasReadAccess(objid, env, dbconn) == E_OK) {
      access = 1;
    }
  
    if (isXRefOnline(objid, dbconn) != E_OK) {
      if (userHasWriteAccess(objid, env, dbconn) != E_OK) {
        sendErrorReply("The document referenced is not online and you do not have write access.", env);
        return;
      }
    }

    if (!access) {
      sendErrorReply("Permission Denied", env);
      return;
    }


    if ((errcode = retrieveXRef(objid, env, dbconn)) != E_OK) {
      logInfo("Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
      sendErrorReply(getErrorMesg(errcode), env);
      return;
    }
    logInfo("Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  } else {
      sendErrorReply(getErrorMesg(INVALIDXPATH), env);
  } 
}

/*************************************************************
* handleRequest...
*
* Retrieve the document and write the response back down the socket.
*************************************************************/
int handleRequest(int sockfd) {
  Env *env = NULL;
  char *client = NULL;
  void *dbconn = NULL;

  env = readEnvFromStream(sockfd);
  if (env == NULL) {
    return INVALIDREQUEST;
  }

  client = getEnvValue("REMOTE_ADDR", env);
  if (getDevLicense() && 
         ((client == NULL) || (strcasecmp(client, "127.0.0.1") != 0))) {
    logError("Remote connections not allowed for development license. Refuse connection from %s\n", client?client:"unknown");
    return INVALIDREQUEST;
  }
  
  if ((dbconn = getDBConnection()) == NULL) {
    sendErrorReply("Could not open a connection to the database", env);
    return DBCONNECTERROR;
  }

  processRequest(env, dbconn);
  writeEnvToStream(sockfd, env);
  freeEnv(env);
  closeDBConnection(dbconn);

  return 0;
}
