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

//#if DATABASE == MYSQLEMBEDDED
#if 0

  static char *server_args[] = {
    "dhufish.cgi",
    "--defaults-file=./my.cnf"
  }; 
  // inits the mysql environment with groups ("server", "embedded", NULL);
  if (mysql_server_init(sizeof(server_args) / sizeof(char *), server_args, NULL) == 0)
    return E_OK;
  return RESOURCEERROR;
#else
  return E_OK;
#endif
}

/*******************************************************************************
* closeDatabase...
*
* Call any database cleanup routines.
*******************************************************************************/
void closeDatabase() {
//#if DATABASE == MYSQLEMBEDDED
#if 0
  // release mysql resources
  mysql_server_end();
#endif
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
  char *dbname = NULL,
       *dbusername = NULL,
       *dbhost = NULL,
       *dbpassword = NULL;
  int retries = 0, connected = 0;
  MYSQL *dbconn = NULL;

  dbname = getDatabaseName();
  dbhost = getDatabaseHost();
  dbusername = getDatabaseUser();
  dbpassword = getDatabasePassword();
 
  dbconn = (MYSQL *) dhumalloc(sizeof(MYSQL));

  mysql_init(dbconn);

  do {
#ifdef MEDIATEMPLE
#warning Compiling with non standard mysql socket location : /var/lib/mysql/mysql.sock
    connected = (mysql_real_connect(dbconn, dbhost, dbusername, dbpassword, dbname, 3306, "/var/lib/mysql/mysql.sock", 0) != NULL);
#else
    connected = (mysql_real_connect(dbconn, dbhost, dbusername, dbpassword, dbname, 3306, "/var/run/mysqld/mysqld.sock", 0) != NULL);
#endif
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
 * getSequenceValue...
 *
 * Used to get the last insert ID.
 *******************************************************************************/
int getSequenceValue(char *seqname, int *seqid, void *sqlsock) {
  MYSQL *connection = (MYSQL *) sqlsock;

  *seqid = mysql_insert_id(connection);
  return 0;
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
  MYSQL *conn = (MYSQL *) sqlsock;
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
void freeSQLResult(void *result) {
  MYSQL_RES *myres = (MYSQL_RES *) result;
  if (myres != NULL)
    mysql_free_result(myres);
}

/*******************************************************************************
* numSQLRows...
*
* How many rows were returned?
*******************************************************************************/
int numSQLRows(void *result) {
  MYSQL_RES *myres = (MYSQL_RES *) result;
  return mysql_num_rows(myres); 
}

/*******************************************************************************
* fetchSQLData...
*
* Get the data out of the MYSQL_RES
*******************************************************************************/
char *fetchSQLData(void *result, int row, int field) {
  MYSQL_RES *myres = (MYSQL_RES *) result;

  mysql_data_seek(myres, row);

  return (char *) (mysql_fetch_row(myres)[field]);
}

char *getSQLError(void *sqlsock) {
  MYSQL *conn = (MYSQL *) sqlsock;
  return (char *)  mysql_error(conn);
}
