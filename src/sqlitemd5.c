#include <string.h>
#include "sqlite3.h"
#include "md5.h"

/*
** The special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
static void md5calc(sqlite3_context *context, int argc, sqlite3_value **argv){
  MD5Context p;
  unsigned char digest[16];
  char zBuf[33];
  const char *zData = NULL;
  if( argc!=1 ) {
	sqlite3_result_null(context);
	return;
  }

  MD5Init(&p);
  zData = sqlite3_value_text(argv[0]);
  if( zData ){
      MD5Update(&p, zData, strlen(zData));
  }

  MD5Final(digest,&p);
  DigestToBase16(digest, zBuf);
  sqlite3_result_text(context, zBuf, -1, SQLITE_TRANSIENT);
  
}

void sqlite3_register_md5(sqlite3 *db){
  sqlite3_create_function(db, "password", -1, SQLITE_UTF8, 0, md5calc, 0, 0);
}
