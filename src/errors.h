#ifndef _EPICTION_ERRORS_H
#define _EPICTION_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif


enum EPICTION_ERRORS {
             E_OK = 0,
             COMMANDLINEERROR = 1,
             CONFIGFILEERROR = 2,
             PREMATURESHUTDOWN = 3,
             PREFIXNOTSET = 4,
             CONFIGFILEREADERROR = 5,
             PORTNOTFREE = 6,
             INVALIDREQUEST = 7,
             SERVERFATALERROR = 8,
             CONNECTERROR = 9,
             INVALIDXPATH = 10,
             DBCONNECTERROR = 11,
             ACCESSDENIED = 12,
             FILEREADERROR = 13,
             INVALIDREGEX = 14,
             REGEXNOMATCH = 15,
             NODATAFOUND = 16,
             SYNTAXERROR = 17,
             RESOURCEERROR = 18,
             NOMATCH = 19,
             PARTIALMATCH = 20,
             TYPEMISMATCH = 21,
             UNDEFINEDTOKEN = 22,
             DIVIDEBYZERO = 23,
             INVALIDOPERATOR = 24,
             FUNCTIONUNDEFINED = 25,
             INVALIDNUMARGS = 26,
             INACTIVEUSER = 27,
             FILEWRITEERROR = 28,
             NOTONLINE = 29,
             NOFILEINREQUEST = 30,
             INCONSISTENTDATA = 31,
             DUPLICATEUSERNAME = 32,
             NOTIMPLEMENTED = 33,
             DUPLICATEGROUPNAME = 34,
             COMPRESSERROR = 35,
             OBJECTLOCKED = 36,
             INVALIDARGUMENTS = 37,
             MISSINGHTTPHEADERS = 38,
             MISSINGHTTPBODY = 39,
             NOMOREELEMENTS = 40,
             INVALIDXML = 41,
             INVALIDXMLRPC = 42,
             FUNCTIONNOTSUPPORTED = 43,
             FILEDELETEERROR = 44,
             FTPCONNECTIONERROR = 45,
             ARRAYINDEXOUTOFBOUNDS = 46,
             FUNCTIONRETURN = 47,
             INVALIDLICENSEKEY = 48,
             USERLICENSELIMIT = 49,
             ISAFOLDER = 50,
             USERISAMEMBER = 51,
             FILEEXISTS = 52,
             USERDETAILSREADONLY = 53,
             LOCKFILETIMEOUT = 54,
	     E_PARSE_ERROR = 55,
	     E_FUNCTION_RETURN = 56
             };

/*******************************************************************************
* getErrorMesg...
*
* Returns the error message from this code.
*******************************************************************************/
char *getErrorMesg(int code);

#ifdef __cplusplus
}
#endif


#endif
