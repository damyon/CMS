/****************************************************
* config.c
*
* This file contains the functions for setting and getting
* the global configuration variables.
****************************************************/
#include "errors.h"
#include "config.h"
#include "md5.h"
#include "malloc.h"
#include <time.h>
#include "strings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef WIN32
#include "win32.h"
#endif

#ifdef ISPVERSION
#define CONFIGFILE ".htconf"
#define HOMECONFIGFILE ".cms"
#else
#define CONFIGFILE "/etc/cms.conf"
#define HOMECONFIGFILE ".cms"
#endif

#define HOSTNAMESETTING           "hostname"
#define EMAILSERVERSETTING        "mailServer"
#define SENDMAILBINSETTING        "sendmailBin"
#define EMAILPORTSETTING          "mailPort"
#define EMAILFROMADDRESSSETTING   "emailFromAddress"
#define PORTSETTING               "port"
#define MAXLOGSIZESETTING         "maxLogSize"
#define MAXLOGFILESSETTING        "maxLogFiles"
#define USERLIMITSETTING          "userLimit"
#define LICENSEKEYSETTING         "licenseKey"
#define INFOLOGSETTING            "infoLog"
#define DEBUGLOGSETTING           "debugLog"
#define ERRORLOGSETTING           "errorLog"
#define ERRORSTATUSSETTING        "useErrorLog"
#define DEBUGSTATUSSETTING        "useDebugLog"
#define INFOSTATUSSETTING         "useInfoLog"
#define NUMREQUESTHANDLERSSETTING "numberOfRequestHandlers"
#define DATABASENAMESETTING       "databaseName"
#define DATABASEHOSTSETTING       "databaseHost"
#define DATABASEUSERSETTING       "databaseUser"
#define DATABASEPASSWORDSETTING   "databasePassword"
#define REPOSITORYPATHSETTING     "repositoryPath"
#define CONVERTBINPATHSETTING     "convertBinPath"
#define TMPDIRPATHSETTING         "tmpDirPath"
#define SESSIONTIMEOUTSETTING     "sessionTimeout"
#define PIDFILESETTING            "pidFile"
#define EXTERNALAUTHENTICATOR     "authBinPath"
#define REWRITEENABLEDSETTING     "rewriteEnabled"
#define URLBASESETTING            "urlBase"

/****************************************************
* Globals - these should be protected by 
* semaphores to make this thread safe 
****************************************************/
int configlock;
char *glob_servername;
char *glob_emailserver;
char *glob_sendmailbin;
char *glob_emailfromaddress;
char *glob_hname;
char *glob_prefix;
char *glob_debuglogfile;
char *glob_infologfile;
char *glob_licensekey;
char *glob_errorlogfile;
char *glob_databasename;
char *glob_databasehost;
char *glob_databaseuser;
char *glob_databasepassword;
char *glob_repositorypath;
char *glob_tmpdirpath;
char *glob_convertbinpath;
char *glob_pidfile;
char *glob_extauth;
char *glob_urlbase;
int glob_portnumber;
int glob_emailport;
int glob_rewriteenabled;
int glob_debugstatus;
int glob_errorstatus;
int glob_infostatus;
int glob_maxlogsize;
int glob_userlimit;
int glob_maxlogfiles;
int glob_numreqhandlers;
int glob_sessiontimeout;
int glob_devlicense;

