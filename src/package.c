#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#ifdef WIN32
#include "win32.h"
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#include <string.h>
#include <time.h>

#include <ctype.h>
#include "env.h"
#include "strings.h"
#include "structs.h"
#include "errors.h"
#include "logging.h"
#include "malloc.h"
#include "package.h"
#include "objects.h"
#include "dbcalls.h"
#include "xml.h"
#include "base64.h"
#include "md5.h"
#include "request.h"

/*********************************************************************
* initPackage...
*
* Initialise a Package structure and
* fill it with default values.
*********************************************************************/
Package * initPackage() {
  Package *p = NULL;
  char *date = NULL, *e = NULL;
  time_t t;

  t = time(NULL);

  date = ctime(&t);

  e = date + strlen(date) -1;
  while (isspace(*e) && e > date) { *e = '\0'; e--;}

  p = (Package *) dhumalloc(sizeof(Package));

  if (p == NULL) {
    return NULL;
  }

  p->header.root[0] = '\0';
  strcpy(p->header.date, date);

  p->files = initStack();
  if (p->files == NULL) {
    dhufree(p);
    return NULL;
  }
  p->header.numfiles = 0;

  return p;
}

/*********************************************************************
* freePackageEntry...
*
* release the memory from a package entry.
*********************************************************************/
void freePackageEntry(PackageEntry **entry) {
  PackageEntry *e = *entry;

  if (e == NULL)
    return;

  freeMap(e->metadata);
  dhufree(e->data);
  dhufree(e);
  *entry = NULL;
  return;
}

/*********************************************************************
* freePackage...
*
* Release the memory from a package structure.
*********************************************************************/
void freePackage(Package **package) {
  Package *p = *package;
  PackageEntry *e = NULL;

  if (p == NULL)
    return;

  while ((e = (PackageEntry *) popStack(p->files)) != NULL) {
    freePackageEntry(&e);
  }
  freeStack(&p->files);

  dhufree(p);

  *package = NULL;
}

/*********************************************************************
* initPackageEntry...
*
* Initialise a PackageEntry structues and fill it with
* default values.
*********************************************************************/
PackageEntry * initPackageEntry() {
  PackageEntry *e = NULL;

  e = (PackageEntry *) dhumalloc(sizeof(PackageEntry));

  if (e == NULL) {
    return NULL;
  }

  e->path[0] = '\0';
  e->mimeType[0] = '\0';
  e->tplt[0] = '\0';
  e->type[0] = '\0';
  e->data = NULL;
  e->length = 0;
  e->relativeOrder = 0;
  e->metadata = initMap(cmpPackageMetadata, freePackageMetadata);
  return e;
}

/*********************************************************************
* initPackageMetadata...
*
* Initialise this struct.
*********************************************************************/
PackageMetadata *initPackageMetadata(char *name, char *value) {
  PackageMetadata *p = NULL;

  p = (PackageMetadata *) malloc(sizeof(PackageMetadata));
  if (!p)
    return NULL;

  p->name = dhustrdup(name?name:(char *) "");
  p->value = dhustrdup(value?value:(char *) "");
  return p;
}

/*********************************************************************
* cmpPackageMetadata...
*
* Compare the names of these 2 metadata objects.
*********************************************************************/
int cmpPackageMetadata(void *a, void *b) {
  PackageMetadata *pa = (PackageMetadata *) a, *pb = (PackageMetadata *) b;

  if (pa && pb && pa->name && pb->name) {
    return strcmp(pa->name, pb->name);
  }
  return 0;
}

/*********************************************************************
* freePackageMetadata...
*
* Release the memory from this struct.
*********************************************************************/
void freePackageMetadata(void *a) {
  PackageMetadata *p = (PackageMetadata *) a;

  if (p) {
    if (p->name)
      dhufree(p->name);
    if (p->value)
      dhufree(p->value);
    dhufree(p);
  }
}

