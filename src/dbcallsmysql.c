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
#include "mysql.h"
#include "env.h"
#include "search.h"
#include "package.h"
#include "file.h"
#include "objects.h"
#include "users.h"
#include "dbcalls.h"
#include "malloc.h"

/*******************************************************************************
* initDatabase...
*
* Call any database initialisation routines.
*******************************************************************************/
int initDatabase() {
  /*****
  static char *server_args[] = {
    "dhufish.cgi",
    "--datadir=./dbfiles",
    "--skip-innodb",
    "--language=./"
  }; 
  // inits the mysql environment with groups ("server", "embedded", NULL);
  if (mysql_server_init(sizeof(server_args) / sizeof(char *), server_args, NULL) == 0)

  **/
    return NOERROR;
  return RESOURCEERROR;
}

/*******************************************************************************
* closeDatabase...
*
* Call any database cleanup routines.
*******************************************************************************/
void closeDatabase() {
  // release mysql resources
  // mysql_server_end();
}


/*******************************************************************************
* nowStr...
*
* Return the number of seconds since 1970 in a char *
*******************************************************************************/
char *nowStr() {
  char *str = NULL;

  int2Str((int)time((time_t) NULL), &str);
  return str;
}


/*******************************************************************************
* getDBConnection...
*
* Connect to the database and return the connection.
*******************************************************************************/
void *getDBConnection() {
  char *dbname = NULL,
       *dbusername = NULL,
       *dbpassword = NULL;
  int retries = 0, connected = 0;
  MYSQL *dbconn = NULL;

  dbname = getDatabaseName();
  dbusername = getDatabaseUser();
  dbpassword = getDatabasePassword();
 
  dbconn = (MYSQL *) dhumalloc(sizeof(MYSQL));

  mysql_init(dbconn);

  do {
    connected = (mysql_real_connect(dbconn, NULL, dbusername, dbpassword, dbname, 0, NULL, 0) != NULL);
  } while ((!connected) && (retries++ < 5));
  
  if (!connected) {
    logError("MYSQL Connect Error: %d, %s\n", mysql_errno(dbconn), mysql_error(dbconn));
    dhufree(dbconn); 
    return NULL;
  }
  
  mysql_select_db(dbconn, dbname);
  return dbconn;
}

/*******************************************************************************
* closeDBConnection...
*
* Close the connection to the database.
*******************************************************************************/
void closeDBConnection(void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;

  if (connection)
    mysql_close(connection);
  dhufree(connection);
}

/*******************************************************************************
* escapeSQLString...
*
* Escape (remove) punctuation.
*******************************************************************************/
char *escapeSQLString(char *string) {
  char *result = NULL, *sptr = NULL, *dptr = NULL;

  if (string == NULL)
    return dhustrdup("");

  result = (char *) dhumalloc(sizeof(char) * ((strlen(string) * 2) + 1));

  sptr = string;
  dptr = result;
 
  while (*sptr != CNULL) { 
    if (*sptr == '\'' || *sptr == '\\')
      *dptr++ = '\'';
    *dptr++ = *sptr++;
  }
  *dptr = CNULL;
  return result; 
}

/*******************************************************************************
* runSQL...
*
* Send the sql to the database.
*******************************************************************************/
int runSQL(MYSQL *conn, MYSQL_RES **result, char *query) {
  MYSQL_RES *res = NULL;

  logDebug("MYSQL QUERY = (%s)\n", query);
  mysql_query(conn, query);
  res = mysql_store_result(conn);
  *result = res;

  return mysql_errno(conn);
}

/*******************************************************************************
* freeSQLResult...
*
* Free the data from the sql query.
*******************************************************************************/
void freeSQLResult(MYSQL_RES *result) {
  if (result != NULL)
    mysql_free_result(result);
}

/*******************************************************************************
* numSQLRows...
*
* How many rows were returned?
*******************************************************************************/
int numSQLRows(MYSQL_RES *result) {
  return mysql_num_rows(result); 
}

/*******************************************************************************
* fetchSQLData...
*
* Get the data out of the MYSQL_RES
*******************************************************************************/
char *fetchSQLData(MYSQL_RES *result, int row, int field) {

  mysql_data_seek(result, row);

  return (char *) (mysql_fetch_row(result)[field]);
}

/*******************************************************************************
* getObjectID...
*
* Find the id of this object.
*******************************************************************************/
int getObjectID(int parentid, char *name, int timestamp, int *objid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *safename = NULL, *query = NULL, *parentstr, *objectid = NULL, *times = NULL, *deleted = NULL;
  
  *objid = 1;

  safename = escapeSQLString(name); 
  int2Str(parentid, &parentstr);
  time2Str(timestamp, &times);

  vstrdupcat(&query, "select objectID,isDeleted from Objects where parentID = ", parentstr, " and objectName = '", safename, "' and version <= ", times, " ORDER BY version DESC", NULL);
  dhufree(safename);
  dhufree(parentstr);
  dhufree(times);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
  return NOERROR;
}

/*******************************************************************************
* deleteObjectDB...
*
* Insert a dummy deleted record in the object table.
*******************************************************************************/
int deleteObjectDB(ObjectDetails *details, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int newid = 0, errcode = 0;
  char *query = NULL, *safename = NULL, *safemimetype = NULL, *objidstr = NULL, *nowstr = NULL;

  if (details == NULL)
    return RESOURCEERROR;

  safename = escapeSQLString(details->objectName);
  safemimetype = escapeSQLString(details->mimeType);
  nowstr = nowStr();
  vstrdupcat(&query, "insert into Objects set objectName = '", safename, "', ", 
                                             "parentID = ", details->parentID, ", ",
                                             "isOnline = '", details->isOnline, "', ",
                                             "isFolder = '", details->isFolder, "', ",
                                             "isPublic = '", details->isPublic, "', ",
					     "isDeleted = 'y', ",
                                             "mimeType = '", safemimetype, "', ",
                                             "version = ", nowstr, ", ",
                                             "lockedByUserID = ", details->lockedByUserID, NULL);
  dhufree(safename);
  dhufree(safemimetype);
  dhufree(nowstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);

  newid = mysql_insert_id(connection);

  vstrdupcat(&query, "create temporary table tmpA (objectID int(11), groupID int(11), mask char(3))", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);
  
  int2Str(newid, &objidstr);
  vstrdupcat(&query, "insert into tmpA select ", objidstr, ", groupID, mask from Permissions where objectID = ", details->parentID, NULL);
  dhufree(objidstr);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);
 
  vstrdupcat(&query, "insert into Permissions select * from tmpA", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);

  vstrdupcat(&query, "drop table tmpA", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);

  return NOERROR;

}

