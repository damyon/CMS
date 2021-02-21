#include "ipc.h"
#include "env.h"
#include "errors.h"
#include <time.h>
#include "strings.h"
#include "cgicommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef WIN32
#include "win32.h"
#include <windows.h>
#endif

int glob_portnum;
char *glob_ipaddress;
char *glob_errorpageurl;

int getPortNum() {
	return glob_portnum;
}

char * getIPAddress() {
	return glob_ipaddress;
}

char * getErrorPageURL() {
	return glob_errorpageurl;
}

/*************************************************************
* authenticateWebDavConnection...
*
* Perform a handshake validation.
*************************************************************/
int authenticateWebDavConnection(int sockfd) {
  char *authstr = NULL;

  if (readData(&authstr, 8, sockfd) != 0)
    return INVALIDREQUEST;

  if (strcasecmp(authstr, AUTHSTR) != 0) {
    closeConnection(sockfd);
    return INVALIDREQUEST;
  }

  sendData(DAVSTR, 8, sockfd);

  free(authstr);

  return 0;
}

/*************************************************************
 * authenticateCGIConnection...
 *
 * Perform a handshake validation.
 *************************************************************/
int authenticateCGIConnection(int sockfd) {
  char *authstr = NULL;

  if (readData(&authstr, 8, sockfd) != 0)
    return INVALIDREQUEST;

  if (strcasecmp(authstr, AUTHSTR) != 0) {
    closeConnection(sockfd);
    return INVALIDREQUEST;
  }

  sendData((void *)STDSTR, 8, sockfd);

  free(authstr);

  return 0;
}

/****************************************************
* getCGIConfigIntValue...
*
* Returns the integer value of a config file setting.
****************************************************/
long int getCGIConfigIntValue(char *line, char *setting) {
  char *ptr = line;
  while (isspace(*ptr)) ptr++;

  ptr += strlen(setting);
  while (isspace(*ptr) || *ptr == '=') ptr++;

  return strtol(ptr, NULL, 10);
}


/****************************************************
* getCGIConfigStringValue...
*
* Returns the string value of a config file setting.
****************************************************/
char *getCGIConfigStringValue(char *line, char *setting) {
  char *ptr = line, *eptr = NULL, c = 0, *retstr = NULL;
  while (isspace(*ptr)) ptr++;

  ptr += strlen(setting);
  while (isspace(*ptr) || *ptr == '=') ptr++;

  eptr = ptr;
  while ((!isspace(*eptr)) && (*eptr != CNULL)) eptr++;
  c = *eptr;
  *eptr = CNULL;
  retstr = strdup(ptr);
  *eptr = c;

  return retstr;
}

/****************************************************
* isCGIConfigSetting...
*
* Returns whether this line matches this config setting.
****************************************************/
int isCGIConfigSetting(char *line, char *setting) {
  char *ptr = NULL;

  ptr = line;
  while (isspace(*ptr)) ptr++;

  return (strncasecmp(ptr, setting, strlen(setting)) == 0);
}


#ifndef WIN32
/***********************************************************
* loadCGIConfigSettings...
*
* Load the config file settings.
***********************************************************/
int loadCGIConfigSettings() {
  char *contents = NULL, *line = NULL, *ptr = NULL;

  contents = readAsciiFile(CGICONFIGFILE);
  
  glob_ipaddress = "127.0.0.1";
  glob_portnum = 500;
  glob_errorpageurl = "/cms/error.html";

  if (contents == NULL)
    return CONFIGFILEREADERROR;

  ptr = contents;
  while ((line = getNextLineFromStream(&ptr)) != NULL) {
    if (isCGIConfigSetting(line, SERVERIPADDRESS)) {
      glob_ipaddress = getCGIConfigStringValue(line, SERVERIPADDRESS);
    } else if (isCGIConfigSetting(line, ERRORPAGEURL)) {
      glob_errorpageurl = getCGIConfigStringValue(line, ERRORPAGEURL);
    } else if (isCGIConfigSetting(line, SERVERPORT)) {
      glob_portnum = getCGIConfigIntValue(line, SERVERPORT);
    }
  }
  n_free(contents);
  return E_OK;
}

#else

#define DHUFISHREGBASEKEY		  "SOFTWARE\\Epiction\\Dhufish CMS"
#define REGSERVERIPADDRESS           "SERVERIPADDRESS"
#define REGSERVERPORT                "PORT"
#define REGERRORPAGEURL              "ERRORPAGEURL"

int getCGIIntRegistryValue(char *keyName) {
  DWORD dwSize, dwType;
  HKEY Regentry;
  char tmpValue[1024];

  if (RegOpenKey(HKEY_LOCAL_MACHINE, DHUFISHREGBASEKEY, &Regentry) != ERROR_SUCCESS)
    return -1;

  RegQueryValueEx(Regentry, keyName, 0, &dwType, NULL, &dwSize);

  if (RegQueryValueEx(Regentry, keyName, 0, &dwType, 
                        (unsigned char*)&tmpValue, &dwSize)!=ERROR_SUCCESS) {
    RegCloseKey(Regentry);
    return -1;
  }
 
  RegCloseKey(Regentry);

  return strtol(tmpValue, NULL, 10);
}

int getCGIBoolRegistryValue(char *keyName) {
  DWORD dwSize, dwType;
  HKEY Regentry;
  char tmpValue[1024];

  if (RegOpenKey(HKEY_LOCAL_MACHINE, DHUFISHREGBASEKEY, &Regentry) != ERROR_SUCCESS)
    return 0;

  RegQueryValueEx(Regentry, keyName, 0, &dwType, NULL, &dwSize);

  if (RegQueryValueEx(Regentry, keyName, 0, &dwType, 
                        (unsigned char*)&tmpValue, &dwSize)!=ERROR_SUCCESS) {
    RegCloseKey(Regentry);
    return 0;
  }
 
  RegCloseKey(Regentry);

  if (tolower(tmpValue[0]) == 'y')
    return 1;
  return 0;
}

char *getCGIStringRegistryValue(char *keyName) {
  DWORD dwSize, dwType;
  HKEY Regentry;
  char tmpValue[1024];
  int err = 0;

  if ((err = RegOpenKey(HKEY_LOCAL_MACHINE, DHUFISHREGBASEKEY, &Regentry)) != ERROR_SUCCESS) {
    return strdup("");
  }

  RegQueryValueEx(Regentry, keyName, 0, &dwType, NULL, &dwSize);

  if (RegQueryValueEx(Regentry, keyName, 0, &dwType, 
                        (unsigned char*)&tmpValue, &dwSize)!=ERROR_SUCCESS) {
    RegCloseKey(Regentry);
    return strdup("");
  }
 
  RegCloseKey(Regentry);

  return strdup(tmpValue);
}


/***********************************************************
* loadCGIConfigSettings...
*
* Load the config file settings.
***********************************************************/
int loadCGIConfigSettings() {
  glob_ipaddress = getCGIStringRegistryValue(REGSERVERIPADDRESS);
  glob_errorpageurl = getCGIStringRegistryValue(REGERRORPAGEURL);
  glob_portnum = getCGIIntRegistryValue(REGSERVERPORT);
  return E_OK;
}
#endif