/*********************************************************************
* addObjectToPackage...
*
* Add another content object to this package.
* This will recursively add all the content objects below this one as well.
*********************************************************************/
int addObjectToPackage(Package *p, int objectID, Env *env, void *dbconn) {
  PackageEntry *e = NULL;
  ObjectDetails *details = NULL;
  int datalen = 0;
  char *data = NULL, **columns = NULL, *value = NULL;
  int retcode = 0, numobjs = 0, *objids = NULL, i = 0, numcols = 0;
  MapNode *node = NULL;
  PackageMetadata *metadata = NULL;

  if (userHasReadAccess(objectID, env, dbconn) != E_OK) {
    return ACCESSDENIED;
  }

  // sanity
  if (p == NULL || objectID < 0 ) {
    return RESOURCEERROR;
  }

  retcode = getObjectDetails(objectID, &details, dbconn);
  if (retcode != E_OK) {
    return retcode;
  }

  retcode = loadObjectContent(objectID, &datalen, &data);
  if (retcode != E_OK) {
    freeObjectDetails(details);
    return retcode;
  }

  e = initPackageEntry();
  if (e == NULL) {
    freeObjectDetails(details);
    return RESOURCEERROR;
  }

  snprintf(e->path, PACKAGE_STRING_LENGTH - 1, "%s", details->path);
  snprintf(e->mimeType, PACKAGE_STRING_LENGTH - 1, "%s", details->mimeType);
  snprintf(e->type, PACKAGE_STRING_LENGTH, "%s", details->type);
  snprintf(e->tplt, PACKAGE_STRING_LENGTH, "%s", details->tplt);
  e->isPublic = *(details->isPublic);
  e->isOnline = *(details->isOnline);
  e->relativeOrder = strtol(details->relativeOrder, NULL, 10);
  e->version = strtol(details->version, NULL, 10);

  // insert all the metadata
  retcode = getAllObjectMetadata(objectID, &columns, &numcols, dbconn);
  if (retcode != E_OK && retcode != NODATAFOUND) {
    freePackageEntry(&e);
    freeObjectDetails(details);
    return retcode;
  }

  for (i = 0; i < numcols; i++) {
    if (getObjectMetadata(objectID, columns[i], &value, dbconn) == E_OK) {
      metadata = initPackageMetadata(columns[i], value);
      node = initMapNode(metadata);
      insertMapValue(node, e->metadata);
      dhufree(value);
    }
    dhufree(columns[i]);
  }
  dhufree(columns);

  e->data = (void *) dhumalloc(sizeof(char) * datalen + 1);
  if (e->data == NULL) {
    freePackageEntry(&e);
    freeObjectDetails(details);
    return RESOURCEERROR;
  }
  memcpy(e->data, data, datalen);
  
  e->length = datalen;

  pushStack(p->files, e);
  p->header.numfiles ++;

  if (strcmp(details->type, "FOLDER") == 0) {
    retcode = loadFolderContentsDB(objectID, "", -1, NULL, &objids, &numobjs, dbconn);
    if (retcode != E_OK) {
      return RESOURCEERROR;
    }

    for (i = 0; i < numobjs; i++) {
      addObjectToPackage(p, objids[i], env, dbconn);
    }

    dhufree(objids);
  }

  freeObjectDetails(details);
  return E_OK;
}

/*********************************************************************
 * packageCRCCheck...
 *
 ********************************************************************/
int packageCRCCheck(Package *p) {
  MD5Context c;
  unsigned char digest[16];
  char crc[33];
  PackageEntry *e = NULL;
  int i = 0;

  return E_OK;
/*

  if (p == NULL)
    return RESOURCEERROR;

  MD5Init(&c);

  for (i = countStack(p->files) - 1; i >= 0; i--) {
    e = (PackageEntry *) sniffNStack(p->files, i);
    MD5Update(&c, (const unsigned char *) e->data, e->length);
  }

  MD5Final(digest, &c);
  DigestToBase16(digest, crc);

  if (strcmp(crc, p->header.crc) != 0) {
    return NOMATCH;
  }
  
  return E_OK;
*/
}

