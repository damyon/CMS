#include <time.h>
#include "strings.h"
#include "structs.h"
#include "errors.h"
#include "logging.h"
#include "config.h"
#include "env.h"
#include "search.h"
#include "package.h"
#include "objects.h"
#include "users.h"
#include "malloc.h"
#include "sqlite3.h"
#include "dbcalls.h"
#include "sqlitemd5.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

/*
** Used to accumulate query results by db_query()
*/
typedef struct sqlite3_result_str {
  int nRows;          /* Number of rows in azElem[] */
  int nCols;          /* Number of columns in azElem[] */
  int nAlloc;         /* Number of slots allocated for azElem[] */
  char ***azElem;      /* The result of the query */
} sqlite3_result;


/*******************************************************************************
* initDatabase...
*
* Call any database initialisation routines.
*******************************************************************************/
int initDatabase() {
    return NOERROR;
  return RESOURCEERROR;
}

/*******************************************************************************
* closeDatabase...
*
* Call any database cleanup routines.
*******************************************************************************/
void closeDatabase() {
}


/*******************************************************************************
* nowStr...
*
* Return the number of seconds since 1970 in a char *
*******************************************************************************/
char *nowStr() {
  char *str = NULL;

  time2Str((int)time((time_t) NULL), &str);
  return str;
}

/*******************************************************************************
* timestampStr...
*
* Return the number of seconds since 1970 in a char *
*******************************************************************************/
char *timestampStr() {
  char str[15];
  struct tm times;
  time_t now;

  now = time(NULL);
  localtime_r(&now, &times);

  sprintf(str, "%.4u%.2u%.2u%.2u%.2u%.2u", times.tm_year + 1900, times.tm_mon + 1, times.tm_mday, times.tm_hour, times.tm_min, times.tm_sec);
  return dhustrdup(str);
}

/*******************************************************************************
* STDCTX struct.
*
* Used to compute the standard deviation of the arguments.
*******************************************************************************/
typedef struct STDCTX {
	int nAlloc;
	double *values;
	double sum;
	int total;
} STDCTX;

/*******************************************************************************
* sqliteStandardDeviationFinalFunction...
*
* Used to calculate the standard deviation of the arguments
* once all the rows have been processed.
*******************************************************************************/
static void sqliteStandardDeviationFinalFunction(sqlite3_context *context) {
	STDCTX *p;
	double stddev = 0.0, mean = 0.0;
	int i = 0;

	p = sqlite3_aggregate_context(context, sizeof(*p));

	// Check for divide by 0
	if (p->total <= 1) {
		sqlite3_result_double(context, 0.0);	
		return;
	}

	// Get the mean
	mean = p->sum / (double)p->total;
	
	// Get the variance
	for (i = 0; i < p->total; i++) {
		stddev += (p->values[i] - mean) * (p->values[i] - mean);
	}

	// Normalize
	stddev /= (double) (p->total - 1);

	// Stddev
	stddev = sqrt(stddev);

	// Cleanup
	if (p->values)
		free(p->values);

	p->values = NULL;

	// Return the answer
	sqlite3_result_double(context, stddev);
}

/*******************************************************************************
* sqliteStandardDeviationStepXFunction...
*
* Compute the standard deviation of the arguments.
* Called once per row.
*******************************************************************************/
static void sqliteStandardDeviationStepXFunction(sqlite3_context *context, int argc, sqlite3_value **argv) {
	STDCTX *p;

	if (argc < 1)
		return;

	p = sqlite3_aggregate_context(context, sizeof(*p));
	if (p && SQLITE_NULL != sqlite3_value_type(argv[0])) {
		if (p->total >= p->nAlloc) {
			p->nAlloc = p->total * 2;
			p->values = realloc(p->values, sizeof(double) * p->nAlloc);
		}
		p->values[p->total] = sqlite3_value_double(argv[0]);
		p->sum += sqlite3_value_double(argv[0]);
		p->total++;
	}
}


