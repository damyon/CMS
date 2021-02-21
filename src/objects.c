#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#ifdef WIN32
#include "win32.h"
#else
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "env.h"
#include "calendar.h"
#include "search.h"
#include "strings.h"
#include "errors.h"
#include "logging.h"
#include "structs.h"
#include "package.h"
#include "objects.h"
#include "users.h"
#include "dbcalls.h"
#include "request.h"
#include "file.h"
#include "malloc.h"
#include "ipc.h"
#include "config.h"

/*********************************************************************
* initObjectDetails...
*
* dhumalloc the memory for this struct.
*********************************************************************/
ObjectDetails *initObjectDetails() {
  ObjectDetails *obj = NULL;

  obj = (ObjectDetails *) dhumalloc(sizeof(ObjectDetails));
  if (obj == NULL)
    return NULL;

  obj->objectID = NULL;
  obj->objectName = NULL;
  obj->path = NULL;
  obj->parentID = NULL;
  obj->isOnline = NULL;
  obj->type = NULL;
  obj->isPublic = NULL;
  obj->mimeType = NULL;
  obj->version = NULL;
  obj->readPermission = NULL;
  obj->writePermission = NULL;
  obj->executePermission = NULL;
  obj->lockedByUserID = NULL;
  obj->publisherUserID = NULL;
  obj->fileSize = NULL;
  obj->tplt = NULL;
 
  return obj;
}

/*********************************************************************
* initVerifierComment...
*
* dhumalloc the memory for this struct.
*********************************************************************/
VerifierComment *initVerifierComment() {
  VerifierComment *obj = NULL;

  obj = (VerifierComment *) dhumalloc(sizeof(VerifierComment));
  if (obj == NULL)
    return NULL;

  obj->objectID = -1;
  obj->userID = -1;
  obj->commentID = -1;
  obj->created = NULL;
  obj->comment = NULL;
 
  return obj;
}

/*********************************************************************
* loadObjectContent...
*
* Load the contents of this object
* into memory.
*********************************************************************/
int loadObjectContent(int objid, int *datalen, char **data) {
  char *filepath = NULL, *filecontents = NULL;
  int lock = 0, errcode = 0;
  int filelength = 0;

  // Read the file
  if ((errcode = generateFilePath(objid, &filepath)) != E_OK)
    return errcode;

  lock = lockDirectory(filepath);
  if (lock < 0) {
    logError("Could not get directory lock.");
    dhufree(filepath);
    return LOCKFILETIMEOUT;
  }

  if ((errcode = readFile(filepath, &filecontents, &filelength)) != E_OK) {
    unlockDirectory(lock);
    dhufree(filepath);
    return errcode;
  }
  
  unlockDirectory(lock);
  *datalen = filelength;
  *data = filecontents;
  return E_OK;
}

/*********************************************************************
* freeVerifierComment...
*
* free this struct
*********************************************************************/
void freeVerifierComment(VerifierComment *obj) {
  if (obj == NULL)
    return;
  dhufree(obj->created);
  dhufree(obj->comment);
}

/*********************************************************************
* freeObjectDetails...
*
* free this struct
*********************************************************************/
void freeObjectDetails(ObjectDetails *obj) {
  if (obj == NULL)
    return;
  dhufree(obj->objectID);
  dhufree(obj->objectName);
  dhufree(obj->path);
  dhufree(obj->parentID);
  dhufree(obj->isOnline);
  dhufree(obj->type);
  dhufree(obj->isPublic);
  dhufree(obj->mimeType);
  dhufree(obj->version);
  dhufree(obj->readPermission);
  dhufree(obj->writePermission);
  dhufree(obj->executePermission);
  dhufree(obj->lockedByUserID);
  dhufree(obj->publisherUserID);
  dhufree(obj->fileSize);
  dhufree(obj->tplt);
}

/*********************************************************************
* getObjectPermissions...
*
* fill out the permissions section of this struct.
*********************************************************************/
int getObjectPermissions(ObjectDetails *details, Env *env, void *sqlsock) {
  int objid = 0;

  objid = strtol(details->objectID, NULL, 10);
  
  details->readPermission = userHasReadAccess(objid, env, sqlsock) == E_OK?dhustrdup((char *)"y"):dhustrdup((char *)"n");
  details->writePermission = userHasWriteAccess(objid, env, sqlsock) == E_OK?dhustrdup((char *)"y"):dhustrdup((char *)"n");
  details->executePermission = userHasExecuteAccess(objid, env, sqlsock) == E_OK?dhustrdup((char *)"y"):dhustrdup((char *)"n");
  
  return E_OK;
}

/*********************************************************************
* extractText...
*
* Extracts the text portions from this binary stream.
*********************************************************************/
char *extractText(char *data, int len) {
  char *ascii = NULL, *pos = NULL, *src = NULL, *dst = NULL;

  return strdup("");
  ascii = (char *) dhumalloc(sizeof(char) * (len*2) + 1);
  if (ascii == NULL)
    return NULL;

  src = data;
  pos = data; 
  dst = ascii;
  *dst = '\0';
  while (src != (data + len)) {
    if (isprint(*src) && isalpha(*src)) {
      pos = src;
      while (isprint(*pos) && (pos != (data + len))) pos++;
      if (pos - src > 10) {
        while ((src < pos) && (src != (data + len))) {*dst = *src; dst++; src++;}
        *dst = ' ';
        dst++;
      }
      src = pos;
    }
    src++;
  }
  *dst = '\0';
  return ascii;
}

/*********************************************************************
* objectInFolder...
*
* Is this object in this folder? (E_OK for yes - NODATAFOUND for no
*********************************************************************/
int objectInFolder(int objectid, int parentid, void *sqlsock) {
	int pid = 0;
	char *name = NULL;

	if (parentid == -1) 
		return E_OK;

	while (pid != -1 && pid != parentid) {
		if (getObjectParent(objectid, &name, &pid, sqlsock) != E_OK) {
			pid = -1;
		}
		dhufree(name);
		objectid = pid;
	}

	if (pid == parentid) {
		return E_OK;
	}

	return NODATAFOUND;
}

