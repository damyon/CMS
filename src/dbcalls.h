/*******************************************************************************
* dbcalls...
*
* This file wraps all of the database accesses into neat function calls.
*******************************************************************************/

#ifndef _DBCALLS_H
#define _DBCALLS_H

#include "search.h"
#include "objects.h"
#include "groups.h"
#include "users.h"
#include "calendar.h"
#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* getObjectParent...
*
* Get the parent details (and the object name).
*******************************************************************************/
int getObjectParent(int objectid, char **name, int *parentid, void *sqlsock);

/*******************************************************************************
* initDatabase...
*
* Call any database initialisation routines.
*******************************************************************************/
int initDatabase(void);

/*******************************************************************************
* closeDatabase...
*
* Call any database cleanup routines.
*******************************************************************************/
void closeDatabase(void);

/*******************************************************************************
* getDBConnection...
*
* Connect to the database and return the connection.
*******************************************************************************/
void *getDBConnection(void);

/*******************************************************************************
* closeDBConnection...
*
* Close the connection to the database.
*******************************************************************************/
void closeDBConnection(void *sqlsock);

/*******************************************************************************
* nowStr...
*
* Return the number of seconds since 1970 in a char *
*******************************************************************************/
char *nowStr();
char *nowStr2();

/*******************************************************************************
* escapeSQLString...
*
* Escape (remove) punctuation.
*******************************************************************************/
char *escapeSQLString(char *string);

/*******************************************************************************
* runSQL...
*
* Send the sql to the database.
*******************************************************************************/
int runSQL(void *sqlsock, void **result, char *query);

/*******************************************************************************
* freeSQLResult...
*
* Free the data from the sql query.
*******************************************************************************/
void freeSQLResult(void *result);

/*******************************************************************************
* numSQLRows...
*
* How many rows were returned?
*******************************************************************************/
int numSQLRows(void *result);

/*******************************************************************************
* fetchSQLData...
*
* Get the data out of the result
*******************************************************************************/
char *fetchSQLData(void *result, int row, int field);

/*******************************************************************************
* getSQLError...
*
* Get the last sql error on this connection
*******************************************************************************/
char *getSQLError(void *sqlsock);

/*******************************************************************************
 * getSequenceValue...
 *
 * Used to get the last insert ID.
 *******************************************************************************/
int getSequenceValue(char *seqname, int *seqid, void *sqlsock);

/*******************************************************************************
* getObjectID...
*
* Get the id of the object with this name and this parent.
*******************************************************************************/
int getObjectID(int parentid, char *name, int timestamp, int *objid, void *connection);

/*******************************************************************************
* isObjectFolder...
*
* Is this object a folder?
*******************************************************************************/
int isObjectFolder(int objid, int *isfolder, void *connection);

/*******************************************************************************
* isObjectPublic...
*
* Is this object ispublic?
*******************************************************************************/
int isObjectPublic(int objid, int *ispublic, void *connection);

/*******************************************************************************
* isGroupPublic...
*
* Is this group ispublic?
*******************************************************************************/
int isGroupPublic(int gid, int *ispublic, void *connection);

/*******************************************************************************
* isObjectOnline...
*
* Is this object online?
*******************************************************************************/
int isObjectOnline(int objid, int *isonline, void *connection);

/*******************************************************************************
* getObjectMimeType...
*
* Do a sql select to get this object's mimetype.
*******************************************************************************/
int getObjectMimeType(int objectid, char **mimetype, void *sqlsock);

/*******************************************************************************
* getUserLoginDetails...
*
* Use the database to check this username/password.
*******************************************************************************/
int getUserLoginDetails(char *username, char *password, int *uid, int *super, char **fullname, char **email, char **usertype, int encrypt, void *connection);

/*******************************************************************************
* isUserActive...
*
* Is this user Live and not deleted?
*******************************************************************************/
int isUserActive(int uid, void *connection);

/*******************************************************************************
* updateObjectDB...
*
* Update the object in the database.
*******************************************************************************/
int updateObjectDB(int objectID, char *name, int ispublic, int relativeOrder, void *connection);

/*******************************************************************************
* deleteObjectDB...
*
* update the objects table and set this version to deleted.
*******************************************************************************/
int deleteObjectDB(int objectid, void *connection);