/*******************************************************************************
* getDBConnection...
*
* Connect to the database and return the connection.
*******************************************************************************/
void *getDBConnection() {
  char *dbname = NULL;
  int err = 0;
  sqlite3 *dbconn = NULL;

  // This will be the filename of the database.
  dbname = getDatabaseName();
 
  // Now open the sqlite database
  err = sqlite3_open(dbname, & dbconn);
 
  if (err != SQLITE_NOERROR) {
    logError("SQLITE Connect Error: %d, %s\n", err, sqlite3_errmsg(dbconn));
    return NULL;
  }

  // Now register the required functions

  sqlite3_register_md5(dbconn);
  
  err = sqlite3_create_function(dbconn, "std", 1, SQLITE_FLOAT, NULL, NULL, sqliteStandardDeviationStepXFunction, sqliteStandardDeviationFinalFunction);
  if (err != SQLITE_NOERROR) {
    logError("SQLITE Register Function Error: %d, %s\n", err, sqlite3_errmsg(dbconn));
    return NULL;
  }

  err = sqlite3_create_function(dbconn, "std", 1, SQLITE_INTEGER, NULL, NULL, sqliteStandardDeviationStepXFunction, sqliteStandardDeviationFinalFunction);
  if (err != SQLITE_NOERROR) {
    logError("SQLITE Register Function Error: %d, %s\n", err, sqlite3_errmsg(dbconn));
    return NULL;
  }
  
  return dbconn;
}