/*********************************************************************
* searchForContent...
*
* Search the database for documents.
*********************************************************************/
int searchForContent(char *query, int min, int max, int parentid, void *sqlsock, Env *env, int **objids, int **thescores, int *numobjs) {
  int numobjects = 0, *objects = NULL, *scores = NULL, errcode = 0, i = 0, timestamp = 0, count = 0;

  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, env), getEnvValue(CTIMETOK, env));

  errcode = searchContent(query, timestamp, &objects, &scores, &numobjects, sqlsock);
  if (errcode != E_OK)
    return errcode;

  *numobjs = numobjects;
  for (i = 0; i < numobjects; i++) {
    if ((!(userHasReadAccess(objects[i], env, sqlsock) == E_OK)) || (objectInFolder(objects[i], parentid, sqlsock) != E_OK)) {
      objects[i] = -1;
      (*numobjs)--;
    }
  }

  *objids = (int *) dhumalloc(sizeof(int) * (*numobjs));
  *thescores = (int *) dhumalloc(sizeof(int) * (*numobjs));
  *numobjs = 0;
  count = 0;
  for (i = 0; i < numobjects; i++) {
    if (objects[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*numobjs) + (min > 0?min:0)< max || max < 0)) {
        (*objids)[(*numobjs)] = objects[i];
        (*thescores)[(*numobjs)] = scores[i];
        (*numobjs)++;
      }
      count++;
    }
  }
  
  dhufree(scores);
  dhufree(objects);
  return (*numobjs == 0)?NODATAFOUND:E_OK;
}

/*********************************************************************
* loadPermissionList...
*
* Load the groups attached to this object.
*********************************************************************/
int loadPermissionList(int objid, char *filter, int min, int max, void *sqlsock, int **gids, int *numg) {
  int numgroups = 0, *groups = NULL, errcode = 0, i = 0, count = 0;

  errcode = loadPermissionListDB(objid, filter, &groups, &numgroups, sqlsock);
  if (errcode != E_OK)
    return errcode;

  
  *gids = (int *) dhumalloc(sizeof(int) * (numgroups));
  *numg = 0;
  count = 0;
  for (i = 0; i < numgroups; i++) {
    if (((count) >= min || min < 0) &&
        ((*numg) + (min > 0?min:0) < max || max < 0)) {
      (*gids)[(*numg)] = groups[i];
      (*numg)++;
    }
    count++;
  }
  
  dhufree(groups);
  if ((*numg) == 0)
    return NODATAFOUND;
  return E_OK;
}

/*********************************************************************
* loadWorkflowList...
*
* Load all the workflow documents and reduce them to the min and max.
*********************************************************************/
int loadWorkflowList(int userid, char *filter, 
                     int min, int max, 
                     void *sqlsock, Env *env, 
                     int **objids, int *numobjs) {
  int numobjects = 0, *objects = NULL, errcode = 0, i = 0, count = 0, timestamp = 0;

  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, env), getEnvValue(CTIMETOK, env));

  errcode = loadWorkflowListDB(userid, filter, timestamp, &objects, &numobjects, sqlsock);
  if (errcode != E_OK)
    return errcode;
  
  *numobjs = numobjects;
  for (i = 0; i < numobjects; i++) {
    if ((!(userHasReadAccess(objects[i], env, sqlsock) == E_OK))) {
      objects[i] = -1;
      (*numobjs)--;
    }
  }

  *objids = (int *) dhumalloc(sizeof(int) * (*numobjs));
  *numobjs = 0;
  count = 0;
  for (i = 0; i < numobjects; i++) {
    if (objects[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*numobjs) + (min>0?min:0)< max || max < 0)) {
        (*objids)[(*numobjs)] = objects[i];
        (*numobjs)++;
      }
      count++;
    }
  }
  
  dhufree(objects);
  if ((*numobjs) == 0)
    return NODATAFOUND;
  return E_OK;
}

/*********************************************************************
* loadObjectVersions...
*
* Load the list of previous versions of this object.
*********************************************************************/
int loadObjectVersions(int objid, int min, int max, void *sqlsock, Env *env, int **objids, int *numobjs) {
  int numobjects = 0, *objects = NULL, errcode = 0, i = 0, count = 0;

  errcode = loadObjectVersionsDB(objid, &objects, &numobjects, sqlsock);
  if (errcode != E_OK)
    return errcode;

  *numobjs = numobjects;
  for (i = 0; i < numobjects; i++) {
    if ((!(userHasReadAccess(objects[i], env, sqlsock) == E_OK))) {
      objects[i] = -1;
      (*numobjs)--;
    }
  }

  *objids = (int *) dhumalloc(sizeof(int) * (*numobjs));
  *numobjs = 0;
  count = 0;
  for (i = 0; i < numobjects; i++) {
    if (objects[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*numobjs) + (min > 0?min:0) < max || max < 0)) {
        (*objids)[(*numobjs)] = objects[i];
        (*numobjs)++;
      }
      count++;
    }
  }
  
  dhufree(objects);
  if ((*numobjs) == 0)
    return NODATAFOUND;
  return E_OK;
}

/*********************************************************************
* loadDeletedFolderContents...
*
* Load the contents of this folder (files that are deleted)
* into an ObjectDetailsList struct.
*********************************************************************/
int loadDeletedFolderContents(int objid, char *filter, int min, int max, char *sort, void *sqlsock, Env *env, int **objids, int *numobjs) {
  int numobjects = 0, *objects = NULL, errcode = 0, i = 0, count = 0, timestamp = 0;

  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, env), getEnvValue(CTIMETOK, env));

  errcode = loadDeletedFolderContentsDB(objid, filter, timestamp, sort, &objects, &numobjects, sqlsock);
  if (errcode != E_OK)
    return errcode;

  *numobjs = numobjects;

  *objids = (int *) dhumalloc(sizeof(int) * (*numobjs));
  *numobjs = 0;
  count = 0;
  for (i = 0; i < numobjects; i++) {
    if (objects[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*numobjs) + (min > 0?min:0) < max || max < 0)) {
        (*objids)[(*numobjs)] = objects[i];
        (*numobjs)++;
      }
      count++;
    }
  }
  
  dhufree(objects);
  if ((*numobjs) == 0)
    return NODATAFOUND;
  return E_OK;
}


