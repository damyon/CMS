#include "errors.h"

/*******************************************************************************
* getErrorMesg...
*
* Returns the error message from this code.
*******************************************************************************/
char *getErrorMesg(int code) {
  switch (code) {
    case E_OK:
      return "No error.";
      break;
    case COMMANDLINEERROR:
      return "Invalid arguments were passed on the command line. Try --help for command line options.";
      break;
    case CONFIGFILEERROR:
      return "The configuration file contains a syntax error.";
      break;
    case PREMATURESHUTDOWN:
      return "The server has shut down prematurely.";
      break;
    case PREFIXNOTSET:
      return "The installation prefix was not set. Try --help for command line options.";
      break;
    case CONFIGFILEREADERROR:
      return "A file i/o error occurred while reading the config file. Please check to see if the config file exists.";
      break;
    case PORTNOTFREE:
      return "Another server as already bound to this port. Please select another port or shutdown the other server.";
      break;
    case INVALIDREQUEST:
      return "Recieved an invalid request.";
      break;
    case SERVERFATALERROR:
      return "The server encountered a fatal error and was not able to return the requested page.";
      break;
    case CONNECTERROR:
      return "Could not initiate a connection to the server.";
      break;
    case ISAFOLDER:
      return "The requested resource is a folder.";
      break;
    case INVALIDXPATH:
      return "The path provided did not exist.";
      break;
    case DBCONNECTERROR:
      return "Could not establish a connection to the database.";
      break;
    case ACCESSDENIED:
      return "You did not have sufficient privledges to perform the requested action.";
      break;
    case INVALIDREGEX:
      return "The supplied regular expression was not valid.";
      break;
    case REGEXNOMATCH:
      return "There was no match for the regex supplied.";
      break;
    case NODATAFOUND:
      return "There was no data found.";
      break;
    case SYNTAXERROR:
      return "A syntax error was found in the input stream.";
      break;
    case RESOURCEERROR:
      return "There was insufficient resources available to complete the request.";
      break;
    case NOMATCH:
      return "There is no match.";
      break;
    case PARTIALMATCH:
      return "If more data was supplied there may have been a match.";
      break;
    case TYPEMISMATCH:
      return "An attempt was made to store data in an invalid container.";
      break;
    case UNDEFINEDTOKEN:
      return "An undefined token was referenced.";
      break;
    case INVALIDOPERATOR:
      return "An invalid operator was used.";
      break;
    case DIVIDEBYZERO:
      return "A divide by zero error occurred.";
      break;
    case FUNCTIONUNDEFINED:
      return "The function called does not exist.";
      break;
    case INVALIDNUMARGS:
      return "The function was called with the wrong number of arguments.";
      break;
    case INACTIVEUSER:
      return "The user requested is inactive.";
      break;
    case FILEDELETEERROR:
      return "An i/o error occured while deleting the document.";
      break;
    case FILEWRITEERROR:
      return "An i/o error occured while writing the document.";
      break;
    case FILEREADERROR:
      return "An i/o error occured while reading the document.";
      break;
    case NOTONLINE:
      return "The document is not online.";
      break;
    case NOFILEINREQUEST:
      return "There was no file in the request.";
      break;
    case INCONSISTENTDATA:
      return "During the request inconsistent data was encountered. The request has been aborted.";
      break;
    case DUPLICATEUSERNAME:
      return "The username specified is in use. Please try again with a different user name.";
      break;
    case DUPLICATEGROUPNAME:
      return "The group name specified is in use. Please try again with a different group name.";
      break;
    case NOTIMPLEMENTED:
      return "The requested feature has not yet been implemented. Please try again with a later version of this software.";
      break;
    case COMPRESSERROR:
      return "An error occurred while compresssing the output stream.";
      break;
    case OBJECTLOCKED:
      return "The document is not available because it is locked.";
      break;
    case INVALIDARGUMENTS:
      return "Invalid arguments.";
      break;
    case MISSINGHTTPHEADERS:
      return "Missing HTTP headers.";
      break;
    case MISSINGHTTPBODY:
      return "Missing HTTP body.";
      break;
    case NOMOREELEMENTS:
      return "No more elements.";
      break;
    case INVALIDXML:
      return "Invalid XML.";
      break;
    case INVALIDXMLRPC:
      return "Invalid XMLRPC.";
      break;
    case FUNCTIONNOTSUPPORTED:
      return "Function not supported.";
      break;
    case FTPCONNECTIONERROR:
      return "An error occurred whilst connected to the ftp server.";
      break;
    case ARRAYINDEXOUTOFBOUNDS:
      return "The specified array index was out of range.";
      break;
    case FUNCTIONRETURN:
      return "The called function has returned a value.";
      break;
    case INVALIDLICENSEKEY:
      return "The installed license key is invalid. Enter the correct key in the config file at '/etc/dhufish.conf' and try again.";
      break;
    case USERLICENSELIMIT:
      return "Your license does not permit you to create any more user accounts on this server.";
      break;
    case USERISAMEMBER:
      return "The user is already a member of the group.";
      break;
    case FILEEXISTS:
      return "The file already exists.";
      break;
    case USERDETAILSREADONLY:
      return "The user details cannot be changed because they are managed by an external system (eg LDAP).";
      break;
    case LOCKFILETIMEOUT:
      return "A timeout occurred while waiting for a lock file.";
      break;
    case E_PARSE_ERROR:
      return "Parse error.";
      break;
    default:
      return "Invalid Error Code";
      break;
  }
}