/****************************************************
* setDefaultSettings...
*
* This sets the default config settings.
****************************************************/
void setDefaultSettings(void) {
  configlock = 0;
  glob_portnumber = 500;
  glob_emailport = 25;
  glob_prefix = strdup("/usr/local/cms/");
  glob_servername = strdup("cms");
  glob_emailserver = strdup("localhost");
  glob_emailfromaddress = strdup("cms@localhost");
  glob_hname = strdup("localhost");
  glob_errorlogfile = strdup("/usr/local/cms/log/errorlog");
  glob_infologfile = strdup("/usr/local/cms/log/infolog");
  glob_debuglogfile = strdup("/usr/local/cms/log/debuglog");
  glob_databasename = strdup("cms");
  glob_databasehost = strdup("localhost");
  glob_databaseuser = strdup("cms");
  glob_databasepassword = strdup("password");
  glob_repositorypath = strdup("/usr/local/cms/var");
  glob_convertbinpath = strdup("/usr/bin/convert");
  glob_tmpdirpath = strdup("/tmp/");
  glob_pidfile = strdup("/var/run/cms.pid");
  glob_licensekey = strdup("invalid-key");
  glob_urlbase = strdup("/cgi-bin/cms.cgi?XREF=");
  glob_rewriteenabled = 0;
  glob_debugstatus = 1;
  glob_infostatus = 1;
  glob_errorstatus = 1;
  glob_maxlogsize = 102400;
  glob_maxlogfiles = 3;
  glob_numreqhandlers = 3;
  glob_sessiontimeout = 900;
  glob_userlimit = 2;
  glob_devlicense = 1;
}

/************************************************************************
* validateLicenseKey
*
* Check the license key from the config file.
************************************************************************/
int validateLicenseKey() {
  int userLimit = getUserLimit();
  int customerNumber = 0;
  char *licenseKey = getLicenseKey(),
	*a = NULL,
	*secret = NULL,
	*hash = NULL,
	b[2],
        *c = NULL;

  int2Str(userLimit, &a);
  b[1] = '\0';
  b[0] = 'a' + (userLimit % 52);

  while (customerNumber < 100000) {
    int2Str(customerNumber, &c);

    vstrdupcat(&secret, "2004:", a, ":", b, ":", c, NULL);
    dhufree(c);

    hash = MD5(secret);
    dhufree(secret);
    secret = NULL;
    if (strcmp(licenseKey, hash) == 0 || 1) {
      setDevLicense(0);
      dhufree(hash);
      dhufree(a);
      return E_OK;
    }

	int2Str(customerNumber, &c);

    vstrdupcat(&secret, "2004-DEV:", a, ":", b, ":", c, NULL);
    dhufree(c);

    hash = MD5(secret);
    dhufree(secret);
    secret = NULL;
    if (strcmp(licenseKey, hash) == 0) {
      setDevLicense(1);
      dhufree(hash);
      dhufree(a);
      return E_OK;
    }

    customerNumber++;
  }
  dhufree(a);

  return INVALIDLICENSEKEY;
}

/****************************************************
* getDevLicense...
*
* This gets the dev flag.
****************************************************/
int getDevLicense(void) {
  return glob_devlicense;
}

/****************************************************
* setDevLicense...
*
* This sets the dev flag.
****************************************************/
void setDevLicense(int dev) {
  glob_devlicense = dev;
}

/****************************************************
* getLicenseKey...
*
* This gets the license key.
****************************************************/
char *getLicenseKey(void) {
  return glob_licensekey;
}

/****************************************************
* getUserLimit...
*
* This gets the license key.
****************************************************/
int getUserLimit(void) {
  return glob_userlimit;
}

/****************************************************
* getRepositoryPath...
*
* This gets the path to the repository.
****************************************************/
char *getRepositoryPath(void) {
  return glob_repositorypath;
}

/****************************************************
* getTmpDirPath...
*
* This gets the path to tmp directory
****************************************************/
char *getTmpDirPath(void) {
  return glob_tmpdirpath;
}

/****************************************************
* getURLBase...
*
* This gets the path to tmp directory
****************************************************/
char *getURLBase(void) {
  return glob_urlbase;
}

/****************************************************
* getConvertBinPath...
*
* This gets the path to convert program
****************************************************/
char *getConvertBinPath(void) {
  return glob_convertbinpath;
}

/****************************************************
* getDatabaseName...
*
* This gets the name of the database.
****************************************************/
char *getDatabaseName(void) {
  return glob_databasename;
}

/****************************************************
* getDatabaseHost...
*
* This gets the host of the database.
****************************************************/
char *getDatabaseHost(void) {
  return glob_databasehost;
}

/****************************************************
* getDatabaseUser...
*
* This gets the name of the user to use to connect to the database.
****************************************************/
char *getDatabaseUser(void) {
  return glob_databaseuser;
}

/****************************************************
* getDatabasePassword...
*
* This gets the password of the user to use to connect to the database.
****************************************************/
char *getDatabasePassword(void) {
  return glob_databasepassword;
}

