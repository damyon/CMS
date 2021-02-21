#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#include "env.h"
#include "strings.h"
#include "errors.h"
#include "logging.h"
#include "structs.h"
#include "package.h"
#include "objects.h"
#include "groups.h"
#include "users.h"
#include "dbcalls.h"
#include "malloc.h"

/*********************************************************************
* initGroupDetails...
*
* dhumalloc the memory for this struct.
*********************************************************************/
GroupDetails *initGroupDetails() {
  GroupDetails *group = NULL;

  group = (GroupDetails *) dhumalloc(sizeof(GroupDetails));
  if (group == NULL)
    return NULL;

  group->groupID = NULL;
  group->groupName = NULL;
  group->isPublic = 0;
  return group;
}

/*********************************************************************
* freeGroupDetails...
*
* free this struct
*********************************************************************/
void freeGroupDetails(GroupDetails *group) {
  if (group == NULL)
    return;
  dhufree(group->groupID);
  dhufree(group->groupName);
  dhufree(group);
}

/*********************************************************************
* loadUsersGroups...
*
* Load a filtered group list belonging to a user.
*********************************************************************/
int loadUsersGroups(int id, char *filter, int min, int max, void *sqlsock, Env *env, int **thegroups, int *thenumgroups) {
  int numgroups = 0, *groups = NULL, errcode = 0, i = 0, count = 0;

  errcode = loadUsersGroupsDB(id, filter, &groups, &numgroups, sqlsock);
  if (errcode != E_OK)
    return errcode;

  *thegroups = (int *) dhumalloc(sizeof(int) * (numgroups));
  *thenumgroups = 0;
  count = 0;
  for (i = 0; i < numgroups; i++) {
    if (groups[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*thenumgroups) + (min>0?min:0) < max || max < 0)) {
        (*thegroups)[(*thenumgroups)] = groups[i];
        (*thenumgroups)++;
      }
      count++;
    }
  }

  dhufree(groups);
  if ((*thenumgroups) == 0)
    return NODATAFOUND;
  return E_OK;
}

/*********************************************************************
* loadGroupMembers...
*
* Load a filtered group list.
*********************************************************************/
int loadGroupMembers(int id, char *filter, int min, int max, void *sqlsock, Env *env, int **theusers, int *thenumusers) {
  int numusers = 0, *users = NULL, errcode = 0, i = 0, count = 0;

  errcode = loadGroupMembersDB(id, filter, &users, &numusers, sqlsock);
  if (errcode != E_OK)
    return errcode;

  *theusers = (int *) dhumalloc(sizeof(int) * (numusers));
  *thenumusers = 0;
  count = 0;
  for (i = 0; i < numusers; i++) {
    if (users[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*thenumusers) + (min>0?min:0) < max || max < 0)) {
        (*theusers)[(*thenumusers)] = users[i];
        (*thenumusers)++;
      }
      count++;
    }
  }

  dhufree(users);
  if ((*thenumusers) == 0)
    return NODATAFOUND;
  return E_OK;
}

/*********************************************************************
* loadGroupList...
*
* Load a filtered group list.
*********************************************************************/
int loadGroupList(char *filter, int min, int max, void *sqlsock, Env *env, int **thegroups, int *thenumgroups) {
  int numgroups = 0, *groups = NULL, errcode = 0, i = 0, count = 0;

  errcode = loadGroupListDB(filter, &groups, &numgroups, sqlsock);
  if (errcode != E_OK)
    return errcode;

  *thegroups = (int *) dhumalloc(sizeof(int) * (numgroups));
  *thenumgroups = 0;
  count = 0;
  for (i = 0; i < numgroups; i++) {
    if (groups[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*thenumgroups) + (min>0?min:0) < max || max < 0)) {
        (*thegroups)[(*thenumgroups)] = groups[i];
        (*thenumgroups)++;
      }
      count++;
    }
  }

  dhufree(groups);
  if ((*thenumgroups) == 0)
    return NODATAFOUND;
  return E_OK;
}