/*******************************************************************************
* createSession...
*
* Try to insert this session in the database.
* If the insert fails the calling function will try a different key.
*******************************************************************************/
int createSession(char *key, char *user, int uid, int super, char *fullname, char *usertype, void *connection);

/*******************************************************************************
* loadSessionData...
*
* Try to insert this session in the database.
* If the insert fails the calling function will try a different key.
*******************************************************************************/
int loadSessionData(char *key, Env *env, void *connection);

/*******************************************************************************
* loadWorkflowListDB...
*
* Retrieve the ids of all the objects that need approving by this user.
*******************************************************************************/
int loadWorkflowListDB(int uid, char *filter, int timestamp, int **list, int *numobjs, void *sqlsock);

/*********************************************************************
* loadObjectVersionsDB...
*
* Load the list of previous versions of this object.
*********************************************************************/
int loadObjectVersionsDB(int objid, int **objids, int *numobjs, void *sqlsock);

/*******************************************************************************
* loadFolderContents...
*
* Retrieve the ids of all the objects within this one.
*******************************************************************************/
int loadFolderContentsDB(int objid, char *filter, int timestamp, char *sort, int **list, int *numobjs, void *sqlsock);

/*******************************************************************************
* loadDeletedFolderContentsDB...
*
* Retrieve the ids of all the objects within this one that are deleted.
*******************************************************************************/
int loadDeletedFolderContentsDB(int objid, char *filter, int timestamp, char *sort, int **list, int *numobjs, void *sqlsock);


/*******************************************************************************
* getObjectDetails...
*
* Find the details about this object.
*******************************************************************************/
int getObjectDetails(int objectid, ObjectDetails **details, void *sqlsock);

/*********************************************************************
* calcIsVerified...
*
* Is this object verified? (Are there no verifiers or is the author
* the only verifier?)
*********************************************************************/
int calcIsVerified(int objid, int *isverified, int *isverifier, Env *env, void *sqlsock);

/*******************************************************************************
* insertObjectDetails...
*
* Insert a new record in the object table.
*******************************************************************************/
int insertObjectDetails(ObjectDetails *details, int *newid, void *sqlsock);

/*******************************************************************************
* insertVerify...
*
* Insert a verify instance.
*******************************************************************************/
int insertVerify(int newid, int userid, void *sqlsock);

/*******************************************************************************
* getWordID...
*
* Get the unique identifier for this word.
*******************************************************************************/
int getWordID(char *w, int *id, void *sqlsock);

/*******************************************************************************
* increaseWordOccurrence...
*
* Insert this occurrence into the DB.
*******************************************************************************/
int increaseWordOccurrence(int objectid, int wordid, int total, void *sqlsock);

/*******************************************************************************
* refreshWordStats...
*
* Update the mean and stddev for this word.
*******************************************************************************/
int refreshWordStats(int wordid, void *sqlsock);

/*******************************************************************************
* removeObjectFromSearchTables...
*
* Removes all occurences of this object from the search tables.
*******************************************************************************/
int removeObjectFromSearchTables(int objectid, void *sqlsock);

/*******************************************************************************
* getSearchHits...
*
* Return a list of search results.
*******************************************************************************/
SearchHit *getSearchHits(int wordID, int timestamp, int *numwhits, void *sqlsock);

/*******************************************************************************
* removeValidSession...
* 
* Delete this session from the db.
*******************************************************************************/
int removeValidSession(char *session, void *sqlsock);

/*******************************************************************************
* createNewGroup...
*
* insert a new group into the db.
*******************************************************************************/
int createNewGroup(char *gname, int ispublic, void *sqlsock);

/*******************************************************************************
* createNewUser...
*
* insert a new user into the db.
*******************************************************************************/
int createNewUser(char *uname, char *pword, char *fullname, int super, int online, char *email, char *usertype, void *sqlsock);

/*******************************************************************************
* isUserOnline...
* 
* Is this user online?
*******************************************************************************/
int isUserOnline(int userid, int *isonline, void *sqlsock);

/*******************************************************************************
* loadGroupListDB...
*
* load a group list from the db.
*******************************************************************************/
int loadGroupListDB(char *filter, int **list, int *numgroups, void *sqlsock);

/*******************************************************************************
* loadUserListDB...
*
* load a user list from the db.
*******************************************************************************/
int loadUserListDB(char *filter, int **list, int *numusers, void *sqlsock);