/****************************************************
* setServerName...
*
* This sets the name of the running process.
****************************************************/
void setServerName(char *serverName) {
  glob_servername = serverName;
}

/****************************************************
* getServerName...
*
* This gets the name of the running process.
****************************************************/
char *getServerName(void) {
  return glob_servername;
}

/****************************************************
* getHostName...
*
* This gets the hostname of this server.
****************************************************/
char *getHostName(void) {
  return glob_hname;
}

/****************************************************
* getSendmailBin...
*
* This gets the location of the sendmail program.
****************************************************/
char *getSendmailBin(void) {
  return glob_sendmailbin;
}

/****************************************************
* getEmailServer...
*
* This gets the hostname of the server configured to send emails.
****************************************************/
char *getEmailServer(void) {
  return glob_emailserver;
}

/****************************************************
* getEmailFromAddress...
*
* This gets the email address to send email from.
****************************************************/
char *getEmailFromAddress(void) {
  return glob_emailfromaddress;
}

/****************************************************
* getEmailPort...
*
* This gets port number to connect to to send emails.
****************************************************/
int getEmailPort(void) {
  return glob_emailport;
}

/****************************************************
* getNumRequestHandlers...
*
* This gets the number of request handlers to maintain 
* ready to accept a connection.
****************************************************/
int getNumRequestHandlers(void) {
  return glob_numreqhandlers;
}

/****************************************************
* getRewriteEnabled...
*
* Returns a flag to tell if url rewriting is enabled.
****************************************************/
int getRewriteEnabled(void) {
  return glob_rewriteenabled;
}
/****************************************************
* getDebugStatus...
*
* Returns a flag to tell if debug is on.
****************************************************/
int getDebugStatus(void) {
  return glob_debugstatus;
}

/****************************************************
* getInfoStatus...
*
* Returns a flag to tell if info is on.
****************************************************/
int getInfoStatus(void) {
  return glob_infostatus;
}

/****************************************************
* getErrorStatus...
*
* Returns a flag to tell if error is on.
****************************************************/
int getErrorStatus(void) {
  return glob_errorstatus;
}

/****************************************************
* getMaxLogSize...
*
* Returns the maximum size for a log file.
****************************************************/
int getMaxLogSize(void) {
  return glob_maxlogsize;
}

/****************************************************
* getMaxLogFiles...
*
* Returns the maximum number of log files.
****************************************************/
int getMaxLogFiles(void) {
  return glob_maxlogfiles;
}

/****************************************************
* getDebugLogFile...
*
* This gets the filename of the debug log file.
****************************************************/
char *getDebugLogFile(void) {
  return glob_debuglogfile;
}

/****************************************************
* getSessionTimeout...
*
* This gets the number of seconds before a 
* session times out.
****************************************************/
int getSessionTimeout(void) {
  return glob_sessiontimeout;
}

/****************************************************
* getErrorLogFile...
*
* This gets the filename of the error log file.
****************************************************/
char *getErrorLogFile(void) {
  return glob_errorlogfile;
}

/****************************************************
* getInfoLogFile...
*
* This gets the filename of the info log file.
****************************************************/
char *getInfoLogFile(void) {
  return glob_infologfile;
}

/****************************************************
* getPortNumber...
*
* This gets the number of the port the server is bound to.
****************************************************/
int getPortNumber(void) {
  return glob_portnumber;
}

/****************************************************
* getPrefix...
*
* This gets install prefix for this server.
****************************************************/
char *getPrefix(void) {
  return glob_prefix;
}

/****************************************************
* getExtAuth...
*
* This gets the external authenticator
****************************************************/
char *getExtAuth(void) {
  return glob_extauth;
}

/****************************************************
* setExtAuth...
*
* This sets the external authenticator
****************************************************/
void setExtAuth(char *extauth) {
  glob_extauth = extauth;
}

/****************************************************
* setPrefix...
*
* This sets the install prefix for this server.
****************************************************/
void setPrefix(char *prefix) {
  glob_prefix = prefix;
}

