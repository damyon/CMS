#ifndef CGI_COMMON_H
#define CGI_COMMON_H

#ifdef MODULE
#define CGICONFIGFILE   "/etc/dhufish_mod.conf"
#else
#define CGICONFIGFILE   ".htconf"
#endif
#define SERVERIPADDRESS "serverIPAddress"
#define SERVERPORT      "serverPort"
#define ERRORPAGEURL    "errorPageURL"
#define AUTHSTR        "CMS INIT"
#define STDSTR         "STD MTHD"
#define DAVSTR         "DAV MTHD"

int getPortNum();
char * getIPAddress();
char * getErrorPageURL();

/*************************************************************
* authenticateWebDavConnection...
*
* Perform a handshake validation.
*************************************************************/
int authenticateWebDavConnection(int sockfd);

/*************************************************************
 * authenticateCGIConnection...
 *
 * Perform a handshake validation.
 *************************************************************/
int authenticateCGIConnection(int sockfd);

/****************************************************
* getCGIConfigIntValue...
*
* Returns the integer value of a config file setting.
****************************************************/
long int getCGIConfigIntValue(char *line, char *setting);

/****************************************************
* getCGIConfigStringValue...
*
* Returns the string value of a config file setting.
****************************************************/
char *getCGIConfigStringValue(char *line, char *setting);

/****************************************************
* isCGIConfigSetting...
*
* Returns whether this line matches this config setting.
****************************************************/
int isCGIConfigSetting(char *line, char *setting);


/***********************************************************
* loadCGIConfigSettings...
*
* Load the config file settings.
***********************************************************/
int loadCGIConfigSettings();

#endif