/*******************************************************************************
* insertObjectDetails...
*
* Insert a new record in the object table.
*******************************************************************************/
int insertObjectDetails(ObjectDetails *details, int *newid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *safename = NULL, *safemimetype = NULL, *objidstr = NULL, *nowstr = NULL;

  if (details == NULL)
    return RESOURCEERROR;

  safename = escapeSQLString(details->objectName);
  safemimetype = escapeSQLString(details->mimeType);
  nowstr = nowStr();
  vstrdupcat(&query, "insert into Objects set objectName = '", safename, "', ", 
                                             "parentID = ", details->parentID, ", ",
                                             "isOnline = '", details->isOnline, "', ",
                                             "isFolder = '", details->isFolder, "', ",
                                             "isPublic = '", details->isPublic, "', ",
					     "isDeleted = 'n', ",
                                             "mimeType = '", safemimetype, "', ",
                                             "version = ", nowstr, ", ",
                                             "lockedByUserID = ", details->lockedByUserID, NULL);
  dhufree(safename);
  dhufree(safemimetype);
  dhufree(nowstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);

  *newid = mysql_insert_id(connection);

  vstrdupcat(&query, "create temporary table tmpA (objectID int(11), groupID int(11), mask char(3))", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);
  
  int2Str(*newid, &objidstr);
  vstrdupcat(&query, "insert into tmpA select ", objidstr, ", groupID, mask from Permissions where objectID = ", details->parentID, NULL);
  dhufree(objidstr);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);
 
  vstrdupcat(&query, "insert into Permissions select * from tmpA", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);

  vstrdupcat(&query, "drop table tmpA", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);

  return NOERROR;
}

/*******************************************************************************
* getObjectName...
*
* Load the name of this object.
*******************************************************************************/
int getObjectParent(int objectid, char **name, int *parentid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *id = NULL, *query = NULL, *tmp = NULL;

  int2Str(objectid, &id);
  vstrdupcat(&query, "select objectName, parentID from Objects where objectID = ", id, NULL);
  dhufree(id);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  tmp = fetchSQLData(result, 0, 0);
  *name = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 1);
  *parentid = strtol(tmp?tmp:"-1", NULL, 10);
  
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* isVerifier...
*
* Is this user a verifier of this document.
*******************************************************************************/
int isVerifier(int objectid, int userid, int *verifier, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, parentid = 0;
  char *query = NULL, *uidstr = NULL, *objidstr = NULL, *tmp = NULL;
  
  *verifier = 0;

  errcode = getObjectParent(objectid, &tmp, &parentid, sqlsock);
  dhufree(tmp);
  if (errcode != NOERROR) {
    return errcode;
  }

  int2Str(parentid, &objidstr);
  int2Str(userid, &uidstr);
  vstrdupcat(&query, "select count(*) from Verifiers as t1, GroupMembers as t2 where", 
                                      " t2.groupID = t1.groupID and t2.userID = ", uidstr, 
                                      " and t1.objectID = ", objidstr, NULL);
  dhufree(objidstr);
  dhufree(uidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    if (strtol(tmp, NULL, 10) > 0) {
      *verifier = 1;
    }
  }
  freeSQLResult(result);

  
  return NOERROR;
}


/*******************************************************************************
* getObjectDetails...
*
* Find the details about this object.
*******************************************************************************/
int getObjectDetails(int objectid, ObjectDetails **details, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, parentID = 0, size = 0;
  ObjectDetails *obj = NULL;
  char *query = NULL, *objectstr = NULL, *tmp = NULL, *path = NULL, *name = NULL, *filepath = NULL;
  
  int2Str(objectid, &objectstr);
  vstrdupcat(&query, "select objectID,objectName,parentID,isOnline,isFolder,isPublic,mimeType,version,lockedByUserID from Objects where objectID = ", objectstr, NULL);
  dhufree(objectstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
 
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  obj = initObjectDetails();
  tmp = fetchSQLData(result, 0, 0);
  obj->objectID = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 1);
  obj->objectName = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 2);
  obj->parentID = dhustrdup(tmp?tmp:"");
  path = dhustrdup(obj->objectName);
  parentID = strtol(obj->parentID, NULL, 10);
  while (getObjectParent(parentID, &name, &parentID, sqlsock) == NOERROR) {
    tmp = NULL;
    vstrdupcat(&tmp, name, "/", path, NULL);
    path = tmp;
  }
  obj->path = path;
  tmp = fetchSQLData(result, 0, 3);
  obj->isOnline = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 4);
  obj->isFolder = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 5);
  obj->isPublic = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 6);
  obj->mimeType = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 7);
  obj->version = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 8);
  obj->lockedByUserID = dhustrdup(tmp?tmp:"");
  
  freeSQLResult(result);

  // Get the file size
  if ((errcode = generateFilePath(objectid, &filepath)) != NOERROR)
    return errcode;

  size = getFileSize(filepath);
  int2Str(size, &(obj->fileSize));

  *details = obj;
  return NOERROR;
}