/*********************************************************************
* loadFolderContents...
*
* Load the contents of this folder (only that the user can see)
* into an ObjectDetailsList struct.
*********************************************************************/
int loadFolderContents(int objid, char *filter, int min, int max, char *sort, void *sqlsock, Env *env, int **objids, int *numobjs) {
  int numobjects = 0, *objects = NULL, errcode = 0, i = 0, count = 0, timestamp = 0;

  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, env), getEnvValue(CTIMETOK, env));

  errcode = loadFolderContentsDB(objid, filter, timestamp, sort, &objects, &numobjects, sqlsock);
  if (errcode != E_OK)
    return errcode;

  *numobjs = numobjects;
  for (i = 0; i < numobjects; i++) {
    if ((!(userHasReadAccess(objects[i], env, sqlsock) == E_OK))) {
      objects[i] = -1;
      (*numobjs)--;
    }
  }

  *objids = (int *) dhumalloc(sizeof(int) * (*numobjs));
  *numobjs = 0;
  count = 0;
  for (i = 0; i < numobjects; i++) {
    if (objects[i] >= 0) {
      if (((count) >= min || min < 0) &&
          ((*numobjs) + (min > 0?min:0) < max || max < 0)) {
        (*objids)[(*numobjs)] = objects[i];
        (*numobjs)++;
      }
      count++;
    }
  }
  
  dhufree(objects);
  if ((*numobjs) == 0)
    return NODATAFOUND;
  return E_OK;
}

/*********************************************************************
* archiveFile...
*
* Write the file to disk and update the database.
*********************************************************************/
int archiveFile(int objectid, int *newid, char *tplt, FileObject *file, int index, Env *env, void *sqlsock) {
  ObjectDetails *details = NULL;
  UserDetails *user = NULL;
  char *title = NULL, *txt = NULL, *tmp = NULL;
  int errcode = 0, verified = 0, uid = 0, parentID = 0;

  if ((errcode = getObjectDetails(objectid, &details, sqlsock)) != E_OK) {
    return errcode;
  }

  title = dhustrdup(details->objectName);
  dhufree(details->mimeType); 
  details->mimeType = dhustrdup(file->contenttype);
  dhufree(details->isOnline);
  dhufree(details->tplt); 
  details->tplt = dhustrdup(tplt);
  parentID = strtol(details->parentID, NULL, 10);


  if ((errcode = isVerified(parentID, -1, &verified, sqlsock)) != E_OK) {
    freeObjectDetails(details);
    return errcode;
  }
  details->isOnline = dhustrdup(verified?(char *)"y":(char *)"n");

  if ((errcode = insertObjectDetails(details, newid, sqlsock)) != E_OK) {
    freeObjectDetails(details);
    return errcode;
  }
  freeObjectDetails(details);

  // update the childrens parents
  if ((errcode = updateChildren(objectid, *newid, sqlsock)) != E_OK) {
    logError("An error occurred while updating the object's children. Error:%s", getErrorMesg(errcode));
  }
 
  // copy the metadata
  if ((errcode = copyMetadata(objectid, *newid, sqlsock)) != E_OK) {
    logError("An error occurred while updating the object's metadata. Error:%s", getErrorMesg(errcode));
  }
  
  // copy the verifier comments
  if ((errcode = copyVerifierComments(objectid, *newid, sqlsock)) != E_OK) {
    logError("An error occurred while updating the object's verifier comments. Error:%s", getErrorMesg(errcode));
  }

  setObjectMetadata(*newid, "dc:format", details->mimeType, sqlsock);
  
  tmp = getISODate();
  setObjectMetadata(*newid, "dc:date", tmp, sqlsock);
  dhufree(tmp);

  tmp = getEnvValue("userID", env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);
  if (getUserDetails(uid, &user, sqlsock) == E_OK) {
    setObjectMetadata(*newid, "dc:publisher", user->fullName, sqlsock);
    freeUserDetails(user);
  }

  // copy permissions
  if ((errcode = copyPermissions(objectid, *newid, sqlsock)) != E_OK) {
    logError("An error occurred while updating the object's permissions. Error:%s", getErrorMesg(errcode));
  }
  
  // copy workflow
  if ((errcode = copyWorkflow(objectid, *newid, sqlsock)) != E_OK) {
    logError("An error occurred while updating the object's workflow. Error:%s", getErrorMesg(errcode));
  }

  // Then write new file to disk
  if ((errcode = writeObjectToDisk(file->data, file->datalen, *newid)) != E_OK) {
    return errcode;
  }
  
  // Update index tables
  if (index) {
    if ((errcode = indexObject(title, *newid, sqlsock)) != E_OK) {
      return errcode;
    }
    if (strncasecmp(file->contenttype, "text", 4) == 0) {
      if ((errcode = indexObject(file->data, *newid, sqlsock)) != E_OK) {
        return errcode;
      }
    } else {
      if ((txt = extractText(file->data, file->datalen))) {
        if ((errcode = indexObject(txt, *newid, sqlsock)) != E_OK) {
          dhufree(txt);
          return errcode;
        }
        dhufree(txt);
      }
    }
  }
  
  if (!verified) {
    if ((errcode = sendWorkflowNotification(parentID, *newid, sqlsock)) != E_OK) {
      logError("An error occurred while sending workflow notifications. Error:%s", getErrorMesg(errcode));
    }
  } else {
    if ((errcode = sendUpdateNotification(parentID, *newid, sqlsock)) != E_OK) {
      logError("An error occurred while sending update notifications. Error:%s", getErrorMesg(errcode));
    }
  }
  return E_OK;
}