/*******************************************************************************
* loadUserPassword...
*
* Load the MD5 hash of username:realm:password
*******************************************************************************/
int loadUserPassword(char *username, int *uid, char **password, void *sqlsock);

/*******************************************************************************
* getUserDetails...
*
* load a user details from the db.
*******************************************************************************/
int getUserDetails(int uid, UserDetails **details, void *sqlsock);

/*******************************************************************************
* getGroupDetails...
*
* load a group details from the db.
*******************************************************************************/
int getGroupDetails(int gid, GroupDetails **details, void *sqlsock);

/*******************************************************************************
* deleteGroup...
*
* delete a group from the db.
*******************************************************************************/
int deleteGroup(int gid, void *sqlsock);

/*******************************************************************************
* deleteUser...
*
* delete a user from the db.
*******************************************************************************/
int deleteUser(int uid, void *sqlsock);

/*******************************************************************************
* editUserDetails...
*
* edit a user in the db.
*******************************************************************************/
int editUserDetails(int uid, char *uname, char *upass, int online, int super, char *fname, char *email, void *sqlsock);

/*******************************************************************************
* editGroupDetails...
*
* edit a group in the db.
*******************************************************************************/
int editGroupDetails(int gid, char *gname, int ispublic, void *sqlsock);

/*******************************************************************************
* loadGroupMembersDB...
*
* load a group list from the db.
*******************************************************************************/
int loadGroupMembersDB(int id, char *filter, int **list, int *numusers, void *sqlsock);

/*******************************************************************************
* loadUsersGroupsDB...
* 
* load a group list from the db belonging to a user.
*******************************************************************************/
int loadUsersGroupsDB(int id, char *filter, int **list, int *numgroups, void *sqlsock);

/*******************************************************************************
* removeGroupMember...
*
* Remove a user from a group
*******************************************************************************/
int removeGroupMember(int gid, int uid, void *sqlsock);

/*******************************************************************************
* addGroupMember...
*
* Add a user to a group
*******************************************************************************/
int addGroupMember(int gid, int uid, void *sqlsock);

/*******************************************************************************
* loadPermissionListDB...
*
* Load the groups attached to this item.
*******************************************************************************/
int loadPermissionListDB(int objid, char *filter, int **groups, int *numgroups, void *sqlsock);

/*******************************************************************************
* loadPermissionMask...
*
* Load the mask for this group/object.
*******************************************************************************/
int loadPermissionMask(int objid, int gid, void *sqlsock, char **mask);

/*******************************************************************************
* setObjectPermission...
*
* Set this permission on this object.
*******************************************************************************/
int setObjectPermission(int objid, int gid, char *mask, void *sqlsock);

/*******************************************************************************
* removeObjectPermission...
*
* Remove this permission on this object.
*******************************************************************************/
int removeObjectPermission(int objid, int gid, void *sqlsock);

/*******************************************************************************
* accessCheck...
*
* Does a select to determin the users permissions on this object.
*******************************************************************************/
int accessCheck(int objid, int uid, char *mask, int *access, void *sqlsock);

/*******************************************************************************
* updateChildren...
*
* Does a update to the children of this object to point them to the new version.
*******************************************************************************/
int updateChildren(int oldid, int newid, void *sqlsock);

/*******************************************************************************
* copyWorkflow...
* 
* Copies the workflow to the new version.
*******************************************************************************/
int copyWorkflow(int oldid, int newid, void *sqlsock);

/*******************************************************************************
* copyPermissions...
* 
* Copies the permissions to the new version.
*******************************************************************************/
int copyPermissions(int oldid, int newid, void *sqlsock);

/*******************************************************************************
* copyMetadata...
* 
* Copies the metadata to the new version.
*******************************************************************************/
int copyMetadata(int oldid, int newid, void *sqlsock);

/*******************************************************************************
* setObjectMetadata...
*
* Sets this metadata field.
*******************************************************************************/
int setObjectMetadata(int objid, char *name, char *value, void *sqlsock);

/*******************************************************************************
* getObjectMetadata...
*
* Gets this metadata field.
*******************************************************************************/
int getObjectMetadata(int objid, char *name, char **value, void *sqlsock);

/*******************************************************************************
* getAllObjectMetadata...
*
* Gets all metadata fields.
*******************************************************************************/
int getAllObjectMetadata(int objid, char ***columns, int *numcols, void *sqlsock);

