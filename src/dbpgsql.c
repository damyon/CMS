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
#include "libpq-fe.h"
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
#endif

/*******************************************************************************
* initDatabase...
*
* Call any database initialisation routines.
*******************************************************************************/
int initDatabase() {
  return E_OK;
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

  int2Str((int)time((time_t) NULL), &str);
  return str;
}

/*******************************************************************************
* nowStr2...
*
* Return the number of seconds since 1970 in a char *
*******************************************************************************/
char *nowStr2() {
  char *str = NULL;

  int2Str((int)time((time_t) NULL) + 1, &str);
  return str;
}


/*******************************************************************************
* getDBConnection...
*
* Connect to the database and return the connection.
*******************************************************************************/
void *getDBConnection() {
  char *dbname = NULL, *dbuser = NULL, *dbpass = NULL;
  int retries = 0, connected = 0;
  char *conninfo = NULL;
  PGconn     *dbconn;

  dbname = getDatabaseName();
  dbuser = getDatabaseUser();
  dbpass = getDatabasePassword();

  vstrdupcat(&conninfo, "dbname = '", dbname, "' user = '", dbuser, "' password = '", dbpass, "'", NULL);
  do {
    dbconn = PQconnectdb(conninfo);
    connected = (PQstatus(dbconn) == CONNECTION_OK);
  } while ((!connected) && (retries++ < 5));
  dhufree(conninfo);
  
  if (!connected) {
    logError("PGSQL Connect Error: %s\n", PQerrorMessage(dbconn));
    PQfinish(dbconn); 
    return NULL;
  }
  
  logInfo("Connected to database with settings: Database=%s, Username=%s\n", PQdb(dbconn), PQuser(dbconn));
  return dbconn;
}

/*******************************************************************************
* closeDBConnection...
*
* Close the connection to the database.
*******************************************************************************/
void closeDBConnection(void *sqlsock) {
  PGconn *connection = (PGconn *) sqlsock;

  if (connection)
    PQfinish(connection);
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
int runSQL(void *sqlsock, void **result, char *query) {
  PGconn *conn = (PGconn *) sqlsock;
  PGresult *res = NULL;

  logDebug("QUERY = (%s)\n", query);
  res = PQexec(conn, query);
  *result = res;

  logDebug("Status Code: %d\n", PQresultStatus(res));
  if (!res || (PQresultStatus(res) == PGRES_FATAL_ERROR)) {
    return SYNTAXERROR;
  }
  return 0;
}

/*******************************************************************************
* freeSQLResult...
*
* Free the data from the sql query.
*******************************************************************************/
void freeSQLResult(void *result) {
	PGresult *pgresult = (PGresult *) result;
  if (pgresult != NULL)
    PQclear(pgresult);
}

/*******************************************************************************
* numSQLRows...
*
* How many rows were returned?
*******************************************************************************/
int numSQLRows(void *result) {
	PGresult *pgresult = (PGresult *) result;
  return PQntuples(pgresult); 
}

/*******************************************************************************
* fetchSQLData...
*
* Get the data out of the result
*******************************************************************************/
char *fetchSQLData(void *result, int row, int field) {
	PGresult *pgresult = (PGresult *) result;

  return (char *) PQgetvalue(pgresult, row, field);
}

char *getSQLError(void *sqlsock) {
  PGconn *connection = (PGconn *) sqlsock;
  return PQerrorMessage(connection);
}

/*******************************************************************************
 * getSequenceValue...
 *
 * Used to get the last insert ID.
 *******************************************************************************/
int getSequenceValue(char *seqname, int *seqid, void *sqlsock) {
  void *result = NULL;
  char *query = NULL;
  int errcode = 0;

  *seqid = -1;

  vstrdupcat(&query, "select currval('", seqname, "');", NULL);
  errcode = runSQL(sqlsock, &result, query);
  dhufree(query);
  if (errcode != 0) {
    logError("Query Error: %s\n", getSQLError(sqlsock));
    return errcode;
  }

  if (numSQLRows(result) < 1)
    return NODATAFOUND;

  *seqid = strtol(fetchSQLData(result, 0, 0), NULL, 10);
  freeSQLResult(result);

  return E_OK;
}