/*********************************************************************
* sendUpdateNotification...
*
* Send emails notifying users of a document that has been changed.
*********************************************************************/
int sendUpdateNotification(int parentid, int objectid, void *sqlsock) {
  int groupid = 0, errcode = 0;
  int *users = NULL, usercount = 0, i = 0;
  char *size = NULL, *body = NULL, *subject = NULL, *title = NULL, *subj = NULL, *description = NULL;
  UserDetails *userdetails = NULL;
  ObjectDetails *objectdetails = NULL;

  if ((errcode = loadNotificationSettings(parentid, &groupid, sqlsock)) != E_OK) {
    return errcode;
  }

  if ((errcode = loadGroupMembersDB(groupid, "", &users, &usercount, sqlsock)) != E_OK) {
    return errcode;
  }

  if ((errcode = getObjectDetails(objectid, &objectdetails, sqlsock)) != E_OK) {
    return errcode;
  }
  
  if ((errcode = getObjectMetadata(objectid, "dc:title", &title, sqlsock)) != E_OK) {
    title = strdup("");
  }
  
  if ((errcode = getObjectMetadata(objectid, "dc:description", &description, sqlsock)) != E_OK) {
    description = strdup("");
  }
  
  if ((errcode = getObjectMetadata(objectid, "dc:subject", &subj, sqlsock)) != E_OK) {
    subj = strdup("");
  }

  for (i = 0; i < usercount; i++) {
    if (getUserDetails(users[i], &userdetails, sqlsock) == E_OK) {
      int2Str((strtol(objectdetails->fileSize, NULL, 10)/1024), &size);
      vstrdupcat(&body, "Dear ", userdetails->fullName, ".\r\n", 
			"\r\n",
                        "A document has recently been updated in the content management system.\r\n",
			"\r\n",
                        "You may view the document here:\r\n",
                        "\r\n",
                        "http://", getHostName(), "/cms/", objectdetails->path, "\r\n",
			"\r\n",
			"Details:\r\n",
			"Title: ", title, "\r\n",
			"Subject: ", subj, "\r\n",
			"Description: ", description, "\r\n",
			"Name: ", objectdetails->objectName, "\r\n",
			"MimeType: ", objectdetails->mimeType, "\r\n",
			"Size: ", size, " kb\r\n",
			"\r\n",
			"You may be required to login before you can view the document.\r\n",
			"\r\n",
			"Login: http://", getHostName(), "/cms/epiction/login.cms\r\n\r\n",
			"This is an auto-generated email. Please do not reply.", NULL);

      vstrdupcat(&subject, "Document \"", objectdetails->objectName, "\" updated.", NULL);
      sendEmail(getEmailServer(), getEmailPort(), getHostName(), getSendmailBin(), getEmailFromAddress(), userdetails->email, subject, body);

      freeUserDetails(userdetails);
      dhufree(size);
      dhufree(subject);
      dhufree(body);
      body = NULL;
    }
  }

  free(title);
  free(subj);
  free(description);
  freeObjectDetails(objectdetails);
  dhufree(users);

  return E_OK;
}

/*********************************************************************
* sendWorkflowNotification...
*
* Send emails notifying users of a document published to workflow.
*********************************************************************/
int sendWorkflowNotification(int parentid, int objectid, void *sqlsock) {
  int groupid = 0, requiresall = 0, errcode = 0;
  int *users = NULL, usercount = 0, i = 0;
  char *size = NULL, *body = NULL, *subject = NULL;
  UserDetails *userdetails = NULL;
  ObjectDetails *objectdetails = NULL;

  if ((errcode = loadWorkflowSettings(parentid, &groupid, &requiresall, sqlsock)) != E_OK) {
    return errcode;
  }

  if ((errcode = loadGroupMembersDB(groupid, "", &users, &usercount, sqlsock)) != E_OK) {
    return errcode;
  }

  if ((errcode = getObjectDetails(objectid, &objectdetails, sqlsock)) != E_OK) {
    return errcode;
  }

  for (i = 0; i < usercount; i++) {
    if (getUserDetails(users[i], &userdetails, sqlsock) == E_OK) {
      int2Str((strtol(objectdetails->fileSize, NULL, 10)/1024), &size);
      vstrdupcat(&body, "Dear ", userdetails->fullName, ".\r\n", 
			"\r\n",
                        "A document has recently been published to the content management system.\r\n",
			"\r\n",
			"Details:\r\n",
			"Name: ", objectdetails->objectName, "\r\n",
			"Path: ", objectdetails->path, "\r\n",
			"MimeType: ", objectdetails->mimeType, "\r\n",
			"Size: ", size, " kb\r\n",
			"\r\n",
                        "This document requires approval before it is made generally available and you are listed as a verifier for this document.\r\n", 
			"Please log into the content management system and review this document.\r\n", 
			"This document will be listed in the workflow section of the content management system.\r\n\r\n",
			"Login: http://", getHostName(), "/cms/Interface/Login.html\r\n\r\n",
			"This is an auto-generated email. Please do not reply.", NULL);

      vstrdupcat(&subject, "Document \"", objectdetails->objectName, "\" requires approval.", NULL);
      sendEmail(getEmailServer(), getEmailPort(), getHostName(), getSendmailBin(), getEmailFromAddress(), userdetails->email, subject, body);

      freeUserDetails(userdetails);
      dhufree(size);
      dhufree(subject);
      dhufree(body);
      body = NULL;
    }
  }

  freeObjectDetails(objectdetails);
  dhufree(users);

  return E_OK;
}

/*********************************************************************
* rollbackObjectVersion...
*
* Revert to a previous version.
*********************************************************************/
int rollbackObjectVersion(int objid, int oldid, int index, Env *env, void *sqlsock) {
  int errcode = 0, len = 0;
  char *data = NULL;
  FileObject *file = NULL;
  ObjectDetails *details = NULL;

  if ((errcode = loadObjectContent(oldid, &len, &data)) != E_OK) {
    return errcode;
  }

  if ((errcode = getObjectDetails(oldid, &details, sqlsock)) != E_OK) {
    dhufree(data);
    return errcode;
  }

  file = initFileObject(strdup(details->objectName), strdup(details->mimeType), strdup("file"), data, len);
  
  if ((errcode = replaceObjectContents(objid, index, details->tplt, file, env, sqlsock)) != E_OK) {
    freeFileObjectList(file);
    freeObjectDetails(details);
    return errcode;
  }

  return E_OK;
}

/*********************************************************************
* replaceObjectContents...
*
* Replace the file on disk.
*********************************************************************/
int replaceObjectContents(int objid, int index, char *tplt, FileObject *file, Env *env, void *sqlsock) {
  int lock = 0, errcode = 0, newid = 0;
  char *filepath = NULL;

  if ((errcode = generateFilePath(objid, &filepath)) != E_OK) {
    return errcode;  
  }

  lock = lockDirectory(filepath);
  
  if ((errcode = archiveFile(objid, &newid, tplt, file, index, env, sqlsock)) != E_OK) {
    unlockDirectory(lock);
    dhufree(filepath);
    return errcode;
  }

  unlockDirectory(lock);
  dhufree(filepath); 

  return E_OK;
}