/*******************************************************************************
* removeObjectMetadata...
*
* Remove this metadata field.
*******************************************************************************/
int removeObjectMetadata(int objid, char *name, void *sqlsock);

/*******************************************************************************
* setUserMetadata...
*
* Sets this metadata field.
*******************************************************************************/
int setUserMetadata(int objid, char *name, char *value, void *sqlsock);

/*******************************************************************************
* getUserMetadata...
*
* Gets this metadata field.
*******************************************************************************/
int getUserMetadata(int objid, char *name, char **value, void *sqlsock);

/*******************************************************************************
* getAllUserMetadata...
*
* Gets all metadata fields.
*******************************************************************************/
int getAllUserMetadata(int objid, char ***columns, int *numcols, void *sqlsock);

/*******************************************************************************
* removeUserMetadata...
*
* Remove this metadata field.
*******************************************************************************/
int removeUserMetadata(int objid, char *name, void *sqlsock);

/*******************************************************************************
* setSessionData...
*
* Sets this data field.
*******************************************************************************/
int setSessionData(char *key, char *name, char *value, void *sqlsock);

/*******************************************************************************
* getSessionData...
*
* Gets this data field.
*******************************************************************************/
int getSessionData(char *key, char *name, char **value, void *sqlsock);

/*******************************************************************************
* getAllSessionData...
*
* Gets all data fields.
*******************************************************************************/
int getAllSessionData(char *key, char ***columns, int *numcols, void *sqlsock);

/*******************************************************************************
* removeSessionData...
*
* Remove this data field.
*******************************************************************************/
int removeSessionData(char *key, char *name, void *sqlsock);

/*******************************************************************************
* lockObjectDB...
* 
* Lock this object from edits.
*******************************************************************************/
int lockObjectDB(int objid, int uid, void *sqlsock);

/*******************************************************************************
* isObjectLocked...
*
* Is this object locked from edits.
*******************************************************************************/
int isObjectLocked(int objid, void *sqlsock);

/*******************************************************************************
* unLockObjectDB...
* 
* UnLock this object from edits.
*******************************************************************************/
int unLockObjectDB(int objid, void *sqlsock);

/*******************************************************************************
* isObjectLockedByUser...
*
* Is this object locked from edits.
*******************************************************************************/
int isObjectLockedByUser(int objid, int uid, void *sqlsock);

/*******************************************************************************
* loadWorkflowSettings...
*
* Load the workflow details for this doc.
*******************************************************************************/
int loadWorkflowSettings(int objid, int *groupid, int *all, void *sqlsock);

/*******************************************************************************
* saveWorkflowSettings...
*
* Save the workflow details for this doc.
*******************************************************************************/
int saveWorkflowSettings(int objid, int groupid, int all, void *sqlsock);

/*******************************************************************************
* removeWorkflowSettings...
*
* Remove the workflow details for this doc.
*******************************************************************************/
int removeWorkflowSettings(int objid, void *sqlsock);

/*******************************************************************************
* isVerified...
*
* Is this document verified?
*******************************************************************************/
int isVerified(int parentID, int objectID, int *verified, void *sqlsock);

/*******************************************************************************
* approveObject...
*
* Approve this document and if last verifier set document to online.
*******************************************************************************/
int approveObject(int objid, int uid, void *sqlsock);

/*******************************************************************************
* moveObject...
*
* Move this object to a new folder.
*******************************************************************************/
int moveObjectDB(int objid, int folderid, void *sqlsock);

/*******************************************************************************
* isVerifier...
*
* Is this user a verifier of this document.
*******************************************************************************/
int isVerifier(int objectid, int userid, int *verifier, void *sqlsock);

/*******************************************************************************
* userLicenseCheck...
*
* Called before creating a new user to check that the license is NOERROR.
*******************************************************************************/
int userLicenseCheck(void *sqlsock);


// ------------------------------ Calendar Module ------------------------------
// The following functions are used by the calendar module for 3.0.5
//

/*******************************************************************************
 * dbCreateInstance...
 *
 * Called to create a new instance of a calendar. A calendar instance is tied
 * to a path in the repository and all the permissions assigned to that path
 * are also assigned to that instance of the calendar module.
 ******************************************************************************/
int dbCalCreateInstance(cal_instance *inst, void *sqlsock);

/*******************************************************************************
 * dbCalDeleteInstance...
 *
 * Called to delete a new instance of a calendar. A calendar instance is tied
 * to a path in the repository and all the permissions assigned to that path
 * are also assigned to that instance of the calendar module.
 ******************************************************************************/
