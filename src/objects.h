/************************************************************************
* objects.h
*
* Object Management functions.
************************************************************************/
#ifndef _OBJECTS_H
#define _OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ObjectDetails_ {
  char *objectID,
       *objectName,
       *path,
       *parentID,
       *isOnline,
       *type,
       *isPublic,
       *mimeType,
       *version,
       *readPermission,
       *writePermission,
       *executePermission,
       *lockedByUserID,
       *publisherUserID,
       *fileSize,
       *tplt,
       *relativeOrder;
} ObjectDetails;

typedef struct _VerifierComment_ {
	int objectID, userID, commentID;
	char *comment;
	struct tm *created;
} VerifierComment;



/*********************************************************************
* isValidFilename...
*
* Is this a valid filename. We enforce this because it makes it easier
* to construct urls to files in the repository.
*********************************************************************/
int isValidFilename(char *name);

/*******************************************************************************
* moveObject...
*
* Move this object to a new location.
*******************************************************************************/
int moveObject(int objectid, int newparent, Env *env, void *sqlsock);

/*******************************************************************************
* editObjectDetails...
*
* Edit the details about this object.
*******************************************************************************/
int editObjectDetails(int objectid, char *title, int ispublic, int relativeOrder, Env *env, void *sqlsock);

/*********************************************************************
* rollbackObjectVersion...
*
* Revert to a previous version.
*********************************************************************/
int rollbackObjectVersion(int objid, int oldid, int index, Env *env, void *sqlsock);

/*********************************************************************
* loadObjectContent...
*
* Load the contents of this object
* into memory.
*********************************************************************/
int loadObjectContent(int objid, int *datalen, char **data);

/*********************************************************************
* loadObjectVersions...
*
* Load the list of previous versions of this object.
*********************************************************************/
int loadObjectVersions(int objid, int min, int max, void *sqlsock, Env *env, int **objids, int *numobjs);

/*********************************************************************
* loadDeletedFolderContents...
*
* Load the contents of this folder (files that are deleted)
* into an ObjectDetailsList struct.
*********************************************************************/
int loadDeletedFolderContents(int objid, char *filter, int min, int max, char *sort, void *sqlsock, Env *env, int **objids, int *numobjs);

/*********************************************************************
* loadFolderContents...
*
* Load the contents of this folder (only that the user can see)
* into an ObjectDetailsList struct.
*********************************************************************/
int loadFolderContents(int objid, char *filter, int min, int max, char *sort, void *sqlsock, Env *env, int **objs, int *numobjs);

/*********************************************************************
* searchForContent...
*
* Search the database for documents.
*********************************************************************/
int searchForContent(char *query, int min, int max, int parentid, void *sqlsock, Env *env, int **objids, int **thescores, int *numobjs);

/*********************************************************************
* initObjectDetails...
*
* Malloc the memory for this struct.
*********************************************************************/
ObjectDetails *initObjectDetails(void);

/*********************************************************************
* initVerifierComment...
*
* Malloc the memory for this struct.
*********************************************************************/
VerifierComment *initVerifierComment(void);

/*********************************************************************
* freeVerifierComment...
*
* free this struct
*********************************************************************/
void freeVerifierComment(VerifierComment *obj);

/*********************************************************************
* getObjectPermissions...
*
* fill out the permissions section of this struct.
*********************************************************************/
int getObjectPermissions(ObjectDetails *details, Env *env, void *sqlsock);

/*********************************************************************
* freeObjectDetails...
*
* free this struct
*********************************************************************/
void freeObjectDetails(ObjectDetails *obj);

/*********************************************************************
* replaceObjectContents...
*
* Replace the file on disk.
*********************************************************************/
int replaceObjectContents(int objid, int index, char *tplt, FileObject *file, Env *env, void *sqlsock);

/*********************************************************************
* createNewObject...
*
* Create a new repository item.
*********************************************************************/
int createNewObject(int parentid, char *title, int ispublic, char * type, int indexObject, char *tplt, int relativeOrder, FileObject *file, Env *env, void *sqlsock);

/*********************************************************************
* deleteObject...
*
* Delete repository item.
*********************************************************************/
int deleteObject(int objectid, Env *env, void *sqlsock);

/*********************************************************************
* loadPermissionList...
* 
* Load the groups attached to this object.
*********************************************************************/
int loadPermissionList(int objid, char *filter, int min, int max, void *sqlsock, int **gids, int *numg);

/*********************************************************************
* loadWorkflowList...
*
* Load all the workflow documents and reduce them to the min and max.
*********************************************************************/
int loadWorkflowList(int userid, char *filter, 
                     int min, int max, 
                     void *sqlsock, Env *env, 
                     int **objids, int *numobjs);

/*********************************************************************
* createNewObject...
*
* Create a new repository item.
*********************************************************************/
int importPackageObject(int parentid, Package *p, Env *env, void *sqlsock);

/*********************************************************************
* sendWorkflowNotification...
*
* Send emails notifying users of a document published to workflow.
*********************************************************************/
int sendWorkflowNotification(int parentid, int objectid, void *sqlsock);

/*********************************************************************
* sendUpdateNotification...
*
* Send emails notifying users of a document published.
*********************************************************************/
int sendUpdateNotification(int parentid, int objectid, void *sqlsock);

/*
 * addVerifierComment...
 *
 * This function will add a comment to the database and email all the people
 * involved in the workflow.
 */
int addVerifierComment(int objid, char *comment, Env *env, void *sqlsock);

/*
 * copyObject
 *
 * Copy object from position a to position b.
 */
int copyObject(int srcid, int destid, int depth, Env *env, void *sqlsock);

#ifdef __cplusplus
}
#endif


#endif // _OBJECTS_H