/*********************************************************************
* importPackageObject...
*
* Create a new repository item.
*********************************************************************/
int importPackageObject(int parentid, Package *p, Env *env, void *sqlsock) {
  int errcode = 0,
      ispublic = 0, 
      index = 0,
      relativeOrder = 0,
      rootlen = 0,
      i = 0,
      objectID = 0,
      pid = 0;
  char *fullpath = NULL, *ptr = NULL, *name = NULL, *type = NULL, *tplt = NULL;
  PackageEntry *e = NULL;
  FileObject *file = NULL;
  ObjectDetails *details = NULL;
  MapNode *node = NULL;
  PackageMetadata *metadata = NULL;

  logDebug("TESTING\n");
  if (!p || !env || !sqlsock) {
    return RESOURCEERROR;
  }
  
  logDebug("TESTING: %d\n", parentid);
  if (parentid != -1) {
    if ((errcode = getObjectDetails(parentid, &details, sqlsock)) != E_OK) {
      return errcode;
    }
  }

  logDebug("TESTING\n");
  // can we write to this folder?
  if (userHasWriteAccess(parentid, env, sqlsock) != E_OK) {
    if (parentid != -1)
      freeObjectDetails(details);
    return ACCESSDENIED;
  }


  logDebug("TESTING\n");
  rootlen = strlen(p->header.root);
  for (i = countStack(p->files) - 1; i >= 0; i--) {
    
    logDebug("TESTING LOOP\n");
    e = (PackageEntry *) sniffNStack(p->files, i);
    if (!e) {
      if (parentid != -1)
        freeObjectDetails(details);
      return errcode;
    }
    logDebug("TESTING\n");
   
    ispublic = (e->isPublic == 'y')?1:0;
    type = (e->type);
    tplt = (e->tplt);
    relativeOrder = (e->relativeOrder);
    index = 0;
    fullpath = NULL;
    name = e->path + rootlen;
    while (*name == '/') name++;
    if (parentid == -1) {
      vstrdupcat(&fullpath, name, NULL);
    } else {
      vstrdupcat(&fullpath, details->path, "/", name, NULL);
    }
    logDebug("TESTING\n");

    errcode = isXRefValid(fullpath, -1, sqlsock, env, &objectID);
    file = initFileObject(strdup(name), strdup(e->mimeType), strdup("file"), (char *) e->data, e->length);
    // null the data so it is not freed twice
    e->data = NULL;

    if (errcode == E_OK) {
      logDebug("TESTING E_OK\n");
      errcode = replaceObjectContents(objectID, index, tplt, file, env, sqlsock);
      if (errcode != E_OK) {
        if (parentid != -1)
          freeObjectDetails(details);
        freeFileObjectList(file); 
        dhufree(fullpath);
        logDebug("REPLACE FAILED E_OK\n");
        return errcode;
      }
      errcode = isXRefValid(fullpath, -1, sqlsock, env, &objectID);
      if (errcode != E_OK) {
        if (parentid != -1)
          freeObjectDetails(details);
        dhufree(fullpath);
        freeFileObjectList(file); 
          logDebug("E_OK isXRefValid return\n");
        return errcode;
      }
    } else if (errcode == INVALIDXPATH) {
      logDebug("TESTING INVALIDXPATH\n");
      ptr = strrchr(fullpath, '/');
      if (ptr) {
        *ptr = '\0';
        errcode = isXRefValid(fullpath, -1, sqlsock, env, &pid);
        if (errcode != E_OK) {
          if (parentid != -1)
            freeObjectDetails(details);
          freeFileObjectList(file); 
          dhufree(fullpath);
          logDebug("TESTING INVALIDXPATH return\n");
          return errcode;
        }
        *ptr = '/';
      } else {
        pid = -1;
      }
      errcode = createNewObject(pid, ptr?ptr + 1:fullpath, ispublic, type, index, tplt, relativeOrder, file, env, sqlsock);
      if (errcode != E_OK) {
        if (parentid != -1)
          freeObjectDetails(details);
        dhufree(fullpath);
        freeFileObjectList(file); 
        logDebug("CREATE NEW OBJECT return\n");
        return errcode;
      }
      errcode = isXRefValid(fullpath, -1, sqlsock, env, &objectID);
      if (errcode != E_OK) {
        if (parentid != -1)
          freeObjectDetails(details);
        dhufree(fullpath);
        freeFileObjectList(file); 
        logDebug("FULLPATH NOT VALID return\n");
        return errcode;
      }
      // create the file
    } else {
      // error
      freeObjectDetails(details);
      freeFileObjectList(file); 
      dhufree(fullpath);
      logDebug("SOME RANDOM ERROR E_OK\n");
      return errcode;
    }

    node = getFirstMapNode(e->metadata);
    while (node != NULL) {
      metadata = (PackageMetadata *) node->ele;
      errcode = setObjectMetadata(objectID, metadata->name, metadata->value, sqlsock);
      if (errcode != E_OK) {
        dhufree(fullpath);
        freeFileObjectList(file); 
        logDebug("set Metadata E_OK\n");
        return errcode;
      }
      node = getNextMapNode(node, e->metadata);
    }
    
    dhufree(fullpath);
    freeFileObjectList(file); 
  }
  freeObjectDetails(details);
  
  logDebug("FINAL E_OK\n");
  return E_OK;
}

/*********************************************************************
* isValidFilename...
*
* Is this a valid filename. We enforce this because it makes it easier
* to construct urls to files in the repository.
*********************************************************************/
int isValidFilename(char *name) {
    char *c = name;

    while ((*c) != '\0') {
        if (!isalnum(*c) && (*c != ' ') && (*c != '-') && (*c != '_') && (*c != '.') && (*c != '%')) {
            return 0;
        }
        c++;
    }
    return 1;
}

