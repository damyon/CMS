/*******************************************************************************
* dbcalls...
*
* This file wraps all of the database accesses into neat function calls.
*******************************************************************************/

#ifndef _SQLITE_MD5_H
#define _SQLITE_MD5_H

/*******************************
* sqlite3_register_md5
*
* This function will register the password function with
* the sqlite database.
*************************************/
void sqlite3_register_md5(sqlite3 *db);

#endif