int dbCalDeleteInstance(cal_instance *inst, void *sqlsock);

/*******************************************************************************
 * dbCalMoveInstance...
 *
 * Called to move a calendar to a new location in the repository.
 ******************************************************************************/
int dbCalMoveInstance(cal_instance *inst, void *sqlsock);

/*******************************************************************************
 * dbCalCreateEvent...
 *
 * Called to create an event in the calendar. Events do not show up on the
 * but serve to group occurrences together so they can be updated all at once.
 ******************************************************************************/
int dbCalCreateEvent(cal_event *evt, void *sqlsock);

/*******************************************************************************
 * dbCalDeleteEvent...
 *
 * Called to delete an event in the calendar.
 * Will delete all occurrences of this event.
 ******************************************************************************/
int dbCalDeleteEvent(cal_event *evt, void *sqlsock);

/*******************************************************************************
 * dbCalCreateOccurrence...
 *
 * Called to create an occurrence of an event in the calendar.
 ******************************************************************************/
int dbCalCreateOccurrence(cal_occurrence *occ, void *sqlsock);

/*******************************************************************************
 * dbCalEditOccurrence...
 *
 * Called to edit an occurrence of an event in the calendar.
 ******************************************************************************/
int dbCalEditOccurrence(cal_occurrence *cal, void *sqlsock);

/*******************************************************************************
 * dbCalDeleteOccurrence...
 *
 * Called to delete an occurrence of an event in the calendar.
 ******************************************************************************/
int dbCalDeleteOccurrence(cal_occurrence *cal, void *sqlsock);

/*******************************************************************************
 * dbCalSearchTime...
 *
 * Called search for events in the calendar by time.
 ******************************************************************************/
int dbCalSearchTime(cal_instance *inst, struct tm *start, struct tm *end, int *count, cal_occurrence *** occlist, void *sqlsock);

/*******************************************************************************
 * dbCalSearchEvent...
 *
 * Called search for events in the calendar by event.
 ******************************************************************************/
int dbCalSearchEvent(cal_event *evt, int *count, cal_occurrence *** occlist, void *sqlsock);

/*******************************************************************************
 * dbCalInstanceDetails...
 *
 * Called to load the details for this instance.
 ******************************************************************************/
int dbCalInstanceDetails(cal_instance *instance, void *sqlsock);

/*******************************************************************************
 * dbCalEventDetails...
 *
 * Called to load the details for this event.
 ******************************************************************************/
int dbCalEventDetails(cal_event *evt, void *sqlsock);

/*******************************************************************************
 * dbCalOccurrenceDetails...
 *
 * Called to load the details for this occurrence.
 ******************************************************************************/
int dbCalOccurrenceDetails(cal_occurrence *occ, void *sqlsock);

// ------------------------------ Board Module ------------------------------
// The following functions are used by the board module for 3.0.5
//

/*******************************************************************************
 * dbCreateInstance... IA
 *
 * Called to create a new instance of a board. A board instance is tied
 * to a path in the repository and all the permissions assigned to that path
 * are also assigned to that instance of the board module.
 ******************************************************************************/
int dbBoardCreateInstance(board_instance *inst, void *sqlsock);

/*******************************************************************************
 * dbBoardDeleteInstance... IA
 *
 * Called to delete a new instance of a board. A board instance is tied
 * to a path in the repository and all the permissions assigned to that path
 * are also assigned to that instance of the board module.
 ******************************************************************************/
int dbBoardDeleteInstance(board_instance *inst, void *sqlsock);

/*******************************************************************************
 * dbBoardMoveInstance... IA
 *
 * Called to move a board to a new location in the repository.
 ******************************************************************************/
int dbBoardMoveInstance(board_instance *inst, void *sqlsock);

/*******************************************************************************
 * dbBoardCreateTopic... IA
 *
 * Called to create an topic in the board. Topics are displayed on the board
 * as groupings for the messages.
 ******************************************************************************/
int dbBoardCreateTopic(board_topic *tpc, void *sqlsock);

/*******************************************************************************
 * dbBoardDeleteTopic... IA
 *
 * Called to delete an topic in the board.
 * Will delete all messages of this topic.
 ******************************************************************************/
int dbBoardDeleteTopic(board_topic *tpc, void *sqlsock);