/*********************************************************************
* createNewObject...
*
* Create a new repository item.
*********************************************************************/
int createNewObject(int parentid, char *title, int ispublic, char *type, int index, char *tplt, int relativeOrder, FileObject *file, Env *env, void *sqlsock) {
  int errcode = 0, newid = 0, verified = 0, uid = 0, timestamp = 0;
  ObjectDetails details;
  UserDetails *user;
  char *txt = NULL, *date = NULL, *tmp = NULL, *userID = NULL;

  if (file == NULL || title == NULL || file->contenttype == NULL)
    return RESOURCEERROR;

  if (!isValidFilename(title)) {
    return INVALIDARGUMENTS;
  }

  if ((errcode = isVerified(parentid, -1, &verified, sqlsock)) != E_OK) {
    return errcode;
  }

  timestamp = time(NULL);
  errcode = getObjectID(parentid, title, timestamp, &newid, sqlsock);
  if (errcode == E_OK) {
    // this file exists
    return FILEEXISTS;
  }
  errcode = E_OK;
  
  details.objectName = dhustrdup(title);
  int2Str(parentid, &(details.parentID));
  details.isPublic = dhustrdup(ispublic?(char *)"y":(char *)"n");
  details.tplt = dhustrdup(tplt);
  int2Str(relativeOrder, &(details.relativeOrder));

  details.isOnline = dhustrdup(verified?(char *)"y":(char *)"n");
  details.type = dhustrdup(type?type:(char *)"");
  details.mimeType = dhustrdup(file->contenttype);
  details.version = dhustrdup("0");
  details.lockedByUserID = dhustrdup("-1");
  userID = getEnvValue("userID", env);
  details.publisherUserID = dhustrdup(userID?userID:"-1");
  
  if ((errcode = insertObjectDetails(&details, &newid, sqlsock)) != E_OK) {
    dhufree(details.objectName);
    dhufree(details.parentID);
    dhufree(details.isPublic);
    dhufree(details.isOnline);
    dhufree(details.type);
    dhufree(details.mimeType);
    dhufree(details.version);
    dhufree(details.lockedByUserID);
    dhufree(details.publisherUserID);
    dhufree(details.tplt);
    return errcode;
  }
  
  // set the metadata

  setObjectMetadata(newid, "dc:title", details.objectName, sqlsock);
  tmp = getEnvValue("userID", env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);
  if (getUserDetails(uid, &user, sqlsock) == E_OK ) {
    setObjectMetadata(newid, "dc:publisher", user->fullName, sqlsock);
    freeUserDetails(user);
  }
  date = getISODate();
  setObjectMetadata(newid, "dc:date", date, sqlsock);
  dhufree(date);
  setObjectMetadata(newid, "dc:format", details.mimeType, sqlsock);
  setObjectMetadata(newid, "dc:identifier", details.objectName, sqlsock);

  dhufree(details.objectName);
  dhufree(details.parentID);
  dhufree(details.isPublic);
  dhufree(details.isOnline);
  dhufree(details.type);
  dhufree(details.mimeType);
  dhufree(details.version);
  dhufree(details.lockedByUserID);
  dhufree(details.publisherUserID);
  dhufree(details.tplt);

  if ((errcode = writeObjectToDisk(file->data, file->datalen, newid)) != E_OK) {
    return errcode;
  }

  if ((index)) {
    if ((errcode = indexObject(title, newid, sqlsock)) != E_OK) {
      return errcode;
    }
    if ((strncasecmp(file->contenttype, "text", 4) == 0)) {
      if ((errcode = indexObject(file->data, newid, sqlsock)) != E_OK) {
        return errcode;
      }
    } else {
      if ((txt = extractText(file->data, file->datalen))) {
        if ((errcode = indexObject(txt, newid, sqlsock)) != E_OK) {
	  dhufree(txt);
          return errcode;
        }
	dhufree(txt);
      }
    }
  }

  if (!verified) {
    if ((errcode = sendWorkflowNotification(parentid, newid, sqlsock)) != E_OK) {
      logError("An error occurred while sending workflow notifications. Error:%s", getErrorMesg(errcode));
    }
  } else {
    if ((errcode = sendUpdateNotification(parentid, newid, sqlsock)) != E_OK) {
      logError("An error occurred while sending update notifications. Error:%s", getErrorMesg(errcode));
    }
  }
  
  return E_OK;
}

/*********************************************************************
* deleteObject...
*
* Delete repository item.
*********************************************************************/
int deleteObject(int objectid, Env *env, void *sqlsock) {
  int errcode = 0, len = 0;
  char *data = NULL;
  FileObject *file = NULL;
  int lock = 0, newid = 0;
  char *filepath = NULL;
  ObjectDetails *details = NULL;
  int i = 0, *objects = NULL, numobjects = 0;

  errcode = loadFolderContentsDB(objectid, "", -1, NULL, &objects, &numobjects, sqlsock);
  if (errcode == E_OK) {
    for (i = 0; i < numobjects; i++) {
      if ((errcode = deleteObject(objects[i], env, sqlsock)) != E_OK) {
        dhufree(objects);  
        return errcode;
      }
    }
  }
   
  // copy the existing version to the next slot  
  if ((errcode = getObjectDetails(objectid, &details, sqlsock)) != E_OK) {
    return errcode;
  }

  if ((errcode = loadObjectContent(objectid, &len, &data)) != E_OK) {
    return errcode;
  }
  
  file = initFileObject(strdup(details->objectName), strdup(details->mimeType), strdup("file"), data, len);
  
  if ((errcode = generateFilePath(objectid, &filepath)) != E_OK) {
    freeFileObjectList(file);
    freeObjectDetails(details);
    return errcode;  
  }

  lock = lockDirectory(filepath);
  
  if ((errcode = archiveFile(objectid, &newid, details->tplt, file, 1, env, sqlsock)) != E_OK) {
    freeFileObjectList(file);
    unlockDirectory(lock);
    dhufree(filepath);
    freeObjectDetails(details);
    return errcode;
  }
    
  // we copy the metadata in case this object gets restored later
  if ((errcode = copyMetadata(objectid, newid, sqlsock)) != E_OK) {
    return errcode;
  }
  
  // we copy the verifier comments in case this object gets restored later
  if ((errcode = copyVerifierComments(objectid, newid, sqlsock)) != E_OK) {
    return errcode;
  }

  if ((errcode = unIndexObject(objectid, sqlsock)) != E_OK) {
    return errcode;
  }
  
  // insert deleted version
  if ((errcode = deleteObjectDB(newid, sqlsock)) != E_OK) {
    return errcode;
  }

  freeObjectDetails(details);
  unlockDirectory(lock);

  dhufree(filepath); 
  freeFileObjectList(file);

  return E_OK;
}