/*******************************************************************************
* isUserOnline...
*
* Is this user online?
*******************************************************************************/
int isUserOnline(int userid, int *isonline, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char uidstr[128], *query = NULL, *online = NULL;
  int errcode = 0;
  
  sprintf(uidstr, "%d", userid);
  vstrdupcat(&query, "select isOnline from Users where userID = ", uidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  online = fetchSQLData(result, 0, 0); 

  *isonline = 0;
  if ((online != NULL) && (*online == 'y'))
    *isonline = 1;

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* isObjectOnline...
*
* Is this object online?
*******************************************************************************/
int isObjectOnline(int objid, int *isonline, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char objstr[128], *query = NULL, *online = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select isOnline from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  online = fetchSQLData(result, 0, 0); 

  *isonline = 0;
  if ((online != NULL) && (*online == 'y'))
    *isonline = 1;

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* isObjectFolder...
*
* Do a sql select to see if this object is a folder.
*******************************************************************************/
int isObjectFolder(int objid, int *isfolder, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char objstr[128], *query = NULL, *folder = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select isFolder from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  folder = fetchSQLData(result, 0, 0); 

  *isfolder = 0;
  if ((folder != NULL) && (*folder == 'y'))
    *isfolder = 1;

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* isGroupPublic...
*
* Do a sql select to see if this group is public.
*******************************************************************************/
int isGroupPublic(int gid, int *ispublic, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char gidstr[128], *query = NULL, *public = NULL;
  int errcode = 0;
  
  sprintf(gidstr, "%d", gid);
  vstrdupcat(&query, "select isPublic from Groups where groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  public = fetchSQLData(result, 0, 0); 

  *ispublic = 0;
  if ((public != NULL) && (*public == 'y'))
    *ispublic = 1;

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* isObjectPublic...
*
* Do a sql select to see if this object is public.
*******************************************************************************/
int isObjectPublic(int objid, int *ispublic, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char objstr[128], *query = NULL, *public = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select isPublic from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  public = fetchSQLData(result, 0, 0); 

  *ispublic = 0;
  if ((public != NULL) && (*public == 'y'))
    *ispublic = 1;

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* getObjectMimeType...
*
* Do a sql select to get this object's mimetype.
*******************************************************************************/
int getObjectMimeType(int objectid, char **mimetype, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char objstr[128], *query = NULL, *mime = NULL;
  int errcode = 0;

  sprintf(objstr, "%d", objectid);
  vstrdupcat(&query, "select mimeType from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) { 
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  mime = fetchSQLData(result, 0, 0);

  *mimetype = dhustrdup(mime?mime:"application/unknown");

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* getUserLoginDetails...
*
* Use the database to check this username/password.
*******************************************************************************/
int getUserLoginDetails(char *username, char *password, int *userid, int *super, char **fullname, int encrypt, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *uid = NULL, *sup = NULL, *full = NULL;
  int errcode = 0;

  if (encrypt)
    vstrdupcat(&query, "select userID, isSuperUser, fullName from Users where userName = '", 
                      username, "' and password = password('", 
                      password, "') and revision = 0 and isOnline = 'y'", NULL);
  else
    vstrdupcat(&query, "select userID, isSuperUser, fullName from Users where userName = '", 
                      username, "' and password = '", 
                      password, "' and revision = 0 and isOnline = 'y'", NULL);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);

  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  uid = fetchSQLData(result, 0, 0);
  sup = fetchSQLData(result, 0, 1); 
  full = fetchSQLData(result, 0, 2);

  *userid = strtol(uid, NULL, 10);
  *super = *sup == 'y';
  *fullname = dhustrdup(full?full:"");
    
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* isUserActive...
*
* Is this user Live and not deleted?
*******************************************************************************/
int isUserActive(int userid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *uid = NULL, *iso = NULL, *isd = NULL;
  int errcode = 0;

  int2Str(userid, &uid);
  // select isOnline, isDeleted from Users where userID = 
  vstrdupcat(&query, "select isOnline, isDeleted from Users where userID = ", 
                      uid, NULL);
  
  dhufree(uid);
  errcode = runSQL(connection, &result, query);
  dhufree(query);

  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  iso = fetchSQLData(result, 0, 0);
  isd = fetchSQLData(result, 0, 1); 

  errcode = NOERROR;
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
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL;
  int errcode = 0; 

  vstrdupcat(&query, "delete from Sessions where sessionKey = '", session, "'", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  freeSQLResult(result);

  vstrdupcat(&query, "delete from SessionData where sessionKey = '", session, "'", NULL);
  errcode = runSQL(connection, &result, query);
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
int createSession(char *key, char *user, int uid, int super, char *fullname, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  MYSQL_RES *result2 = NULL;
  char *query = NULL, *uidstr = NULL, *superstr = NULL, *timeoutstr = NULL, *nowstr = nowStr(), *keystr = NULL;
  int errcode = 0, timeout = 0, i = 0;

  timeout = getSessionTimeout();

  int2Str(timeout, &timeoutstr);
  
  vstrdupcat(&query, "select sessionKey from Sessions where lastAccess <= (", nowstr, " - ", timeoutstr, ")", NULL);
  runSQL(connection, &result, query);
  dhufree(query);
  
  for (i = 0; i < numSQLRows(result); i++) {
    keystr = fetchSQLData(result, i, 0);
    vstrdupcat(&query, "delete from SessionData where sessionKey = '", keystr, "'", NULL);
    runSQL(connection, &result2, query);
    dhufree(query);
    freeSQLResult(result2);
  }  

  freeSQLResult(result);

  vstrdupcat(&query, "delete from Sessions where lastAccess < (", nowstr, " - ", timeoutstr, ")", NULL);
  runSQL(connection, &result, query);
  dhufree(query);
  freeSQLResult(result);
  dhufree(timeoutstr);
  

  int2Str(uid, &uidstr);
  int2Str(super, &superstr);

  vstrdupcat(&query, "insert into Sessions values('", key, "', ", nowstr, " , '", user, "', ", uidstr, ", ", superstr, ", '", fullname, "')", NULL);
  dhufree(uidstr);
  dhufree(superstr);
  dhufree(nowstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  freeSQLResult(result); 
  return NOERROR;
}

/*********************************************************************
* loadObjectVersionsDB...
*
* Load the list of previous versions of this object.
*********************************************************************/
int loadObjectVersionsDB(int objid, int **objids, int *numobjs, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *objidstr = NULL, *name = NULL, *parentID = NULL;
  int i = 0, errcode = 0;

  int2Str(objid, &objidstr);

  vstrdupcat(&query, "select objectName, parentID from Objects where objectID = ", objidstr, NULL);
  dhufree(objidstr);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MySQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  if (((*numobjs) = numSQLRows(result)) < 1) {
    return NODATAFOUND;
  }

name = escapeSQLString(fetchSQLData(result, 0, 0));
  parentID = fetchSQLData(result, 0, 1);
  vstrdupcat(&query, "select objectID from Objects where objectName = '", name, "' and isDeleted != 'y' and parentID = ", parentID, " ORDER BY version DESC", NULL);
  dhufree(name);
  freeSQLResult(result);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MySQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  *numobjs = numSQLRows(result);
  *objids = (int *) dhumalloc(sizeof(int) * (*numobjs));

  for (i = 0; i < (*numobjs); i++) {
    (*objids)[i] = strtol(fetchSQLData(result, i, 0), NULL, 10);
  }
  freeSQLResult(result);

  return NOERROR;
}


/*******************************************************************************
* loadWorkflowListDB...
*
* Retrieve the ids of all the objects that require verification.
*******************************************************************************/
int loadWorkflowListDB(int userid, char *filter, int timestamp, int **list, int *numobjs, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
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

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", mysql_error(connection));
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
      if (getObjectID(parentID, name, timestamp, &currentID, sqlsock) == NOERROR &&
          objectID == currentID) {
        (*list)[total++] = objectID;
      }
    }
  }
  freeSQLResult(result);
  *numobjs = total;
  
  return NOERROR;
}

/*******************************************************************************
* loadFolderContentsDB...
*
* Retrieve the ids of all the objects within this one.
*******************************************************************************/
int loadFolderContentsDB(int objid, char *filter, int timestamp, char *sort, int **list, int *numobjs, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *objidstr = NULL, *safefilter = NULL, *times = NULL, *name = NULL, *order = NULL;
  int i = 0, errcode = 0, id = 0, j = 0;

  int2Str(objid, &objidstr);
  safefilter = escapeSQLString(filter);
  time2Str(timestamp, &times);

  if (sort == NULL) {
    order = dhustrdup("ORDER BY objectName ASC");
  } else {
    if (strcasecmp(sort, "date") == 0) {
      order = dhustrdup("ORDER BY version ASC");
    } else if (strcasecmp(sort, "mimetype") == 0) {
      order = dhustrdup("ORDER BY mimeType ASC");
    } else if (strcasecmp(sort, "name") == 0) {
      order = dhustrdup("ORDER BY objectName ASC");
    } else {
      order = dhustrdup("ORDER BY objectName ASC");
    }
  }

  vstrdupcat(&query, "select distinct objectName from Objects where parentID = ", objidstr, " and version <= ", times, " and objectName like '", safefilter, "%' ", order, NULL);
  dhufree(objidstr);
  dhufree(safefilter);
  dhufree(times);
  dhufree(order);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  if (((*numobjs) = numSQLRows(result)) < 1) {
    return NODATAFOUND;
  }

  *list = (int *) dhumalloc(sizeof(int) * (*numobjs));

  j = 0;
  for (i = 0; i < (*numobjs); i++) {
    name = fetchSQLData(result, i, 0);
    if (getObjectID(objid, name, timestamp, &id, sqlsock) == NOERROR) {
      (*list)[j++] = id;
    }
  }
  freeSQLResult(result);

  *numobjs = j;
  return NOERROR;
}

/*******************************************************************************
* loadSessionData...
*
* Try to insert this session in the database.
* If the insert fails the calling function will try a different key.
*******************************************************************************/
int loadSessionData(char *key, Env *env, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *timeoutstr = NULL, *nowstr = nowStr();
  int errcode = 0, timeout = 0;

  timeout = getSessionTimeout();
  
  int2Str(timeout, &timeoutstr);

  vstrdupcat(&query, "select userName, userID, isSuperUser, fullName from Sessions where sessionKey = '", key, "' and lastAccess >= (", nowstr, " - ", timeoutstr, ")", NULL);
  dhufree(timeoutstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
  
  freeSQLResult(result);

  vstrdupcat(&query, "update Sessions set lastAccess = ", nowstr, " where sessionKey = '", key, "'", NULL);
  runSQL(connection, &result, query);
  dhufree(nowstr);
  dhufree(query);
  freeSQLResult(result);

  return NOERROR;
}

/*********************************************************************
* loadGroupMembersStack...
*
* Get all the members of this group.
*********************************************************************/
int loadGroupMembersStack(int groupid, Stack **thestack, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  Stack *stack = NULL;
  char *gstr = NULL, *query = NULL;
  int errcode = 0, i = 0, total = 0, *uid = NULL;

  int2Str(groupid, &gstr);
  vstrdupcat(&query, "select userID from GroupMembers where groupID = ", gstr, NULL);
  dhufree(gstr);
 
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
  return NOERROR;
}

/*********************************************************************
* calcIsVerified...
*
* Is this object verified? (Are there no verifiers or is the author
* the only verifier?)
*********************************************************************/
int calcIsVerified(int objid, int *isverified, int *isverifier, Env *env, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *idstr = NULL, *reqall = NULL, *uidstr = NULL;
  int errcode = 0, groupid = 0, all = 0, *uid = NULL;
  Stack *stack = NULL;

  int2Str(objid, &idstr);

  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", idstr, NULL);
  dhufree(idstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  if (numSQLRows(result) < 1) {
    *isverified = 1;
    *isverifier = 0;
    return NOERROR;
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
  return NOERROR;
}

/*******************************************************************************
* getWordID...
*
* Get the unique identifier for this word.
*******************************************************************************/
int getWordID(char *w, int *id, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *safeWord = NULL, *query = NULL, *lower = NULL;
  int errcode = 0;

  safeWord = escapeSQLString(w);
  lower = safeWord;
  do {
    *lower = tolower(*lower);
  } while (*lower++ != '\0'); 
 
  vstrdupcat(&query, "select wordID from Dictionary where wordStr = '", safeWord, "'", NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode == 0) && (numSQLRows(result) > 0)) {
    *id = strtol(fetchSQLData(result, 0, 0), NULL, 10);
    freeSQLResult(result);
    dhufree(safeWord);
    return NOERROR;
  }

  vstrdupcat(&query, "insert into Dictionary set wordStr = '", safeWord, "'", NULL);
  dhufree(safeWord);
  errcode = runSQL(connection, &result, query);
  dhufree(query);

  *id = mysql_insert_id(connection);
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* getNextVersion...
*
* What is the next version number?
*******************************************************************************/
int getNextVersion(char *objectName, char *parentID, int *version, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *safeName = NULL, *query = NULL;
  int errcode = 0;

  safeName = escapeSQLString(objectName);
  vstrdupcat(&query, "select MAX(version) from Objects where objectName = '", safeName, "' and parentID = ", parentID, NULL);
  dhufree(safeName);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  *version = strtol(fetchSQLData(result, 0, 0), NULL, 10) + 1;

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
 * getSequenceValue...
 *
 * Used to get the last insert ID.
 *******************************************************************************/
int getSequenceValue(char *seqname, int *seqid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  return mysql_insert_id(connection);
}

/*******************************************************************************
* updateVersion...
*
* Just update the version.
*******************************************************************************/
int updateVersion(int objectid, int version, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *objstr = NULL, *verstr = NULL, *query = NULL;
  int errcode = 0;

  int2Str(objectid, &objstr);
  int2Str(version, &verstr);
  vstrdupcat(&query, "update Objects set version = ", verstr, " where objectID = ", objstr, NULL);
  dhufree(objstr);
  dhufree(verstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* deleteGroup...
*
* Relieves this group from active duty!
*******************************************************************************/
int deleteGroup(int gid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *gidstr = NULL, *query = NULL;
  int errcode = 0;

  int2Str(gid, &gidstr);
  vstrdupcat(&query, "delete from Groups where groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);

  vstrdupcat(&query, "delete from GroupMembers where groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  freeSQLResult(result);
  
  vstrdupcat(&query, "delete from Permissions where groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  freeSQLResult(result);
  vstrdupcat(&query, "delete from Verifiers where groupID = ", gidstr, NULL);
  dhufree(gidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* deleteUser...
*
* Relieves this user from active duty!
*******************************************************************************/
int deleteUser(int uid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *uidstr = NULL, *revstr = NULL, *query = NULL;
  int errcode = 0, revision = 0;

  int2Str(uid, &uidstr);
  vstrdupcat(&query, "select MAX(t1.revision) from Users=t1, Users=t2 where t1.userName = t2.userName and t2.userID = ", uidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(uidstr);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
    
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* insertVerify...
*
* Insert a verify instance.
*******************************************************************************/
int insertVerify(int objectid, int userid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *objstr = NULL, *usrstr = NULL, *query = NULL;
  int errcode = 0;

  int2Str(objectid, &objstr);
  int2Str(userid, &usrstr);
 
  vstrdupcat(&query, "insert into Verifiers set userID = ", usrstr, ", objectID = ", objstr, NULL);
  dhufree(objstr);
  dhufree(usrstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* increaseWordOccurrence...
*
* Increase this occurrence in the DB.
*******************************************************************************/
int increaseWordOccurrence(int objectid, int wordid, int total, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *o = NULL, *w = NULL, *t = NULL, *query = NULL, *tmp = NULL;
  int errcode = 0, insert = 0;

  int2Str(objectid, &o);
  int2Str(wordid, &w);

  vstrdupcat(&query, "select total from Occurrence where objectID = ", o, " and wordID = ", w, NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(o);
    dhufree(w);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
    vstrdupcat(&query, "insert into Occurrence set objectID = ", o, ", wordID = ", w, ", total = ", t, NULL);
  } else {
    vstrdupcat(&query, "update Occurrence set total = ", t, " where wordID = ", w, " and objectID = ", o, NULL);
  }
  dhufree(o);
  dhufree(w);
  dhufree(t);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* refreshWordStats...
*
* Update the mean and stddev for this word.
*******************************************************************************/
int refreshWordStats(int wordid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *w = NULL, *query = NULL, average[32], stddev[32];
  int errcode = 0;
  double std = 0.0, avg = 0.0;

  int2Str(wordid, &w);
  
  vstrdupcat(&query, "select AVG(total), STD(total) from Occurrence where wordID = ", w, NULL);

  errcode = runSQL(connection, &result, query);
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
    errcode = runSQL(connection, &result, query);
    dhufree(query);
    if ((errcode != 0)) {
      logError("MYSQL Query Error: %s\n", mysql_error(connection));
      return errcode;
    }
    freeSQLResult(result);
    return NOERROR;
  } else {
    dhufree(w);
    if (errcode == 0)
      errcode = INCONSISTENTDATA;
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
}

/*******************************************************************************
* removeObjectFromSearchTables...
*
* Removes all occurences of this object from the search tables.
*******************************************************************************/
int removeObjectFromSearchTables(int objectid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *o = NULL, *query = NULL;
  int errcode = 0;

  int2Str(objectid, &o);

  vstrdupcat(&query, "delete from Occurrence where objectID = ", o, NULL);
  dhufree(o);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* getSearchScore...
*
* Return the score for a single search result.
*******************************************************************************/
int getSearchScore(int objectID, int wordID, void *sqlsock) {
  int score = 0, errcode = 0;
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *w = NULL, *o = NULL, *query = NULL;

  int2Str(wordID, &w);
  int2Str(objectID, &o);

  vstrdupcat(&query, "select (((t1.total - t2.wordMean) / (t2.wordSTD ))*20 + 50)  from Occurrence=t1, Dictionary=t2 where t1.wordID = t2.wordID and t1.wordID = ", w, " and t1.objectID = ", o, NULL);
  dhufree(w);
  dhufree(o);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    logDebug("< getSearchScore: %d\n", 0);
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
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *w = NULL, *query = NULL, *times = NULL, *objectName = NULL;

  time2Str(timestamp, &times);
  int2Str(wordID, &w);

  vstrdupcat(&query, "select t3.objectID, t3.objectName, t3.parentID from Occurrence=t1, Objects=t3 where t1.wordID = ", w, " and t1.objectID = t3.objectID and t3.version <= ", times, NULL);

  dhufree(w);
  dhufree(times);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    *numwhits = 0;
    logDebug("< getSearchHits: %d\n", 0);
    return NULL;
  }
  
  num = numSQLRows(result);
   
  hits = (SearchHit *) dhumalloc(sizeof(SearchHit) * num);
  for (i = 0; i < num; i++) {
    objectID = strtol(fetchSQLData(result, i, 0), NULL, 10);
    objectName = fetchSQLData(result, i, 1);
    parentID = strtol(fetchSQLData(result, i, 2), NULL, 10);

    if (getObjectID(parentID, objectName, timestamp, &currentID, sqlsock) == NOERROR &&
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
int createNewGroup(char *gname, int public, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *safegname = NULL, *query = NULL;
  int errcode = 0;
  
  safegname = escapeSQLString(gname);
 
  vstrdupcat(&query, "insert into Groups set groupName = '", safegname, "', isPublic = '", (public?"y":"n"), "'", NULL);

  dhufree(safegname);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return DUPLICATEGROUPNAME;
  }
  freeSQLResult(result);

  return NOERROR;
}

/*******************************************************************************
* createNewUser...
*
* insert a new user into the db.
*******************************************************************************/
int createNewUser(char *uname, char *pword, char *fname, int super, int online, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *safeuname = NULL, *safefname = NULL, *safepword = NULL, *query = NULL;
  int errcode = 0;
  
  safeuname = escapeSQLString(uname);
  safefname = escapeSQLString(fname);
  safepword = escapeSQLString(pword);
 
  vstrdupcat(&query, "insert into Users set userName = '", safeuname, "', \
                                            password = password('", safepword, "'), \
                                            fullName = '", safefname, "', \
                                            isSuperUser = '", (super?"y":"n"), "', \
                                            isOnline = '", (online?"y":"n"), "'", NULL);

  dhufree(safeuname);
  dhufree(safepword);
  dhufree(safefname);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return DUPLICATEUSERNAME;
  }
  freeSQLResult(result);

  return NOERROR;
}

/*******************************************************************************
* loadUsersGroupsDB...
*
* load a group list from the db belonging to a user.
*******************************************************************************/
int loadUsersGroupsDB(int id, char *filter, int **list, int *numgroups, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *safefilter = NULL, *idstr = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  int2Str(id, &idstr);
  vstrdupcat(&query, "select t1.groupID from GroupMembers=t1, Groups=t2 where t1.userID = ", idstr, " and t1.groupID = t2.groupID and t2.groupName like '", safefilter, "%' order by t2.groupName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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

  return NOERROR;
}

/*******************************************************************************
* loadGroupMembersDB...
*
* load a group list from the db.
*******************************************************************************/
int loadGroupMembersDB(int id, char *filter, int **list, int *numusers, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *safefilter = NULL, *idstr = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  int2Str(id, &idstr);
  vstrdupcat(&query, "select t1.userID from GroupMembers=t1, Users=t2 where t1.groupID = ", idstr, " and t1.userID = t2.userID and t2.userName like '", safefilter, "%' and t2.revision = 0 order by t2.userName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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

  return NOERROR;
}

/*******************************************************************************
* loadGroupListDB...
*
* load a group list from the db.
*******************************************************************************/
int loadGroupListDB(char *filter, int **list, int *numgroups, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *safefilter = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select groupID from Groups where groupName like '", safefilter, "%' order by groupName", NULL);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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

  return NOERROR;
}

/*******************************************************************************
* loadUserListDB...
*
* load a user list from the db.
*******************************************************************************/
int loadUserListDB(char *filter, int **list, int *numusers, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *query = NULL, *safefilter = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select userID from Users where userName like '", safefilter, "%' and revision = 0 order by userName", NULL);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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

  return NOERROR;
}

/*******************************************************************************
* getGroupDetails...
*
* Find the details about this group.
*******************************************************************************/
int getGroupDetails(int gid, GroupDetails **details, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  GroupDetails *group = NULL;
  char *query = NULL, *gidstr = NULL, *tmp = NULL;
  
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "select groupID,groupName,isPublic from Groups where groupID = ", gidstr, NULL);
  dhufree(gidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
 
  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  group = initGroupDetails();
  tmp = fetchSQLData(result, 0, 0);
  group->groupID = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 1);
  group->groupName = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 2);
  group->isPublic = *tmp == 'y';
  
  freeSQLResult(result);

  *details = group;
  return NOERROR;
}

/*******************************************************************************
* getUserDetails...
*
* Find the details about this user.
*******************************************************************************/
int getUserDetails(int uid, UserDetails **details, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  UserDetails *user = NULL;
  char *query = NULL, *uidstr = NULL, *tmp = NULL;
  
  int2Str(uid, &uidstr);
  vstrdupcat(&query, "select userID,userName,isOnline,isSuperUser,fullName from Users where userID = ", uidstr, NULL);
  dhufree(uidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
 
  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  user = initUserDetails();
  tmp = fetchSQLData(result, 0, 0);
  user->userID = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 1);
  user->userName = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 2);
  user->isOnline = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 3);
  user->isSuperUser = dhustrdup(tmp?tmp:"");
  tmp = fetchSQLData(result, 0, 4);
  user->fullName = dhustrdup(tmp?tmp:"");
  
  freeSQLResult(result);

  *details = user;
  return NOERROR;
}

/*******************************************************************************
* editGroupDetails...
*
* edit a group in the db.
*******************************************************************************/
int editGroupDetails(int gid, char *gname, int public, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *gidstr = NULL, *safegname = NULL;

  int2Str(gid, &gidstr);
  safegname = escapeSQLString(gname);
  vstrdupcat(&query, "update Groups set groupName = '", safegname, "', ", 
                                       "isPublic = '", (public?"y":"n"), "' ",
                                       "where groupID = ", gidstr, NULL);
  dhufree(gidstr);
  dhufree(safegname);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
 
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* editUserDetails...
*
* edit a user in the db.
*******************************************************************************/
int editUserDetails(int uid, char *uname, char *upass, int online, int super, char *fname, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *uidstr = NULL, *safeuname = NULL, *safefname = NULL, *safepword = NULL;

  int2Str(uid, &uidstr);
  safefname = escapeSQLString(fname);
  safeuname = escapeSQLString(uname);
  if (upass != NULL && *upass != '\0') {
    safepword = escapeSQLString(upass);
    vstrdupcat(&query, "update Users set userName = '", safeuname, "', \
                                       password = password('", safepword, "'), \
                                       isOnline = '", (online?"y":"n"), "', \
                                       isSuperUser = '", (super?"y":"n"), "', \
                                       fullName = '", safefname, "' \
                                       where userID = ", uidstr, NULL);
    dhufree(safepword);
  } else {
    vstrdupcat(&query, "update Users set userName = '", safeuname, "', \
                                       isOnline = '", (online?"y":"n"), "', \
                                       isSuperUser = '", (super?"y":"n"), "', \
                                       fullName = '", safefname, "' \
                                       where userID = ", uidstr, NULL);
  }
  dhufree(uidstr);
  dhufree(safefname);
  dhufree(safeuname);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
 
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* removeGroupMember...
*
* Remove a user from a group
*******************************************************************************/
int removeGroupMember(int gid, int uid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *uidstr = NULL, *gidstr = NULL;

  int2Str(uid, &uidstr);
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "delete from GroupMembers where userID = ", uidstr, 
                                                 " and groupID = ", gidstr, NULL);
  dhufree(uidstr);
  dhufree(gidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
 
  freeSQLResult(result);
  return NOERROR;
}

/******************************************************************************* 
* loadPermissionListDB...
*
* Load the groups attached to this item.
*******************************************************************************/
int loadPermissionListDB(int objid, char *filter, int **list, int *numgroups, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *idstr = NULL, *safefilter = NULL;

  int2Str(objid, &idstr);
  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select t1.groupID from Permissions=t1, Groups=t2 where t1.objectID = ", idstr, " and t2.groupName like '", safefilter, "%' group by t1.groupID order by t2.groupName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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

  return NOERROR;
}

/*******************************************************************************
* isObjectLockedByUser...
*
* Is this object locked from edits.
*******************************************************************************/
int isObjectLockedByUser(int objid, int uid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "select lockedByUserID from Objects where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
  return NOERROR;
}

/*******************************************************************************
* isObjectLocked...
*
* Is this object locked from edits.
*******************************************************************************/
int isObjectLocked(int objid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "select lockedByUserID from Objects where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }

  if (numSQLRows(result) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  if (strtol(fetchSQLData(result, 0, 0), NULL, 10) != -1) {
    freeSQLResult(result);
    return NOERROR;
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
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;  

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "update Objects set lockedByUserID = -1 where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);
  return NOERROR;
}


/*******************************************************************************
* lockObjectDB...
*
* Lock this object from edits.
*******************************************************************************/
int lockObjectDB(int objid, int uid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *uidstr = NULL;  

  int2Str(objid, &oidstr);
  int2Str(uid, &uidstr);
  vstrdupcat(&query, "update Objects set lockedByUserID = ", uidstr, " where objectID = ", oidstr, NULL);
  dhufree(oidstr);
  dhufree(uidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* loadPermissionMask...
*
* Load the mask for this group/object.
*******************************************************************************/
int loadPermissionMask(int objid, int gid, void *sqlsock, char **mask) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *gidstr = NULL, *tmp = NULL;

  int2Str(objid, &oidstr);
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "select mask from Permissions where objectID = ", oidstr, " and groupID = ", gidstr, NULL);
  dhufree(oidstr);
  dhufree(gidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  if ((numSQLRows(result)) < 1) {
    freeSQLResult(result);
    return NODATAFOUND;
  }

  tmp = fetchSQLData(result, 0, 0);
  *mask = dhustrdup(tmp?tmp:"---");

  freeSQLResult(result);
  return NOERROR;
}


/*******************************************************************************
* addGroupMember...
*
* Add a user to a group
*******************************************************************************/
int addGroupMember(int gid, int uid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *uidstr = NULL, *gidstr = NULL;

  int2Str(uid, &uidstr);
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "insert into GroupMembers set userID = ", uidstr, 
                                                ", groupID = ", gidstr, NULL);
  dhufree(uidstr);
  dhufree(gidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
 
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* setObjectPermission...
*
* Set this permission on this object.
*******************************************************************************/
int setObjectPermission(int objid, int gid, char *mask, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, exists = 0;
  char *query = NULL, *objidstr = NULL, *gidstr = NULL, *safemask = NULL, *tmp = NULL;

  int2Str(objid, &objidstr);
  int2Str(gid, &gidstr);
  safemask = escapeSQLString(mask);

  vstrdupcat(&query, "select count(*) from  Permissions where objectID = ", objidstr, 
                                                                         " and groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
    vstrdupcat(&query, "insert into Permissions set objectID = ", objidstr, 
                                                 ", groupID = ", gidstr,
                                                 ", mask = '", safemask, "'", NULL);
  }


  dhufree(objidstr);
  dhufree(gidstr);
  dhufree(safemask);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
 
  freeSQLResult(result);
  return NOERROR;
}


/*******************************************************************************
* removeObjectPermission...
*
* Remove this permission on this object.
*******************************************************************************/
int removeObjectPermission(int objid, int gid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *objidstr = NULL, *gidstr = NULL;

  int2Str(objid, &objidstr);
  int2Str(gid, &gidstr);

  vstrdupcat(&query, "delete from Permissions where objectID = ", objidstr, 
                                               " and groupID = ", gidstr, NULL);
  dhufree(objidstr);
  dhufree(gidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);

  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* accessCheck...
*
* Does a select to determin the users permissions on this object.
*******************************************************************************/
int accessCheck(int objid, int uid, char *mask, int *access, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, valid = 0;
  char *query = NULL, *objidstr = NULL, *uidstr = NULL, *safemask = NULL, *tmp = NULL;

  if (mask == NULL || *mask == CNULL) {
    *access = valid;
    return RESOURCEERROR;
  }
  
  int2Str(objid, &objidstr);
  int2Str(uid, &uidstr);
  safemask = escapeSQLString(mask);

  vstrdupcat(&query, "select count(*) from Permissions = t1, GroupMembers = t2, Objects = t3, Users = t4", 
                                           " where t1.mask like '", safemask,
                                           "' and t2.userID = ", uidstr,
                                           " and t2.groupID = t1.groupID and t1.objectID = ", objidstr,
                                           " and t3.objectID = t1.objectID and t2.userID = t4.userID", NULL);

  dhufree(safemask);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
    if (mask[1] != 'w') {
      isObjectPublic(objid, &valid, connection);
    }
  }

  if (valid && *mask == 'r') {
    vstrdupcat(&query, "select isOnline from Objects where objectID = ", objidstr, NULL);
    errcode = runSQL(connection, &result, query);
    dhufree(query);
    if (errcode != 0) {
      logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
        if (errcode != NOERROR) {
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
  return NOERROR; 
}

/*******************************************************************************
* copyWorkflow...
*
* Copy the workflow to the new version.
*******************************************************************************/
int copyWorkflow(int oldid, int newid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  vstrdupcat(&query, "create temporary table tmpW (groupID int(11), requiresAll enum('y','n'))", NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);

  vstrdupcat(&query, "insert into tmpW select groupID, requiresAll from Verifiers where objectID = ", oldidstr, NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into Verifiers select ", newidstr, ", groupID, requiresAll from tmpW", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpW", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  return NOERROR;
}

/*******************************************************************************
* copyPermissions...
*
* Copy the permissions to the new version.
*******************************************************************************/
int copyPermissions(int oldid, int newid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  vstrdupcat(&query, "create temporary table tmpP (groupID int(11), mask char(3))", NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);
  vstrdupcat(&query, "delete from Permissions where objectID = ", newidstr, NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 

  vstrdupcat(&query, "insert into tmpP select groupID, mask from Permissions where objectID = ", oldidstr, NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into Permissions select ", newidstr, ", groupID, mask from tmpP", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpP", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  return NOERROR;
}

/*******************************************************************************
* copyMetadata...
*
* Copy the metadata to the new version.
*******************************************************************************/
int copyMetadata(int oldid, int newid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  vstrdupcat(&query, "create temporary table tmpM (fieldName varchar(255), fieldValue varchar(255))", NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);

  vstrdupcat(&query, "insert into tmpM select fieldName, fieldValue from ObjectMetadata where objectID = ", oldidstr, NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into ObjectMetadata select ", newidstr, ", fieldName, fieldValue from tmpM", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpM", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  freeSQLResult(result); 
  return NOERROR;
}

/*******************************************************************************
* updateChildren...
*
* Does a update to the children of this object to point them to the new version.
*******************************************************************************/
int updateChildren(int oldid, int newid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);
  vstrdupcat(&query, "update Objects set parentID = ", newidstr, " where parentID = ", oldidstr, NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* setObjectMetadata...
*
* Sets this metadata field.
*******************************************************************************/
int setObjectMetadata(int objid, char *name, char *value, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *idstr = NULL, *safename = NULL, *safevalue = NULL;

  int2Str(objid, &idstr);
  safename = escapeSQLString(name);
  safevalue = escapeSQLString(value);

  vstrdupcat(&query, "delete from ObjectMetadata where objectID = ", idstr, " and fieldName = '", safename, "'", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);

  freeSQLResult(result);
  vstrdupcat(&query, "insert into ObjectMetadata set objectID = ", idstr, ", fieldName = '", safename, "', fieldValue = '", safevalue, "'", NULL);
  dhufree(idstr);
  dhufree(safename);
  dhufree(safevalue);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* getObjectMetadata...
*
* Gets this metadata field.
*******************************************************************************/
int getObjectMetadata(int objid, char *name, char **value, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *idstr = NULL, *safename = NULL, *tmp = NULL;
  
  int2Str(objid, &idstr);
  safename = escapeSQLString(name);

  vstrdupcat(&query, "select fieldValue from ObjectMetadata where objectID = ", idstr, " and fieldName = '", safename, "'", NULL);
  dhufree(idstr);
  dhufree(safename);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
  
  return NOERROR;
}

/*******************************************************************************
* getAllObjectMetadata...
*
* Gets all metadata fields.
*******************************************************************************/
int getAllObjectMetadata(int objid, char ***columns, int *numcols, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *idstr = NULL, *tmp = NULL;

  int2Str(objid, &idstr);

  vstrdupcat(&query, "select fieldName from ObjectMetadata where objectID = ", idstr, NULL);
  dhufree(idstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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

  return NOERROR;
}

/*******************************************************************************
* removeObjectMetadata...
*
* Remove this metadata field.
*******************************************************************************/
int removeObjectMetadata(int objid, char *name, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *idstr = NULL, *safename = NULL;

  int2Str(objid, &idstr);
  safename = escapeSQLString(name);

  vstrdupcat(&query, "delete from ObjectMetadata where objectID = ", idstr, " and fieldName = '", safename, "'", NULL);
  dhufree(idstr);
  dhufree(safename);

  errcode = runSQL(connection, &result, query);
  dhufree(query);

  freeSQLResult(result);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  return NOERROR;
}

/*******************************************************************************
* setSessionData...
*
* Sets this data field.
*******************************************************************************/
int setSessionData(char *key, char *name, char *value, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *safekey = NULL, *safename = NULL, *safevalue = NULL;

  if (key == NULL)
    return NODATAFOUND;

  safekey = escapeSQLString(key);
  safename = escapeSQLString(name);
  safevalue = escapeSQLString(value);

  vstrdupcat(&query, "delete from SessionData where sessionKey = '", safekey, "' and fieldName = '", safename, "'", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);

  freeSQLResult(result);
  vstrdupcat(&query, "insert into SessionData set sessionKey = '", safekey, "', fieldName = '", safename, "', fieldValue = '", safevalue, "'", NULL);
  dhufree(safekey);
  dhufree(safename);
  dhufree(safevalue);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* getSessionData...
*
* Gets this data field.
*******************************************************************************/
int getSessionData(char *key, char *name, char **value, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *safekey = NULL, *safename = NULL, *tmp = NULL;
  
  if (key == NULL)
    return NODATAFOUND;

  safekey = escapeSQLString(key);
  safename = escapeSQLString(name);

  vstrdupcat(&query, "select fieldValue from SessionData where sessionKey = '", safekey, "' and fieldName = '", safename, "'", NULL);
  dhufree(safekey);
  dhufree(safename);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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
  
  return NOERROR;
}

/*******************************************************************************
* getAllSessionData...
*
* Gets all data fields.
*******************************************************************************/
int getAllSessionData(char *key, char ***columns, int *numcols, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *safekey = NULL, *tmp = NULL;

  if (key == NULL)
    return NODATAFOUND;

  safekey = escapeSQLString(key);

  vstrdupcat(&query, "select fieldName from SessionData where sessionKey = '", safekey, "'", NULL);
  dhufree(safekey);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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

  return NOERROR;
}

/*******************************************************************************
* removeSessionData...
*
* Remove this data field.
*******************************************************************************/
int removeSessionData(char *key, char *name, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *safekey = NULL, *safename = NULL;

  if (key == NULL)
    return NODATAFOUND;

  safekey = escapeSQLString(key);
  safename = escapeSQLString(name);

  vstrdupcat(&query, "delete from SessionData where sessionKey = '", safekey, "' and fieldName = '", safename, "'", NULL);
  dhufree(safename);
  dhufree(safekey);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);

  freeSQLResult(result);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  return NOERROR;
}

/*******************************************************************************
* loadWorkflowSettings...
*
* Load the workflow details for this doc.
*******************************************************************************/
int loadWorkflowSettings(int objid, int *groupid, int *all, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL, *tmp = NULL;

  int2Str(objid, &objstr);
  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", objstr, NULL);
  dhufree(objstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);

  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
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

  dhufree(result);
  return (*groupid >= 0)?NOERROR:NODATAFOUND;
}

/*******************************************************************************
* removeWorkflowSettings...
*
* Remove the workflow details from this doc.
*******************************************************************************/
int removeWorkflowSettings(int objid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL;

  int2Str(objid, &objstr);
  vstrdupcat(&query, "delete from Verifiers where objectID = ", objstr, NULL);
  dhufree(objstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    dhufree(result);
    return errcode;
  }
  dhufree(result);
  return NOERROR;
}

/*******************************************************************************
* saveWorkflowSettings...
*
* Save the workflow details for this doc.
*******************************************************************************/
int saveWorkflowSettings(int objid, int groupid, int all, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL, *gidstr = NULL;

  int2Str(objid, &objstr);
  int2Str(groupid, &gidstr);
  vstrdupcat(&query, "delete from Verifiers where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);

  vstrdupcat(&query, "insert into Verifiers set objectID = ", objstr, ", ",
                                              "groupID = ", gidstr, ", ",
                                              "requiresAll = '", all?"y":"n", "'", NULL);
  dhufree(objstr);
  dhufree(gidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    dhufree(result);
    return errcode;
  }

  dhufree(result);
  return NOERROR;
}

/*******************************************************************************
* isVerified...
*
* Is this document verified?
*******************************************************************************/
int isVerified(int parentID, int objectID, int *verified, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, count = 0;
  char *query = NULL, *pidstr = NULL, *oidstr = NULL, *gidstr = NULL, *reqall = NULL, *tmp = NULL;

  *verified = 0;
  int2Str(parentID, &pidstr);
  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", pidstr, NULL);
  dhufree(pidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    dhufree(result);
    return errcode;
  }

  if (numSQLRows(result) < 1) {
    *verified = 1;
    dhufree(result);
    return NOERROR; 
  }

  gidstr = fetchSQLData(result, 0, 0);
  reqall = fetchSQLData(result, 0, 1);
  
  int2Str(objectID, &oidstr);
  vstrdupcat(&query, "select COUNT(userID) from VerifierInstance where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    dhufree(result);
    return errcode;
  }
  
  tmp = fetchSQLData(result, 0, 0);
  count = strtol(tmp?tmp:"-1", NULL, 10);
  dhufree(result);

  if (tolower(*reqall) == 'y') {
    vstrdupcat(&query, "select COUNT(userID) from GroupMembers where groupID = ", gidstr, NULL);
    errcode = runSQL(connection, &result, query);
    dhufree(query);
    if (errcode != NOERROR) {
      logError("MYSQL Query Error: %s\n", mysql_error(connection));
      dhufree(result);
      return errcode;
    }
    
    tmp = fetchSQLData(result, 0, 0);
    if (count >= strtol(tmp?tmp:"-1", NULL, 10)) {
      *verified = 1;
      dhufree(result);
      return NOERROR;
    }
    dhufree(result);
  } else {
    if (count > 0) {
      *verified = 1;
      dhufree(result);
      return NOERROR; 
    }
  }
  return NOERROR;
}

/*******************************************************************************
* updateOnline...
*
* Set the online flag for this document.
*******************************************************************************/
int updateOnline(int objid, int online, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "update Objects set isOnline = '", online?"y":"n", "' where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  dhufree(result);

  return NOERROR;
}

/*******************************************************************************
* approveObject...
*
* Approve this document and if last verifier set document to online.
*******************************************************************************/
int approveObject(int objid, int uid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0, verified = 0, parentID = 0;
  char *query = NULL, *oidstr = NULL, *uidstr = NULL, *tmp = NULL;

  int2Str(objid, &oidstr);
  int2Str(uid, &uidstr);

  vstrdupcat(&query, "insert into VerifierInstance set objectID = ", oidstr, ", userID = ", uidstr, NULL);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    dhufree(oidstr);
    dhufree(uidstr);
    dhufree(result);
    return errcode;
  }
  dhufree(result);
  
  vstrdupcat(&query, "select parentID from Objects where objectID = ", oidstr, NULL);
  dhufree(oidstr);
  dhufree(uidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    dhufree(result);
    return errcode;
  }

  if (numSQLRows(result) < 0) {
    dhufree(result);
    return NODATAFOUND;
  }

  tmp = fetchSQLData(result, 0, 0);
  parentID = strtol(tmp?tmp:"-1", NULL, 10);
  dhufree(result);

  if ((errcode = isVerified(parentID, objid, &verified, sqlsock)) != NOERROR) {
    return errcode;
  }

  if (verified) {
    if ((errcode = updateOnline(objid, 1, sqlsock)) != NOERROR) {
      return errcode;
    }
  }

  return NOERROR;
}

/*******************************************************************************
* moveObject...
*
* Move this object to a new folder.
*******************************************************************************/
int moveObjectDB(int objid, int folderid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *fidstr = NULL, *tmp = NULL;

  int2Str(objid, &oidstr);
  int2Str(folderid, &fidstr);

  if (folderid != -1) {
    vstrdupcat(&query, "select isFolder from Objects where objectID = ", fidstr, NULL);

    errcode = runSQL(connection, &result, query);
    dhufree(query);
    if (errcode != NOERROR) {
      logError("MYSQL Query Error: %s\n", mysql_error(connection));
      dhufree(result);
      dhufree(oidstr);
      dhufree(fidstr);
      return errcode;
    }
  
    if (numSQLRows(result) < 0) {
      dhufree(result);
      dhufree(oidstr);
      dhufree(fidstr);
      return NODATAFOUND;
    }
  
    tmp = fetchSQLData(result, 0, 0);
    
    if (tolower(*tmp) != 'y') {
      dhufree(result);
      return INVALIDXPATH;
    }
    dhufree(result);

  }
  vstrdupcat(&query, "update Objects set parentID = ", fidstr, " where objectID = ", oidstr, NULL);
  dhufree(oidstr);
  dhufree(fidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MYSQL Query Error: %s\n", mysql_error(connection));
    dhufree(result);
    return errcode;
  }
  dhufree(result);

  return NOERROR;
}

/*******************************************************************************
* updateObjectDB...
*
* Update the object in the database.
*******************************************************************************/
int updateObjectDB(int objectID, char *name, int ispublic, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *safename = NULL;


  safename = escapeSQLString(name);
  int2Str(objectID, &oidstr);
  vstrdupcat(&query, "update Objects set isPublic = '", ispublic?"y":"n", "',objectName = '", safename, "' where objectID = ", oidstr, NULL);
  dhufree(oidstr);
  dhufree(safename);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("MySQL Query Error: %s\n", mysql_error(connection));
    return errcode;
  }
  dhufree(result);

  return NOERROR;
}

int userLicenseCheck(void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;
  MYSQL_RES *result = NULL;
  char *tmp = NULL;
  int userCount = 0, errcode = 0;

  errcode = runSQL(connection, &result, "select count(*) from Users where revision = 0;");
  if (errcode != 0) {
    logError("Query Error: %s\n", mysql_error(connection));
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

  return NOERROR;
}