/*********************************************************************
 * updatePackageCRC...
 *
 ********************************************************************/
int updatePackageCRC(Package *p) {
  MD5Context c;
  unsigned char digest[16];
  PackageEntry *e = NULL;
  int i = 0;

  if (p == NULL)
    return RESOURCEERROR;

  MD5Init(&c);

  for (i = countStack(p->files) - 1; i >= 0; i--) {
    e = (PackageEntry *) sniffNStack(p->files, i);
    MD5Update(&c, (const unsigned char *) e->data, e->length);
  }

  MD5Final(digest, &c);
  DigestToBase16(digest, p->header.crc);
  
  return E_OK;
}

/*********************************************************************
 * importPackage...
 *
 * This unpack a memory buffer into a proper package structure.
 ********************************************************************/
int importPackage(char *data, Package **result) {
  Package *p = NULL;
  PackageEntry *e = NULL;
  PackageMetadata *metadata = NULL;
  MapNode *node = NULL;
  XMLParser *xml = NULL;
  XMLNode *current = NULL;
  char *path, *mimetype, *ispublic, *online, *type, *version, *bytes, *tplt, *relativeOrder;
  const char *name = NULL;
  int err = E_OK;

  if (data == NULL || strlen(data) <= 0) {
    return RESOURCEERROR;
  }

  p = initPackage();
  if (p == NULL)
    return RESOURCEERROR;

  if ((err = importXML(data, &xml)) != E_OK) {
    freePackage(&p);
    return err;
  }


  // verify root node
  if (xml->current == NULL) {
    freePackage(&p);
    freeXMLParser(&xml);
    return INCONSISTENTDATA;
  }

  if (strcasecmp(xml->current->name, "package") != 0) {
    freePackage(&p);
    freeXMLParser(&xml);
    return INCONSISTENTDATA;
  }

  moveToFirstChild(xml);
  if (strcasecmp(xml->current->name, "details") != 0) {
    freePackage(&p);
    freeXMLParser(&xml);
    return INCONSISTENTDATA;
  }
  moveToFirstChild(xml);
 
  // get the package details
  do {
    if (strcasecmp(xml->current->name, "created") == 0) {
      strncpy(p->header.date, xml->current->value, PACKAGE_STRING_LENGTH);
    } else if (strcasecmp(xml->current->name, "root") == 0) {
      strncpy(p->header.root, xml->current->value, PACKAGE_STRING_LENGTH);
    } else if (strcasecmp(xml->current->name, "crc") == 0) {
      strncpy(p->header.crc, xml->current->value, PACKAGE_STRING_LENGTH);
    }
  } while (moveToNextSibling(xml) == E_OK);

  moveToParent(xml); // details
  moveToNextSibling(xml); // objects

  if (moveToFirstChild(xml) != E_OK) { // object
    freePackage(&p);
    freeXMLParser(&xml);
    return INCONSISTENTDATA;
  }
  
  if (strcasecmp(xml->current->name, "object") != 0) {
    freePackage(&p);
    freeXMLParser(&xml);
    return INCONSISTENTDATA;
  }

  current = xml->current;
  while (current) {
    xml->current = current;
    moveToFirstChild(xml);

    path = NULL;
    mimetype = NULL;
    ispublic = NULL;
    online = NULL;
    type = NULL;
    tplt = NULL;
    relativeOrder = NULL;
    version = NULL;
    bytes = NULL;

    if ((e = initPackageEntry()) == NULL) {
      freeXMLParser(&xml);
      freePackage(&p);
      return RESOURCEERROR;
    }

    do {
      if (strcasecmp(xml->current->name, "path") == 0) {
        path =  xml->current->value;
      } else if (strcasecmp(xml->current->name, "mimetype") == 0) {
        mimetype = xml->current->value;
      } else if (strcasecmp(xml->current->name, "public") == 0) {
        ispublic = xml->current->value;
      } else if (strcasecmp(xml->current->name, "online") == 0) {
        online = xml->current->value;
      } else if (strcasecmp(xml->current->name, "type") == 0) {
        type = xml->current->value;
      } else if (strcasecmp(xml->current->name, "template") == 0) {
        tplt = xml->current->value;
      } else if (strcasecmp(xml->current->name, "relativeOrder") == 0) {
        relativeOrder = xml->current->value;
      } else if (strcasecmp(xml->current->name, "version") == 0) {
        version = xml->current->value;
      } else if (strcasecmp(xml->current->name, "data") == 0) {
        bytes = xml->current->value;
      } else if (strcasecmp(xml->current->name, "metadata") == 0) {
        getCurrentTagAttributeValue(xml, "name", NULL, &name);
        metadata = initPackageMetadata((char *)name, xml->current->value);
        node = initMapNode(metadata);
        insertMapValue(node, e->metadata);
      }
    } while (moveToNextSibling(xml) == E_OK);

    if (path == NULL || 
        mimetype == NULL || 
        ispublic == NULL || 
        online == NULL || 
        type == NULL || 
        tplt == NULL || 
        version == NULL || 
        bytes == NULL) {
      freeXMLParser(&xml);
      freePackage(&p);
      freePackageEntry(&e);
      return INCONSISTENTDATA;
    }

    snprintf(e->path, PACKAGE_STRING_LENGTH - 1, "%s", path);
    snprintf(e->mimeType, PACKAGE_STRING_LENGTH - 1, "%s", mimetype);
    e->isPublic = *ispublic;
    e->isOnline = *online;
    snprintf(e->type, PACKAGE_STRING_LENGTH - 1, "%s", type);
    snprintf(e->tplt, PACKAGE_STRING_LENGTH - 1, "%s", tplt);
    e->version = strtol(version, NULL, 10);
    e->relativeOrder = strtol(relativeOrder, NULL, 10);
    if ((err = base64Decode(bytes, (char **) &(e->data), &(e->length))) != E_OK) {
      freeXMLParser(&xml);
      freePackage(&p);
      freePackageEntry(&e);
      return err;
    }
    pushStack(p->files, e);

    current = current->rightSibling;
  }
  
  freeXMLParser(&xml);
  *result = p;
  return E_OK;
}