/*******************************************************************************
* editObjectDetails...
*
* Edit the details about this object.
*******************************************************************************/
int editObjectDetails(int objectid, char *title, int ispublic, int relativeOrder, Env *env, void *sqlsock) {
  int errcode = 0, len = 0;
  char *data = NULL;
  FileObject *file = NULL;
  ObjectDetails *details = NULL;
  int lock = 0, newid = 0;
  char *filepath = NULL;
  
  if (!isValidFilename(title)) {
    return INVALIDARGUMENTS;
  }
  // copy the existing version to the next slot  
  if ((errcode = getObjectDetails(objectid, &details, sqlsock)) != E_OK) {
    return errcode;
  }


  if (strcmp(title, details->objectName) != 0) {
    if ((errcode = loadObjectContent(objectid, &len, &data)) != E_OK) {
      return errcode;
    }
  
    file = initFileObject(strdup(details->objectName), strdup(details->mimeType), strdup("file"), data, len);
  
    if ((errcode = generateFilePath(objectid, &filepath)) != E_OK) {
      freeFileObjectList(file);
      freeObjectDetails(details);
      return errcode;  
    }

    lock = lockDirectory(filepath);
  
    if ((errcode = archiveFile(objectid, &newid, details->tplt, file, 1, env, sqlsock)) != E_OK) {
      freeFileObjectList(file);
      unlockDirectory(lock);
      dhufree(filepath);
      freeObjectDetails(details);
      return errcode;
    }
    
    if ((errcode = updateObjectDB(newid, title, ispublic, relativeOrder, sqlsock)) != E_OK) {
      unlockDirectory(lock);
      dhufree(filepath); 
      freeFileObjectList(file);
      freeObjectDetails(details);
      return errcode;
    }

    unlockDirectory(lock);
    dhufree(filepath); 
    freeFileObjectList(file);
    if ((errcode = deleteObject(objectid, env, sqlsock)) != E_OK) {
      freeObjectDetails(details);
      return errcode;
    }

    // if calendar or board, update instance
    //
    if (strcmp(details->type, "calendar") == 0) {
      cal_instance * c = calInitInstance();
      c->objectPath = strdup(details->path);
      dbCalInstanceDetails(c, sqlsock);

      // update the objectPath in the calendar instance
      char *sep = NULL;
      sep = strrchr(c->objectPath, '/');
      if (sep != NULL) {
	*sep = '\0';
      }
      sep = NULL;
      vstrdupcat(&sep, c->objectPath, "/", title, NULL);
      dhufree(c->objectPath);

      c->objectPath = sep;
      dbCalMoveInstance(c, sqlsock);
      calFreeInstance(c);
    }

    

  } else {
    if ((errcode = updateObjectDB(objectid, title, ispublic, relativeOrder, sqlsock)) != E_OK) {
      freeObjectDetails(details);
      return errcode;
    }
  }

  freeObjectDetails(details);

  return E_OK;
}

/*******************************************************************************
* moveObject...
*
* Move this object to a new location.
*******************************************************************************/
int moveObject(int objectid, int newparent, Env *env, void *sqlsock) {
  int errcode = 0, len = 0;
  char *data = NULL;
  FileObject *file = NULL;
  ObjectDetails *details = NULL, *newdetails = NULL;;
  int lock = 0, newid = 0;
  char *filepath = NULL;


  // copy the existing version to the next slot  
  if ((errcode = getObjectDetails(objectid, &details, sqlsock)) != E_OK) {
    return errcode;
  }

  if ((errcode = loadObjectContent(objectid, &len, &data)) != E_OK) {
    return errcode;
  }

  file = initFileObject(strdup(details->objectName), strdup(details->mimeType), strdup("file"), data, len);
  
  if ((errcode = generateFilePath(objectid, &filepath)) != E_OK) {
    freeFileObjectList(file);
    freeObjectDetails(details);
    return errcode;  
  }

  lock = lockDirectory(filepath);
  
  if ((errcode = archiveFile(objectid, &newid, details->tplt, file, 1, env, sqlsock)) != E_OK) {
    freeFileObjectList(file);
    unlockDirectory(lock);
    dhufree(filepath);
    freeObjectDetails(details);
    return errcode;
  }

  unlockDirectory(lock);
  dhufree(filepath); 
  freeFileObjectList(file);

  if ((errcode = moveObjectDB(newid, newparent, sqlsock)) != E_OK) {
    freeObjectDetails(details);
    return errcode;
  }

  if ((errcode = deleteObject(objectid, env, sqlsock)) != E_OK) {
    freeObjectDetails(details);
    return errcode;
  }

  // if calendar or forum, update instance details
  //
  if (strcmp(details->type, "calendar") == 0) {
    // update the instance details.
    if ((errcode = getObjectDetails(newid, &newdetails, sqlsock)) != E_OK) {
      return errcode;
    }

    cal_instance *c = calInitInstance();
    c->objectPath = strdup(details->path); 
    dbCalInstanceDetails(c, sqlsock);

    dhufree(c->objectPath);
    c->objectPath = strdup(newdetails->path);
    dbCalMoveInstance(c, sqlsock);

    calFreeInstance(c);

    freeObjectDetails(newdetails);
  }

  freeObjectDetails(details);

  return E_OK;
}

/*
 * copyObject
 *
 * Copy object from position a to position b.
 */
