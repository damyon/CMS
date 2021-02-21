#ifndef _REQUEST_H
#define _REQUEST_H

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif



#define AUTHSTR "CMS INIT"
#define DAVSTR "DAV MTHD"
#define STDSTR "STD MTHD"
#define XREFTOK "XREF"
#define XMETHODTOK "XMETHOD"
#define THISTOK "THIS"
#define ISOTIMETOK "ISOTIME"
#define CTIMETOK "CTIME"

typedef enum _RequestTypes_ {STANDARDREQUEST, WEBDAVREQUEST} RequestType;

/*************************************************************
* authenticateConnection...
*
* Perform a handshake validation.
*************************************************************/
int authenticateConnection(int sockfd, int *type);

/*************************************************************
* handleRequest...
*
* Retrieve the document and write the response back down the socket.
*************************************************************/
int handleRequest(int sockfd);

/*************************************************************
* loadXRef...
*
* Load the file from disk but don't parse it.
*************************************************************/
int loadXRef(int objectid, char **contents, int *len, ParserInfo *info);

/*************************************************************
* isXRefValid...
*
* Is this a valid XRef?
*************************************************************/
int isXRefValid(char *xref, int timestamp, void *dbconn, Env *env, int *objectid);

/*************************************************************
* isDeletedXRefValid...
*
* Is this a valid DeletedXRef?
*************************************************************/
int isDeletedXRefValid(char *xref, int timestamp, void *dbconn, Env *env, int *objectid);

/*************************************************************
* isXRefPublic...
*
* Is this a public XRef?
*************************************************************/
int isXRefPublic(int objectid, void *dbconn);

/*************************************************************
* userHasAccess...
*   
* Perform a permission check.
*************************************************************/
int userHasReadAccess(int objectid, Env *env, void *dbconn);

/*************************************************************
* userHasWriteAccess...
*
* Perform a permission check.
*************************************************************/
int userHasWriteAccess(int objectid, Env *env, void *dbconn);

/*************************************************************
* userHasExecuteAccess...
*
* Perform a permission check.
*************************************************************/
int userHasExecuteAccess(int objectid, Env *env, void *dbconn);

/*************************************************************
* processRequest...
*
* Get the tokens of of the env and use them to retrieve the document.
*************************************************************/
void processRequest(Env *env, void *dbconn);

#ifdef __cplusplus
}
#endif


#endif
