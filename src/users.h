/************************************************************************
* users.h
*
* User Management functions.
************************************************************************/
#ifndef _USERS_H
#define _USERS_H

#ifdef __cplusplus
extern "C" {
#endif



typedef struct _UserDetails_ {
  char *userID;
  char *userName;
  char *isOnline;
  char *isSuperUser;
  char *fullName;
  char *email;
  char *userType;
} UserDetails;

/*********************************************************************
* initUserDetails...
*
* Malloc the memory for this struct.
*********************************************************************/
UserDetails *initUserDetails(void);

/*********************************************************************
* freeUserDetails...
*
* free this struct
*********************************************************************/
void freeUserDetails(UserDetails *user);

/*********************************************************************
* validateUser...
*
* Does this user exist and are they active and not deleted.
* If so return their userid and super user status.
*********************************************************************/
int validateUser(char *username, char *password, int *uid, int *super, char **fullname, char **email, char **usertype, void *connection);

/*********************************************************************
 * validateExternalUser...
 *
 * Execute the command line app to authenticate this user.
 * This is used for LDAP authentication.
 *********************************************************************/
int validateExternalUser(char *username, char *password);

/*********************************************************************
* createValidSession...
*
* Add a session to the session manager and return the key.
*********************************************************************/
int createValidSession(char *username, char *password, int uid, int super, char *fullname, char *email, char *usertype, char **sess, void *connection, Env *env);

/*********************************************************************
* loadUserList...
*
* Load a filtered user list.
*********************************************************************/
int loadUserList(char *filter, int min, int max, void *sqlsock, Env *env, int **theusers, int *thenumusers);

#ifdef __cplusplus
}
#endif


#endif // _USERS_H