int copyObject(int srcid, int destid, int depth, Env *env, void *sqlsock) {
  int err = 0, timestamp = 0, datalen = 0, objid = 0, *children = NULL, numchildren = 0, i = 0;
  char *data = NULL, *title = NULL;
  FileObject *file = NULL;
  ObjectDetails *details = NULL;
  
  if (srcid <= 0 || destid <= 0 || env == NULL || sqlsock == NULL) {
      err = RESOURCEERROR;
  }

  timestamp = time(NULL);

  if (err == 0) {
    // even works for root folder
    err = userHasWriteAccess(destid, env, sqlsock);
  }

  if (err == 0) {
    err = getObjectDetails(srcid, &details, sqlsock);
  } 

  if (err == 0) {
    err = loadObjectContent(srcid, &datalen, &data);
  } 

  if (err == 0) {

    file = initFileObject(dhustrdup("file"),
                        dhustrdup(details->mimeType),
                        dhustrdup(details->objectName),
                        data,
                        datalen);

    title = dhustrdup(details->objectName);
    err = createNewObject(destid, title, (details->isPublic[0] == 'y')?1:0,
                                             details->type, // type
                                             1, // index
                                             details->tplt,
					     strtol(details->relativeOrder, NULL, 10),
                                             file,
                                             env,
                                             sqlsock);
    dhufree(title);

    freeFileObjectList(file);
  }
  if (err == 0) {
    // copy the metadata
    timestamp++;
    err = getObjectID(destid, details->objectName, timestamp, &objid, sqlsock);

    if (err == 0) {
      err = copyMetadata(srcid, objid, sqlsock);
      if (err == 0) {
        err = copyVerifierComments(srcid, objid, sqlsock);
      }
    } else {
      logError("Could not find new object: %d, %s\n", destid, details->objectName);
    }
  }

  // check for folder and depth header (or lack of) 
  if (err == 0) {
    if ((strcasecmp(details->type, "FOLDER") == 0) && depth == -1) {
      // ok - load up the children and copy them too.
      err = loadFolderContentsDB(srcid, "", timestamp, "name", &children, &numchildren, sqlsock);

      if (err == 0) {
        for (i = 0; i < numchildren; i++) {
          err = copyObject(children[i], objid, depth, env, sqlsock);
        }
      } else if (err == NODATAFOUND) {
        err = E_OK;
      }
    }
  }


  // if calendar or forum - create new instance at this location and copy all the data
  if (strcmp(details->type, "calendar") == 0) {
    cal_instance * c = calInitInstance();
    c->objectPath = strdup(details->path);

    dbCalCreateInstance(c, sqlsock);
    calFreeInstance(c);
  }

  if (details != NULL)
    freeObjectDetails(details);

  return err;
}

/*
 * addVerifierComment...
 *
 * This function will add a comment to the database and email all the people
 * involved in the workflow.
 */
int addVerifierComment(int objid, char *comment, Env *env, void *sqlsock) {
  char *tmp = NULL;
  int uid = 0;
  VerifierComment *c = initVerifierComment();
  UserDetails *userdetails = NULL, *commentuser = NULL;
  ObjectDetails *objectdetails = NULL;
  int err = E_OK, all = 0, groupid = 0, pubuid = 0;
  int *users = NULL, usercount = 0;
  int i = 0;
  char *body = NULL, *subject = NULL;

  tmp = getEnvValue("userID", env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  c->objectID = objid;
  c->userID = uid;
  c->comment = dhustrdup(comment);

  err = addVerifierCommentDB(c, sqlsock);

  freeVerifierComment(c);

  // now send the notifications
  if (err == E_OK) {
    err = loadWorkflowSettings(objid, &groupid, &all, sqlsock);
  }

  if (err == E_OK) {
    err = loadGroupMembersDB(groupid, "", &users, &usercount, sqlsock);
  }
  
  if (err == E_OK) {
    err = getObjectDetails(objid, &objectdetails, sqlsock);
  }

  if (err == E_OK) {
    err = getUserDetails(c->userID, &commentuser, sqlsock);
  }

  if (err == E_OK) {
    pubuid = strtol(objectdetails->publisherUserID, NULL, 10);
  }

  if (err == E_OK) {
    for (i = 0; i < usercount; i += 1) {
      if (pubuid != users[i]) {
        if (getUserDetails(users[i], &userdetails, sqlsock) == E_OK) {
          vstrdupcat(&body, "Dear ", userdetails->fullName, ".\r\n", 
			"\r\n",
                        "A workflow comment has been posted to the content management system.\r\n",
			"\r\n",
			"Details:\r\n",
			"Name: ", objectdetails->objectName, "\r\n",
			"Path: ", objectdetails->path, "\r\n",
			"User: ", commentuser->fullName, " (", commentuser->userName, ")\r\n",
			"\r\n",
			"Comment:\r\n", comment, "\r\n",
                        "\r\n", 
			"Please log into the content management system and review this document.\r\n", 
			"This document will be listed in the workflow section of the content management system.\r\n\r\n",
			"Login: http://", getHostName(), "/cms/epiction/login.cms\r\n\r\n",
			"This is an auto-generated email. Please do not reply.", NULL);
          vstrdupcat(&subject, "Workflow Comment For \"", objectdetails->objectName, "\" .", NULL);
          sendEmail(getEmailServer(), getEmailPort(), getHostName(), getSendmailBin(), getEmailFromAddress(), userdetails->email, subject, body);

	  dhufree(body);
  	  dhufree(subject);
          freeUserDetails(userdetails);
        }
      }
    }
  }

  if (err == E_OK) {
    if (pubuid > 0) {
      if (getUserDetails(pubuid, &userdetails, sqlsock) == E_OK) {
        vstrdupcat(&body, "Dear ", userdetails->fullName, ".\r\n", 
			"\r\n",
                        "A workflow comment has been posted to the content management system.\r\n",
			"\r\n",
			"Details:\r\n",
			"Name: ", objectdetails->objectName, "\r\n",
			"Path: ", objectdetails->path, "\r\n",
			"User: ", commentuser->fullName, " (", commentuser->userName, ")\r\n",
			"\r\n",
			"Comment:\r\n", comment, "\r\n",
                        "\r\n", 
			"Please log into the content management system and review this document.\r\n", 
			"This document will be listed in the workflow section of the content management system.\r\n\r\n",
			"Login: http://", getHostName(), "/cms/epiction/login.cms\r\n\r\n",
			"This is an auto-generated email. Please do not reply.", NULL);
        vstrdupcat(&subject, "Workflow Comment For \"", objectdetails->objectName, "\" .", NULL);
        sendEmail(getEmailServer(), getEmailPort(), getHostName(), getSendmailBin(), getEmailFromAddress(), userdetails->email, subject, body);

        dhufree(body);
        dhufree(subject);
        freeUserDetails(userdetails);
      }
    }
  }

  freeObjectDetails(objectdetails);
  freeUserDetails(commentuser);
  dhufree(users);

  return err;
}