/*******************************************************************************
 * dbBoardCreateMessage... IA
 *
 * Called to create an message of an topic in the board.
 ******************************************************************************/
int dbBoardCreateMessage(board_message *msg, void *sqlsock);

/*******************************************************************************
 * dbBoardEditMessage... IA
 *
 * Called to edit an message of an topic in the board.
 ******************************************************************************/
int dbBoardEditMessage(board_message *msg, void *sqlsock);

/*******************************************************************************
 * dbBoardEditTopic... IA
 *
 * Called to edit an topic in the board.
 ******************************************************************************/
int dbBoardEditTopic(board_topic *tpc, void *sqlsock);

/*******************************************************************************
 * dbBoardDeleteMessage... IA
 *
 * Called to delete an message in the board.
 ******************************************************************************/
int dbBoardDeleteMessage(board_message *msg, void *sqlsock);

/*******************************************************************************
 * dbBoardSearch... IA
 *
 * Called search for messages in the board by time and keywords.
 ******************************************************************************/
int dbBoardSearch(board_instance *inst, struct tm *start, struct tm *end, char *query, int *count, board_message *** msglist, void *sqlsock);

/*******************************************************************************
 * dbBoardSearchTopic... IA
 *
 * Called search for messages in the board by topic.
 ******************************************************************************/
int dbBoardSearchTopic(board_topic *tpc, int *count, board_message *** msglist, void *sqlsock);

/*******************************************************************************
 * dbBoardListTopics... IA
 *
 * Called search for topics in the board by instance.
 ******************************************************************************/
int dbBoardListTopics(board_instance *inst, int *count, board_topic *** topiclist, void *sqlsock);

/*******************************************************************************
 * dbBoardInstanceDetails... IA
 *
 * Called to load the details for this instance.
 ******************************************************************************/
int dbBoardInstanceDetails(board_instance *instance, void *sqlsock);

/*******************************************************************************
 * dbBoardTopicDetails... IA
 *
 * Called to load the details for this topic.
 ******************************************************************************/
int dbBoardTopicDetails(board_topic *tpc, void *sqlsock);

/*******************************************************************************
 * dbBoardMessageDetails... IA
 *
 * Called to load the details for this message.
 ******************************************************************************/
int dbBoardMessageDetails(board_message *msg, void *sqlsock);

/*******************************************************************************
 * dbBoardIncrementViews... IA
 *
 * Called to count a view on a topic
 ******************************************************************************/
int dbBoardIncrementViews(board_topic *topic, void *sqlsock);

/*******************************************************************************
* getDeletedObjectID...
*
* Find the id of this object only if it is deleted.
*******************************************************************************/
int getDeletedObjectID(int parentid, char *name, int timestamp, int *objid, void *sqlsock);

/*******************************************************************************
* loadVerifierCommentObjectID...
*
* Find the object id of this verifier comment.
*******************************************************************************/
int loadVerifierCommentObjectID(int commentid, int *objid, void *sqlsock);

/*******************************************************************************
* getVerifierComment...
*
* Find the details about this comment.
*******************************************************************************/
int getVerifierComment(int commentid, VerifierComment **details, void *sqlsock);

/*******************************************************************************
* loadAllVerifierComments...
*
* Gets the ids of all comments for this object
*******************************************************************************/
int loadAllVerifierComments(int objectID, char ***columns, int *numcols, void *sqlsock);

/*******************************************************************************
* copyVerifierComments...
*
* Copy the verifier comments to the new version.
*******************************************************************************/
int copyVerifierComments(int oldid, int newid, void *sqlsock);

/*******************************************************************************
* addVerifierComment...
*
* Sets this comment in the database.
*******************************************************************************/
int addVerifierCommentDB(VerifierComment *comment, void *sqlsock);

/*******************************************************************************
* loadNotificationSettings...
*
* Load the notificaiton details for this doc.
*******************************************************************************/
int loadNotificationSettings(int objid, int *groupid, void *sqlsock);

/*******************************************************************************
* saveNotificationSettings...
*
* Save the notification details for this doc.
*******************************************************************************/
int saveNotificationSettings(int objid, int groupid, void *sqlsock);

/*******************************************************************************
* removeNotificationSettings...
*
* Remove the notification details from this doc.
*******************************************************************************/
int removeNotificationSettings(int objid, void *sqlsock);

#ifdef __cplusplus
}
#endif


#endif