/*******************************************************************************
* closeDBConnection...
*
* Close the connection to the database.
*******************************************************************************/
void closeDBConnection(void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;

  if (connection)
    sqlite3_close(connection);
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
* sqlStoreResult...
*
* Save each row returned by the query into a result structure.
*******************************************************************************/
static int sqlStoreResult(void *arg, int nargs, char **values, char **names) {
  int i;
  sqlite3_result *res = (sqlite3_result *) arg;

  // do we have room?
  if ((res->nRows) >= res->nAlloc ) {
    if (res->nAlloc == 0) {
      res->nAlloc = 2;
    } else {
      // double the available room.
      res->nAlloc = res->nAlloc*2 + 1;
    }

    res->azElem = dhurealloc(res->azElem, res->nAlloc*(sizeof(char **)));
  }

  if (nargs == 0) {
    return 0;
  }

  res->azElem[res->nRows] = dhumalloc(sizeof(char *) * nargs);
  for (i = 0; i < nargs; i++) {
    res->azElem[res->nRows][i] = dhustrdup(values[i]?values[i]:"");
  }

  res->nRows++;
  res->nCols = nargs;
  
  return 0;
}

/*******************************************************************************
* runSQL...
*
* Send the sql to the database.
*******************************************************************************/
int runSQL(sqlite3 *conn, sqlite3_result **result, char *query) {
  sqlite3_result *res = NULL;
  char *errmsg = NULL;
  int err = 0;

  logDebug("SQLITE QUERY = (%s)\n", query);

  res = (sqlite3_result *) dhumalloc(sizeof(sqlite3_result));
  res->nRows = 0;
  res->azElem = NULL;
  res->nAlloc = 0;
  
  // Keep trying until we get through - 
  // could try a random nanosleep, but it would be slower
  while ((err = sqlite3_exec(conn, query, sqlStoreResult, res, &errmsg)) == SQLITE_BUSY) {
    dhufree(errmsg);
  }
  
  // This is an error
  if (err != SQLITE_NOERROR) {
    logError("SQLITE QUERY ERROR = (%s) - %s\n", query, (char *)(errmsg!=NULL?errmsg:"No error message"));
    dhufree(errmsg);
    return err;
  }

  *result = res;
  return err;
}

/*******************************************************************************
* freeSQLResult...
*
* Free the data from the sql query.
*******************************************************************************/
void freeSQLResult(sqlite3_result *result) {
  int i = 0, j = 0; 
  if (result != NULL) {
    for (i = 0; i < result->nRows; i++) {
      for (j = 0; j < result->nCols; j++) {
        dhufree(result->azElem[i][j]);
      }
    }
    dhufree(result->azElem);
    dhufree(result);
  }
}

/*******************************************************************************
* numSQLRows...
*
* How many rows were returned?
*******************************************************************************/
int numSQLRows(sqlite3_result *result) {
  return (result->nRows); 
}

/*******************************************************************************
* fetchSQLData...
*
* Get the data out of the sqlite3_result
*******************************************************************************/
char *fetchSQLData(sqlite3_result *result, int row, int field) {

  if (result && row < result->nRows && field < result->nCols && row >= 0 && field >= 0) {
    return result->azElem[row][field];
  }

  return (char *) NULL;
}

/*******************************************************************************
* getObjectID...
*
* Find the id of this object.
*******************************************************************************/
int getObjectID(int parentid, char *name, int timestamp, int *objid, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *safename = NULL, *query = NULL, *parentstr, *objectid = NULL, *times = NULL, *deleted = NULL;
  
  *objid = 1;

  safename = escapeSQLString(name); 
  int2Str(parentid, &parentstr);
  time2Str(timestamp, &times);

  vstrdupcat(&query, "SELECT objectID,isDeleted FROM Objects WHERE parentID = ", parentstr, " AND objectName = '", safename, "' AND version <= ", times," ORDER BY version DESC ", NULL);
  dhufree(safename);
  dhufree(parentstr);
  dhufree(times);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
* insertObjectDetails...
*
* Insert a new record in the object table.
*******************************************************************************/
int insertObjectDetails(ObjectDetails *details, int *newid, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *safename = NULL, *safemimetype = NULL, *objidstr = NULL, *nowstr = NULL;

  if (details == NULL)
    return RESOURCEERROR;

  safename = escapeSQLString(details->objectName);
  safemimetype = escapeSQLString(details->mimeType);
  nowstr = nowStr();
  vstrdupcat(&query, "insert into Objects (objectID, objectName, parentID, isOnline, isFolder, isPublic, isDeleted, mimeType, version, lockedByUserID) values (NULL, ", 
					"'", safename, "', ", 
					details->parentID, ", ", 
					"'", details->isOnline, "', ", 
					"'", details->isFolder, "', ", 
					"'", details->isPublic, "', ",
 					"'n', ",
					"'", safemimetype, "', ", 
					nowstr, ", ", 
					details->lockedByUserID, ");", NULL);
  dhufree(safename);
  dhufree(safemimetype);
  dhufree(nowstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  
  freeSQLResult(result);

  *newid = sqlite3_last_insert_rowid(connection);

  vstrdupcat(&query, "create temporary table tmpA (objectID int(11), groupID int(11), mask char(3))", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result);
  
  int2Str(*newid, &objidstr);
  vstrdupcat(&query, "insert into tmpA select ", objidstr, ", groupID, mask from Permissions where objectID = ", details->parentID, NULL);
  dhufree(objidstr);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result);
 
  vstrdupcat(&query, "insert into Permissions select * from tmpA", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result);

  vstrdupcat(&query, "drop table tmpA", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *id = NULL, *query = NULL, *tmp = NULL;

  int2Str(objectid, &id);
  vstrdupcat(&query, "select objectName, parentID from Objects where objectID = ", id, NULL);
  dhufree(id);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
  vstrdupcat(&query, "select count(*) from Verifiers, GroupMembers where", 
                                      " GroupMembers.groupID = Verifiers.groupID and GroupMembers.userID = ", uidstr, 
                                      " and Verifiers.objectID = ", objidstr, NULL);
  dhufree(objidstr);
  dhufree(uidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0, parentID = 0;
  ObjectDetails *obj = NULL;
  char *query = NULL, *objectstr = NULL, *tmp = NULL, *path = NULL, *name = NULL;
  
  int2Str(objectid, &objectstr);
  vstrdupcat(&query, "select objectID,objectName,parentID,isOnline,isFolder,isPublic,mimeType,version,lockedByUserID from Objects where objectID = ", objectstr, NULL);
  dhufree(objectstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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

  *details = obj;
  return NOERROR;
}

/*******************************************************************************
* isUserOnline...
*
* Is this user online?
*******************************************************************************/
int isUserOnline(int userid, int *isonline, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char uidstr[128], *query = NULL, *online = NULL;
  int errcode = 0;
  
  sprintf(uidstr, "%d", userid);
  vstrdupcat(&query, "select isOnline from Users where userID = ", uidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char objstr[128], *query = NULL, *online = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select isOnline from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char objstr[128], *query = NULL, *folder = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select isFolder from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char gidstr[128], *query = NULL, *public = NULL;
  int errcode = 0;
  
  sprintf(gidstr, "%d", gid);
  vstrdupcat(&query, "select isPublic from Groups where groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char objstr[128], *query = NULL, *public = NULL;
  int errcode = 0;
  
  sprintf(objstr, "%d", objid);
  vstrdupcat(&query, "select isPublic from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char objstr[128], *query = NULL, *mime = NULL;
  int errcode = 0;

  sprintf(objstr, "%d", objectid);
  vstrdupcat(&query, "select mimeType from Objects where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) { 
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  sqlite3_result *result2 = NULL;
  char *query = NULL, *uidstr = NULL, *timeoutstr = NULL, *nowstr = nowStr(), *keystr = NULL;
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

  vstrdupcat(&query, "delete from Sessions where lastAccess <= (", nowstr, " - ", timeoutstr, ")", NULL);
  runSQL(connection, &result, query);
  dhufree(query);
  freeSQLResult(result);
  dhufree(timeoutstr);
  

  int2Str(uid, &uidstr);

  vstrdupcat(&query, "insert into Sessions values('", key, "', ", nowstr, " , '", user, "', ", uidstr, ", '", (char *)(super?"y":"n"), "', '", fullname, "')", NULL);
  dhufree(uidstr);
  dhufree(nowstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }

  freeSQLResult(result); 
  return NOERROR;
}

/*******************************************************************************
* loadWorkflowListDB...
*
* Retrieve the ids of all the objects that require verification.
*******************************************************************************/
int loadWorkflowListDB(int userid, char *filter, int **list, int *numobjs, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *query = NULL, *uidstr = NULL, *safefilter = NULL, *tmp = NULL;
  int i = 0, errcode = 0, total = 0;

  int2Str(userid, &uidstr);
  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select Objects.objectID, VerifierInstance.objectID from Objects, Verifiers, GroupMembers ", 
                                                          "LEFT JOIN VerifierInstance on ",
                                                          "VerifierInstance.objectID = Objects.objectID and ",
                                                          "VerifierInstance.userID = ", uidstr, " where ",
                                                          "Objects.parentID = Verifiers.objectID and ",
                                                          "Verifiers.groupID = GroupMembers.groupID and ",
                                                          "GroupMembers.userID = ", uidstr, " and ",
                                                          "Objects.isOnline = 'n' and ",
                                                          "Objects.objectName like '", safefilter, "%' ",
                                                          "ORDER BY objectName ASC", NULL);
  dhufree(uidstr);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }

  if (((*numobjs) = numSQLRows(result)) < 1) {
    return NODATAFOUND;
  }
  
  *list = (int *) dhumalloc(sizeof(int) * (*numobjs));

  for (i = 0; i < (*numobjs); i++) {
    tmp = fetchSQLData(result, i, 1);
    if (tmp == NULL)  
      (*list)[total++] = strtol(fetchSQLData(result, i, 0), NULL, 10);
  }
  freeSQLResult(result);
  *numobjs = total;
  
  return NOERROR;
}

/*********************************************************************
* loadObjectVersionsDB...
*
* Load the list of previous versions of this object.
*********************************************************************/
int loadObjectVersionsDB(int objid, int **objids, int *numobjs, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *query = NULL, *objidstr = NULL, *name = NULL, *parentID = NULL;
  int i = 0, errcode = 0;

  int2Str(objid, &objidstr);

  vstrdupcat(&query, "select objectName, parentID from Objects where objectID = ", objidstr, NULL);
  dhufree(objidstr);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
* loadFolderContentsDB...
*
* Retrieve the ids of all the objects within this one.
*******************************************************************************/
int loadFolderContentsDB(int objid, char *filter, int timestamp, char *sort, int **list, int *numobjs, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }

  if (((*numobjs) = numSQLRows(result)) < 1) {
    return NODATAFOUND;
  }
  
  *list = (int *) dhumalloc(sizeof(int) * (*numobjs));

  j = 0;
  for (i = 0; i < (*numobjs); i++) {
    name = fetchSQLData(result, i, 0);
    if (getObjectID(objid, name, timestamp, &id, sqlsock) == NOERROR)
      (*list)[j++] = id;
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *query = NULL, *timeoutstr = NULL, *nowstr = nowStr();
  int errcode = 0, timeout = 0;

  timeout = getSessionTimeout();
  
  int2Str(timeout, &timeoutstr);

  vstrdupcat(&query, "select userName, userID, isSuperUser, fullName from Sessions where sessionKey = '", key, "' and lastAccess >= (", nowstr, " - ", timeoutstr, ")", NULL);
  dhufree(timeoutstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  Stack *stack = NULL;
  char *gstr = NULL, *query = NULL;
  int errcode = 0, i = 0, total = 0, *uid = NULL;

  int2Str(groupid, &gstr);
  vstrdupcat(&query, "select userID from GroupMembers where groupID = ", gstr, NULL);
  dhufree(gstr);
 
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }

  stack = initStack();
  total = numSQLRows(result);
  for (i = 0; i < total; i++) {
    uid = (int *) dhumalloc(sizeof(int));
    *uid = strtol(fetchSQLData(result, i, 0), NULL, 10);
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *query = NULL, *idstr = NULL, *reqall = NULL, *uidstr = NULL;
  int errcode = 0, groupid = 0, all = 0, *uid = NULL;
  Stack *stack = NULL;

  int2Str(objid, &idstr);

  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", idstr, NULL);
  dhufree(idstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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

  vstrdupcat(&query, "insert into Dictionary (wordStr) values('", safeWord, "');", NULL);
  dhufree(safeWord);
  errcode = runSQL(connection, &result, query);
  dhufree(query);

  *id = sqlite3_last_insert_rowid(connection);
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* deleteGroup...
*
* Relieves this group from active duty!
*******************************************************************************/
int deleteGroup(int gid, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *gidstr = NULL, *query = NULL;
  int errcode = 0;

  int2Str(gid, &gidstr);
  vstrdupcat(&query, "delete from Groups where groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  
  freeSQLResult(result);

  vstrdupcat(&query, "delete from GroupMembers where groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }

  freeSQLResult(result);
  
  vstrdupcat(&query, "delete from Permissions where groupID = ", gidstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(gidstr);
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }

  freeSQLResult(result);
  vstrdupcat(&query, "delete from Verifiers where groupID = ", gidstr, NULL);
  dhufree(gidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *uidstr = NULL, *revstr = NULL, *query = NULL;
  int errcode = 0, revision = 0;

  int2Str(uid, &uidstr);
  vstrdupcat(&query, "select MAX(Users.revision) from Users where Users.userID = ", uidstr, " GROUP BY Users.userID;", NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(uidstr);
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *objstr = NULL, *usrstr = NULL, *query = NULL;
  int errcode = 0;

  int2Str(objectid, &objstr);
  int2Str(userid, &usrstr);
 
  vstrdupcat(&query, "insert into Verifiers (userID, objectID) values (", usrstr, ", ", objstr, ");", NULL);
  dhufree(objstr);
  dhufree(usrstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *o = NULL, *w = NULL, *t = NULL, *query = NULL, *tmp = NULL;
  int errcode = 0;

  int2Str(objectid, &o);
  int2Str(wordid, &w);

  vstrdupcat(&query, "select total from Occurrence where objectID = ", o, " and wordID = ", w, NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(o);
    dhufree(w);
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }

  if ((numSQLRows(result)) > 0) {
    tmp = fetchSQLData(result, 0, 0);
    total += strtol(tmp, NULL, 10);
  }
  
  int2Str(total, &t);
  freeSQLResult(result);
  
  vstrdupcat(&query, "insert into Occurrence (objectID, wordID, total) values (", o, ", ", w, ", ", t, ");", NULL);
  dhufree(o);
  dhufree(w);
  dhufree(t);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *w = NULL, *query = NULL, average[10], stddev[10];
  int errcode = 0;
  double std = 0.0, avg = 0.0;

  int2Str(wordid, &w);
  
  vstrdupcat(&query, "select AVG(total), STD(total) from Occurrence where wordID = ", w, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode == 0) && (numSQLRows(result) > 0)) {
    avg = strtod(fetchSQLData(result, 0, 0), NULL);
    std = strtod(fetchSQLData(result, 0, 1), NULL);

    freeSQLResult(result);
    sprintf(average, "%.8f", avg);
    sprintf(stddev, "%.8f", std);
    vstrdupcat(&query, "update Dictionary set wordMean = ", average, ", wordStd = ", stddev, " where wordID = ", w, NULL);
    dhufree(w);
    errcode = runSQL(connection, &result, query);
    dhufree(query);
    if ((errcode != 0)) {
      logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
      return errcode;
    }
    freeSQLResult(result);
    return NOERROR;
  } else {
    dhufree(w);
    if (errcode == 0)
      errcode = INCONSISTENTDATA;
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
}

/*******************************************************************************
* deleteObjectDB...
*
* Inserts a dummy "deleted" entry in the object table.
*******************************************************************************/
int deleteObjectDB(ObjectDetails *details, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0, newid = 0;
  char *query = NULL, *safename = NULL, *safemimetype = NULL, *objidstr = NULL, *nowstr = NULL;

  if (details == NULL)
    return RESOURCEERROR;

  safename = escapeSQLString(details->objectName);
  safemimetype = escapeSQLString(details->mimeType);
  nowstr = nowStr();
  vstrdupcat(&query, "insert into Objects (objectID, objectName, parentID, isOnline, isFolder, isPublic, isDeleted, mimeType, version, lockedByUserID) values (NULL, ", 
					"'", safename, "', ", 
					details->parentID, ", ", 
					"'", details->isOnline, "', ", 
					"'", details->isFolder, "', ", 
					"'", details->isPublic, "', ",
					"'y', ",
					"'", safemimetype, "', ", 
					nowstr, ", ", 
					details->lockedByUserID, ");", NULL);
  dhufree(safename);
  dhufree(safemimetype);
  dhufree(nowstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  
  freeSQLResult(result);

  newid = sqlite3_last_insert_rowid(connection);
  vstrdupcat(&query, "create temporary table tmpA (objectID int(11), groupID int(11), mask char(3))", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result);
  
  int2Str(newid, &objidstr);
  vstrdupcat(&query, "insert into tmpA select ", objidstr, ", groupID, mask from Permissions where objectID = ", details->parentID, NULL);
  dhufree(objidstr);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result);
 
  vstrdupcat(&query, "insert into Permissions select * from tmpA", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result);

  vstrdupcat(&query, "drop table tmpA", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result);

  return NOERROR;
  
  return NOERROR;
}

/*******************************************************************************
* removeObjectFromSearchTables...
*
* Removes all occurences of this object from the search tables.
*******************************************************************************/
int removeObjectFromSearchTables(int objectid, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *o = NULL, *query = NULL;
  int errcode = 0;

  int2Str(objectid, &o);

  vstrdupcat(&query, "delete from Occurrence where objectID = ", o, NULL);
  dhufree(o);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result);
  return NOERROR;
}

/*******************************************************************************
* getSearchHits...
*
* Return a list of search results.
*******************************************************************************/
SearchHit *getSearchHits(int wordID, int timestamp, int *numwhits, void *sqlsock) {
  SearchHit *hits = NULL;
  int num = 0, i = 0, errcode = 0;
  float total, wordMean, wordSTD;
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *w = NULL, *query = NULL, *s = NULL, *times = NULL;

  int2Str(wordID, &w);
  int2Str(timestamp, &times);

  vstrdupcat(&query, "select Occurrence.objectID, Occurrence.total, Dictionary.wordMean, Dictionary.wordSTD from Occurrence, Dictionary, Objects where Occurrence.wordID = Dictionary.wordID and Occurrence.wordID = ", w, " and Occurrence.objectID = Objects.objectID and Objects.version <= ", times, NULL);
  dhufree(w);
  dhufree(times);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    *numwhits = 0;
    return NULL;
  }
  
  num = numSQLRows(result);
   
  hits = (SearchHit *) dhumalloc(sizeof(SearchHit) * num);
  for (i = 0; i < num; i++) {
    hits[i].objectID = strtol(fetchSQLData(result, i, 0), NULL, 10);
    s = fetchSQLData(result, i, 1);
    total = strtod(s?s:"0", NULL);
    s = fetchSQLData(result, i, 2);
    wordMean = strtod(s?s:"0", NULL);
    s = fetchSQLData(result, i, 3);
    wordSTD = strtod(s?s:"0", NULL);

    if (wordSTD != 0.0) {
      hits[i].score = (((total - wordMean) / wordSTD)*30 + 50);
    } else {
      hits[i].score = 50;
    }
  }
  freeSQLResult(result);
  *numwhits = num;
  return hits;
}

/*******************************************************************************
* createNewGroup...
*
* insert a new group into the db.
*******************************************************************************/
int createNewGroup(char *gname, int public, void *sqlsock) {
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *safegname = NULL, *query = NULL;
  int errcode = 0;
  
  safegname = escapeSQLString(gname);
 
  vstrdupcat(&query, "insert into Groups (groupName, isPublic) values ('", safegname, "', '", (public?"y":"n"), "');", NULL);

  dhufree(safegname);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *safeuname = NULL, *safefname = NULL, *safepword = NULL, *query = NULL;
  int errcode = 0;
  
  safeuname = escapeSQLString(uname);
  safefname = escapeSQLString(fname);
  safepword = escapeSQLString(pword);
 
  vstrdupcat(&query, "insert into Users (userName, password, fullName, isSuperUser, isOnline) values (\
					  	'", safeuname, "', \
                                            	password('", safepword, "'), \
                                            	'", safefname, "', \
                                            	'", (super?"y":"n"), "', \
                                            	'", (online?"y":"n"), "');", NULL);

  dhufree(safeuname);
  dhufree(safepword);
  dhufree(safefname);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if ((errcode != 0)) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *query = NULL, *safefilter = NULL, *idstr = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  int2Str(id, &idstr);
  vstrdupcat(&query, "select GroupMembers.groupID from GroupMembers, Groups where GroupMembers.userID = ", idstr, " and GroupMembers.groupID = Groups.groupID and Groups.groupName like '", safefilter, "%' order by Groups.groupName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *query = NULL, *safefilter = NULL, *idstr = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  int2Str(id, &idstr);
  vstrdupcat(&query, "select GroupMembers.userID from GroupMembers, Users where GroupMembers.groupID = ", idstr, " and GroupMembers.userID = Users.userID and Users.userName like '", safefilter, "%' and Users.revision = 0 order by Users.userName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *query = NULL, *safefilter = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select groupID from Groups where groupName like '", safefilter, "%' order by groupName", NULL);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  char *query = NULL, *safefilter = NULL;
  int i = 0, errcode = 0;

  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select userID from Users where userName like '", safefilter, "%' and revision = 0 order by userName", NULL);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  GroupDetails *group = NULL;
  char *query = NULL, *gidstr = NULL, *tmp = NULL;
  
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "select groupID,groupName,isPublic from Groups where groupID = ", gidstr, NULL);
  dhufree(gidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  UserDetails *user = NULL;
  char *query = NULL, *uidstr = NULL, *tmp = NULL;
  
  int2Str(uid, &uidstr);
  vstrdupcat(&query, "select userID,userName,isOnline,isSuperUser,fullName from Users where userID = ", uidstr, NULL);
  dhufree(uidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *idstr = NULL, *safefilter = NULL;

  int2Str(objid, &idstr);
  safefilter = escapeSQLString(filter);
  vstrdupcat(&query, "select Permissions.groupID from Permissions, Groups where Permissions.objectID = ", idstr, " and Groups.groupName like '", safefilter, "%' group by Permissions.groupID order by Groups.groupName", NULL);
  dhufree(idstr);
  dhufree(safefilter);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "select lockedByUserID from Objects where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "select lockedByUserID from Objects where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;  

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "update Objects set lockedByUserID = -1 where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *uidstr = NULL, *gidstr = NULL;

  int2Str(uid, &uidstr);
  int2Str(gid, &gidstr);
  vstrdupcat(&query, "insert into GroupMembers (userID, groupID) values (", uidstr, ", ", gidstr, ");", NULL);
  dhufree(uidstr);
  dhufree(gidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
						objidstr, 
                                                 ", ", gidstr,
                                                 ", '", safemask, "');", NULL);
  }


  dhufree(objidstr);
  dhufree(gidstr);
  dhufree(safemask);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0, valid = 0;
  char *query = NULL, *objidstr = NULL, *uidstr = NULL, *safemask = NULL, *tmp = NULL;

  if (mask == NULL || *mask == CNULL) {
    *access = valid;
    return RESOURCEERROR;
  }
  
  int2Str(objid, &objidstr);
  int2Str(uid, &uidstr);
  safemask = escapeSQLString(mask);

  vstrdupcat(&query, "select count(*) from Permissions, GroupMembers, Objects, Users", 
                                           " where Permissions.mask like '", safemask,
                                           "' and GroupMembers.userID = ", uidstr,
                                           " and GroupMembers.groupID = Permissions.groupID and Permissions.objectID = ", objidstr,
                                           " and Objects.objectID = Permissions.objectID and GroupMembers.userID = Users.userID", NULL);

  dhufree(safemask);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
      logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  vstrdupcat(&query, "create temporary table tmpW (groupID int(11), requiresAll varchar(1))", NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into Verifiers select ", newidstr, ", groupID, requiresAll from tmpW", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpW", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL;

  vstrdupcat(&query, "create temporary table tmpP (groupID int(11), mask char(3))", NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result); 

  vstrdupcat(&query, "insert into tmpP select groupID, mask from Permissions where objectID = ", oldidstr, NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into Permissions select ", newidstr, ", groupID, mask from tmpP", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpP", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *oldidstr = NULL, *newidstr = NULL, *nowstr = NULL;

  vstrdupcat(&query, "create temporary table tmpM (fieldName varchar(255), fieldValue varchar(255))", NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result); 

  int2Str(oldid, &oldidstr);
  int2Str(newid, &newidstr);
  nowstr = nowStr();

  vstrdupcat(&query, "insert into tmpM select fieldName, fieldValue from ObjectMetadata where objectID = ", oldidstr, NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  dhufree(nowstr);
  if (errcode != 0) {
    dhufree(oldidstr);
    dhufree(newidstr);
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "insert into ObjectMetadata select ", newidstr, ", fieldName, fieldValue from tmpM", NULL);
  dhufree(oldidstr);
  dhufree(newidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  freeSQLResult(result); 
  
  vstrdupcat(&query, "drop table tmpM", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *idstr = NULL, *safename = NULL, *safevalue = NULL;

  int2Str(objid, &idstr);
  safename = escapeSQLString(name);
  safevalue = escapeSQLString(value);

  vstrdupcat(&query, "delete from ObjectMetadata where objectID = ", idstr, " and fieldName = '", safename, "'", NULL);
  errcode = runSQL(connection, &result, query);
  dhufree(query);

  freeSQLResult(result);
  vstrdupcat(&query, "insert into ObjectMetadata (objectID, fieldName, fieldValue) values (", idstr, ", '", safename, "', '", safevalue, "');", NULL);
  dhufree(idstr);
  dhufree(safename);
  dhufree(safevalue);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0, i = 0;
  char *query = NULL, *idstr = NULL, *tmp = NULL;

  int2Str(objid, &idstr);

  vstrdupcat(&query, "select fieldName from ObjectMetadata where objectID = ", idstr, NULL);
  dhufree(idstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
  vstrdupcat(&query, "insert into SessionData (sessionKey, fieldName, fieldValue) values ('", safekey, "', '", safename, "', '", safevalue, "');", NULL);
  dhufree(safekey);
  dhufree(safename);
  dhufree(safevalue);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }

  *numcols = numSQLRows(result);
  
  if (*numcols > 0) {
    *columns = (char **) dhumalloc(sizeof (char *) * (*numcols));
    for (i = 0; i < *numcols; i++) {
      tmp = fetchSQLData(result, i, 0);
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL, *tmp = NULL;

  int2Str(objid, &objstr);
  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", objstr, NULL);
  dhufree(objstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);

  if (errcode != NOERROR) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL;

  int2Str(objid, &objstr);
  vstrdupcat(&query, "delete from Verifiers where objectID = ", objstr, NULL);
  dhufree(objstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *objstr = NULL, *gidstr = NULL;

  int2Str(objid, &objstr);
  int2Str(groupid, &gidstr);
  vstrdupcat(&query, "delete from Verifiers where objectID = ", objstr, NULL);

  errcode = runSQL(connection, &result, query);
  dhufree(query);

  vstrdupcat(&query, "insert into Verifiers (objectID, groupID, requiresAll) values (", objstr, ", ",
                                              gidstr, ", ",
                                              "'", all?"y":"n", "');", NULL);
  dhufree(objstr);
  dhufree(gidstr);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0, count = 0;
  char *query = NULL, *pidstr = NULL, *oidstr = NULL, *gidstr = NULL, *reqall = NULL, *tmp = NULL;

  *verified = 0;
  int2Str(parentID, &pidstr);
  vstrdupcat(&query, "select groupID, requiresAll from Verifiers where objectID = ", pidstr, NULL);
  dhufree(pidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
      logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL;

  int2Str(objid, &oidstr);
  vstrdupcat(&query, "update Objects set isOnline = '", online?"y":"n", "' where objectID = ", oidstr, NULL);
  dhufree(oidstr);

  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0, verified = 0, parentID = 0;
  char *query = NULL, *oidstr = NULL, *uidstr = NULL, *tmp = NULL;

  int2Str(objid, &oidstr);
  int2Str(uid, &uidstr);

  vstrdupcat(&query, "insert into VerifierInstance (objectID, userID) values (", oidstr, ", ", uidstr, ");", NULL);
  
  errcode = runSQL(connection, &result, query);
  dhufree(query);
  if (errcode != NOERROR) {
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
  int errcode = 0;
  char *query = NULL, *oidstr = NULL, *fidstr = NULL, *tmp = NULL;

  int2Str(objid, &oidstr);
  int2Str(folderid, &fidstr);

  if (folderid != -1) {
    vstrdupcat(&query, "select isFolder from Objects where objectID = ", fidstr, NULL);

    errcode = runSQL(connection, &result, query);
    dhufree(query);
    if (errcode != NOERROR) {
      logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
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
  sqlite3 *connection = (sqlite3 *) sqlsock;
  sqlite3_result *result = NULL;
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
    logError("SQLite Query Error: %s\n", sqlite3_errmsg(connection));
    return errcode;
  }
  dhufree(result);

  return NOERROR;
}

