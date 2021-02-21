/********************************************************
* This file contains the functions to access the global 
* variables in the server configuration
********************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

void setDevLicense(int dev);

int getDevLicense(void);

char *getPrefix(void);

char *getPidFile(void);

char *getServerName(void);

char *getEmailServer(void);

char *getSendmailBin(void);

char *getEmailFromAddress(void);

char *getHostName(void);

int getEmailPort(void);

int getUserLimit(void);

char *getLicenseKey(void);

char *getDatabaseName(void);

char *getDatabaseHost(void);

char *getDatabaseUser(void);

char *getDatabasePassword(void);
 
int getPortNumber(void);

int getMaxLogSize(void);

int getMaxLogFiles(void);

void setPrefix(char *prefix);

void setServerName(char *name);

int loadConfigFile(void);

int reloadConfigFile(void);

void setDefaultSettings(void);

int getDebugStatus(void);

char *getDebugLogFile(void);

int getErrorStatus(void);

char *getErrorLogFile(void);

int getInfoStatus(void);

char *getInfoLogFile(void);

int getNumRequestHandlers(void);

char *getRepositoryPath(void);

char *getTmpDirPath(void);

char *getConvertBinPath(void);

char *getExtAuth(void);

int getSessionTimeout(void);

int isConfigSetting(char *line, char *setting);

int getConfigIntValue(char *line, char *setting);

int getConfigBoolValue(char *line, char *setting);

char *getConfigStringValue(char *line, char *setting);

int getRewriteEnabled(void);

char * getURLBase(void);

int validateLicenseKey(void);

#ifdef WIN32

char *getServerBinPath(void);

void setServerBinPath(char *serverBinPath);

#endif

#ifdef __cplusplus
}
#endif


#endif