/*********************************************************************
 * exportPackage...
 *
 * This will take a package structure and write it into a buffer.
 ********************************************************************/
int exportPackage(Package *p, char **result) {
  char *data = NULL, boolean[2];
  int i = 0;
  PackageEntry *e = NULL;
  XMLParser *xml = NULL;
  char *numstr = NULL, *base64buf = NULL;
  int err = E_OK;
  MapNode *node = NULL;

  if (p == NULL)
    return RESOURCEERROR;
  
  if ((err = initXMLParser(&xml)) != E_OK) {
    return err;
  }

  if ((err = addChildNode(xml, strdup("package"), strdup(""), NULL)) != E_OK) {
    freeXMLParser(&xml);
    return err;
  }

  if ((err = addChildNode(xml, strdup("details"), strdup(""), NULL)) != E_OK) {
    freeXMLParser(&xml);
    return err;
  }
  moveToFirstChild(xml);

  int2Str(p->header.numfiles, &numstr);
  if ((err = addChildNode(xml, strdup("created"), strdup(p->header.date), NULL)) != E_OK) {
    freeXMLParser(&xml);
    return err;
  }
  if ((err = addChildNode(xml, strdup("root"), strdup(p->header.root), NULL)) != E_OK) {
    freeXMLParser(&xml);
    return err;
  }
  if ((err = addChildNode(xml, strdup("crc"), strdup(p->header.crc), NULL)) != E_OK) {
    freeXMLParser(&xml);
    return err;
  }
  moveToParent(xml);
  if ((err = addChildNode(xml, strdup("objects"), strdup(""), NULL)) != E_OK) {
    freeXMLParser(&xml);
    return err;
  }
  moveToFirstChild(xml);
  moveToNextSibling(xml);
    
  // now add each file from the package
  for (i = p->header.numfiles - 1; i >= 0; i--) {
    e = (PackageEntry *) sniffNStack(p->files, i); 
  
    if ((err = addChildNode(xml, strdup("object"), strdup(""), NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    moveToFirstChild(xml);
    while (moveToNextSibling(xml) == E_OK) ;

    if ((err = addChildNode(xml, strdup("path"), strdup(e->path), NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    if ((err = addChildNode(xml, strdup("mimetype"), strdup(e->mimeType), NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    boolean[1] = '\0';
    boolean[0] = e->isPublic;
    if ((err = addChildNode(xml, strdup("public"), strdup(boolean), NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    boolean[0] = e->isOnline;
    if ((err = addChildNode(xml, strdup("online"), strdup(boolean), NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    if ((err = addChildNode(xml, strdup("type"), strdup(e->type), NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    if ((err = addChildNode(xml, strdup("template"), strdup(e->tplt), NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    int2Str(e->version, &numstr);
    if ((err = addChildNode(xml, strdup("version"), numstr, NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    int2Str(e->relativeOrder, &numstr);
    if ((err = addChildNode(xml, strdup("relativeOrder"), numstr, NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }

    node = getFirstMapNode(e->metadata);
    while (node != NULL) {
      addChildNode(xml, strdup("metadata"), 
                        strdup(((PackageMetadata *)node->ele)->value), NULL);

      moveToFirstChild(xml);

      while (moveToNextSibling(xml) == E_OK);

      addAttribute(xml, strdup("name"), strdup(((PackageMetadata *)node->ele)->name), NULL);

      moveToParent(xml);
      
      node = getNextMapNode(node, e->metadata);
    }

    base64buf = NULL;
    if ((err = base64Encode(e->data, e->length, &base64buf)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    // add node to package: 
    if ((err = addChildNode(xml, strdup("data"), base64buf, NULL)) != E_OK) {
      freeXMLParser(&xml);
      return err;
    }
    
    moveToParent(xml);
  }
  
  if ((err = exportXML(xml, &data)) != E_OK) {
    freeXMLParser(&xml);
    return err;
  }

  freeXMLParser(&xml);

  *result = data;
  return E_OK;
}

/*********************************************************************
 * createPackageFromObject...
 *
 * This will create a package from the entire file structure below
 * and including a single objectID.
 ********************************************************************/
int createPackageFromObject(int objectID, Env *env, Package **result, void *dbconn) {
  Package *p = NULL;
  ObjectDetails *details = NULL;
  int retcode = E_OK, parentID = 0;

  p = initPackage();
  if (p == NULL) {
    return RESOURCEERROR;
  }

  if (userHasReadAccess(objectID, env, dbconn) != E_OK) {
    return ACCESSDENIED;
  }

  retcode = getObjectDetails(objectID, &details, dbconn);
  if (retcode != E_OK) {
    dhufree(p);
    return retcode;
  }

  parentID = strtol(details->parentID, NULL, 10);
  freeObjectDetails(details);

  if (parentID == -1) {
    p->header.root[0] = '\0';
  } else {
    retcode = getObjectDetails(parentID, &details, dbconn);
    if (retcode != E_OK) {
      dhufree(p);
      return retcode;
    }
    
    strncpy(p->header.root, details->path, PACKAGE_STRING_LENGTH);
    p->header.root[PACKAGE_STRING_LENGTH - 1] = '\0';
    freeObjectDetails(details);
  }

  addObjectToPackage(p, objectID, env, dbconn);

  updatePackageCRC(p);
  *result = p;
  return E_OK;
}
