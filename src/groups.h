/************************************************************************
* groups.h
*
* Group Management functions.
************************************************************************/
#ifndef _GROUPS_H
#define _GROUPS_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _GroupDetails_ {
  char *groupName,
       *groupID;
  int isPublic;
} GroupDetails;

/*********************************************************************
* initGroupDetails...
*
* Malloc the memory for this struct.
*********************************************************************/
GroupDetails *initGroupDetails(void);

/*********************************************************************
* freeGroupDetails...
*
* free this struct
*********************************************************************/
void freeGroupDetails(GroupDetails *group);

/*********************************************************************
* loadGroupList...
*
* Load a filtered group list.
*********************************************************************/
int loadGroupList(char *filter, int min, int max, void *sqlsock, Env *env, int **thegroups, int *thenumgroups);

/*********************************************************************
* loadUsersGroups...
* 
* Load a filtered group list belonging to a user.
*********************************************************************/
int loadUsersGroups(int id, char *filter, int min, int max, void *sqlsock, Env *env, int **thegroups, int *thenumgroups);


/*********************************************************************
* loadGroupMembers...
* 
* Load a filtered group list.
*********************************************************************/
int loadGroupMembers(int id, char *filter, int min, int max, void *sqlsock, Env *env, int **theusers, int *thenumusers);

#ifdef __cplusplus
}
#endif


#endif // _GROUPS_H
