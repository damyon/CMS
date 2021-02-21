#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include "strings.h"
#include "structs.h"
#include "errors.h"
#include "logging.h"
#include "config.h"
#include "env.h"
#include "search.h"
#include "file.h"
#include "package.h"
#include "objects.h"
#include "users.h"
#include "dbcalls.h"
#include "malloc.h"
#include "md5.h"
#ifdef WIN32
#include "win32.h"
#else
#include <unistd.h>
#endif

int getVersionObjectID(int parentid, char *name, int timestamp, int *objid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *safename = NULL, *query = NULL, *parentstr = NULL, *objectid = NULL, *times = NULL;
  
  *objid = 1;

  safename = escapeSQLString(name); 
  int2Str(parentid, &parentstr);
  time2Str(timestamp, &times);

  vstrdupcat(&query, "select objectID from Objects where parentID = ", parentstr, " and objectName = '", safename, "' and version <= ", times, " ORDER BY version DESC, objectID DESC", NULL);
  dhufree(safename);
  dhufree(parentstr);
  dhufree(times);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  objectid = fetchSQLData(result, 0, 0);

  *objid = strtol(objectid?objectid:"-1", NULL, 10);
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* getDeletedObjectID...
*
* Find the id of this object only if it is deleted.
*******************************************************************************/
int getDeletedObjectID(int parentid, char *name, int timestamp, int *objid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *safename = NULL, *query = NULL, *parentstr = NULL, *objectid = NULL, *times = NULL, *deleted = NULL;
  
  *objid = 1;

  safename = escapeSQLString(name); 
  int2Str(parentid, &parentstr);
  time2Str(timestamp, &times);

  vstrdupcat(&query, "select objectID,isDeleted from Objects where parentID = ", parentstr, " and objectName = '", safename, "' and version <= ", times, " ORDER BY version DESC, objectID DESC", NULL);
  dhufree(safename);
  dhufree(parentstr);
  dhufree(times);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  objectid = fetchSQLData(result, 0, 0);
  deleted = fetchSQLData(result, 0, 1);

  if (*deleted != 'y') {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *objid = strtol(objectid?objectid:"-1", NULL, 10);
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* getObjectID...
*
* Find the id of this object.
*******************************************************************************/
int getObjectID(int parentid, char *name, int timestamp, int *objid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *safename = NULL, *query = NULL, *parentstr = NULL, *objectid = NULL, *times = NULL, *deleted = NULL;
  
  *objid = 1;

  safename = escapeSQLString(name); 
  int2Str(parentid, &parentstr);
  time2Str(timestamp, &times);

  vstrdupcat(&query, "select objectID,isDeleted from Objects where parentID = ", parentstr, " and objectName = '", safename, "' and version <= ", times, " ORDER BY version DESC, objectID DESC", NULL);
  dhufree(safename);
  dhufree(parentstr);
  dhufree(times);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  objectid = fetchSQLData(result, 0, 0);
  deleted = fetchSQLData(result, 0, 1);

  if (*deleted == 'y') {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *objid = strtol(objectid?objectid:"-1", NULL, 10);
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* deleteObjectDB...
*
* Update the objects table and set this version to deleted.
*******************************************************************************/
int deleteObjectDB(int objectid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *objidstr = NULL;

  int2Str(objectid, &objidstr);
  vstrdupcat(&query, "update Objects set isDeleted = 'y' where objectID = ", objidstr, NULL);
  dhufree(objidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  if (result) {
    freeSQLResult(result);
  }

  return E_OK;
}

/*******************************************************************************
* insertObjectDetails...
*
* Insert a new record in the object table.
*******************************************************************************/
int insertObjectDetails(ObjectDetails *details, int *newid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, updateid = 0, timestamp = 0;
  char *query = NULL, *safename = NULL, *safetemplate = NULL, *safemimetype = NULL, *objidstr = NULL, *nowstr = NULL;

  if (details == NULL)
    return RESOURCEERROR;

  safename = escapeSQLString(details->objectName);
  safemimetype = escapeSQLString(details->mimeType);
  safetemplate = escapeSQLString(details->tplt);

  nowstr = nowStr();
  vstrdupcat(&query, "insert into Objects (objectName, parentID, isOnline, type, isPublic, isDeleted, mimeType, version, lockedByUserID, template, relativeOrder) values (",
                                             "'", safename, "', ", 
                                             details->parentID, ", ",
                                             "'", details->isOnline, "', ",
                                             "'", details->type, "', ",
                                             "'", details->isPublic, "', ",
					     "'n', ",
                                             "'", safemimetype, "', ",
                                             nowstr, ", ",
                                             details->lockedByUserID, ", '",
					     safetemplate, "', ",
					     details->relativeOrder, ")", NULL);
  dhufree(safename);
  dhufree(safemimetype);
  dhufree(safetemplate);
  dhufree(nowstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    // this can happen if we insert stuff too fast - so wait 1 second and try again.
    safename = escapeSQLString(details->objectName);
    safemimetype = escapeSQLString(details->mimeType);
    safetemplate = escapeSQLString(details->tplt);

    sleep(1);
    nowstr = nowStr();
    timestamp = time(NULL) + 1;
    vstrdupcat(&query, "insert into Objects (objectName, parentID, isOnline, type, isPublic, isDeleted, mimeType, version, lockedByUserID, template, relativeOrder) values (",
                                             "'", safename, "', ", 
                                             details->parentID, ", ",
                                             "'", details->isOnline, "', ",
                                             "'", details->type, "', ",
                                             "'", details->isPublic, "', ",
					     "'n', ",
                                             "'", safemimetype, "', ",
                                             nowstr, ", ",
                                             details->lockedByUserID, ", '",
					     safetemplate, "', ",
					     details->relativeOrder, ")", NULL);

    dhufree(safename);
    dhufree(safemimetype);
    dhufree(safetemplate);
    dhufree(nowstr);

    errcode = runSQL(sqlsock, &result, query);
    dhufree(query);
  
    if (errcode != 0) {
      logError("Query Error: %s\n", getSQLError(sqlsock));
      return errcode;
    }
  }
  freeSQLResult(result);
  if (updateid <= 0) {
    getSequenceValue("Objects_objectID_seq", newid, sqlsock);
  } else {
    *newid = updateid;
  }

  vstrdupcat(&query, "create temporary table tmpA (objectID integer, groupID integer, mask char(3))", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);
  
  int2Str(*newid, &objidstr);
  vstrdupcat(&query, "insert into tmpA select ", objidstr, ", groupID, mask from Permissions where objectID = ", details->parentID, NULL);
  dhufree(objidstr);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);
 
  vstrdupcat(&query, "insert into Permissions select * from tmpA", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);

  vstrdupcat(&query, "drop table tmpA", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* getObjectParent...
*
* Load the name of this object.
*******************************************************************************/
int getObjectParent(int objectid, char **name, int *parentid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *id = NULL, *query = NULL, *tmp = NULL;

  int2Str(objectid, &id);
  vstrdupcat(&query, "select objectName, parentID from Objects where objectID = ", id, NULL);
  dhufree(id);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  tmp = fetchSQLData(result, 0, 0);
  *name = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 1);
  *parentid = strtol(tmp?tmp:"-1", NULL, 10);
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* isVerifier...
*
* Is this user a verifier of this document.
*******************************************************************************/
int isVerifier(int objectid, int userid, int *verifier, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, parentid = 0;
  char *query = NULL, *uidstr = NULL, *objidstr = NULL, *tmp = NULL;
  
  *verifier = 0;

  errcode = getObjectParent(objectid, &tmp, &parentid, sqlsock);
  dhufree(tmp);
  if (errcode != E_OK) {
    return errcode;
  }

  int2Str(parentid, &objidstr);
  int2Str(userid, &uidstr);
  vstrdupcat(&query, "select count(*) from Verifiers as t1, GroupMembers as t2 where", 
                                      " t2.groupID = t1.groupID and t2.userID = ", uidstr, 
                                      " and t1.objectID = ", objidstr, NULL);
  dhufree(objidstr);
  dhufree(uidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    if (strtol(tmp, NULL, 10) > 0) {
      *verifier = 1;
    }
  }
  freeSQLResult(result);

  
  return E_OK;
}


/*******************************************************************************
* getObjectDetails...
*
* Find the details about this object.
*******************************************************************************/
int getObjectDetails(int objectid, ObjectDetails **details, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, parentID = 0, size = 0;
  ObjectDetails *obj = NULL;
  char *query = NULL, *objectstr = NULL, *tmp = NULL, *path = NULL, *name = NULL, *filepath = NULL;
  
  int2Str(objectid, &objectstr);
  vstrdupcat(&query, "select objectID,objectName,parentID,isOnline,type,isPublic,mimeType,version,lockedByUserID,template,relativeOrder from Objects where objectID = ", objectstr, NULL);
  dhufree(objectstr);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  obj = initObjectDetails();
  tmp = fetchSQLData(result, 0, 0);
  obj->objectID = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 1);
  obj->objectName = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 2);
  obj->parentID = dhustrdup(tmp?tmp:(char *)"");
  path = dhustrdup(obj->objectName);
  parentID = strtol(obj->parentID, NULL, 10);
  while (getObjectParent(parentID, &name, &parentID, sqlsock) == E_OK) {
    tmp = NULL;
    vstrdupcat(&tmp, name, "/", path, NULL);
    path = tmp;
  }
  obj->path = path;
  tmp = fetchSQLData(result, 0, 3);
  obj->isOnline = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 4);
  obj->type = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 5);
  obj->isPublic = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 6);
  obj->mimeType = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 7);
  obj->version = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 8);
  obj->lockedByUserID = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 9);
  obj->tplt = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 10);
  obj->relativeOrder = dhustrdup(tmp?tmp:(char *)"");

  freeSQLResult(result);

  // Get the file size
  if (strcmp(obj->type, "FOLDER") != 0) {
    if ((errcode = generateFilePath(objectid, &filepath)) != E_OK)
    return errcode;

    size = getFileSize(filepath);
    int2Str(size, &(obj->fileSize));
  } else {
    int2Str(0, &(obj->fileSize));
  }

  *details = obj;
  return E_OK;
}