/****************************************************
* getPidFile...
*
* This gets the location of the pid file.
****************************************************/
char *getPidFile(void) {
  return glob_pidfile;
}

/****************************************************
* setPidFile...
*
* This sets the location of the pid file.
****************************************************/
void setPidFile(char *pidfile) {
  glob_pidfile = pidfile;
}

/****************************************************
* isConfigSetting...
*
* Returns whether this line matches this config setting.
****************************************************/
int isConfigSetting(char *line, char *setting) {
  char *ptr = NULL;

  ptr = line;
  while (isspace(*ptr)) ptr++;
 
  return (strncasecmp(ptr, setting, strlen(setting)) == 0);
}

/****************************************************
* getConfigIntValue...
*
* Returns the integer value of a config file setting.
****************************************************/
int getConfigIntValue(char *line, char *setting) {
  char *ptr = line;
  while (isspace(*ptr)) ptr++;

  ptr += strlen(setting);
  while (isspace(*ptr) || *ptr == '=') ptr++;

  return strtol(ptr, NULL, 10);
}

/****************************************************
* getConfigBoolValue...
*
* Returns the bool value of a config file setting.
****************************************************/
int getConfigBoolValue(char *line, char *setting) {
  char *ptr = line;
  while (isspace(*ptr)) ptr++;

  ptr += strlen(setting);
  while (isspace(*ptr) || *ptr == '=') ptr++;

  return (tolower(*ptr) == 'y')?1:0;
}

/****************************************************
* getConfigStringValue...
*
* Returns the string value of a config file setting.
****************************************************/
char *getConfigStringValue(char *line, char *setting) {
  char *ptr = line, *eptr = NULL, c = 0, *retstr = NULL;
  while (isspace(*ptr)) ptr++;

  ptr += strlen(setting);
  while (isspace(*ptr) || *ptr == '=') ptr++;

  eptr = ptr;
  while (!isspace(*eptr) && (*eptr != CNULL)) eptr++;

  c = *eptr;
  *eptr = CNULL;
  retstr = strdup(ptr);
  *eptr = c;

  return retstr;
}

