/************************************************************************
* package.h
*
* Group Management functions.
************************************************************************/
#ifndef _PACKAGE_H
#define _PACKAGE_H

#ifdef __cplusplus
extern "C" {
#endif


#define PACKAGE_STRING_LENGTH 2048
#define PACKAGE_MAGIC_NUMBER 32451

typedef struct _PackageMetadata_ {
	char *name, *value;
} PackageMetadata;

typedef struct _PackageHeader_ {
	char root[PACKAGE_STRING_LENGTH];
	char date[PACKAGE_STRING_LENGTH];
	char crc[PACKAGE_STRING_LENGTH];
	int numfiles;
} PackageHeader;

typedef struct _PackageEntry_ {
	char path[PACKAGE_STRING_LENGTH];
	char mimeType[PACKAGE_STRING_LENGTH];
	char tplt[PACKAGE_STRING_LENGTH];
	char type[PACKAGE_STRING_LENGTH];
        char isPublic, isOnline;
        Map *metadata;
        int version;
        int relativeOrder;
	int length;
	void *data;
} PackageEntry;

typedef struct _Package_ {
	PackageHeader header;
	Stack *files;
} Package;

/*********************************************************************
* initPackage...
*
* Initialise a Package structure and
* fill it with default values.
*********************************************************************/
Package * initPackage(void);

/*********************************************************************
* initPackageMetadata...
*
* Initialise this struct.
*********************************************************************/
PackageMetadata *initPackageMetadata(char *name, char *value);

/*********************************************************************
* cmpPackageMetadata...
*
* Compare the names of these 2 metadata objects.
*********************************************************************/
int cmpPackageMetadata(void *a, void *b);

/*********************************************************************
* freePackageMetadata...
*
* Release the memory from this struct.
*********************************************************************/
void freePackageMetadata(void *a);

/*********************************************************************
* initPackageEntry...
*
* Initialise a PackageEntry structues and fill it with
* default values.
*********************************************************************/
PackageEntry * initPackageEntry(void);

/*********************************************************************
* freePackage...
*
* Release the memory from a package structure.
*********************************************************************/
void freePackage(Package **package);

/*********************************************************************
* addObjectToPackage...
*
* Add another content object to this package.
* This will recursively add all the content objects below this one as well.
*********************************************************************/
int addObjectToPackage(Package *p, int objectID, Env *env, void *dbconn);

/*********************************************************************
* packageCRCCheck...
*
* This will compute a md5 crc from all the files in this package
* and compare it with the crc stored in the header.
********************************************************************/
int packageCRCCheck(Package *p);

/*********************************************************************
* updatePackageCRC...
*
* This will compute a new md5 CRC from all the files in this package
* and store it in the header. This should be done before a package is
* exported so it can be verified before it is re-imported.
********************************************************************/
int updatePackageCRC(Package *p);

/*********************************************************************
 * importPackage...
 *
 * This unpack a memory buffer into a proper package structure.
 ********************************************************************/
int importPackage(char *data, Package **p);

/*********************************************************************
 * exportPackage...
 *
 * This will take a package structure and write it into a buffer.
 ********************************************************************/
int exportPackage(Package *p, char **result);

/*********************************************************************
 * createPackageFromObject...
 *
 * This will create a package from the entire file structure below
 * and including a single objectID.
 ********************************************************************/
int createPackageFromObject(int objectID, Env *env, Package **result, void *dbconn);

#ifdef __cplusplus
}
#endif


#endif // _PACKAGE_H