/*******************************************************************************
* isUserOnline...
*
* Is this user online?
*******************************************************************************/
int isUserOnline(int userid, int *isonline, void *sqlsock) {
  void *result = NULL;
  char uidstr[128], *query = NULL, *online = NULL;
  int errcode = 0;
  
  sprintf(uidstr, "%d", userid);
  vstrdupcat(&query, "select isOnline from Users where userID = ", uidstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  online = fetchSQLData(result, 0, 0); 

  *isonline = 0;
  if ((online != NULL) && (*online == 'y'))
    *isonline = 1;

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* isObjectOnline...
*
* Is this object online?
*******************************************************************************/
int isObjectOnline(int objid, int *isonline, void *sqlsock) {
  void *result = NULL;
  char objstr[128], *query = NULL, *online = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select isOnline from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  online = fetchSQLData(result, 0, 0); 

  *isonline = 0;
  if ((online != NULL) && (*online == 'y'))
    *isonline = 1;

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* isObjectFolder...
*
* Do a sql select to see if this object is a folder.
*******************************************************************************/
int isObjectFolder(int objid, int *isfolder, void *sqlsock) {
  void *result = NULL;
  char objstr[128], *query = NULL, *type = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select type from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  type = fetchSQLData(result, 0, 0); 

  *isfolder = 0;
  if ((type != NULL) && (strcmp(type, "FOLDER") == 0))
    *isfolder = 1;

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* isGroupPublic...
*
* Do a sql select to see if this group is ispublic.
*******************************************************************************/
int isGroupPublic(int gid, int *ispublic, void *sqlsock) {
  void *result = NULL;
  char gidstr[128], *query = NULL, *ispublicc = NULL;
  int errcode = 0;
  
  sprintf(gidstr, "%d", gid);
  vstrdupcat(&query, "select isPublic from Groups where groupID = ", gidstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  ispublicc = fetchSQLData(result, 0, 0); 

  *ispublic = 0;
  if ((ispublicc != NULL) && (*ispublicc == 'y'))
    *ispublic = 1;

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* isObjectPublic...
*
* Do a sql select to see if this object is ispublic.
*******************************************************************************/
int isObjectPublic(int objid, int *ispublic, void *sqlsock) {
  void *result = NULL;
  char objstr[128], *query = NULL, *ispublicc = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select isPublic from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  ispublicc = fetchSQLData(result, 0, 0); 

  *ispublic = 0;
  if ((ispublicc != NULL) && (*ispublicc == 'y'))
    *ispublic = 1;

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* getObjectMimeType...
*
* Do a sql select to get this object's mimetype.
*******************************************************************************/
int getObjectMimeType(int objectid, char **mimetype, void *sqlsock) {
  void *result = NULL;
  char objstr[128], *query = NULL, *mime = NULL;
  int errcode = 0;

  sprintf(objstr, "%d", objectid);
  vstrdupcat(&query, "select mimeType from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) { 
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  mime = fetchSQLData(result, 0, 0);

  *mimetype = dhustrdup(mime?mime:(char *)"application/unknown");

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* getUserLoginDetails...
*
* Use the database to check this username/password.
*******************************************************************************/
int getUserLoginDetails(char *username, char *password, int *userid, int *super, char **fullname, char **email, char **userType, int encrypt, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *uid = NULL, *sup = NULL, *full = NULL, *mail = NULL, *type = NULL;
  char *hash = NULL, *seed = NULL;
  char *safeuname = NULL, *safepword = NULL;
  int errcode = 0;

  // first get userID
  vstrdupcat(&query, "select userID, userType from Users where userName = '", 
                      username, "' and revision = 0 and isOnline = 'y'", NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  uid = fetchSQLData(result, 0, 0);
  type = fetchSQLData(result, 0, 1);
  *userid = strtol(uid, NULL, 10);
  freeSQLResult(result);
 
  if (strcasecmp(type, "INTERNAL") != 0) {
    errcode = validateExternalUser(username, password);

    if (errcode != 0) {
      logError("External Authentication Failed: %s\n", getErrorMesg(errcode));
      return errcode;
    }


    vstrdupcat(&query, "select isSuperUser, fullName, email from Users where userName = '", 
                        username, "' and revision = 0 and isOnline = 'y'", NULL);
  } else {
    safeuname = escapeSQLString(username);
    safepword = escapeSQLString(password);
    if (encrypt) {
      vstrdupcat(&seed, safeuname, ":Epiction CMS:", safepword, NULL);
      hash = MD5(seed);
      vstrdupcat(&query, "select isSuperUser, fullName, email from Users where userName = '", 
                        username, "' and password = '", hash, "' and revision = 0 and isOnline = 'y'", NULL);
      dhufree(hash);
      dhufree(seed);
    } else {
      vstrdupcat(&query, "select isSuperUser, fullName, email from Users where userName = '", 
                        username, "' and password = '", 
                        password, "' and revision = 0 and isOnline = 'y'", NULL);
    }
    dhufree(safeuname);
    dhufree(safepword);
  } 

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  sup = fetchSQLData(result, 0, 0); 
  full = fetchSQLData(result, 0, 1);
  mail = fetchSQLData(result, 0, 2);

  *super = *sup == 'y';
  *fullname = dhustrdup(full?full:(char *)"");
  *email = dhustrdup(mail?mail:(char *)"");
  *userType = dhustrdup(type?type:(char *)"");
    
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* isUserActive...
*
* Is this user Live and not deleted?
*******************************************************************************/
int isUserActive(int userid, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *uid = NULL, *iso = NULL, *isd = NULL;
  int errcode = 0;

  int2Str(userid, &uid);
  // select isOnline, isDeleted from Users where userID = 
  vstrdupcat(&query, "select isOnline, isDeleted from Users where userID = ", 
                      uid, NULL);
  
  dhufree(uid);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  iso = fetchSQLData(result, 0, 0);
  isd = fetchSQLData(result, 0, 1); 

  errcode = E_OK;
  if (*iso != 'y' || *isd != 'n')
    errcode =  INACTIVEUSER;
  freeSQLResult(result);
    
  return errcode;
}

/*******************************************************************************
* removeValidSession...
*
* Delete this session from the db.
*******************************************************************************/
int removeValidSession(char *session, void *sqlsock) {
  void *result = NULL;
  char *query = NULL;
  int errcode = 0; 

  if (session == NULL) 
	return 0;
  vstrdupcat(&query, "delete from Sessions where sessionKey = '", session, "'", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  vstrdupcat(&query, "delete from SessionData where sessionKey = '", session, "'", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  return errcode;
}


/*******************************************************************************
* createSession...
*
* Try to insert this session in the database.
* If the insert fails the calling function will try a different key.
*******************************************************************************/
int createSession(char *key, char *user, int uid, int super, char *fullname, char *usertype, void *sqlsock) {
  void *result = NULL;
  void *result2 = NULL;
  char *query = NULL, *uidstr = NULL, *timeoutstr = NULL, *nowstr = nowStr(), *keystr = NULL, 
       *safeuser = NULL, *safefullname = NULL, *safeusertype = NULL;
  int errcode = 0, timeout = 0, i = 0;

  timeout = getSessionTimeout();

  int2Str(timeout, &timeoutstr);
  
  vstrdupcat(&query, "select sessionKey from Sessions where lastAccess <= (", nowstr, " - ", timeoutstr, ")", NULL);
  runSQL(sqlsock, &result, query);
  dhufree(query);
  
  for (i = 0; i < numSQLRows(result); i++) {
    keystr = fetchSQLData(result, i, 0);
    vstrdupcat(&query, "delete from SessionData where sessionKey = '", keystr, "'", NULL);
    runSQL(sqlsock, &result2, query);
    dhufree(query);
    freeSQLResult(result2);
  }  

  freeSQLResult(result);

  vstrdupcat(&query, "delete from Sessions where lastAccess < (", nowstr, " - ", timeoutstr, ")", NULL);
  runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);
  dhufree(timeoutstr);
  

  int2Str(uid, &uidstr);

  safeuser = escapeSQLString(user);
  safefullname = escapeSQLString(fullname);
  safeusertype = escapeSQLString(usertype);
  vstrdupcat(&query, "insert into Sessions values('", key, "', ", nowstr, " , '", safeuser, "', ", uidstr, ", '", super?"y":"n", "', '", safefullname, "', '", safeusertype, "')", NULL);
  dhufree(uidstr);
  dhufree(safeuser);
  dhufree(safefullname);
  dhufree(safeusertype);
  dhufree(nowstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  freeSQLResult(result); 
  return E_OK;
}

/*********************************************************************
* loadObjectVersionsDB...
*
* Load the list of previous versions of this object.
*********************************************************************/
int loadObjectVersionsDB(int objid, int **objids, int *numobjs, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *objidstr = NULL, *name = NULL, *parentID = NULL;
  int i = 0, errcode = 0;

  int2Str(objid, &objidstr);

  vstrdupcat(&query, "select objectName, parentID from Objects where objectID = ", objidstr, NULL);
  dhufree(objidstr);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (((*numobjs) = numSQLRows(result)) < 1) {
    return NODATAFOUND;
  }

  name = escapeSQLString(fetchSQLData(result, 0, 0));
  parentID = fetchSQLData(result, 0, 1);
  vstrdupcat(&query, "select objectID from Objects where objectName = '", name, "' and isDeleted != 'y' and parentID = ", parentID, " ORDER BY version DESC, objectID DESC", NULL);
  dhufree(name);
  freeSQLResult(result);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  *numobjs = numSQLRows(result);
  *objids = (int *) dhumalloc(sizeof(int) * (*numobjs));

  for (i = 0; i < (*numobjs); i++) {
    (*objids)[i] = strtol(fetchSQLData(result, i, 0), NULL, 10);
  }
  freeSQLResult(result);

  return E_OK;
}



/*******************************************************************************
* loadWorkflowListDB...
*
* Retrieve the ids of all the objects that require verification.
*******************************************************************************/
int loadWorkflowListDB(int userid, char *filter, int timestamp, int **list, int *numobjs, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *uidstr = NULL, *safefilter = NULL, *tmp = NULL, *name = NULL;
  int i = 0, errcode = 0, total = 0, objectID = 0, parentID = 0, currentID = 0;

  int2Str(userid, &uidstr);
  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select t1.objectID, t1.objectName, t1.parentID, t4.objectID from Verifiers as t2, GroupMembers as t3, Objects as t1 ", 
                                                          "LEFT JOIN VerifierInstance as t4 on ",
                                                          "t4.objectID = t1.objectID and ",
                                                          "t4.userID = ", uidstr, " where ",
                                                          "t1.parentID = t2.objectID and ",
                                                          "t2.groupID = t3.groupID and ",
                                                          "t3.userID = ", uidstr, " and ",
                                                          "t1.isOnline = 'n' and ",
                                                          "t1.objectName like '", safefilter, "%' ",
                                                          "ORDER BY objectName ASC", NULL);
  dhufree(uidstr);
  dhufree(safefilter);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (((*numobjs) = numSQLRows(result)) < 1) {
    return NODATAFOUND;
  }
  
  *list = (int *) dhumalloc(sizeof(int) * (*numobjs));

  for (i = 0; i < (*numobjs); i++) {
    tmp = fetchSQLData(result, i, 3);
    if (tmp == NULL || *tmp == '\0') { 
      name = fetchSQLData(result, i, 1);
      parentID = strtol(fetchSQLData(result, i, 2), NULL, 10);
      objectID = strtol(fetchSQLData(result, i, 0), NULL, 10);
      if (getObjectID(parentID, name, timestamp, &currentID, sqlsock) == E_OK &&
          objectID == currentID) {
        (*list)[total++] = objectID;
      }
    }
  }
  freeSQLResult(result);
  *numobjs = total;
  
  return E_OK;
}

/*******************************************************************************
* loadDeletedFolderContentsDB...
*
* Retrieve the ids of all the objects within this one that are deleted.
*******************************************************************************/
int loadDeletedFolderContentsDB(int objid, char *filter, int timestamp, char *sort, int **list, int *numobjs, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *objidstr = NULL, *safefilter = NULL, *times = NULL, *name = NULL, *order = NULL;
  int i = 0, errcode = 0, id = 0, j = 0, vid = 0;

  int2Str(objid, &objidstr);
  safefilter = escapeSQLString(filter);
  time2Str(timestamp, &times);

  if (sort == NULL) {
    order = dhustrdup("ORDER BY relativeOrder, objectName ASC");
  } else {
    if (strcasecmp(sort, "date") == 0) {
      order = dhustrdup("ORDER BY version ASC, objectID DESC");
    } else if (strcasecmp(sort, "relative") == 0) {
      order = dhustrdup("ORDER BY relativeOrder, objectName ASC");
    } else if (strcasecmp(sort, "mimetype") == 0) {
      order = dhustrdup("ORDER BY mimeType ASC");
    } else if (strcasecmp(sort, "name") == 0) {
      order = dhustrdup("ORDER BY objectName ASC");
    } else {
      order = dhustrdup("ORDER BY objectName ASC");
    }
  }

  vstrdupcat(&query, "select objectName, objectID from Objects where parentID = ", objidstr, " and version <= ", times, " and objectName like '", safefilter, "%' ", order, NULL);
  dhufree(objidstr);
  dhufree(safefilter);
  dhufree(times);
  dhufree(order);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (((*numobjs) = numSQLRows(result)) < 1) {
    return NODATAFOUND;
  }

  *list = (int *) dhumalloc(sizeof(int) * (*numobjs));

  j = 0;
  for (i = 0; i < (*numobjs); i++) {
    name = fetchSQLData(result, i, 0);
    vid = strtol(fetchSQLData(result, i, 1), NULL, 10);
    if (getDeletedObjectID(objid, name, timestamp, &id, sqlsock) == E_OK) {
      if (vid == id)
        (*list)[j++] = id;
    }
  }
  freeSQLResult(result);

  *numobjs = j;
  return E_OK;
}

/*******************************************************************************
* loadFolderContentsDB...
*
* Retrieve the ids of all the objects within this one.
*******************************************************************************/
int loadFolderContentsDB(int objid, char *filter, int timestamp, char *sort, int **list, int *numobjs, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *objidstr = NULL, *safefilter = NULL, *times = NULL, *name = NULL, *order = NULL;
  int i = 0, errcode = 0, id = 0, j = 0, vid = 0;

  int2Str(objid, &objidstr);
  safefilter = escapeSQLString(filter);
  time2Str(timestamp, &times);

  if (sort == NULL) {
    order = dhustrdup("ORDER BY relativeOrder, objectName ASC");
  } else {
    if (strcasecmp(sort, "date") == 0) {
      order = dhustrdup("ORDER BY version ASC, objectID DESC");
    } else if (strcasecmp(sort, "relative") == 0) {
      order = dhustrdup("ORDER BY relativeOrder, objectName ASC");
    } else if (strcasecmp(sort, "mimetype") == 0) {
      order = dhustrdup("ORDER BY mimeType ASC");
    } else if (strcasecmp(sort, "name") == 0) {
      order = dhustrdup("ORDER BY objectName ASC");
    } else {
      order = dhustrdup("ORDER BY objectName ASC");
    }
  }

  vstrdupcat(&query, "select objectName, objectID from Objects where parentID = ", objidstr, " and version <= ", times, " and objectName like '", safefilter, "%' ", order, NULL);
  dhufree(objidstr);
  dhufree(safefilter);
  dhufree(times);
  dhufree(order);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (((*numobjs) = numSQLRows(result)) < 1) {
    return NODATAFOUND;
  }

  *list = (int *) dhumalloc(sizeof(int) * (*numobjs));

  j = 0;
  for (i = 0; i < (*numobjs); i++) {
    name = fetchSQLData(result, i, 0);
    vid = strtol(fetchSQLData(result, i, 1), NULL, 10);
    if (getObjectID(objid, name, timestamp, &id, sqlsock) == E_OK) {
      if (vid == id)
        (*list)[j++] = id;
    }
  }
  freeSQLResult(result);

  *numobjs = j;
  return E_OK;
}

/*******************************************************************************
* loadSessionData...
*
* Try to insert this session in the database.
* If the insert fails the calling function will try a different key.
*******************************************************************************/
int loadSessionData(char *key, Env *env, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *timeoutstr = NULL, *nowstr = nowStr();
  int errcode = 0, timeout = 0;

  timeout = getSessionTimeout();
  
  int2Str(timeout, &timeoutstr);

  vstrdupcat(&query, "select userName, userID, isSuperUser, fullName, userType from Sessions where sessionKey = '", key, "' and lastAccess >= (", nowstr, " - ", timeoutstr, ")", NULL);
  dhufree(timeoutstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    dhufree(nowstr);
    return errcode;
  }
 
  if (numSQLRows(result) < 1) {
    dhufree(nowstr);
    return NODATAFOUND;
  }

  setTokenValue(strdup("USERNAME"), dhustrdup(fetchSQLData(result, 0, 0)), env);
  setTokenValue(strdup("USERID"), dhustrdup(fetchSQLData(result, 0, 1)), env);
  setTokenValue(strdup("ISSUPERUSER"), dhustrdup(fetchSQLData(result, 0, 2)), env);
  setTokenValue(strdup("FULLNAME"), dhustrdup(fetchSQLData(result, 0, 3)), env);
  setTokenValue(strdup("USERTYPE"), dhustrdup(fetchSQLData(result, 0, 4)), env);
  
  freeSQLResult(result);

  vstrdupcat(&query, "update Sessions set lastAccess = ", nowstr, " where sessionKey = '", key, "'", NULL);
  runSQL(sqlsock, &result, query);
  dhufree(nowstr);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*********************************************************************
* loadGroupMembersStack...
*
* Get all the members of this group.
*********************************************************************/
int loadGroupMembersStack(int groupid, Stack **thestack, void *sqlsock) {
  void *result = NULL;
  Stack *stack = NULL;
  char *gstr = NULL, *query = NULL;
  int errcode = 0, i = 0, total = 0, *uid = NULL;

  int2Str(groupid, &gstr);
  vstrdupcat(&query, "select userID from GroupMembers where groupID = ", gstr, NULL);
  dhufree(gstr);
 
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  stack = initStack();
  total = numSQLRows(result);
  for (i = 0; i < total; i++) {
    uid = (int *) dhumalloc(sizeof(int));
    *uid = strtol(fetchSQLData(result, 0, 0), NULL, 10);
    pushStack(stack, uid);
  }

  freeSQLResult(result);
  *thestack = stack;
  return E_OK;
}

/*********************************************************************
* calcIsVerified...
*
* Is this object verified? (Are there no verifiers or is the author
* the only verifier?)
*********************************************************************/
int calcIsVerified(int objid, int *isverified, int *isverifier, Env *env, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *idstr = NULL, *reqall = NULL, *uidstr = NULL;
  int errcode = 0, groupid = 0, all = 0, *uid = NULL;
  Stack *stack = NULL;

  int2Str(objid, &idstr);

  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", idstr, NULL);
  dhufree(idstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) < 1) {
    *isverified = 1;
    *isverifier = 0;
    return E_OK;
  }

  groupid = strtol(fetchSQLData(result, 0, 0), NULL, 10);
  reqall = fetchSQLData(result, 0, 1);
  all = (*reqall == 'y')?1:0; 

  loadGroupMembersStack(groupid, &stack, sqlsock);

  if (stack->size == 0) {
    *isverified = 1;
    *isverifier = 0;
  } else if ((stack->size == 1) && (!reqall)) {
    uid = (int *) popStack(stack);
    uidstr = getEnvValue("USERID", env);
    if (*uid == (strtol(uidstr?uidstr:"-1", NULL, 10))) {
      *isverifier = 1;
      *isverified = 1;
    } else {
      *isverifier = 0;
      *isverified = 0;
    }
    dhufree(uid);
  } else {
    *isverified = 0;
    *isverifier = 0;
    while ((uid = (int *) popStack(stack)) != NULL)  {
      if (*uid == strtol(uidstr?uidstr:"-1", NULL, 10))
        *isverifier = 1;
      dhufree(uid); 
    }
  }
  
  freeSQLResult(result);
  freeStack(&stack);
  return E_OK;
}

/*******************************************************************************
* getWordID...
*
* Get the unique identifier for this word.
*******************************************************************************/
int getWordID(char *w, int *id, void *sqlsock) {
  void *result = NULL;
  char *safeWord = NULL, *query = NULL, *lower = NULL;
  int errcode = 0;

  safeWord = escapeSQLString(w);
  lower = safeWord;
  do {
    *lower = tolower(*lower);
  } while (*lower++ != '\0'); 
 
  vstrdupcat(&query, "select wordID from Dictionary where wordStr = '", safeWord, "'", NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if ((errcode == 0) && (numSQLRows(result) > 0)) {
    *id = strtol(fetchSQLData(result, 0, 0), NULL, 10);
    freeSQLResult(result);
    dhufree(safeWord);
    return E_OK;
  }

  vstrdupcat(&query, "insert into Dictionary (wordStr) values ('", safeWord, "')", NULL);
  dhufree(safeWord);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  getSequenceValue("Dictionary_wordID_seq", id, sqlsock);
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* getNextVersion...
*
* What is the next version number?
*******************************************************************************/
int getNextVersion(char *objectName, char *parentID, int *version, void *sqlsock) {
  void *result = NULL;
  char *safeName = NULL, *query = NULL;
  int errcode = 0;

  safeName = escapeSQLString(objectName);
  vstrdupcat(&query, "select MAX(version) from Objects where objectName = '", safeName, "' and parentID = ", parentID, NULL);
  dhufree(safeName);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *version = strtol(fetchSQLData(result, 0, 0), NULL, 10) + 1;

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* updateVersion...
*
* Just update the version.
*******************************************************************************/
int updateVersion(int objectid, int version, void *sqlsock) {
  void *result = NULL;
  char *objstr = NULL, *verstr = NULL, *query = NULL;
  int errcode = 0;

  int2Str(objectid, &objstr);
  int2Str(version, &verstr);
  vstrdupcat(&query, "update Objects set version = ", verstr, " where objectID = ", objstr, NULL);
  dhufree(objstr);
  dhufree(verstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* deleteGroup...
*
* Relieves this group from active duty!
*******************************************************************************/
int deleteGroup(int gid, void *sqlsock) {
  void *result = NULL;
  char *gidstr = NULL, *query = NULL;
  int errcode = 0;

  int2Str(gid, &gidstr);
  vstrdupcat(&query, "delete from Groups where groupID = ", gidstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  freeSQLResult(result);

  vstrdupcat(&query, "delete from GroupMembers where groupID = ", gidstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  freeSQLResult(result);
  
  vstrdupcat(&query, "delete from Permissions where groupID = ", gidstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  freeSQLResult(result);
  vstrdupcat(&query, "delete from Verifiers where groupID = ", gidstr, NULL);
  dhufree(gidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* deleteUser...
*
* Relieves this user from active duty!
*******************************************************************************/
int deleteUser(int uid, void *sqlsock) {
  void *result = NULL;
  char *uidstr = NULL, *revstr = NULL, *query = NULL;
  int errcode = 0, revision = 0;

  int2Str(uid, &uidstr);
  vstrdupcat(&query, "select MAX(t1.revision) from Users as t1, Users as t2 where t1.userName = t2.userName and t2.userID = ", uidstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(uidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    dhufree(uidstr);
    return NODATAFOUND;
  }
  revision = strtol(fetchSQLData(result, 0, 0), NULL, 10) + 1;

  revision++;
  int2Str(revision, &revstr);
  vstrdupcat(&query, "update Users set revision = ", revstr, " where userID = ", uidstr, NULL);
  dhufree(uidstr);
    
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* insertVerify...
*
* Insert a verify instance.
*******************************************************************************/
int insertVerify(int objectid, int userid, void *sqlsock) {
  void *result = NULL;
  char *objstr = NULL, *usrstr = NULL, *query = NULL;
  int errcode = 0;

  int2Str(objectid, &objstr);
  int2Str(userid, &usrstr);
 
  vstrdupcat(&query, "insert into Verifiers (userID, objectID) values (", usrstr, ", ", objstr, ")", NULL);
  dhufree(objstr);
  dhufree(usrstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* increaseWordOccurrence...
*
* Increase this occurrence in the DB.
*******************************************************************************/
int increaseWordOccurrence(int objectid, int wordid, int total, void *sqlsock) {
  void *result = NULL;
  char *o = NULL, *w = NULL, *t = NULL, *query = NULL, *tmp = NULL;
  int errcode = 0, insert = 0;

  int2Str(objectid, &o);
  int2Str(wordid, &w);

  vstrdupcat(&query, "select total from Occurrence where objectID = ", o, " and wordID = ", w, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(o);
    dhufree(w);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  insert = 1;
  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    total += strtol(tmp, NULL, 10);
    insert = 0;
  }
  
  int2Str(total, &t);
  freeSQLResult(result);
  
  if (insert) {
    vstrdupcat(&query, "insert into Occurrence (wordID, objectID, total) values (", w, ", ", o, ", ", t, ");", NULL);
  } else {
    vstrdupcat(&query, "update Occurrence set total = ", t, " where objectID = ", o, " and wordID = ", w, ";", NULL);
  }
  dhufree(o);
  dhufree(w);
  dhufree(t);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* refreshWordStats...
*
* Update the mean and stddev for this word.
*******************************************************************************/
int refreshWordStats(int wordid, void *sqlsock) {
  void *result = NULL;
  char *w = NULL, *query = NULL, average[32], stddev[32];
  int errcode = 0;
  double std = 0.0, avg = 0.0;

  int2Str(wordid, &w);
  
  vstrdupcat(&query, "select AVG(total), STDDEV(total) from Occurrence where wordID = ", w, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if ((errcode == 0) && (numSQLRows(result) > 0)) {
    avg = strtod(fetchSQLData(result, 0, 0), NULL);
    std = strtod(fetchSQLData(result, 0, 1), NULL);

    if (std < 1.0)
      std = 1.0;
    freeSQLResult(result);
    sprintf(average, "%.8f", avg);
    sprintf(stddev, "%.8f", std);
    vstrdupcat(&query, "update Dictionary set wordMean = ", average, ", wordStd = ", stddev, " where wordID = ", w, NULL);
    dhufree(w);
    errcode = runSQL(sqlsock, &result, query);
    dhufree(query);
    if ((errcode != 0)) {
      logError("Query Error: %s\n", getSQLError(sqlsock));
      return errcode;
    }
    freeSQLResult(result);
    return E_OK;
  } else {
    dhufree(w);
    if (errcode == 0)
      errcode = INCONSISTENTDATA;
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
}

/*******************************************************************************
* removeObjectFromSearchTables...
*
* Removes all occurences of this object from the search tables.
*******************************************************************************/
int removeObjectFromSearchTables(int objectid, void *sqlsock) {
  void *result = NULL;
  char *o = NULL, *query = NULL;
  int errcode = 0;

  int2Str(objectid, &o);

  vstrdupcat(&query, "delete from Occurrence where objectID = ", o, NULL);
  dhufree(o);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* getSearchScore...
*
* Return the score for a single search result.
*******************************************************************************/
int getSearchScore(int objectID, int wordID, void *sqlsock) {
  int score = 0, errcode = 0;
  void *result = NULL;
  char *w = NULL, *o = NULL, *query = NULL;

  int2Str(wordID, &w);
  int2Str(objectID, &o);

  vstrdupcat(&query, "select (((t1.total - t2.wordMean) / (t2.wordSTD ))*20 + 50)  from Occurrence as t1, Dictionary as t2 where t1.wordID = t2.wordID and t1.wordID = ", w, " and t1.objectID = ", o, NULL);
  dhufree(w);
  dhufree(o);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return 0;
  }

  if (numSQLRows(result) > 0) {
    score = strtol(fetchSQLData(result, 0, 0), NULL, 10);
  }
  freeSQLResult(result);
  if (score < 0) score = 0;
  if (score > 100) score = 100;
  return score;
}

/*******************************************************************************
* getSearchHits...
*
* Return a list of search results.
*******************************************************************************/
SearchHit *getSearchHits(int wordID, int timestamp, int *numwhits, void *sqlsock) {
  SearchHit *hits = NULL;
  int num = 0, i = 0, errcode = 0, objectID = 0, parentID = 0, count = 0, currentID = 0;
  void *result = NULL;
  char *w = NULL, *query = NULL, *times = NULL, *objectName = NULL;

  time2Str(timestamp, &times);
  int2Str(wordID, &w);

  vstrdupcat(&query, "select t3.objectID, t3.objectName, t3.parentID from Occurrence as t1, Objects as t3 where t1.wordID = ", w, " and t1.objectID = t3.objectID and t3.version <= ", times, NULL);

  dhufree(w);
  dhufree(times);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    *numwhits = 0;
    return NULL;
  }
  
  num = numSQLRows(result);
   
  hits = (SearchHit *) dhumalloc(sizeof(SearchHit) * num);
  for (i = 0; i < num; i++) {
    objectID = strtol(fetchSQLData(result, i, 0), NULL, 10);
    objectName = fetchSQLData(result, i, 1);
    parentID = strtol(fetchSQLData(result, i, 2), NULL, 10);

    if (getObjectID(parentID, objectName, timestamp, &currentID, sqlsock) == E_OK &&
        objectID == currentID) {
      // This object is current;
      hits[count].score = getSearchScore(objectID, wordID, sqlsock);
      hits[count++].objectID = objectID;
    }
  }
  freeSQLResult(result);
  *numwhits = count;
  return hits;
}

/*******************************************************************************
* createNewGroup...
*
* insert a new group into the db.
*******************************************************************************/
int createNewGroup(char *gname, int ispublic, void *sqlsock) {
  void *result = NULL;
  char *safegname = NULL, *query = NULL;
  int errcode = 0;
  
  safegname = escapeSQLString(gname);
 
  vstrdupcat(&query, "insert into Groups (groupName,isPublic) values ('", safegname, "', '", (ispublic?"y":"n"), "')", NULL);

  dhufree(safegname);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return DUPLICATEGROUPNAME;
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* createNewUser...
*
* insert a new user into the db.
*******************************************************************************/
int createNewUser(char *uname, char *pword, char *fname, int super, int online, char *email, char *usertype, void *sqlsock) {
  void *result = NULL;
  char *safeuname = NULL, *safefname = NULL, *safepword = NULL, *query = NULL, *safeemail = NULL;
  char *hash = NULL, *seed = NULL;
  int errcode = 0;
  
  safeuname = escapeSQLString(uname);
  safefname = escapeSQLString(fname);
  safepword = escapeSQLString(pword);
  safeemail = escapeSQLString(email);

  vstrdupcat(&seed, safeuname, ":Epiction CMS:", safepword, NULL);
  hash = MD5(seed);
 
  vstrdupcat(&query, "insert into Users (userName, password, fullName, isSuperUser, isOnline, email, userType) values (",
						"'", safeuname, "', ",
                                            	"'", hash, "', ",
                                            	"'", safefname, "', ",
                                            	"'", (super?"y":"n"), "', ",
                                            	"'", (online?"y":"n"), "', ",
						"'", safeemail, "', ",
						"'", usertype, "')", NULL);

  dhufree(safeuname);
  dhufree(safepword);
  dhufree(safefname);
  dhufree(safeemail);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return DUPLICATEUSERNAME;
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* loadUserPassword...
*
* Load the user password hash from the db. the hash is username:realm:password.
* For more information read rfc 2518.
*******************************************************************************/
int loadUserPassword(char *username, int *uid, char **password, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *safeuname = NULL;
  int errcode = 0;

  safeuname = escapeSQLString(username);
  vstrdupcat(&query, "select password, userID from Users where userName = '", safeuname, "' and isOnline = 'y' and revision = 0", NULL);
  dhufree(safeuname);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *password = dhustrdup(fetchSQLData(result, 0, 0));
  *uid = strtol(fetchSQLData(result, 0, 1), NULL, 10);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* loadUsersGroupsDB...
*
* load a group list from the db belonging to a user.
*******************************************************************************/
int loadUsersGroupsDB(int id, char *filter, int **list, int *numgroups, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *safefilter = NULL, *idstr = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  int2Str(id, &idstr);
  vstrdupcat(&query, "select t1.groupID from GroupMembers as t1, Groups as t2 where t1.userID = ", idstr, " and t1.groupID = t2.groupID and t2.groupName like '", safefilter, "%' order by t2.groupName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (((*numgroups) = numSQLRows(result)) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *list = (int *) dhumalloc(sizeof(int) * (*numgroups));

  for (i = 0; i < (*numgroups); i++) {
    (*list)[i] = strtol(fetchSQLData(result, i, 0), NULL, 10);
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* loadGroupMembersDB...
*
* load a group list from the db.
*******************************************************************************/
int loadGroupMembersDB(int id, char *filter, int **list, int *numusers, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *safefilter = NULL, *idstr = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  int2Str(id, &idstr);
  vstrdupcat(&query, "select t1.userID from GroupMembers as t1, Users as t2 where t1.groupID = ", idstr, " and t1.userID = t2.userID and t2.userName like '", safefilter, "%' and t2.revision = 0 order by t2.userName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (((*numusers) = numSQLRows(result)) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *list = (int *) dhumalloc(sizeof(int) * (*numusers));

  for (i = 0; i < (*numusers); i++) {
    (*list)[i] = strtol(fetchSQLData(result, i, 0), NULL, 10);
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* loadGroupListDB...
*
* load a group list from the db.
*******************************************************************************/
int loadGroupListDB(char *filter, int **list, int *numgroups, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *safefilter = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select groupID from Groups where groupName like '", safefilter, "%' order by groupName", NULL);
  dhufree(safefilter);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (((*numgroups) = numSQLRows(result)) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *list = (int *) dhumalloc(sizeof(int) * (*numgroups));

  for (i = 0; i < (*numgroups); i++) {
    (*list)[i] = strtol(fetchSQLData(result, i, 0), NULL, 10);
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* loadUserListDB...
*
* load a user list from the db.
*******************************************************************************/
int loadUserListDB(char *filter, int **list, int *numusers, void *sqlsock) {
  void *result = NULL;
  char *query = NULL, *safefilter = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select userID from Users where userName like '", safefilter, "%' and revision = 0 order by userName", NULL);
  dhufree(safefilter);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (((*numusers) = numSQLRows(result)) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *list = (int *) dhumalloc(sizeof(int) * (*numusers));

  for (i = 0; i < (*numusers); i++) {
    (*list)[i] = strtol(fetchSQLData(result, i, 0), NULL, 10);
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* getGroupDetails...
*
* Find the details about this group.
*******************************************************************************/
int getGroupDetails(int gid, GroupDetails **details, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  GroupDetails *group = NULL;
  char *query = NULL, *gidstr = NULL, *tmp = NULL;
  
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "select groupID,groupName,isPublic from Groups where groupID = ", gidstr, NULL);
  dhufree(gidstr);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  group = initGroupDetails();
  tmp = fetchSQLData(result, 0, 0);
  group->groupID = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 1);
  group->groupName = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 2);
  group->isPublic = *tmp == 'y';
  
  freeSQLResult(result);

  *details = group;
  return E_OK;
}

/*******************************************************************************
* getUserDetails...
*
* Find the details about this user.
*******************************************************************************/
int getUserDetails(int uid, UserDetails **details, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  UserDetails *user = NULL;
  char *query = NULL, *uidstr = NULL, *tmp = NULL;
  
  int2Str(uid, &uidstr);
  vstrdupcat(&query, "select userID,userName,isOnline,isSuperUser,fullName,email,userType from Users where userID = ", uidstr, NULL);
  dhufree(uidstr);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  user = initUserDetails();
  tmp = fetchSQLData(result, 0, 0);
  user->userID = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 1);
  user->userName = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 2);
  user->isOnline = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 3);
  user->isSuperUser = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 4);
  user->fullName = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 5);
  user->email = dhustrdup(tmp?tmp:(char *)"");
  tmp = fetchSQLData(result, 0, 6);
  user->userType = dhustrdup(tmp?tmp:(char *)"");
  
  freeSQLResult(result);

  *details = user;
  return E_OK;
}

/*******************************************************************************
* editGroupDetails...
*
* edit a group in the db.
*******************************************************************************/
int editGroupDetails(int gid, char *gname, int ispublic, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *gidstr = NULL, *safegname = NULL;

  int2Str(gid, &gidstr);
  safegname = escapeSQLString(gname);
  vstrdupcat(&query, "update Groups set groupName = '", safegname, "', ", 
                                       "isPublic = '", (ispublic?"y":"n"), "' ",
                                       "where groupID = ", gidstr, NULL);
  dhufree(gidstr);
  dhufree(safegname);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* userIsExternal...
*
* See if this is an ldap user (or other).
*******************************************************************************/
int userIsExternal(int uid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, ext = 0;
  char *query = NULL, *uidstr = NULL;

  int2Str(uid, &uidstr);
  vstrdupcat(&query, "select userType from Users where userID = ", uidstr, NULL);
  dhufree(uidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return 0;
  }

  if (strcasecmp(fetchSQLData(result, 0, 0), "INTERNAL") != 0) {
	  ext = 1;
  }
 
  freeSQLResult(result);
  return ext;
}

/*******************************************************************************
* editUserDetails...
*
* edit a user in the db.
*******************************************************************************/
int editUserDetails(int uid, char *uname, char *upass, int online, int super, char *fname, char *email, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *uidstr = NULL, *safeuname = NULL, *safefname = NULL, *safepword = NULL, *safeemail = NULL;
  char *seed = NULL, *hash = NULL;

  if (userIsExternal(uid, sqlsock)) {
	  return USERDETAILSREADONLY;
  }

  int2Str(uid, &uidstr);
  safefname = escapeSQLString(fname);
  safeuname = escapeSQLString(uname);
  safeemail = escapeSQLString(email);
  if (upass != NULL && *upass != '\0') {
    safepword = escapeSQLString(upass);
    vstrdupcat(&seed, safeuname, ":Epiction CMS:", safepword, NULL);
    hash = MD5(seed);
    vstrdupcat(&query, "update Users set userName = '", safeuname, "', \
                                       password = '", hash, "', \
                                       isOnline = '", (online?"y":"n"), "', \
                                       isSuperUser = '", (super?"y":"n"), "', \
                                       fullName = '", safefname, "', \
                                       email = '", safeemail, "' \
                                       where userID = ", uidstr, NULL);
    dhufree(safepword);
    dhufree(hash);
    dhufree(seed);
  } else {
    vstrdupcat(&query, "update Users set userName = '", safeuname, "', \
                                       isOnline = '", (online?"y":"n"), "', \
                                       isSuperUser = '", (super?"y":"n"), "', \
                                       fullName = '", safefname, "', \
                                       email = '", safeemail, "' \
                                       where userID = ", uidstr, NULL);
  }
  dhufree(uidstr);
  dhufree(safefname);
  dhufree(safeuname);
  dhufree(safeemail);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* removeGroupMember...
*
* Remove a user from a group
*******************************************************************************/
int removeGroupMember(int gid, int uid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *uidstr = NULL, *gidstr = NULL;

  int2Str(uid, &uidstr);
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "delete from GroupMembers where userID = ", uidstr, 
                                                 " and groupID = ", gidstr, NULL);
  dhufree(uidstr);
  dhufree(gidstr);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  freeSQLResult(result);
  return E_OK;
}

/******************************************************************************* 
* loadPermissionListDB...
*
* Load the groups attached to this item.
*******************************************************************************/
int loadPermissionListDB(int objid, char *filter, int **list, int *numgroups, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *idstr = NULL, *safefilter = NULL;

  int2Str(objid, &idstr);
  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select t1.groupID from Permissions as t1, Groups as t2 where t1.objectID = ", idstr, " and t1.groupID = t2.groupID and t2.groupName like '", safefilter, "%' group by t1.groupID,t2.groupName order by t2.groupName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (((*numgroups) = numSQLRows(result)) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *list = (int *) dhumalloc(sizeof(int) * (*numgroups));

  for (i = 0; i < (*numgroups); i++) {
    (*list)[i] = strtol(fetchSQLData(result, i, 0), NULL, 10);
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* isObjectLockedByUser...
*
* Is this object locked from edits.
*******************************************************************************/
int isObjectLockedByUser(int objid, int uid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "select lockedByUserID from Objects where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  if (strtol(fetchSQLData(result, 0, 0), NULL, 10) != uid) {
    freeSQLResult(result);
    return NODATAFOUND;
  }
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* isObjectLocked...
*
* Is this object locked from edits.
*******************************************************************************/
int isObjectLocked(int objid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "select lockedByUserID from Objects where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  if (strtol(fetchSQLData(result, 0, 0), NULL, 10) != -1) {
    freeSQLResult(result);
    return E_OK;
  }
  
  freeSQLResult(result);
  return NODATAFOUND;
}

/*******************************************************************************
* unLockObjectDB...
*
* UnLock this object from edits.
*******************************************************************************/
int unLockObjectDB(int objid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;  

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "update Objects set lockedByUserID = -1 where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  freeSQLResult(result);
  return E_OK;
}


/*******************************************************************************
* lockObjectDB...
*
* Lock this object from edits.
*******************************************************************************/
int lockObjectDB(int objid, int uid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *uidstr = NULL;  

  int2Str(objid, &oidstr);
  int2Str(uid, &uidstr);
  vstrdupcat(&query, "update Objects set lockedByUserID = ", uidstr, " where objectID = ", oidstr, NULL);
  dhufree(oidstr);
  dhufree(uidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* loadPermissionMask...
*
* Load the mask for this group/object.
*******************************************************************************/
int loadPermissionMask(int objid, int gid, void *sqlsock, char **mask) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *gidstr = NULL, *tmp = NULL;

  int2Str(objid, &oidstr);
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "select mask from Permissions where objectID = ", oidstr, " and groupID = ", gidstr, NULL);
  dhufree(oidstr);
  dhufree(gidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if ((numSQLRows(result)) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  tmp = fetchSQLData(result, 0, 0);
  *mask = dhustrdup(tmp?tmp:(char *)"---");

  freeSQLResult(result);
  return E_OK;
}


/*******************************************************************************
* addGroupMember...
*
* Add a user to a group
*******************************************************************************/
int addGroupMember(int gid, int uid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *uidstr = NULL, *gidstr = NULL;

  int2Str(uid, &uidstr);
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "insert into GroupMembers (userID, groupID) values (", uidstr, ", ", gidstr, ")", NULL);
  dhufree(uidstr);
  dhufree(gidstr);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return USERISAMEMBER;
  }
 
  freeSQLResult(result);
  return E_OK;
}

int userLicenseCheck(void *sqlsock) {
  void *result = NULL;
  char *tmp = NULL;
  int userCount = 0, errcode = 0;

  errcode = runSQL(sqlsock, &result, "select count(*) from Users where revision = 0;");
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    userCount = strtol(tmp, NULL, 10);
  }
  freeSQLResult(result);

  if (getUserLimit() <= userCount) {
    return USERLICENSELIMIT;
  }

  return E_OK;
}

/*******************************************************************************
* setObjectPermission...
*
* Set this permission on this object.
*******************************************************************************/
int setObjectPermission(int objid, int gid, char *mask, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, exists = 0, now = 0, *children = NULL, numchildren = 0, i = 0;
  char *query = NULL, *objidstr = NULL, *gidstr = NULL, *safemask = NULL, *tmp = NULL;

  int2Str(objid, &objidstr);
  int2Str(gid, &gidstr);
  safemask = escapeSQLString(mask);

  vstrdupcat(&query, "select count(*) from  Permissions where objectID = ", objidstr, 
                                                                         " and groupID = ", gidstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    if (strtol(tmp, NULL, 10) > 0) {
      exists = 1;
    }
  }
  freeSQLResult(result);

  if (exists) {
      vstrdupcat(&query, "update Permissions set mask = '", safemask, "' where objectID = ", objidstr,
                                                                              " and groupID = ", gidstr, NULL);
  } else {
    vstrdupcat(&query, "insert into Permissions (objectID, groupID, mask) values (", 
						objidstr, ", ", 
                                                gidstr, ", ",
                                                "'", safemask, "')", NULL);
  }

  dhufree(objidstr);
  dhufree(gidstr);
  dhufree(safemask);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
 
  freeSQLResult(result);


  // now update the children
  //
  //
  now = time(NULL);
  errcode = loadFolderContentsDB(objid, "", now, "name", &children, &numchildren, sqlsock);

  if (errcode == E_OK) {
    for (i = 0; i < numchildren; i++) {
      setObjectPermission(children[i], gid, mask, sqlsock);      
    }
    dhufree(children);
  }

  return E_OK;
}


/*******************************************************************************
* removeObjectPermission...
*
* Remove this permission on this object.
*******************************************************************************/
int removeObjectPermission(int objid, int gid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, *children = NULL, numchildren = 0, i = 0, now = 0;
  char *query = NULL, *objidstr = NULL, *gidstr = NULL;

  int2Str(objid, &objidstr);
  int2Str(gid, &gidstr);

  vstrdupcat(&query, "delete from Permissions where objectID = ", objidstr, 
                                               " and groupID = ", gidstr, NULL);
  dhufree(objidstr);
  dhufree(gidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);
  
  // now update the children
  //
  //
  now = time(NULL);
  errcode = loadFolderContentsDB(objid, "", now, "name", &children, &numchildren, sqlsock);

  if (errcode == E_OK) {
    for (i = 0; i < numchildren; i++) {
      removeObjectPermission(children[i], gid, sqlsock);      
    }
    dhufree(children);
  }
  return E_OK;
}

/*******************************************************************************
* accessCheck...
*
* Does a select to determin the users permissions on this object.
*******************************************************************************/
int accessCheck(int objid, int uid, char *mask, int *access, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, valid = 0;
  char *query = NULL, *objidstr = NULL, *uidstr = NULL, *safemask = NULL, *tmp = NULL;

  if (mask == NULL || *mask == CNULL) {
    *access = valid;
    return RESOURCEERROR;
  }
  
  int2Str(objid, &objidstr);
  int2Str(uid, &uidstr);
  safemask = escapeSQLString(mask);

  vstrdupcat(&query, "select count(*) from Permissions as t1, GroupMembers as t2, Objects as t3, Users as t4", 
                                           " where t1.mask like '", safemask,
                                           "' and t2.userID = ", uidstr,
                                           " and t2.groupID = t1.groupID and t1.objectID = ", objidstr,
                                           " and t3.objectID = t1.objectID and t2.userID = t4.userID", NULL);

  dhufree(safemask);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    dhufree(objidstr);
    dhufree(uidstr);
    *access = valid;
    return errcode;
  }

  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    if (strtol(tmp, NULL, 10) > 0) {
      valid = 1;
    }
  }

  if (!valid) {
    if (mask[1] != 'w' && mask[2] != 'x') {
      isObjectPublic(objid, &valid, sqlsock);
    }
  }

  if (valid && *mask == 'r') {
    vstrdupcat(&query, "select isOnline from Objects where objectID = ", objidstr, NULL);
    errcode = runSQL(sqlsock, &result, query);
    dhufree(query);
    if (errcode != 0) {
      logError("Query Error: %s\n", getSQLError(sqlsock));
      dhufree(objidstr);
      dhufree(uidstr);
      *access = valid;
      return errcode;
    }

    if ((numSQLRows(result)) > 0) {
      tmp = fetchSQLData(result, 0, 0);
      if (tolower(*tmp) == 'n') {
	// must have write access
        errcode = accessCheck(objid, uid, "_w_", &valid, sqlsock);
        if (errcode != E_OK) {
          dhufree(objidstr);
          dhufree(uidstr);
          *access = valid;
          return errcode;
        }
      }
    }

  }

  dhufree(uidstr);
  dhufree(objidstr);
  freeSQLResult(result);
  *access = valid;
  return E_OK; 
}

/*******************************************************************************
* copyWorkflow...
*
* Copy the workflow to the new version.
*******************************************************************************/
int copyWorkflow(int oldid, int newid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  vstrdupcat(&query, "create temporary table tmpW (groupID integer, requiresAll char(1))", NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);

  vstrdupcat(&query, "insert into tmpW select groupID, requiresAll from Verifiers where objectID = ", oldidstr, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into Verifiers select ", newidstr, ", groupID, requiresAll from tmpW", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpW", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  return E_OK;
}

/*******************************************************************************
* copyPermissions...
*
* Copy the permissions to the new version.
*******************************************************************************/
int copyPermissions(int oldid, int newid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  vstrdupcat(&query, "create temporary table tmpP (groupID integer, mask char(3))", NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);
  vstrdupcat(&query, "delete from Permissions where objectID = ", newidstr, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 

  vstrdupcat(&query, "insert into tmpP select groupID, mask from Permissions where objectID = ", oldidstr, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into Permissions select ", newidstr, ", groupID, mask from tmpP", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpP", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  return E_OK;
}

/*******************************************************************************
* copyMetadata...
*
* Copy the metadata to the new version.
*******************************************************************************/
int copyMetadata(int oldid, int newid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);
  vstrdupcat(&query, "delete from ObjectMetadata where objectID = ", newidstr, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result); 

  vstrdupcat(&query, "create temporary table tmpM (fieldName varchar(255), fieldValue varchar(255))", NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 

  vstrdupcat(&query, "insert into tmpM select fieldName, fieldValue from ObjectMetadata where objectID = ", oldidstr, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into ObjectMetadata select ", newidstr, ", fieldName, fieldValue from tmpM", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpM", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result); 
  return E_OK;
}

/*******************************************************************************
* updateChildren...
*
* Does a update to the children of this object to point them to the new version.
*******************************************************************************/
int updateChildren(int oldid, int newid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);
  vstrdupcat(&query, "update Objects set parentID = ", newidstr, " where parentID = ", oldidstr, NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* setObjectMetadata...
*
* Sets this metadata field.
*******************************************************************************/
int setObjectMetadata(int objid, char *name, char *value, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *idstr = NULL, *safename = NULL, *safevalue = NULL;

  if ((name != NULL) && (strlen(name) > 250)) {
	  name[250] = '\0';
  }
  if ((value != NULL) && (strlen(value) > 250)) {
	  value[250] = '\0';
  }

  int2Str(objid, &idstr);
  safename = escapeSQLString(name);
  safevalue = escapeSQLString(value);

  vstrdupcat(&query, "delete from ObjectMetadata where objectID = ", idstr, " and fieldName = '", safename, "'", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  freeSQLResult(result);
  vstrdupcat(&query, "insert into ObjectMetadata (objectID, fieldName, fieldValue) values (", idstr, ", '", safename, "', '", safevalue, "')", NULL);
  dhufree(idstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(safename);
    dhufree(safevalue);
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  freeSQLResult(result);

  if ((strcmp(safename, "dc:format") == 0) && (strcmp(safevalue, "") != 0)) {
    // update the mimetype in the objects table
    int2Str(objid, &idstr);
    query = NULL;
    vstrdupcat(&query, "update Objects set mimeType = '", safevalue, "' where objectID = ", idstr, NULL);
    dhufree(idstr);
    errcode = runSQL(sqlsock, &result, query);
    dhufree(query);

    freeSQLResult(result);
  }
  dhufree(safename);
  dhufree(safevalue);
  return E_OK;
}

/*******************************************************************************
* getObjectMetadata...
*
* Gets this metadata field.
*******************************************************************************/
int getObjectMetadata(int objid, char *name, char **value, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *idstr = NULL, *safename = NULL, *tmp = NULL;
  
  int2Str(objid, &idstr);
  safename = escapeSQLString(name);

  vstrdupcat(&query, "select fieldValue from ObjectMetadata where objectID = ", idstr, " and fieldName = '", safename, "'", NULL);
  dhufree(idstr);
  dhufree(safename);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    *value = dhustrdup(tmp);
  }

  freeSQLResult(result);
  if (tmp == NULL) {
    return NODATAFOUND;
  }
  
  return E_OK;
}

/*******************************************************************************
* getAllObjectMetadata...
*
* Gets all metadata fields.
*******************************************************************************/
int getAllObjectMetadata(int objid, char ***columns, int *numcols, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *idstr = NULL, *tmp = NULL;

  int2Str(objid, &idstr);

  vstrdupcat(&query, "select fieldName from ObjectMetadata where objectID = ", idstr, " order by fieldName", NULL);
  dhufree(idstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  *numcols = numSQLRows(result);
  
  if (*numcols > 0) {
    *columns = (char **) dhumalloc(sizeof (char *) * (*numcols));
    for (i = 0; i < *numcols; i++) {
      tmp = fetchSQLData(result, i, 0);
      (*columns)[i] = dhustrdup(tmp);
    }
  }

  freeSQLResult(result);
  if (*numcols <= 0) {
    return NODATAFOUND;
  }

  return E_OK;
}

/*******************************************************************************
* removeObjectMetadata...
*
* Remove this metadata field.
*******************************************************************************/
int removeObjectMetadata(int objid, char *name, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *idstr = NULL, *safename = NULL;

  int2Str(objid, &idstr);
  safename = escapeSQLString(name);

  vstrdupcat(&query, "delete from ObjectMetadata where objectID = ", idstr, " and fieldName = '", safename, "'", NULL);
  dhufree(idstr);
  dhufree(safename);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  freeSQLResult(result);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  return E_OK;
}

/*******************************************************************************
* setSessionData...
*
* Sets this data field.
*******************************************************************************/
int setSessionData(char *key, char *name, char *value, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *safekey = NULL, *safename = NULL, *safevalue = NULL;

  if (key == NULL)
    return NODATAFOUND;

  safekey = escapeSQLString(key);
  safename = escapeSQLString(name);
  safevalue = escapeSQLString(value);

  vstrdupcat(&query, "delete from SessionData where sessionKey = '", safekey, "' and fieldName = '", safename, "'", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  freeSQLResult(result);
  vstrdupcat(&query, "insert into SessionData (sessionKey, fieldName, fieldValue) values ('", safekey, "', '", safename, "', '", safevalue, "')", NULL);
  dhufree(safekey);
  dhufree(safename);
  dhufree(safevalue);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* getSessionData...
*
* Gets this data field.
*******************************************************************************/
int getSessionData(char *key, char *name, char **value, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *safekey = NULL, *safename = NULL, *tmp = NULL;
  
  if (key == NULL)
    return NODATAFOUND;

  safekey = escapeSQLString(key);
  safename = escapeSQLString(name);

  vstrdupcat(&query, "select fieldValue from SessionData where sessionKey = '", safekey, "' and fieldName = '", safename, "'", NULL);
  dhufree(safekey);
  dhufree(safename);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    *value = dhustrdup(tmp);
  }

  freeSQLResult(result);
  if (tmp == NULL) {
    return NODATAFOUND;
  }
  
  return E_OK;
}

/*******************************************************************************
* getAllSessionData...
*
* Gets all data fields.
*******************************************************************************/
int getAllSessionData(char *key, char ***columns, int *numcols, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *safekey = NULL, *tmp = NULL;

  if (key == NULL)
    return NODATAFOUND;

  safekey = escapeSQLString(key);

  vstrdupcat(&query, "select fieldName from SessionData where sessionKey = '", safekey, "'", NULL);
  dhufree(safekey);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  *numcols = numSQLRows(result);
  
  if (*numcols > 0) {
    *columns = (char **) dhumalloc(sizeof (char *) * (*numcols));
    for (i = 0; i < *numcols; i++) {
      tmp = fetchSQLData(result, 0, 0);
      *columns[i] = dhustrdup(tmp);
    }
  }

  freeSQLResult(result);
  if (*numcols <= 0) {
    return NODATAFOUND;
  }

  return E_OK;
}

/*******************************************************************************
* removeSessionData...
*
* Remove this data field.
*******************************************************************************/
int removeSessionData(char *key, char *name, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *safekey = NULL, *safename = NULL;

  if (key == NULL)
    return NODATAFOUND;

  safekey = escapeSQLString(key);
  safename = escapeSQLString(name);

  vstrdupcat(&query, "delete from SessionData where sessionKey = '", safekey, "' and fieldName = '", safename, "'", NULL);
  dhufree(safename);
  dhufree(safekey);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  freeSQLResult(result);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  return E_OK;
}

/*******************************************************************************
* loadWorkflowSettings...
*
* Load the workflow details for this doc.
*******************************************************************************/
int loadWorkflowSettings(int objid, int *groupid, int *all, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL, *tmp = NULL;

  int2Str(objid, &objstr);
  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", objstr, NULL);
  dhufree(objstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  
  if (numSQLRows(result) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    *groupid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, 0, 1);
    *all = (*tmp == 'y')?1:0;
  } else {
    *groupid = -1;
    *all = 0;
  }

  freeSQLResult(result);
  return (*groupid >= 0)?E_OK:NODATAFOUND;
}

/*******************************************************************************
* removeWorkflowSettings...
*
* Remove the workflow details from this doc.
*******************************************************************************/
int removeWorkflowSettings(int objid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL;

  int2Str(objid, &objstr);
  vstrdupcat(&query, "delete from Verifiers where objectID = ", objstr, NULL);
  dhufree(objstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    freeSQLResult(result);
    return errcode;
  }
  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* saveWorkflowSettings...
*
* Save the workflow details for this doc.
*******************************************************************************/
int saveWorkflowSettings(int objid, int groupid, int all, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL, *gidstr = NULL;

  int2Str(objid, &objstr);
  int2Str(groupid, &gidstr);
  vstrdupcat(&query, "delete from Verifiers where objectID = ", objstr, NULL);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);

  vstrdupcat(&query, "insert into Verifiers (objectID, groupID, requiresAll) values (", 
						objstr, ", ",
                                              	gidstr, ", ",
                                              	"'", all?"y":"n", "')", NULL);
  dhufree(objstr);
  dhufree(gidstr);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    freeSQLResult(result);
    return errcode;
  }

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
* isVerified...
*
* Is this document verified?
*******************************************************************************/
int isVerified(int parentID, int objectID, int *verified, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, count = 0;
  char *query = NULL, *pidstr = NULL, *oidstr = NULL, *gidstr = NULL, *reqall = NULL, *tmp = NULL;

  *verified = 0;
  int2Str(parentID, &pidstr);
  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", pidstr, NULL);
  dhufree(pidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    freeSQLResult(result);
    return errcode;
  }

  if (numSQLRows(result) < 1) {
    *verified = 1;
    freeSQLResult(result);
    return E_OK; 
  }

  gidstr = fetchSQLData(result, 0, 0);
  reqall = fetchSQLData(result, 0, 1);
  
  int2Str(objectID, &oidstr);
  vstrdupcat(&query, "select COUNT(userID) from VerifierInstance where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    freeSQLResult(result);
    return errcode;
  }
  
  tmp = fetchSQLData(result, 0, 0);
  count = strtol(tmp?tmp:"-1", NULL, 10);
  freeSQLResult(result);

  if (tolower(*reqall) == 'y') {
    vstrdupcat(&query, "select COUNT(userID) from GroupMembers where groupID = ", gidstr, NULL);
    errcode = runSQL(sqlsock, &result, query);
    dhufree(query);
    if (errcode != E_OK) {
      logError("Query Error: %s\n", getSQLError(sqlsock));
      freeSQLResult(result);
      return errcode;
    }
    
    tmp = fetchSQLData(result, 0, 0);
    if (count >= strtol(tmp?tmp:"-1", NULL, 10)) {
      *verified = 1;
      freeSQLResult(result);
      return E_OK;
    }
    freeSQLResult(result);
  } else {
    if (count > 0) {
      *verified = 1;
      return E_OK; 
    }
  }
  return E_OK;
}

/*******************************************************************************
* updateOnline...
*
* Set the online flag for this document.
*******************************************************************************/
int updateOnline(int objid, int online, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "update Objects set isOnline = '", online?"y":"n", "' where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* approveObject...
*
* Approve this document and if last verifier set document to online.
*******************************************************************************/
int approveObject(int objid, int uid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0, verified = 0, parentID = 0;
  char *query = NULL, *oidstr = NULL, *uidstr = NULL, *tmp = NULL;

  int2Str(objid, &oidstr);
  int2Str(uid, &uidstr);

  vstrdupcat(&query, "insert into VerifierInstance ( objectID, userID) values (", oidstr, ", ", uidstr, ")", NULL);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    dhufree(oidstr);
    dhufree(uidstr);
    freeSQLResult(result);
    return errcode;
  }
  freeSQLResult(result);
  
  vstrdupcat(&query, "select parentID from Objects where objectID = ", oidstr, NULL);
  dhufree(oidstr);
  dhufree(uidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    freeSQLResult(result);
    return errcode;
  }

  if (numSQLRows(result) < 0) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  tmp = fetchSQLData(result, 0, 0);
  parentID = strtol(tmp?tmp:"-1", NULL, 10);
  freeSQLResult(result);

  if ((errcode = isVerified(parentID, objid, &verified, sqlsock)) != E_OK) {
    return errcode;
  }

  if (verified) {
    if ((errcode = updateOnline(objid, 1, sqlsock)) != E_OK) {
      return errcode;
    }
  }

  return E_OK;
}

/*******************************************************************************
* moveObject...
*
* Move this object to a new folder.
*******************************************************************************/
int moveObjectDB(int objid, int folderid, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *fidstr = NULL, *tmp = NULL;

  int2Str(objid, &oidstr);
  int2Str(folderid, &fidstr);

  if (folderid != -1) {
    vstrdupcat(&query, "select type from Objects where objectID = ", fidstr, NULL);

    errcode = runSQL(sqlsock, &result, query);
    dhufree(query);
    if (errcode != E_OK) {
      logError("Query Error: %s\n", getSQLError(sqlsock));
      freeSQLResult(result);
      dhufree(oidstr);
      dhufree(fidstr);
      return errcode;
    }
  
    if (numSQLRows(result) < 0) {
      freeSQLResult(result);
      dhufree(oidstr);
      dhufree(fidstr);
      return NODATAFOUND;
    }
  
    tmp = fetchSQLData(result, 0, 0);
    
    if (strcmp(tmp, "FOLDER") != 0) {
      freeSQLResult(result);
      return INVALIDXPATH;
    }
    freeSQLResult(result);

  }
  vstrdupcat(&query, "update Objects set parentID = ", fidstr, " where objectID = ", oidstr, NULL);
  dhufree(oidstr);
  dhufree(fidstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    freeSQLResult(result);
    return errcode;
  }
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
* updateObjectDB...
*
* Update the object in the database.
*******************************************************************************/
int updateObjectDB(int objectID, char *name, int ispublic, int relativeOrder, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *safename = NULL, *nowstr = NULL, *relstr = NULL;

  safename = escapeSQLString(name);
  int2Str(objectID, &oidstr);
  int2Str(relativeOrder, &relstr);
  nowstr = nowStr();
  vstrdupcat(&query, "update Objects set isPublic = '", ispublic?"y":"n", "',objectName = '", safename, "', version = '", nowstr, "', relativeOrder = '", relstr, "' where objectID = ", oidstr, NULL);
  dhufree(oidstr);
  dhufree(safename);
  dhufree(nowstr);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }
  freeSQLResult(result);

  return E_OK;
}

// -------------- Used by the calendar module --------------
/*******************************************************************************
 * dbCreateInstance...
 *
 * Called to create a new instance of a calendar. A calendar instance is tied
 * to a path in the repository and all the permissions assigned to that path
 * are also assigned to that instance of the calendar module.
 ******************************************************************************/
int dbCalCreateInstance(cal_instance *inst, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *safepath = NULL;

  safepath = escapeSQLString(inst->objectPath);
  vstrdupcat(&query, "insert into cal_instance (objectPath) values ('", safepath, "');");
  dhufree(safepath);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  getSequenceValue("cal_instance_calid_seq", &(inst->calid), sqlsock);

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
 * dbCalDeleteInstance...
 *
 * Called to delete a new instance of a calendar. A calendar instance is tied
 * to a path in the repository and all the permissions assigned to that path
 * are also assigned to that instance of the calendar module.
 ******************************************************************************/
int dbCalDeleteInstance(cal_instance *inst, void *sqlsock) {
  void *result = NULL, *tmp = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *id = NULL, *eid = NULL;
  
  int2Str(inst->calid, &id);
  vstrdupcat(&query, "select eventid from cal_events where calid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }

  if (numSQLRows(result) >= 0) {
    for (i = 0; i < numSQLRows(result); i++) {
      eid = fetchSQLData(result, i, 0);
      vstrdupcat(&query, "delete from cal_occurrence where eventid = ", eid, NULL);
      errcode = runSQL(sqlsock, &tmp, query);
      dhufree(query);
      freeSQLResult(tmp);
    }
  }
  freeSQLResult(result);

  vstrdupcat(&query, "delete from cal_events where calid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  vstrdupcat(&query, "delete from cal_instance where calid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  dhufree(id);
  return E_OK;
}

/*******************************************************************************
 * dbCalMoveInstance...
 *
 * Called to move a calendar to a new location in the repository.
 ******************************************************************************/
int dbCalMoveInstance(cal_instance *inst, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL, *safepath = NULL;

  int2Str(inst->calid, &id);
  safepath = escapeSQLString(inst->objectPath);
  vstrdupcat(&query, "update cal_instance set objectPath = '", safepath, "' where calid = ", id, NULL);
  dhufree(safepath);
  dhufree(id);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbCalCreateEvent...
 *
 * Called to create an event in the calendar. Events do not show up on the
 * but serve to group occurrences together so they can be updated all at once.
 ******************************************************************************/
int dbCalCreateEvent(cal_event *evt, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL, *nowstr = NULL;
  struct tm *now = NULL;
  time_t stamp;

  stamp = time(NULL);

  now = localtime(&stamp);

  int2Str(evt->calid, &id);
  nowstr = formatDateTime(now);
  vstrdupcat(&query, "insert into cal_events (calid, created, modified) values (", id, ", '", nowstr, "', '", nowstr, "')", NULL);
  dhufree(id);
  dhufree(nowstr);
  errcode = runSQL(sqlsock, &result, query);

  getSequenceValue("cal_events_eventid_seq", &(evt->eventid), sqlsock);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbCalDeleteEvent...
 *
 * Called to delete an event in the calendar.
 * Will delete all occurrences of this event.
 ******************************************************************************/
int dbCalDeleteEvent(cal_event *evt, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL;
  
  int2Str(evt->eventid, &id);
  vstrdupcat(&query, "delete from cal_events where eventid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  vstrdupcat(&query, "delete from cal_occurrence where eventid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  dhufree(id);

  return E_OK;
}

/*******************************************************************************
 * dbCalCreateOccurrence...
 *
 * Called to create an occurrence of an event in the calendar.
 ******************************************************************************/
int dbCalCreateOccurrence(cal_occurrence *occ, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, 
       *id = NULL, 
       *nowstr = NULL, 
       *safesummary = NULL, 
       *safelocation = NULL, 
       *safedescription = NULL, 
       *eventdate = NULL, 
       *starttime = NULL, 
       *endtime = NULL;
  struct tm *now = NULL;
  time_t stamp;

  stamp = time(NULL);

  now = localtime(&stamp);

  int2Str(occ->eventid, &id);
  nowstr = formatDateTime(now);
  
  safesummary = escapeSQLString(occ->summary);
  safelocation = escapeSQLString(occ->location);
  safedescription = escapeSQLString(occ->description);
  eventdate = formatDate(occ->eventdate);
  starttime = formatTime(occ->starttime);
  endtime = formatTime(occ->endtime);
  
  vstrdupcat(&query, "insert into cal_occurrence (eventid, summary, location, description, created, modified, eventdate, starttime, endtime, allday) values (", id, ", '", safesummary, "', '", safelocation, "', '", safedescription, "', '", nowstr, "', '", nowstr, "', '", eventdate, "', '", starttime, "', '", endtime, "', '", (occ->allday?"y":"n"), "')", NULL);
  dhufree(id);
  dhufree(nowstr);
  dhufree(safesummary);
  dhufree(safelocation);
  dhufree(safedescription);
  dhufree(eventdate);
  dhufree(starttime);
  dhufree(endtime);
  errcode = runSQL(sqlsock, &result, query);

  getSequenceValue("cal_occurrence_occurrenceid_seq", &(occ->occurrenceid), sqlsock);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;

}

/*******************************************************************************
 * dbCalEditOccurrence...
 *
 * Called to edit an occurrence of an event in the calendar.
 ******************************************************************************/
int dbCalEditOccurrence(cal_occurrence *occ, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, 
       *id = NULL, 
       *nowstr = NULL, 
       *safesummary = NULL, 
       *safelocation = NULL, 
       *safedescription = NULL, 
       *eventdate = NULL,
       *starttime = NULL, 
       *endtime = NULL;
  struct tm *now = NULL;
  time_t stamp;

  stamp = time(NULL);

  now = localtime(&stamp);

  int2Str(occ->occurrenceid, &id);
  nowstr = formatDateTime(now);
  
  safesummary = escapeSQLString(occ->summary);
  safelocation = escapeSQLString(occ->location);
  safedescription = escapeSQLString(occ->description);
  starttime = formatTime(occ->starttime);
  endtime = formatTime(occ->endtime);
  eventdate = formatDate(occ->eventdate);
  
  vstrdupcat(&query, "update cal_occurrence set summary = '", safesummary, "', location = '", safelocation, "', description = '", safedescription, "', modified = '", nowstr, "', starttime = '", starttime, "', endtime = '", endtime, "', allday = '", (occ->allday?"y":"n"), "', eventdate = '", eventdate, "'  where occurrenceid =  ", id, NULL);
  dhufree(id);
  dhufree(nowstr);
  dhufree(safesummary);
  dhufree(safelocation);
  dhufree(safedescription);
  dhufree(starttime);
  dhufree(endtime);
  dhufree(eventdate);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbCalDeleteOccurrence...
 *
 * Called to delete an occurrence of an event in the calendar.
 ******************************************************************************/
int dbCalDeleteOccurrence(cal_occurrence *occ, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL;
  
  int2Str(occ->occurrenceid, &id);
  vstrdupcat(&query, "delete from cal_occurrence where occurrenceid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  dhufree(id);

  return E_OK;
}

/*******************************************************************************
 * dbCalSearchTime...
 *
 * Called search for events in the calendar by time.
 ******************************************************************************/
int dbCalSearchTime(cal_instance *inst, struct tm *start, struct tm *end, int *count, cal_occurrence *** occlist, void *sqlsock) {
  char *startdate = NULL,
       *enddate = NULL,
       *query = NULL,
       *id = NULL,
       *tmp = NULL;
  void *result = NULL;
  cal_occurrence **list = NULL, *occ = NULL;
  int i = 0, numresults = 0, validcount = 0, errcode = 0;

  startdate = formatDate(start);
  enddate = formatDate(end);

  int2Str(inst->calid, &id);
#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select t1.occurrenceid, t1.eventid, t1.summary, t1.location, t1.description, t1.eventdate, t1.created, t1.modified, t1.starttime, t1.endtime, t1.allday from cal_occurrence as t1, cal_events as t2 where t1.eventid = t2.eventid and t2.calid = ", id, " and t1.eventdate >= '", startdate, "' and t1.eventdate <= '", enddate, "' order by t1.eventdate, t1.starttime", NULL);
#else
  vstrdupcat(&query, "select t1.occurrenceid, t1.eventid, t1.summary, t1.location, t1.description, t1.eventdate, DATE_FORMAT(t1.created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(t1.modified, '%Y-%m-%d %H:%i:%s'), t1.starttime, t1.endtime, t1.allday from cal_occurrence as t1, cal_events as t2 where t1.eventid = t2.eventid and t2.calid = ", id, " and t1.eventdate >= '", startdate, "' and t1.eventdate <= '", enddate, "' order by t1.eventdate, t1.starttime", NULL);
#endif
  dhufree(startdate);
  dhufree(id);
  dhufree(enddate);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }

  numresults = numSQLRows(result);
  if (numresults <= 0) {
    return NODATAFOUND;
  }

  list = (cal_occurrence **) malloc(sizeof(cal_occurrence *) * numresults);

  for (i = 0; i < numresults; i++) {
    occ = (cal_occurrence *) malloc(sizeof(cal_occurrence));
    memset(occ, 0, sizeof(cal_occurrence));

    tmp = fetchSQLData(result, i, 0);
    occ->occurrenceid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 1);
    occ->eventid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 2);
    occ->summary = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 3);
    occ->location = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 4);
    occ->description = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 5);
    occ->eventdate = parseDate(tmp);
    tmp = fetchSQLData(result, i, 6);
    occ->created = parseDateTime(tmp);
    tmp = fetchSQLData(result, i, 7);
    occ->modified = parseDateTime(tmp);
    tmp = fetchSQLData(result, i, 8);
    occ->starttime = parseTime(tmp);
    tmp = fetchSQLData(result, i, 9);
    occ->endtime = parseTime(tmp);
    tmp = fetchSQLData(result, i, 10);
    occ->allday = (tolower(*tmp) == 'y');

    if ((((compareDate(start, occ->eventdate) == 0) && ((compareTime(start, occ->endtime) <= 0) || (occ->allday)))
	   || (compareDate(start, occ->eventdate) < 0))
		   && 
	(((compareDate(end, occ->eventdate) == 0) && ((compareTime(end, occ->starttime) >= 0) || (occ->allday)))
	   || (compareDate(end, occ->eventdate) > 0))) {
      list[validcount++] = occ;
    } else {
      calFreeOccurrence(occ);
    }
  }

  if (validcount == 0) {
    dhufree(list);
    list = NULL;
    freeSQLResult(result);
    return NODATAFOUND;
  }
  *count = validcount;
  *occlist = list;
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbCalSearchEvent...
 *
 * Called search for events in the calendar by event.
 ******************************************************************************/
int dbCalSearchEvent(cal_event *evt, int *count, cal_occurrence *** occlist, void *sqlsock) {
  char *id = NULL,
       *query = NULL,
       *tmp = NULL;
  void *result = NULL;
  cal_occurrence **list = NULL, *occ = NULL;
  int i = 0, numresults = 0, validcount = 0, errcode = 0;

  int2Str(evt->eventid, &id);
#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select occurrenceid, eventid, summary, location, description, eventdate, created, modified, starttime, endtime, allday from cal_occurrence where eventid = ", id, " order by eventdate, starttime", NULL);
#else
  vstrdupcat(&query, "select occurrenceid, eventid, summary, location, description, eventdate, DATE_FORMAT(created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(modified, '%Y-%m-%d %H:%i:%s'), starttime, endtime, allday from cal_occurrence where eventid = ", id, " order by eventdate, starttime", NULL);
#endif
  dhufree(id);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }

  numresults = numSQLRows(result);
  if (numresults <= 0) {
    return NODATAFOUND;
  }

  list = (cal_occurrence **) malloc(sizeof(cal_occurrence *) * numresults);

  for (i = 0; i < numresults; i++) {
    occ = (cal_occurrence *) malloc(sizeof(cal_occurrence));
    memset(occ, 0, sizeof(cal_occurrence));

    tmp = fetchSQLData(result, i, 0);
    occ->occurrenceid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 1);
    occ->eventid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 2);
    occ->summary = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 3);
    occ->location = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 4);
    occ->description = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 5);
    occ->eventdate = parseDate(tmp);
    tmp = fetchSQLData(result, i, 6);
    occ->created = parseDateTime(tmp);
    tmp = fetchSQLData(result, i, 7);
    occ->modified = parseDateTime(tmp);
    tmp = fetchSQLData(result, i, 8);
    occ->starttime = parseTime(tmp);
    tmp = fetchSQLData(result, i, 9);
    occ->endtime = parseTime(tmp);
    tmp = fetchSQLData(result, i, 10);
    occ->allday = (tolower(*tmp) == 'y');
    list[validcount++] = occ;
  }
  *count = validcount;
  *occlist = list;
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbCalEventDetails...
 *
 * Called to load the details for this event.
 ******************************************************************************/
int dbCalEventDetails(cal_event *evt, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL, *tmp = NULL;
  
  int2Str(evt->eventid, &id);
#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select calid, created, modified from cal_events where eventid = ", id, NULL);
#else
  vstrdupcat(&query, "select calid, DATE_FORMAT(created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(modified, '%Y-%m-%d %H:%i:%s') from cal_events where eventid = ", id, NULL);
#endif
  dhufree(id);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    return E_OK;
  }
  
  if (numSQLRows(result) <= 0) {
    return NODATAFOUND;
  }

  tmp = fetchSQLData(result, 0, 0);
  evt->calid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 1);
  evt->created = parseDateTime(tmp);
  tmp = fetchSQLData(result, 0, 2);
  evt->modified = parseDateTime(tmp);

  freeSQLResult(result);
  
  return E_OK;
}

/*******************************************************************************
 * dbCalOccurrenceDetails...
 *
 * Called to load the details for this occurrence.
 ******************************************************************************/
int dbCalOccurrenceDetails(cal_occurrence *occ, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL, *tmp = NULL;
  
  int2Str(occ->occurrenceid, &id);
#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select eventid, summary, location, description, eventdate, created, modified, starttime, endtime, allday from cal_occurrence where occurrenceid = ", id, NULL);
#else
  vstrdupcat(&query, "select eventid, summary, location, description, eventdate, DATE_FORMAT(created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(modified, '%Y-%m-%d %H:%i:%s'), starttime, endtime, allday from cal_occurrence where occurrenceid = ", id, NULL);
#endif
  dhufree(id);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }
  
  if (numSQLRows(result) <= 0) {
    return NODATAFOUND;
  }

  tmp = fetchSQLData(result, 0, 0);
  occ->eventid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 1);
  occ->summary = dhustrdup(tmp);
  tmp = fetchSQLData(result, 0, 2);
  occ->location = dhustrdup(tmp);
  tmp = fetchSQLData(result, 0, 3);
  occ->description = dhustrdup(tmp);
  tmp = fetchSQLData(result, 0, 4);
  occ->eventdate = parseDate(tmp);
  tmp = fetchSQLData(result, 0, 5);
  occ->created = parseDateTime(tmp);
  tmp = fetchSQLData(result, 0, 6);
  occ->modified = parseDateTime(tmp);
  tmp = fetchSQLData(result, 0, 7);
  occ->starttime = parseTime(tmp);
  tmp = fetchSQLData(result, 0, 8);
  occ->endtime = parseTime(tmp);
  tmp = fetchSQLData(result, 0, 9);
  occ->allday = (tolower(*tmp) == 'y');

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
 * dbCalInstanceDetails...
 *
 * Called to load the details for this instance.
 ******************************************************************************/
int dbCalInstanceDetails(cal_instance *instance, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *tmp = NULL;
  
  if (instance->objectPath != NULL) {
    tmp = dhustrdup(instance->objectPath);
    vstrdupcat(&query, "select calid, objectPath from cal_instance where objectPath = '", tmp, "'", NULL);
  } else {
    int2Str(instance->calid, &tmp);
    vstrdupcat(&query, "select calid, objectPath from cal_instance where calid = ", tmp, NULL);
  }
  dhufree(tmp);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }

  if (numSQLRows(result) <= 0) {
    return NODATAFOUND;
  }
  tmp = fetchSQLData(result, 0, 0);
  instance->calid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 1);
  if (instance->objectPath != NULL)
    dhufree(instance->objectPath);
  instance->objectPath = dhustrdup(tmp);

  freeSQLResult(result);
  
  return E_OK;
}

// -------------- Used by the board module --------------
/*******************************************************************************
 * dbBoardCreateInstance...
 *
 * Called to create a new instance of a board. A board instance is tied
 * to a path in the repository and all the permissions assigned to that path
 * are also assigned to that instance of the board module.
 ******************************************************************************/
int dbBoardCreateInstance(board_instance *inst, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *safepath = NULL;

  safepath = escapeSQLString(inst->objectPath);
  vstrdupcat(&query, "insert into board_instance (objectPath) values ('", safepath, "');");
  dhufree(safepath);

  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  getSequenceValue("board_instance_boardid_seq", &(inst->boardid), sqlsock);

  freeSQLResult(result);
  return E_OK;
}

/*******************************************************************************
 * dbBoardDeleteInstance...
 *
 * Called to delete a new instance of a board. A board instance is tied
 * to a path in the repository and all the permissions assigned to that path
 * are also assigned to that instance of the board module.
 ******************************************************************************/
int dbBoardDeleteInstance(board_instance *inst, void *sqlsock) {
  void *result = NULL, *tmp = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *id = NULL, *eid = NULL;
  
  int2Str(inst->boardid, &id);
  vstrdupcat(&query, "select topicid from board_topics where boardid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {

    if (numSQLRows(result) >= 0) {
      for (i = 0; i < numSQLRows(result); i++) {
        eid = fetchSQLData(result, i, 0);
        vstrdupcat(&query, "delete from board_messages where topicid = ", eid, NULL);
        errcode = runSQL(sqlsock, &tmp, query);
        dhufree(query);
        freeSQLResult(tmp);
      }
    }
    freeSQLResult(result);
  }

  vstrdupcat(&query, "delete from board_topics where boardid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  vstrdupcat(&query, "delete from board_instance where boardid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  dhufree(id);
  return E_OK;
}

/*******************************************************************************
 * dbBoardMoveInstance...
 *
 * Called to move a board to a new location in the repository.
 ******************************************************************************/
int dbBoardMoveInstance(board_instance *inst, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL, *safepath = NULL;

  int2Str(inst->boardid, &id);
  safepath = escapeSQLString(inst->objectPath);
  vstrdupcat(&query, "update board_instance set objectPath = '", safepath, "' where boardid = ", id, NULL);
  dhufree(safepath);
  dhufree(id);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbBoardCreateTopic...
 *
 * Called to create an topic in the board. Topic show up on the board
 * as groupings of messages.
 ******************************************************************************/
int dbBoardCreateTopic(board_topic *tpc, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL, *nowstr = NULL, *safesummary = NULL, *safedesc = NULL, *authid = NULL;
  struct tm *now = NULL;
  time_t stamp;

  stamp = time(NULL);

  now = localtime(&stamp);

  int2Str(tpc->boardid, &id);
  int2Str(tpc->authorid, &authid);
  nowstr = formatDateTime(now);
  safesummary = escapeSQLString((char *) (tpc->summary?tpc->summary:""));
  safedesc = escapeSQLString((char *) (tpc->description?tpc->description:""));
  vstrdupcat(&query, "insert into board_topics (boardid, created, modified, summary, description, authorid, sticky, locked, views) values (", id, ", '", nowstr, "', '", nowstr, "', '", safesummary, "', '", safedesc, "', ", authid, ", '", (tpc->sticky?"y":"n"), "', '", (tpc->locked?"y":"n"), "', 0)", NULL);
  dhufree(id);
  dhufree(authid);
  dhufree(safesummary);
  dhufree(safedesc);
  dhufree(nowstr);
  errcode = runSQL(sqlsock, &result, query);

  getSequenceValue("board_topics_topicid_seq", &(tpc->topicid), sqlsock);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbBoardDeleteTopic...
 *
 * Called to delete a topic in the board.
 * Will delete all messages for this topic.
 ******************************************************************************/
int dbBoardDeleteTopic(board_topic *tpc, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL;
  
  int2Str(tpc->topicid, &id);
  vstrdupcat(&query, "delete from board_topics where topicid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  vstrdupcat(&query, "delete from board_messages where topicid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  dhufree(id);

  return E_OK;
}

/*******************************************************************************
 * dbBoardCreateMessage...
 *
 * Called to create a message in a topic of the board.
 ******************************************************************************/
int dbBoardCreateMessage(board_message *msg, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, 
       *id = NULL, 
       *authid = NULL, 
       *nowstr = NULL, 
       *safedescription = NULL;
  struct tm *now = NULL;
  time_t stamp;

  stamp = time(NULL);

  now = localtime(&stamp);

  int2Str(msg->topicid, &id);
  int2Str(msg->authorid, &authid);
  nowstr = formatDateTime(now);
  
  safedescription = escapeSQLString(msg->description);
  
  vstrdupcat(&query, "insert into board_messages (topicid, description, created, modified, authorid) values (", id, ", '", safedescription, "', '", nowstr, "', '", nowstr, "', ", authid, ")", NULL);
  dhufree(id);
  dhufree(nowstr);
  dhufree(safedescription);
  dhufree(authid);
  errcode = runSQL(sqlsock, &result, query);

  getSequenceValue("board_message_messageid_seq", &(msg->messageid), sqlsock);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbBoardEditMessage...
 *
 * Called to edit a message of a topic in the board.
 ******************************************************************************/
int dbBoardEditMessage(board_message *msg, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, 
       *id = NULL, 
       *nowstr = NULL, 
       *safedescription = NULL;
  struct tm *now = NULL;
  time_t stamp;

  stamp = time(NULL);

  now = localtime(&stamp);

  int2Str(msg->messageid, &id);
  nowstr = formatDateTime(now);
  
  safedescription = escapeSQLString(msg->description);
  
  vstrdupcat(&query, "update board_messages set description = '", safedescription, "', modified = '", nowstr, "' where messageid =  ", id, NULL);
  dhufree(id);
  dhufree(nowstr);
  dhufree(safedescription);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbBoardEditTopic...
 *
 * Called to edit a topic in the board.
 ******************************************************************************/
int dbBoardEditTopic(board_topic *tpc, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, 
       *id = NULL, 
       *nowstr = NULL, 
       *safesummary = NULL,
       *safedescription = NULL;
  struct tm *now = NULL;
  time_t stamp;

  stamp = time(NULL);

  now = localtime(&stamp);

  int2Str(tpc->topicid, &id);
  nowstr = formatDateTime(now);
  
  safedescription = escapeSQLString(tpc->description);
  safesummary = escapeSQLString(tpc->summary);
  
  vstrdupcat(&query, "update board_topics set description = '", safedescription, "', summary = '", safesummary, "', modified = '", nowstr, "', sticky = '", (tpc->sticky?"y":"n"), "', locked = '", (tpc->locked?"y":"n"), "' where topicid =  ", id, NULL);
  dhufree(id);
  dhufree(nowstr);
  dhufree(safedescription);
  dhufree(safesummary);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbBoardDeleteMessage...
 *
 * Called to delete a message from the board.
 ******************************************************************************/
int dbBoardDeleteMessage(board_message *msg, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL;
  
  int2Str(msg->messageid, &id);
  vstrdupcat(&query, "delete from board_messages where messageid = ", id, NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  dhufree(id);

  return E_OK;
}

/*******************************************************************************
 * dbBoardInstanceDetails...
 *
 * Called to load the details for this instance.
 ******************************************************************************/
int dbBoardInstanceDetails(board_instance *instance, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *tmp = NULL;
  
  if (instance->objectPath != NULL) {
    tmp = dhustrdup(instance->objectPath);
    vstrdupcat(&query, "select boardid, objectPath from board_instance where objectPath = '", tmp, "'", NULL);
  } else {
    int2Str(instance->boardid, &tmp);
    vstrdupcat(&query, "select boardid, objectPath from board_instance where boardid = ", tmp, NULL);
  }
  dhufree(tmp);
  errcode = runSQL(sqlsock, &result, query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }
  dhufree(query);

  if (numSQLRows(result) <= 0) {
    return NODATAFOUND;
  }
  tmp = fetchSQLData(result, 0, 0);
  instance->boardid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 1);
  if (instance->objectPath != NULL)
    dhufree(instance->objectPath);
  instance->objectPath = dhustrdup(tmp);

  freeSQLResult(result);
  
  return E_OK;
}

/*******************************************************************************
 * dbBoardTopicDetails...
 *
 * Called to load the details for this topic.
 ******************************************************************************/
int dbBoardTopicDetails(board_topic *tpc, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL, *tmp = NULL;
  
  int2Str(tpc->topicid, &id);
#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select topicid, boardid, created, modified, summary, description, locked, sticky, authorid, views from board_topics where topicid = ", id, NULL);
#else
  vstrdupcat(&query, "select topicid, boardid, DATE_FORMAT(created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(modified, '%Y-%m-%d %H:%i:%s'), summary, description, locked, sticky, authorid, views from board_topics where topicid = ", id, NULL);
#endif
  dhufree(id);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }
  
  if (numSQLRows(result) <= 0) {
    return NODATAFOUND;
  }

  tmp = fetchSQLData(result, 0, 0);
  tpc->topicid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 1);
  tpc->boardid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 2);
  dhufree(tpc->created);
  tpc->created = parseDateTime(tmp);
  tmp = fetchSQLData(result, 0, 3);
  dhufree(tpc->modified);
  tpc->modified = parseDateTime(tmp);
  tmp = fetchSQLData(result, 0, 4);
  dhufree(tpc->summary);
  tpc->summary = dhustrdup(tmp);
  tmp = fetchSQLData(result, 0, 5);
  dhufree(tpc->description);
  tpc->description = dhustrdup(tmp);
  tmp = fetchSQLData(result, 0, 6);
  tpc->locked = (*tmp == 'y');
  tmp = fetchSQLData(result, 0, 7);
  tpc->sticky = (*tmp == 'y');
  tmp = fetchSQLData(result, 0, 8);
  tpc->authorid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 9);
  tpc->views = strtol(tmp, NULL, 10);

  freeSQLResult(result);
  
  return E_OK;
}

/*******************************************************************************
 * dbBoardMessageDetails...
 *
 * Called to load the details for this message.
 ******************************************************************************/
int dbBoardMessageDetails(board_message *msg, void *sqlsock) {
  void *result = NULL;
  int errcode = 0;
  char *query = NULL, *id = NULL, *tmp = NULL;
  
  int2Str(msg->messageid, &id);
  
#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select messageid, topicid, created, modified, description, authorid from board_messages where messageid = ", id, NULL);
#else
  vstrdupcat(&query, "select messageid, topicid, DATE_FORMAT(created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(modified, '%Y-%m-%d %H:%i:%s'), description, authorid from board_messages where messageid = ", id, NULL);
#endif
  dhufree(id);
  errcode = runSQL(sqlsock, &result, query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }
  dhufree(query);
  
  if (numSQLRows(result) <= 0) {
    return NODATAFOUND;
  }

  tmp = fetchSQLData(result, 0, 0);
  msg->messageid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 1);
  msg->topicid = strtol(tmp, NULL, 10);
  tmp = fetchSQLData(result, 0, 2);
  dhufree(msg->created);
  msg->created = parseDateTime(tmp);
  tmp = fetchSQLData(result, 0, 3);
  dhufree(msg->modified);
  msg->modified = parseDateTime(tmp);
  tmp = fetchSQLData(result, 0, 4);
  dhufree(msg->description);
  msg->description = dhustrdup(tmp);
  tmp = fetchSQLData(result, 0, 5);
  msg->authorid = strtol(tmp, NULL, 10);

  freeSQLResult(result);
  
  return E_OK;
}

char *buildSearchQuery(char *terms, char *column) {
  char *start = NULL, *end = NULL, *safe = NULL, *query = NULL, *c = NULL;

  safe = escapeSQLString(terms);

  start = safe;
  end = strchr(start, ' ');
  vstrdupcat(&query, " ", NULL);

  while (start != NULL) {
    if (end) {
      *end = '\0';
    }
    c = start;
    while (*c != '\0') {
	    *c = tolower(*c);
	    c++;
    }
    if (strlen(start) > 0) {
      vstrdupcat(&query, " and lower(", column, ") like '%", start, "%' ", NULL);
    }
    if (end) {
      *end = ' ';
      end++;
    }

    start = end;
  }
  
  dhufree(safe);
  return query;
}

/*******************************************************************************
 * dbBoardSearch...
 *
 * Called to search for messages in the board.
 ******************************************************************************/
int dbBoardSearch(board_instance *inst, struct tm *start, struct tm *end, char *terms, int *count, board_message *** msglist, void *sqlsock) {
  char *startdate = NULL,
       *enddate = NULL,
       *id = NULL,
       *query = NULL,
       *where = NULL,
       *tmp = NULL;
  void *result = NULL;
  board_message **list = NULL, *msg = NULL;
  int i = 0, numresults = 0, validcount = 0, errcode = 0;

  startdate = formatDateTime(start);
  enddate = formatDateTime(end);
  int2Str(inst->boardid, &id);

  where = buildSearchQuery(terms, "t1.description");

#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select t1.messageid, t1.topicid, t1.description, t1.authorid, t1.created, t1.modified from board_messages as t1, board_topics as t2 where t1.topicid = t2.topicid and t2.boardid = ", id, " and t1.created >= '", startdate, "' and t1.created <= '", enddate, "' ", where, " order by t1.created DESC", NULL);
#else
  vstrdupcat(&query, "select t1.messageid, t1.topicid, t1.description, t1.authorid, DATE_FORMAT(t1.created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(t1.modified, '%Y-%m-%d %H:%i:%s') from board_messages as t1, board_topics as t2 where t1.topicid = t2.topicid and t2.boardid = ", id, " and t1.created >= '", startdate, "' and t1.created <= '", enddate, "' ", where, " order by t1.created DESC", NULL);
#endif
  dhufree(startdate);
  dhufree(id);
  dhufree(enddate);
  
  errcode = runSQL(sqlsock, &result, query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }
  dhufree(query);

  numresults = numSQLRows(result);
  if (numresults <= 0) {
    return NODATAFOUND;
  }

  list = (board_message **) malloc(sizeof(board_message *) * numresults);

  for (i = 0; i < numresults; i++) {
    msg = (board_message *) malloc(sizeof(board_message));
    memset(msg, 0, sizeof(board_message));

    tmp = fetchSQLData(result, i, 0);
    msg->messageid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 1);
    msg->topicid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 2);
    msg->description = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 3);
    msg->authorid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 4);
    msg->created = parseDateTime(tmp);
    tmp = fetchSQLData(result, i, 5);
    msg->modified = parseDateTime(tmp);

    list[validcount++] = msg;
  }

  if (validcount == 0) {
    dhufree(list);
    list = NULL;
    freeSQLResult(result);
    return NODATAFOUND;
  }
  *count = validcount;
  *msglist = list;
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbBoardSearchTopic...
 *
 * Called to search for messages in the board.
 ******************************************************************************/
int dbBoardSearchTopic(board_topic *topic, int *count, board_message *** msglist, void *sqlsock) {
  char *id = NULL,
       *query = NULL,
       *tmp = NULL;
  void *result = NULL;
  board_message **list = NULL, *msg = NULL;
  int i = 0, numresults = 0, validcount = 0, errcode = 0;

  int2Str(topic->topicid, &id);

#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select messageid, topicid, description, authorid, created, modified from board_messages where topicid = ", id, " order by created DESC", NULL);
#else
  vstrdupcat(&query, "select messageid, topicid, description, authorid, DATE_FORMAT(created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(modified, '%Y-%m-%d %H:%i:%s') from board_messages where topicid = ", id, " order by created DESC", NULL);
#endif
  dhufree(id);
  
  errcode = runSQL(sqlsock, &result, query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }
  dhufree(query);

  numresults = numSQLRows(result);
  if (numresults <= 0) {
    return NODATAFOUND;
  }

  list = (board_message **) malloc(sizeof(board_message *) * numresults);

  for (i = 0; i < numresults; i++) {
    msg = (board_message *) malloc(sizeof(board_message));
    memset(msg, 0, sizeof(board_message));

    tmp = fetchSQLData(result, i, 0);
    msg->messageid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 1);
    msg->topicid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 2);
    msg->description = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 3);
    msg->authorid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 4);
    msg->created = parseDateTime(tmp);
    tmp = fetchSQLData(result, i, 5);
    msg->modified = parseDateTime(tmp);

    list[validcount++] = msg;
  }

  if (validcount == 0) {
    dhufree(list);
    list = NULL;
    freeSQLResult(result);
    return NODATAFOUND;
  }
  *count = validcount;
  *msglist = list;
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbBoardListTopics...
 *
 * Called to search for topics in the board.
 ******************************************************************************/
int dbBoardListTopics(board_instance *instance, int *count, board_topic *** tpclist, void *sqlsock) {
  char *id = NULL,
       *query = NULL,
       *tmp = NULL;
  void *result = NULL;
  board_topic **list = NULL, *tpc = NULL;
  int i = 0, numresults = 0, validcount = 0, errcode = 0;

  int2Str(instance->boardid, &id);

#ifdef PGSQL_DATABASE
  vstrdupcat(&query, "select topicid, boardid, summary, description, authorid, created, modified, sticky, locked, views from board_topics where boardid = ", id, " order by sticky DESC, created DESC", NULL);
#else
  vstrdupcat(&query, "select topicid, boardid, summary, description, authorid, DATE_FORMAT(created, '%Y-%m-%d %H:%i:%s'), DATE_FORMAT(modified, '%Y-%m-%d %H:%i:%s'), sticky, locked, views from board_topics where boardid = ", id, " order by sticky ASC, created DESC", NULL);
#endif
  dhufree(id);
  
  errcode = runSQL(sqlsock, &result, query);
  if (errcode != E_OK) {
    return NODATAFOUND;
  }
  dhufree(query);

  numresults = numSQLRows(result);
  if (numresults <= 0) {
    return NODATAFOUND;
  }

  list = (board_topic **) malloc(sizeof(board_topic *) * numresults);

  for (i = 0; i < numresults; i++) {
    tpc = (board_topic *) malloc(sizeof(board_topic));
    memset(tpc, 0, sizeof(board_topic));

    tmp = fetchSQLData(result, i, 0);
    tpc->topicid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 1);
    tpc->boardid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 2);
    tpc->summary = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 3);
    tpc->description = dhustrdup(tmp);
    tmp = fetchSQLData(result, i, 4);
    tpc->authorid = strtol(tmp, NULL, 10);
    tmp = fetchSQLData(result, i, 5);
    tpc->created = parseDateTime(tmp);
    tmp = fetchSQLData(result, i, 6);
    tpc->modified = parseDateTime(tmp);
    tmp = fetchSQLData(result, i, 7);
    tpc->locked = *tmp == 'y';
    tmp = fetchSQLData(result, i, 8);
    tpc->sticky = *tmp == 'y';
    tmp = fetchSQLData(result, i, 9);
    tpc->views = strtol(tmp, NULL, 10);

    list[validcount++] = tpc;
  }

  if (validcount == 0) {
    dhufree(list);
    list = NULL;
    freeSQLResult(result);
    return NODATAFOUND;
  }
  *count = validcount;
  *tpclist = list;
  freeSQLResult(result);

  return E_OK;
}

/*******************************************************************************
 * dbBoardIncrementViews...
 *
 * Called to count a view on a topic
 ******************************************************************************/
int dbBoardIncrementViews(board_topic *topic, void *sqlsock) {
  char *id = NULL,
       *query = NULL;
  void *result = NULL;
  int errcode = 0;

  int2Str(topic->topicid, &id);
  vstrdupcat(&query, "update board_topics set views = (views + 1) where topicid = ", id, NULL);
  dhufree(id);
  
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  freeSQLResult(result);

  return E_OK;
}