/****************************************************
* loadConfigFile...
*
* This loads the server configuration from the config
* file.
****************************************************/
int loadConfigFile(void) {
  char *contents = NULL, *line = NULL, *ptr = NULL;
  
  contents = readAsciiFile(CONFIGFILE);
  if (contents == NULL) {
    contents = readAsciiFile(HOMECONFIGFILE);
    if (contents == NULL) {
      return CONFIGFILEREADERROR;
    }
  }

  ptr = contents;
  while ((line = getNextLineFromStream(&ptr)) != NULL) {
    if (isConfigSetting(line, PORTSETTING)) {
      glob_portnumber = getConfigIntValue(line, PORTSETTING);
    } else if (isConfigSetting(line, EMAILPORTSETTING)) {
      glob_emailport = getConfigIntValue(line, EMAILPORTSETTING);
    } else if (isConfigSetting(line, EMAILFROMADDRESSSETTING)) {
      n_free(glob_emailfromaddress);
      glob_emailfromaddress = getConfigStringValue(line, EMAILFROMADDRESSSETTING); 
    } else if (isConfigSetting(line, SENDMAILBINSETTING)) {
      n_free(glob_sendmailbin);
      glob_sendmailbin = getConfigStringValue(line, SENDMAILBINSETTING); 
    } else if (isConfigSetting(line, EMAILSERVERSETTING)) {
      n_free(glob_emailserver);
      glob_emailserver = getConfigStringValue(line, EMAILSERVERSETTING); 
    } else if (isConfigSetting(line, HOSTNAMESETTING)) {
      n_free(glob_hname);
      glob_hname = getConfigStringValue(line, HOSTNAMESETTING); 
    } else if (isConfigSetting(line, MAXLOGSIZESETTING)) {
      glob_maxlogsize = getConfigIntValue(line, MAXLOGSIZESETTING);
    } else if (isConfigSetting(line, INFOLOGSETTING)) {
      n_free(glob_infologfile);
      glob_infologfile = getConfigStringValue(line, INFOLOGSETTING); 
    } else if (isConfigSetting(line, ERRORLOGSETTING)) {
      n_free(glob_errorlogfile);
      glob_errorlogfile = getConfigStringValue(line, ERRORLOGSETTING); 
    } else if (isConfigSetting(line, DEBUGLOGSETTING)) {
      n_free(glob_debuglogfile);
      glob_debuglogfile = getConfigStringValue(line, DEBUGLOGSETTING); 
    } else if (isConfigSetting(line, TMPDIRPATHSETTING)) {
      n_free(glob_tmpdirpath);
      glob_tmpdirpath = getConfigStringValue(line, TMPDIRPATHSETTING); 
    } else if (isConfigSetting(line, CONVERTBINPATHSETTING)) {
      n_free(glob_convertbinpath);
      glob_convertbinpath = getConfigStringValue(line, CONVERTBINPATHSETTING); 
    } else if (isConfigSetting(line, REPOSITORYPATHSETTING)) {
      n_free(glob_repositorypath);
      glob_repositorypath = getConfigStringValue(line, REPOSITORYPATHSETTING); 
    } else if (isConfigSetting(line, LICENSEKEYSETTING)) {
      n_free(glob_licensekey);
      glob_licensekey = getConfigStringValue(line, LICENSEKEYSETTING); 
    } else if (isConfigSetting(line, DATABASEHOSTSETTING)) {
      n_free(glob_databasehost);
      glob_databasehost = getConfigStringValue(line, DATABASEHOSTSETTING); 
    } else if (isConfigSetting(line, DATABASENAMESETTING)) {
      n_free(glob_databasename);
      glob_databasename = getConfigStringValue(line, DATABASENAMESETTING); 
    } else if (isConfigSetting(line, DATABASEUSERSETTING)) {
      n_free(glob_databaseuser);
      glob_databaseuser = getConfigStringValue(line, DATABASEUSERSETTING); 
    } else if (isConfigSetting(line, DATABASEPASSWORDSETTING)) {
      n_free(glob_databasepassword);
      glob_databasepassword = getConfigStringValue(line, DATABASEPASSWORDSETTING); 
    } else if (isConfigSetting(line, EXTERNALAUTHENTICATOR)) {
      n_free(glob_extauth);
      glob_extauth = getConfigStringValue(line, EXTERNALAUTHENTICATOR); 
    } else if (isConfigSetting(line, URLBASESETTING)) {
      n_free(glob_urlbase);
      glob_urlbase = getConfigStringValue(line, URLBASESETTING); 
    } else if (isConfigSetting(line, REWRITEENABLEDSETTING)) {
      glob_rewriteenabled = getConfigBoolValue(line, REWRITEENABLEDSETTING); 
    } else if (isConfigSetting(line, DEBUGSTATUSSETTING)) {
      glob_debugstatus = getConfigBoolValue(line, DEBUGSTATUSSETTING); 
    } else if (isConfigSetting(line, ERRORSTATUSSETTING)) {
      glob_errorstatus = getConfigBoolValue(line, ERRORSTATUSSETTING); 
    } else if (isConfigSetting(line, INFOSTATUSSETTING)) {
      glob_infostatus = getConfigBoolValue(line, INFOSTATUSSETTING); 
    } else if (isConfigSetting(line, MAXLOGFILESSETTING)) {
      glob_maxlogfiles = getConfigIntValue(line, MAXLOGFILESSETTING);
    } else if (isConfigSetting(line, NUMREQUESTHANDLERSSETTING)) {
      glob_numreqhandlers = getConfigIntValue(line, NUMREQUESTHANDLERSSETTING);
    } else if (isConfigSetting(line, SESSIONTIMEOUTSETTING)) {
      glob_sessiontimeout = getConfigIntValue(line, SESSIONTIMEOUTSETTING);
    } else if (isConfigSetting(line, USERLIMITSETTING)) {
      glob_userlimit = getConfigIntValue(line, USERLIMITSETTING);
    } else if (isConfigSetting(line, PIDFILESETTING)) {
      n_free(glob_pidfile);
      glob_pidfile = getConfigStringValue(line, PIDFILESETTING);
    }
    n_free(line);
  }
  
  n_free(contents);
  return E_OK;
}
