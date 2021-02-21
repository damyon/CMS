#include "xml.h"
#include "webdav.h"
#include "errors.h"
#include "strings.h"
#include "malloc.h"
#include "logging.h"
#include "ipc.h"
#include "md5.h"
#include "base64.h"
#include "env.h"
#include "structs.h"
#include "package.h"
#include "request.h"
#include "config.h"
#include "dbcalls.h"
#include <stdio.h>
#include <ctype.h>
#ifdef WIN32
#include "win32.h"
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

char * getMethodName(int method);
/*******************************************************************************
* mapMimeType...
*
* convert the file name to a mime type.
*******************************************************************************/
char *mapMimeType(char *filename) {
  char *ext = NULL;

  ext = strrchr(filename, '.');

  if (ext == NULL)
    return "text/html";

  ext++;

  // msoffice types
  if (strcasecmp(ext, "doc") == 0) {
    return "application/msword";
  } else if (strcasecmp(ext, "xls") == 0) {
    return "application/x-msexcel";
  } else if (strcasecmp(ext, "ppt") == 0) {
    return "application/vnd.ms-powerpoint";

  // openoffice types
  } else if ((strcasecmp(ext, "odb") == 0)) {
    return "application/vnd.oasis.opendocument.database";
  } else if ((strcasecmp(ext, "odc") == 0)) {
    return "application/vnd.oasis.opendocument.chart";
  } else if ((strcasecmp(ext, "odf") == 0)) {
    return "application/vnd.oasis.opendocument.formula";
  } else if ((strcasecmp(ext, "odg") == 0)) {
    return "application/vnd.oasis.opendocument.graphics";
  } else if ((strcasecmp(ext, "odi") == 0)) {
    return "application/vnd.oasis.opendocument.image";
  } else if ((strcasecmp(ext, "odm") == 0)) {
    return "application/vnd.oasis.opendocument.text-master";
  } else if ((strcasecmp(ext, "odp") == 0)) {
    return "application/vnd.oasis.opendocument.presentation";
  } else if ((strcasecmp(ext, "ods") == 0)) {
    return "application/vnd.oasis.opendocument.spreadsheet";
  } else if ((strcasecmp(ext, "odt") == 0)) {
    return "application/vnd.oasis.opendocument.text";
  } else if ((strcasecmp(ext, "otg") == 0)) {
    return "application/vnd.oasis.opendocument.graphics-template";
  } else if ((strcasecmp(ext, "oth") == 0)) {
    return "application/vnd.oasis.opendocument.text-web";
  } else if ((strcasecmp(ext, "otp") == 0)) {
    return "application/vnd.oasis.opendocument.presentation-template";
  } else if ((strcasecmp(ext, "ots") == 0)) {
    return "application/vnd.oasis.opendocument.spreadsheet-template";
  } else if ((strcasecmp(ext, "ott") == 0)) {
    return "application/vnd.oasis.opendocument.text-template";
  } else if ((strcasecmp(ext, "stc") == 0)) {
    return "application/vnd.sun.xml.calc.template";
  } else if ((strcasecmp(ext, "std") == 0)) {
    return "application/vnd.sun.xml.draw.template";
  } else if ((strcasecmp(ext, "sti") == 0)) {
    return "application/vnd.sun.xml.impress.template";
  } else if ((strcasecmp(ext, "stw") == 0)) {
    return "application/vnd.sun.xml.writer.template";
  } else if ((strcasecmp(ext, "sxc") == 0)) {
    return "application/vnd.sun.xml.calc";
  } else if ((strcasecmp(ext, "sxd") == 0)) {
    return "application/vnd.sun.xml.draw";
  } else if ((strcasecmp(ext, "sxg") == 0)) {
    return "application/vnd.sun.xml.writer.global";
  } else if ((strcasecmp(ext, "sxi") == 0)) {
    return "application/vnd.sun.xml.impress";
  } else if ((strcasecmp(ext, "sxm") == 0)) {
    return "application/vnd.sun.xml.math";
  } else if ((strcasecmp(ext, "sxw") == 0)) {
    return "application/vnd.sun.xml.writer";
  // standard types
  } else if ((strcasecmp(ext, "ai") == 0) ||
             (strcasecmp(ext, "eps") == 0) ||
             (strcasecmp(ext, "ps") == 0)) {
    return "application/postscript";
  } else if ((strcasecmp(ext, "js") == 0)) {
    return "application/javascript";
  } else if ((strcasecmp(ext, "css") == 0)) {
    return "text/css";
  } else if ((strcasecmp(ext, "rtf") == 0)) {
    return "application/rtf";
  } else if ((strcasecmp(ext, "tex") == 0)) {
    return "application/x-tex";
  } else if ((strcasecmp(ext, "texinfo") == 0) ||
             (strcasecmp(ext, "texi") == 0)) {
    return "application/x-texinfo";
  } else if ((strcasecmp(ext, "t") == 0) ||
             (strcasecmp(ext, "tr") == 0) ||
             (strcasecmp(ext, "roff") == 0)) {
    return "application/x-troff";
  } else if ((strcasecmp(ext, "au") == 0) ||
             (strcasecmp(ext, "snd") == 0)) {
    return "audio/basic";
  } else if ((strcasecmp(ext, "aif") == 0) ||
             (strcasecmp(ext, "aiff") == 0) ||
             (strcasecmp(ext, "aifc") == 0)) {
    return "audio/x-aiff";
  } else if ((strcasecmp(ext, "wav") == 0)) {
    return "audio/x-wav";
  } else if ((strcasecmp(ext, "gif") == 0)) {
    return "image/gif";
  } else if ((strcasecmp(ext, "png") == 0)) {
    return "image/png";
  } else if ((strcasecmp(ext, "ief") == 0)) {
    return "image/ief";
  } else if ((strcasecmp(ext, "jpg") == 0) ||
             (strcasecmp(ext, "jpe") == 0) ||
             (strcasecmp(ext, "jpeg") == 0)) {
    return "image/jpeg";
  } else if ((strcasecmp(ext, "tif") == 0) ||
             (strcasecmp(ext, "tiff") == 0)) {
    return "image/tiff";
  } else if ((strcasecmp(ext, "xwd") == 0)) {
    return "image/x-windowdump";
  } else if ((strcasecmp(ext, "htm") == 0) ||
             (strcasecmp(ext, "html") == 0) ||
             (strcasecmp(ext, "cms") == 0)) {
    return "text/html";
  } else if ((strcasecmp(ext, "inc") == 0) ||
             (strcasecmp(ext, "txt") == 0)) {
    return "text/plain";
  } else if ((strcasecmp(ext, "mp3") == 0)) {
    return "audio/mp3";
  } else if ((strcasecmp(ext, "mpg") == 0) ||
             (strcasecmp(ext, "mpeg") == 0) ||
             (strcasecmp(ext, "mpe") == 0)) {
    return "video/mpeg";
  } else if ((strcasecmp(ext, "mov") == 0) ||
             (strcasecmp(ext, "qt") == 0)) {
    return "video/quicktime";
  } else if ((strcasecmp(ext, "avi") == 0)) {
    return "video/x-msvideo";
  } else if ((strcasecmp(ext, "movie") == 0)) {
    return "video/x-sgi-movie";
  }
  return "application/unknown";
}

/*******************************************************************************
* DAVEncodeSpaces...
*
* This is the same as DAVEncode but sends spaces to the %20 pile.
*******************************************************************************/
char *DAVEncodeSpaces(char *value) {
  int i = 0,
           j = 0,
           len = 0;
  char hex[3], *encoded = NULL;

  encoded = (char *) dhumalloc(sizeof(char) * (strlen(value)*3 + 1));
  len = strlen(value);
  hex[2] = CNULL;

  i = 0;
  while (i < len) {
    if ((!isalnum(value[i])) && (value[i] != '/') && (value[i] != '.') && (value[i] != '_') && (value[i] != '-')) {
      encoded[j++] = '%';
      sprintf(&encoded[j++], "%.2x", (int) value[i]);
    } else {
      encoded[j] = value[i];
    }
    i++; j++;
  }
  encoded[j] = CNULL;
  return encoded;
}

/*******************************************************************************
* DAVEncode...
*
* Encode the value.
*******************************************************************************/
char *DAVEncode(char *value) {
  int i = 0,
           j = 0,
           len = 0;
  char hex[3], *encoded = NULL;

  encoded = (char *) dhumalloc(sizeof(char) * (strlen(value)*3 + 1));
  len = strlen(value);
  hex[2] = CNULL;

  i = 0;
  while (i < len) {
    if ((!isalnum(value[i])) && (value[i] != '/') && (value[i] != '.') && (value[i] != '_') && (value[i] != '-') && (value[i] != ' ')) {
      encoded[j++] = '%';
      sprintf(&encoded[j++], "%.2x", (int) value[i]);
    } else {
      encoded[j] = value[i];
    }
    i++; j++;
  }
  encoded[j] = CNULL;
  return encoded;
}

void printWebDavRequest(webdav_request *w) {
	if (w == NULL) {
		logDebug("NULL REQUEST\n");
	} else {
		logDebug("Webdav request {\n");
		logDebug("Depth: %d\n", w->depth);
		logDebug("Overwrite: %d\n", w->overwrite);
		logDebug("Content-Length: %d\n", w->contentlength);
		logDebug("Method: %s\n", getMethodName(w->method));
		logDebug("Destination: %s\n", w->destination);
		logDebug("Lock-Token: %s\n", w->locktoken);
		logDebug("Authorization: %s\n", w->authorization);
		logDebug("User-Agent: %s\n", w->useragent);
		logDebug("Resource: %s\n", w->resource);
		logDebug("XML: %s\n", w->xml);
		logDebug("}\n");
	}
}

/*
 * initWebDavRequest
 *
 * Malloc and zero this struct.
 */
webdav_request * initWebDavRequest() {
  webdav_request *w = NULL;

  w = (webdav_request *) malloc(sizeof(webdav_request));
  memset(w, 0, sizeof(webdav_request));

  w->depth = -1;
  return w;
}

/*
 * freeWebDavRequest
 *
 * Free this struct and its members.
 */
void freeWebDavRequest(webdav_request *r) {
  if (r) {
    if (r->destination)
      dhufree(r->destination);
    if (r->locktoken)
      dhufree(r->locktoken);
    if (r->useragent)
      dhufree(r->useragent);
    if (r->xml)
      dhufree(r->xml);
    dhufree(r);
  }
}

char *getHttpDescription(int code) {
  switch (code) {
    case HTTP_PRECONDITION_FAILED:
      return "Precondition Failed";
    case HTTP_UNSUPPORTED_MEDIA_TYPE:
      return "Unsupported Media Type";
    case HTTP_CREATED:
      return "Created";
    case HTTP_NO_CONTENT:
      return "No Content";
    case HTTP_FORBIDDEN:
      return "Forbidden";
    case HTTP_AUTH_REQUIRED:
      return "Authorization Required";
    case HTTP_OK:
      return "OK";
    case HTTP_METHOD_NOT_ALLOWED:
      return "Method not allowed";
    case HTTP_BAD_REQUEST:
      return "Bad Request";
    case HTTP_NOT_FOUND:
      return "Not Found";
    case HTTP_CONFLICT:
      return "Conflict";
    case HTTP_INTERNAL_SERVER_ERROR:
      return "Internal Server Error";
    default:
      return "Unknown Code";
  }
}

/*
 * handleWebDavError
 *
 * Return an appropriate error response.
 */
int handleWebDavError(int code, int sockfd) {
  char *response = NULL, *codestr = NULL;

  int2Str(code, &codestr);
  vstrdupcat(&response, "HTTP/1.1 ", codestr, " ", getHttpDescription(code), "\n\n", NULL);
 
  sendData(response, strlen(response) * sizeof(char), sockfd);

  dhufree(codestr);
  dhufree(response);
  return code;
}

/*
 * getMethodName
 *
 * Returns an enumeration for the method based on the recongised methods.
 */
char * getMethodName(int method) {
  switch (method) {
    case GET_METHOD:
      return "GET";
    case PROPFIND_METHOD:
      return "PROPFIND";
    case PROPPATCH_METHOD:
      return "PROPPATCH"; 
    case MKCOL_METHOD:
      return "MKCOL";
    case DELETE_METHOD:
      return "DELETE";
    case PUT_METHOD:
      return "PUT";
    case COPY_METHOD:
      return "COPY";
    case MOVE_METHOD:
      return "MOVE";
    case LOCK_METHOD:
      return "LOCK";
    case UNLOCK_METHOD:
      return "UNLOCK";
    case HEAD_METHOD:
      return "HEAD";
    case OPTIONS_METHOD:
      return "OPTIONS";
    default:
      return "Unknown Method";
  }
}

/*
 * getMethod
 *
 * Returns an enumeration for the method based on the recongised methods.
 */
int getMethod(const char *method) {
  if (strcasecmp(method, "propfind") == 0) {
    return PROPFIND_METHOD;
  } else if (strcasecmp(method, "get") == 0) {
    return GET_METHOD;
  } else if (strcasecmp(method, "proppatch") == 0) {
    return PROPPATCH_METHOD;
  } else if (strcasecmp(method, "mkcol") == 0) {
    return MKCOL_METHOD;
  } else if (strcasecmp(method, "delete") == 0) {
    return DELETE_METHOD;
  } else if (strcasecmp(method, "put") == 0) {
    return PUT_METHOD;
  } else if (strcasecmp(method, "copy") == 0) {
    return COPY_METHOD;
  } else if (strcasecmp(method, "move") == 0) {
    return MOVE_METHOD;
  } else if (strcasecmp(method, "head") == 0) {
    return HEAD_METHOD;
  } else if (strcasecmp(method, "options") == 0) {
    return OPTIONS_METHOD;
  } else if (strcasecmp(method, "lock") == 0) {
    return LOCK_METHOD;
  } else if (strcasecmp(method, "unlock") == 0) {
    return UNLOCK_METHOD;
  }
  return UNKNOWN_METHOD;
}

int generateNOnce(char **nonce, char **opaque) {
  char *seed = NULL, *tstamp = NULL, *e = NULL;
  time_t t;

  t = time(NULL);

  tstamp = ctime(&t);

  e = strchr(tstamp, '\n');
  if (e) 
    *e = '\0';
  
  vstrdupcat(&seed, tstamp, ":dhufish", NULL);

  
  *nonce = MD5(seed);
  dhufree(seed);
  *opaque = dhustrdup(tstamp);

  return E_OK;
}

int issueDigestChallenge(webdav_request *r, int sockfd, Env *env) {
  char *response = NULL, *nonce = NULL, *opaque = NULL;
  char *useragent = NULL;
    
  useragent = r->useragent;
  
  if (useragent && ((strstr(useragent, "Microsoft") != NULL) || (strstr(useragent, "MSIE") != NULL))) {
    // use lamo basic authentication
    vstrdupcat(&response, "HTTP/1.1 401 Authorization Required\n",
		    	"Connection: close\n",
	    		"WWW-Authenticate: Basic realm=\"Epiction CMS\"\n\n", NULL);
    
    sendData(response, strlen(response) * sizeof(char), sockfd);

    dhufree(response);
    
  } else { 
    // use much cooler digest authentication
    generateNOnce(&nonce, &opaque);

    vstrdupcat(&response, "HTTP/1.1 401 Authorization Required\n",
		    	"Connection: close\n",
	    		"WWW-Authenticate: Digest realm=\"Epiction CMS\", ",
		    	"nonce=\"", nonce, "\", opaque=\"", opaque, "\"\n\n", NULL);
    dhufree(nonce);
    dhufree(opaque);
 
    sendData(response, strlen(response) * sizeof(char), sockfd);

    dhufree(response);
  }
  return E_OK;
}

void getHeaderParamValue(char *header, char *param, char **value) {
  char *s = NULL, *e = NULL, *rule = NULL, c;

  vstrdupcat(&rule, param, "=\"", NULL);
  s = strstr(header, rule);
  if (s == NULL) {
    *value = dhustrdup("");
    dhufree(rule);
    return;
  }

  s += strlen(rule);
  dhufree(rule);
  e = strchr(s, '"');
  if (s == NULL) {
    *value = dhustrdup("");
    return;
  }

  c = *e;
  *e = '\0';
  *value = dhustrdup(s);
  *e = c;
}

int isValidBasic(webdav_request *r, int *uid, void *sockfd) {
    char *base64 = NULL,
       *decoded = NULL,
       *username = NULL,
       *password = NULL,
       *digest = NULL,
       *end = NULL;
  int err = 0, userid = 0, len = 0;
  
  // move to the start of the digest string
  base64 = strstr(r->authorization, "Basic");
  base64 += 5;
  // skip whitespace
  while (isspace(*base64)) base64++;

  // terminate the string
  end = base64;
  while (!isspace(*end) && *end != '\0') end++;
  *end = '\0';
  
  err = base64Decode(base64, &decoded, &len);
  
  if (err == E_OK) {
    end = strchr(decoded, ':');
    if (end == NULL)
      err = NODATAFOUND;
  }
  
  if (err == E_OK) {
    *end = '\0';
    username = strdup(decoded);
    password = strdup(end + 1);
    dhufree(decoded);
  }
  
  if (err == E_OK) {
    base64 = NULL;
    vstrdupcat(&base64, username, ":Epiction CMS:", password, NULL);
    digest = MD5(base64);
  }
  
  // digest = MD5(concat(MD5(username:realm:password), concat(method:uri)))
  // stored in the db is MD5(username:realm:password) so

  if (err == E_OK) {
    dhufree(password);
    if ((err = loadUserPassword(username, &userid, &password, sockfd)) != E_OK) {
      dhufree(digest);
      dhufree(username);
      return err;
    }
  }

  if (strcmp(digest, password) != 0) {
    err = NODATAFOUND;
  }

  dhufree(password);
  dhufree(username);
  dhufree(digest);
  *uid = userid;
  return err;
}

int isValidDigest(webdav_request *r, int *uid, void *sockfd) {
  char *username = NULL,
       *realm = NULL,
       *nonce = NULL,
       *uri = NULL,
       *response = NULL,
       *digest = NULL,
       *computed = NULL,
       *password = NULL,
       *ha2 = NULL,
       *opaque = NULL;
  int err = 0, userid = 0;
  
    
    
  getHeaderParamValue(r->authorization, "username", &username);
  getHeaderParamValue(r->authorization, "realm", &realm);
  getHeaderParamValue(r->authorization, "nonce", &nonce);
  getHeaderParamValue(r->authorization, "uri", &uri);
  getHeaderParamValue(r->authorization, "response", &response);
  getHeaderParamValue(r->authorization, "opaque", &opaque);

  // digest = MD5(concat(MD5(username:realm:password), concat(method:uri)))
  // stored in the db is MD5(username:realm:password) so


  if ((err = loadUserPassword(username, &userid, &password, sockfd)) != E_OK) {
    dhufree(opaque);
    dhufree(response);
    dhufree(uri);
    dhufree(nonce);
    dhufree(realm);
    dhufree(username);
    return err;
  }

  vstrdupcat(&ha2, getMethodName(r->method), ":", uri, NULL);
  digest = MD5(ha2);
  dhufree(ha2);
  ha2 = digest;
  digest = NULL;
  vstrdupcat(&digest, password, ":", nonce, ":", ha2, NULL);
  dhufree(password);
  dhufree(ha2);
  computed = MD5(digest);
  dhufree(digest);

  err = E_OK;
  if (strcmp(computed, response) != 0) {
    err = NODATAFOUND;
  }

  dhufree(computed);
  dhufree(opaque);
  dhufree(response);
  dhufree(uri);
  dhufree(nonce);
  dhufree(realm);
  dhufree(username);
  *uid = userid;
  return err;
}

int addUserDetailsToEnv(UserDetails *user, Env *env) {
  int err = 0;

  setTokenValue(dhustrdup("USERNAME"), dhustrdup(user->userName), env);
  setTokenValue(dhustrdup("EMAIL"), dhustrdup(user->email), env);
  setTokenValue(dhustrdup("USERID"), dhustrdup(user->userID), env);
  setTokenValue(dhustrdup("ISSUPERUSER"), dhustrdup(user->isSuperUser), env);
  setTokenValue(dhustrdup("FULLNAME"), dhustrdup(user->fullName), env);

  return err;
}

int authoriseRequest(webdav_request *r, Env *env, int sockfd, void *sqlsock) {
  int err = 0, uid = 0;
  UserDetails *user = NULL;

    
    
  // only accept authenticated requests.
  if (r->authorization == NULL) {
    issueDigestChallenge(r, sockfd, env);
    return NODATAFOUND;
  } 
    
    

  // check for
  if (strstr(r->authorization, "Basic") != NULL) {
    err = isValidBasic(r, &uid, sqlsock);
    if (err != E_OK) {
      issueDigestChallenge(r, sockfd, env);
      return err;
    }  
  } else {
    err = isValidDigest(r, &uid, sqlsock);
    if (err != E_OK) {
      issueDigestChallenge(r, sockfd, env);
      return err;
    }
  }
    
  // The digest is valid
  // Create a env for the rest of this request.

  // load user details
  err = getUserDetails(uid, &user, sqlsock);
  
  // add details to env
  addUserDetailsToEnv(user, env);
  freeUserDetails(user);
 
  return err;
}

/*
 * handleGetRequest
 *
 * Handle this head request.
 */
int handleGetRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  int err = 0, objectID = 0, timestamp = 0, datalen = 0;
  char *headers = NULL, *data = NULL, *lenstr = NULL, *timestr = NULL;
  Env *env = NULL;
  ObjectDetails *details = NULL;
  
  env = initEnv();
  err = authoriseRequest(r, env, sockfd, sqlsock);

  if (err != 0) {
    // digest challenge sent
    return E_OK;
  }

  if ((strcasecmp(r->resource, "/") == 0) || (*(r->resource) == '\0')) {
    vstrdupcat(&headers, "HTTP/1.1 200 OK\r\n", NULL);
    vstrdupcat(&headers, "Content-Type: application/folder\r\n", NULL);
    vstrdupcat(&headers, "Content-Length: 0\r\n", NULL);
    vstrdupcat(&headers, "Last-Modified: Sat, 01 Jan 2000 01:00:00 GMT\r\n\r\n", NULL);
    sendData(headers, strlen(headers) * sizeof(char), sockfd);
    freeEnv(env);
    return E_OK;
  } else {
    timestamp = time(NULL);
    err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);
    if (err != E_OK) {
      if (err == ACCESSDENIED) {
        err = HTTP_FORBIDDEN;
      } else {
        err = HTTP_CONFLICT;
      }
    } else {
      // write the file back down the socket
      err = loadObjectContent(objectID, &datalen, &data);

      if (err == 0) {
        err = getObjectDetails(objectID, &details, sqlsock);
      }

      if (err == 0) {
        int2Str(datalen, &lenstr);
	timestr = getISO8601(strtol(details->version, NULL, 10));
        vstrdupcat(&headers, "HTTP/1.1 200 OK\r\n", NULL);
        vstrdupcat(&headers, "Content-Type: ", details->mimeType, "\r\n", NULL);
        vstrdupcat(&headers, "Last-Modified: ", timestr, "\r\n", NULL);
        vstrdupcat(&headers, "Content-Length: ", lenstr, "\r\n\r\n", NULL);
        sendData(headers, strlen(headers) * sizeof(char), sockfd);
        sendData(data, datalen * sizeof(char), sockfd);
        dhufree(lenstr);
        dhufree(timestr);
      }

      if (data != NULL)
        dhufree(data);

      if (details != NULL)
        freeObjectDetails(details);
      
    }
  }

  
  freeEnv(env);
  return err;
}

/*
 * handleHeadRequest
 *
 * Handle this head request.
 */
int handleHeadRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  int err = 0, objectID = 0, timestamp = 0, datalen = 0;
  char *headers = NULL, *data = NULL, *lenstr = NULL, *timestr = NULL;
  Env *env = NULL;
  ObjectDetails *details = NULL;
  
  env = initEnv();
  err = authoriseRequest(r, env, sockfd, sqlsock);

  if (err != 0) {
    freeEnv(env);
    // digest challenge sent
    return E_OK;
  }

  if ((strcasecmp(r->resource, "/") == 0) || (*(r->resource) == '\0')) {
    vstrdupcat(&headers, "HTTP/1.1 200 OK\r\n", NULL);
    vstrdupcat(&headers, "Content-Type: application/folder\r\n", NULL);
    vstrdupcat(&headers, "Content-Length: 0\r\n", NULL);
    vstrdupcat(&headers, "Last-Modified: Sat, 01 Jan 2000 01:00:00 GMT\r\n\r\n", NULL);
    sendData(headers, strlen(headers) * sizeof(char), sockfd);
    freeEnv(env);
    return E_OK;
  } else {
    timestamp = time(NULL);
    err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);
    if (err != E_OK) {
      if (err == ACCESSDENIED) {
        err = HTTP_FORBIDDEN;
      } else if (err == INVALIDXPATH) {
        err = HTTP_NOT_FOUND;
      } else {
        err = HTTP_CONFLICT;
      }
    } else {
      // write the file back down the socket
      err = loadObjectContent(objectID, &datalen, &data);

      if (err == 0) {
        err = getObjectDetails(objectID, &details, sqlsock);
      }

      if (err == 0) {
        int2Str(datalen, &lenstr);
	timestr = getISO8601(strtol(details->version, NULL, 10));
        vstrdupcat(&headers, "HTTP/1.1 200 OK\r\n", NULL);
        vstrdupcat(&headers, "Content-Type: ", details->mimeType, "\r\n", NULL);
        vstrdupcat(&headers, "Last-Modified: ", timestr, "\r\n", NULL);
        vstrdupcat(&headers, "Content-Length: ", lenstr, "\r\n\r\n", NULL);
        sendData(headers, strlen(headers) * sizeof(char), sockfd);
        dhufree(lenstr);
        dhufree(timestr);
      }

      if (data != NULL)
        dhufree(data);

      if (details != NULL)
        freeObjectDetails(details);

    }
  }

  
  freeEnv(env);
  return err;
}

/*
 * handleOptionsRequest
 *
 * Handle this options request.
 */
int handleOptionsRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  char *response = NULL;
  Env *env = NULL;
  int err = 0;
  
  env = initEnv();
  err = authoriseRequest(r, env, sockfd, sqlsock);

  if (err != 0) {
    freeEnv(env);
    // digest challenge sent
    return E_OK;
  }
  freeEnv(env);
    
  vstrdupcat(&response, "HTTP/1.1 200 OK\n", 
			"Dav: 1\n", 
            "Allow: OPTIONS, PROPFIND, PROPPATCH, MKCOL, DELETE, PUT, COPY, MOVE, HEAD\n\n", NULL);

  sendData(response, strlen(response) * sizeof(char), sockfd);

  dhufree(response);  
  
  return E_OK;
}

/*
 * handleDeleteRequest
 *
 * Handle this delete request.
 */
int handleDeleteRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  int err = 0, objectID = 0, timestamp = 0;
  Env *env = NULL;
  
  env = initEnv();
  err = authoriseRequest(r, env, sockfd, sqlsock);

  if (err != 0) {
    // digest challenge sent
    freeEnv(env);
    return E_OK;
  }

  if ((strcasecmp(r->resource, "/") == 0) || (*(r->resource) == '\0')) {
    err = HTTP_FORBIDDEN;
    return E_OK;
  }

  if (err == 0) {

    // read access
    timestamp = time(NULL);
    err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);
    if (err != E_OK) {
      if (err == ACCESSDENIED) {
        err = HTTP_FORBIDDEN;
      } else if (err == INVALIDXPATH) {
        err = HTTP_NOT_FOUND;
      } else {
        err = HTTP_CONFLICT;
      }
    } 
  }

  if (err == 0) {
    // write access
    if (!userHasWriteAccess(objectID, env, sqlsock) == E_OK) {
      err = HTTP_FORBIDDEN;
    }
  }
  
  if (err == 0) {
    // delete the object
    err = deleteObject(objectID, env, sqlsock);

    if (err == 0) {
      err = HTTP_NO_CONTENT;
    } else {
      err = HTTP_INTERNAL_SERVER_ERROR;
    }
  }

  freeEnv(env);
  return err;
}

/*
 * handleMoveRequest
 *
 * Handle this move request.
 */
int handleMoveRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  int err = 0, parentid = 0, timestamp = 0, srcid = 0, datalen = 0, 
      objid = 0, *children = NULL, numchildren = 0, i = 0, successcode = HTTP_CREATED;
  Env *env = NULL;
  char *parent = NULL, *e = NULL, *title = NULL, *data = NULL, *src = NULL, *dest = NULL, *tmpa = NULL, *tmpb = NULL, *newtitle = NULL;
  FileObject *file = NULL;
  ObjectDetails *details = NULL, *child = NULL;

  
  if (r->resource == NULL || r->destination == NULL) {
      err = HTTP_INTERNAL_SERVER_ERROR;
  }

  env = initEnv();
  if (err == 0) {
    err = authoriseRequest(r, env, sockfd, sqlsock);
  }

  if (err != 0) {
    // digest challenge sent
    freeEnv(env);
    return E_OK;
  }

  timestamp = time(NULL);
  // take any trailing / off the end
  e = r->resource + strlen(r->resource) - 1;
  if (*e == '/') {
    *e = '\0';
  }
  // take any trailing / off the end
  e = r->destination + strlen(r->destination) - 1;
  if (*e == '/') {
    *e = '\0';
  }
  parent = strdup(r->destination);
  e = strrchr(parent, '/');
  if (e != NULL) {
    *e = '\0';
    title = e + 1;
  } else {
    title = parent;
  }
  newtitle = dhustrdup(title);

  if ((strcmp(parent, "/") != 0) && (*parent != '\0')) {
    err = isXRefValid(parent, timestamp, sqlsock, env, &parentid);
    if (err != E_OK) {
      if (err == ACCESSDENIED) {
        err = HTTP_FORBIDDEN;
      } else {
        err = HTTP_CONFLICT;
      }
    }
  } else {
    parentid = -1;
  }
  
  if (err == 0) {
    // even works for root folder
    if (!userHasWriteAccess(parentid, env, sqlsock) == E_OK) {
      err = HTTP_FORBIDDEN;
    }
  }

  if (err == 0) { 
    // create the file
    err = isXRefValid(r->resource, timestamp, sqlsock, env, &srcid);
  }

  if (err == 0) {
    err = userHasWriteAccess(srcid, env, sqlsock);
    if (err != 0)
      err = HTTP_FORBIDDEN;
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
                        dhustrdup(title),
                        data,
                        datalen);

    err = createNewObject(parentid, title, (details->isPublic[0] == 'y')?1:0,
                                             details->type, // type
                                             1, // index
                                             details->tplt,
					     0,
                                             file,
                                             env,
                                             sqlsock);


    if (err == FILEEXISTS) {
      successcode = HTTP_NO_CONTENT;
      // overwrite test is go
      if (r->overwrite) {
        err = getObjectID(parentid, title, timestamp, &objid, sqlsock);

        if (err == 0) {
          err = replaceObjectContents(objid, 1, details->tplt, file, env, sqlsock);
        }
      } else {
        err = HTTP_PRECONDITION_FAILED;
      }
    }

    freeFileObjectList(file);
  }

  if (err == 0) {
    // copy the metadata
    timestamp++;
    err = getObjectID(parentid, newtitle, timestamp, &objid, sqlsock);

    if (err == 0) {
      err = copyMetadata(srcid, objid, sqlsock);
    }
    if (err == 0) {
      err = copyVerifierComments(srcid, objid, sqlsock);
    }
  }
  dhufree(newtitle);

  // check for folder and depth header (or lack of) 
  if (err == 0) {
    if ((strcasecmp(details->type, "FOLDER") == 0) && r->depth == -1) {
      err = loadFolderContentsDB(srcid, "", timestamp, "name", &children, &numchildren, sqlsock);

      if (err == 0) {
	tmpa = strdup(r->resource);
	tmpb = strdup(r->destination);
        for (i = 0; i < numchildren; i++) {
          err = getObjectDetails(children[i], &child, sqlsock);
 
          if (err == 0) {
            src = NULL;
            vstrdupcat(&src, tmpa, "/", child->objectName, NULL);
            dest = NULL;
            vstrdupcat(&dest, tmpb, "/", child->objectName, NULL);

            dhufree(r->resource);
            r->resource = src;
            dhufree(r->destination);
            r->destination = dest;
            err = handleMoveRequest(r, xml, sockfd, sqlsock);
          }

          if (child != NULL) 
            freeObjectDetails(child);
          err = E_OK;
        }
	free(tmpa);
	free(tmpb);
      } else if (err == NODATAFOUND) {
        err = E_OK;
      }
    }
  }

  // delete old version
  if (err == 0) {
    // we can only insert object versions with an accuracy of 1 second,
    // so we must wait 1 second to delete this object.
    err = deleteObject(srcid, env, sqlsock);
  }

  if (err == 0) {
    err = successcode;
  }

  if (details != NULL)
    freeObjectDetails(details);
  freeEnv(env);
  free(parent);

  return err;
}

/*
 * handleCopyRequest
 *
 * Handle this copy request.
 */
int handleCopyRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  int err = 0, parentid = 0, timestamp = 0, srcid = 0, datalen = 0, objid = 0, *children = NULL, numchildren = 0, i = 0;
  Env *env = NULL;
  char *parent = NULL, *e = NULL, *title = NULL, *data = NULL, *src = NULL, *dest = NULL, *newtitle = NULL;
  FileObject *file = NULL;
  ObjectDetails *details = NULL, *child = NULL;
  int replace = 0;
  
  if (r->resource == NULL || r->destination == NULL) {
      err = HTTP_INTERNAL_SERVER_ERROR;
  }

  env = initEnv();
  if (err == 0) {
    err = authoriseRequest(r, env, sockfd, sqlsock);
  }

  if (err != 0) {
    // digest challenge sent
    freeEnv(env);
    return E_OK;
  }

  timestamp = time(NULL);
  // take any trailing / off the end
  e = r->resource + strlen(r->resource) - 1;
  if (*e == '/') {
    *e = '\0';
  }
  // take any trailing / off the end
  e = r->destination + strlen(r->destination) - 1;
  if (*e == '/') {
    *e = '\0';
  }
  parent = strdup(r->destination);
  e = strrchr(parent, '/');
  if (e != NULL) {
    *e = '\0';
    title = e + 1;
  } else {
    title = parent;
  }

  title = dhustrdup(title);
  newtitle = dhustrdup(title);

  if ((strcmp(parent, "/") != 0) && (*parent != '\0')) {
    err = isXRefValid(parent, timestamp, sqlsock, env, &parentid);
    if (err != E_OK) {
      if (err == ACCESSDENIED) {
        err = HTTP_FORBIDDEN;
      } else {
        err = HTTP_CONFLICT;
      }
    }
  } else {
    parentid = -1;
  }

  
  if (err == 0) {
    // even works for root folder
    if (!userHasWriteAccess(parentid, env, sqlsock) == E_OK) {
      err = HTTP_FORBIDDEN;
    }
  }

  if (err == 0) { 
    // create the file
    // do the copy
    err = isXRefValid(r->resource, timestamp, sqlsock, env, &srcid);
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
                        dhustrdup(title),
                        data,
                        datalen);

    err = createNewObject(parentid, title, (details->isPublic[0] == 'y')?1:0,
                                             details->type, // type
                                             1, // index
                                             details->tplt,
					     0,
                                             file,
                                             env,
                                             sqlsock);


    if (err == FILEEXISTS) {
      // overwrite test is go
      if (r->overwrite) {
        err = getObjectID(parentid, title, timestamp, &objid, sqlsock);

        if (err == 0) {
          replace = 1;
          err = replaceObjectContents(objid, 1, details->tplt, file, env, sqlsock);
        }
	
      } else {
        err = HTTP_PRECONDITION_FAILED;
      }
    }

    freeFileObjectList(file);
  }
  if (err == 0) {
    // copy the metadata
    timestamp++;
    err = getObjectID(parentid, newtitle, timestamp, &objid, sqlsock);

    if (err == 0) {
      err = copyMetadata(srcid, objid, sqlsock);
    }
    if (err == 0) {
      err = copyVerifierComments(srcid, objid, sqlsock);
    }
  }
  dhufree(newtitle);

  // check for folder and depth header (or lack of) 
  if (err == 0) {
    if ((strcasecmp(details->type, "FOLDER") == 0) && r->depth == -1) {
      // ok - load up the children and copy them too.
      err = loadFolderContentsDB(srcid, "", timestamp, "name", &children, &numchildren, sqlsock);

      if (err == 0) {
	free(parent);
	free(title);
	parent = strdup(r->resource);
	title = strdup(r->destination);
        for (i = 0; i < numchildren; i++) {
          err = getObjectDetails(children[i], &child, sqlsock);
 
          if (err == 0) {
            src = NULL;

            vstrdupcat(&src, parent, "/", child->objectName, NULL);
            dest = NULL;
            vstrdupcat(&dest, title, "/", child->objectName, NULL);

            dhufree(r->resource);
            r->resource = src;
            dhufree(r->destination);
            r->destination = dest;
            err = handleCopyRequest(r, xml, sockfd, sqlsock);
          }

          if (child != NULL) 
            freeObjectDetails(child);
          err = E_OK;
        }
      } else if (err == NODATAFOUND) {
        err = E_OK;
      }
    }
  }

  if (err == 0) {
    if (replace == 0) {
      err = HTTP_CREATED;
    } else {
      err = HTTP_NO_CONTENT;
    }
  }

  if (details != NULL)
    freeObjectDetails(details);
  freeEnv(env);
  free(parent);
  free(title);

  return err;
}

/*
 * handlePutRequest
 *
 * Handle this put request.
 */
int handlePutRequest(webdav_request *r, int sockfd, void *sqlsock) {
  int err = 0, parentid = 0, timestamp = 0, objectID = 0;
  Env *env = NULL;
  char *parent = NULL, *e = NULL, *title = NULL, *data = NULL;
  FileObject *file = NULL;
  ObjectDetails *details = NULL;
  
  env = initEnv();
  err = authoriseRequest(r, env, sockfd, sqlsock);

  if (err != 0) {
    // digest challenge sent
    freeEnv(env);
    return E_OK;
  }

  timestamp = time(NULL);
  // take any trailing / off the end
  e = r->resource + strlen(r->resource) - 1;
  if (*e == '/') {
    *e = '\0';
  }
  parent = strdup(r->resource);
  e = strrchr(parent, '/');
  if (e != NULL) {
    *e = '\0';
    title = e + 1;
  } else {
    title = parent;
  }

  if ((strcmp(parent, "/") != 0) && (*parent != '\0')) {
    err = isXRefValid(parent, timestamp, sqlsock, env, &parentid);
    if (err != E_OK) {
      if (err == ACCESSDENIED) {
        err = HTTP_FORBIDDEN;
      } else {
        err = HTTP_CONFLICT;
      }
    }
  } else {
    parentid = -1;
  }
  
  if (err == 0) {
    // even works for root folder
    if (!userHasWriteAccess(parentid, env, sqlsock) == E_OK) {
      err = HTTP_FORBIDDEN;
    }
  }

  if (err == 0) {
    data = (char *) calloc(sizeof(char), r->contentlength + 1);
    memcpy(data, r->xml, sizeof(char) * r->contentlength);
    // create the file
    file = initFileObject(dhustrdup("file"),
                        dhustrdup(mapMimeType(title)),
                        dhustrdup(title),
                        data,
                        r->contentlength);

    err = createNewObject(parentid, title, 'n',
                                             "RESOURCE", // type
                                             1, // do not index
                                             "",
					     0,
                                             file,
                                             env,
                                             sqlsock);

    if (err == 0) {
      err = HTTP_CREATED;
    } else if (err == FILEEXISTS) {
      err = getObjectID(parentid, title, timestamp, &objectID, sqlsock);

      // replace
      if (err == 0) {
        err = getObjectDetails(objectID, &details, sqlsock);
      }

      if (err == 0) {
        err = replaceObjectContents(objectID, 1, details->tplt, file, env, sqlsock);
        if (err == 0) {
          err = HTTP_NO_CONTENT;
        }
      }
      if (details != NULL) 
        freeObjectDetails(details);
    } else {
      err = HTTP_CONFLICT;
    }
    freeFileObjectList(file);
  }

  freeEnv(env);
  free(parent);
  return err;
}

/*
 * handleMkColRequest
 *
 * Handle this mkcol request.
 */
int handleMkColRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  int err = 0, parentid = 0, timestamp = 0;
  Env *env = NULL;
  char *parent = NULL, *e = NULL, *title = NULL;
  FileObject *file = NULL;
  
  env = initEnv();
  err = authoriseRequest(r, env, sockfd, sqlsock);

  if (err != 0) {
    // digest challenge sent
    freeEnv(env);
    return E_OK;
  }

  if (r->xml != NULL && strlen(r->xml) > 0) {
    return HTTP_UNSUPPORTED_MEDIA_TYPE;
  }

  timestamp = time(NULL);
  // take any trailing / off the end
  e = r->resource + strlen(r->resource) - 1;
  if (*e == '/') {
    *e = '\0';
  }
  parent = strdup(r->resource);
  e = strrchr(parent, '/');
  if (e != NULL) {
    *e = '\0';
    title = e + 1;
  } else {
    title = parent;
  }
  
  if ((strcmp(parent, "/") != 0) && (*parent != '\0')) {
    err = isXRefValid(parent, timestamp, sqlsock, env, &parentid);
    if (err != E_OK) {
      if (err == ACCESSDENIED) {
        err = HTTP_FORBIDDEN;
      } else {
        err = HTTP_CONFLICT;
      }
    }
  } else {
    parentid = -1;
  }
  
  if (err == 0) {
    // even works for root folder
    if (!userHasWriteAccess(parentid, env, sqlsock) == E_OK) {
      err = HTTP_FORBIDDEN;
    }
  }

  if (err == 0) { 
    // create the folder
    file = initFileObject(dhustrdup("file"),
                        dhustrdup("application/folder"),
                        dhustrdup("file.txt"),
                        dhustrdup(""),
                        0);

    err = createNewObject(parentid, title, 'n',
                                             "FOLDER", // type
                                             0, // do not index
                                             "",
					     0,
                                             file,
                                             env,
                                             sqlsock);

    if (err == 0) {
      err = HTTP_CREATED;
    } else if (err == FILEEXISTS) {
      err = HTTP_METHOD_NOT_ALLOWED;
    }
    freeFileObjectList(file);
  }

  freeEnv(env);
  free(parent);
  return err;
}


/*
 * addObjectPropToXML
 *
 * Adds a single property to the xml response.
 */
int addObjectPropToXML(char *propname, char *namespace, ObjectDetails *details, XMLParser *xml, Stack *notfound, void *sqlsock) {
  int err = 0;
  time_t t;
  char *value = NULL, *p = NULL, *pname = NULL;
  Pair *nf = NULL;
  
  if (namespace == NULL) {
    namespace = "DAV:";
  }

  if ((strcasecmp(propname, "creationdate") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    t = strtol(details->version, NULL, 10);
    value = ctime(&t);   

    p = strpbrk(value, "\r\n");
    if (p) *p = '\0';
 
    addChildNode(xml, dhustrdup("creationdate"), dhustrdup(value?value:""), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "displayname") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("displayname"), dhustrdup(details->objectName?details->objectName:""), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "getcontenttype") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("getcontenttype"), dhustrdup(details->mimeType?details->mimeType:""), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "getcontentlanguage") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    if (getObjectMetadata(strtol(details->objectID, NULL, 10), "dc:language", &value, sqlsock) == E_OK) {
      addChildNode(xml, dhustrdup("getcontentlanguage"), value?value:"", dhustrdup(namespace));
    } else {
      addChildNode(xml, dhustrdup("getcontentlanguage"), dhustrdup("en-au"), dhustrdup(namespace));
    }
  } else if ((strcasecmp(propname, "getcontentlength") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("getcontentlength"), dhustrdup(details->fileSize?details->fileSize:""), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "iscollection") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("iscollection"), dhustrdup((strcasecmp(details->type, "FOLDER") == 0)?"1":"0"), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "getlastmodified") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    t = strtol(details->version, NULL, 10);
    value = ctime(&t);   

    p = strpbrk(value, "\r\n");
    if (p) *p = '\0';
 
    addChildNode(xml, dhustrdup("modifieddate"), dhustrdup(value?value:""), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "executable") == 0) && (strcasecmp(namespace, "http://apache.org/dav/props/") == 0)) {
    if (strcasecmp(details->type, "FOLDER") == 0) {
      addChildNode(xml, dhustrdup("executable"), dhustrdup("T"), dhustrdup(namespace));
    } else {
      addChildNode(xml, dhustrdup("executable"), dhustrdup("F"), dhustrdup(namespace));
    }
  } else if ((strcasecmp(propname, "resourcetype") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    if (strcasecmp(details->type, "FOLDER") == 0) {
      addChildNode(xml, dhustrdup("resourcetype"), dhustrdup(""), dhustrdup(namespace));
      moveToFirstChild(xml);
      while (moveToNextSibling(xml) == E_OK) ;
      addChildNode(xml, dhustrdup("collection"), dhustrdup(""), dhustrdup(namespace));
      moveToParent(xml);
    } else {
      addChildNode(xml, dhustrdup("resourcetype"), dhustrdup(""), dhustrdup(namespace));
    }
  } else {
    // try and find this property in the object metadata
    if (strcasecmp(namespace, "") == 0) {
      vstrdupcat(&pname, "dc:", propname, NULL);
    } else {
      vstrdupcat(&pname, propname, "[", namespace, "]", NULL);
    }
    if (getObjectMetadata(strtol(details->objectID, NULL, 10), pname, &value, sqlsock) == E_OK) {
      if (strcasecmp(namespace, "unknown") == 0) {
        addChildNode(xml, dhustrdup(propname), value, NULL);
      } else {
        addChildNode(xml, dhustrdup(propname), value, dhustrdup(namespace));
      }
    } else {
      nf = initPair(dhustrdup(propname), dhustrdup(namespace));
      pushStack(notfound, nf);
    }
    dhufree(pname);
  }

  return err;
}

/*
 * addRootPropToXML
 *
 * Adds a single property to the xml response.
 */
int addRootPropToXML(char *propname, char *namespace, XMLParser *xml, Stack *notfound) {
  int err = 0;
  time_t t;
  char *value = NULL, *p = NULL;
  Pair *nf = NULL;
  
  if (namespace == NULL) {
    namespace = "DAV:";
  }
  
  if ((strcasecmp(propname, "creationdate") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    t = time(NULL) - 60*60*24*365;
    value = ctime(&t);

    p = strpbrk(value, "\r\n");
    if (p) *p = '\0';
 
    addChildNode(xml, dhustrdup("creationdate"), dhustrdup(value?value:""), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "displayname") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("displayname"), dhustrdup("dav"), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "getcontentlanguage") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("getcontentlanguage"), dhustrdup("en-au"), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "getcontenttype") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("getcontenttype"), dhustrdup("application/folder"), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "getcontentlength") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("getcontentlength"), dhustrdup("0"), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "getlastmodified") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    t = time(NULL);
    value = ctime(&t);

    p = strpbrk(value, "\r\n");
    if (p) *p = '\0';
 
    addChildNode(xml, dhustrdup("getlastmodified"), dhustrdup(value?value:""), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "iscollection") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("iscollection"), dhustrdup("1"), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "executable") == 0) && (strcasecmp(namespace, "http://apache.org/dav/props/") == 0)) {
    addChildNode(xml, dhustrdup("executable"), dhustrdup("T"), dhustrdup(namespace));
  } else if ((strcasecmp(propname, "resourcetype") == 0) && (strcasecmp(namespace, "DAV:") == 0)) {
    addChildNode(xml, dhustrdup("resourcetype"), dhustrdup(""), dhustrdup(namespace));
    moveToFirstChild(xml);
    while (moveToNextSibling(xml) == E_OK) ;
    addChildNode(xml, dhustrdup("collection"), dhustrdup(""), dhustrdup(namespace));
    moveToParent(xml);
  } else {
    nf = initPair(dhustrdup(propname), dhustrdup(namespace));
    pushStack(notfound, nf);
  }

  return err;
}

/*
 * addEmptyObjectPropsToXMLResponse
 *
 * Adds the empty properties to the xml response.
 */
int addEmptyObjectPropsToXMLResponse(webdav_request *r, int objectID, int timestamp, Stack *props, XMLParser *xml, void *sqlsock, int depth) {
  int err = 0, count = 0, i = 0, *children = NULL, numchildren = 0, colcount = 0;
  ObjectDetails *details = NULL;
  char *href = NULL, **colnames = NULL, *path = NULL, *sep = NULL;
  Pair *pair = NULL;
  
  if (objectID != -1) {
    err = getObjectDetails(objectID, &details, sqlsock);

    if (err == E_OK) {
      addChildNode(xml, strdup("response"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
      while (moveToNextSibling(xml) == E_OK);

      path = DAVEncodeSpaces(details->path);
      vstrdupcat(&href, "http://", getHostName(), "/dav/", path, NULL);
      dhufree(path);
      if (strcmp(details->type, "FOLDER") == 0) {
        vstrdupcat(&href, "/", NULL);
      }
      addChildNode(xml, strdup("href"), href, strdup("DAV:"));
      moveToFirstChild(xml);
      addSiblingNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
      moveToNextSibling(xml);
      addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
  
      // now add all the requested properties
      count = countStack(props);

      for (i = 0; i < count; i++) {
        pair = sniffNStack(props, i);
        addChildNode(xml, dhustrdup((char *)(pair->first)), dhustrdup(""), dhustrdup((char *)(pair->second)));
      }
  
      getAllObjectMetadata(objectID, &colnames, &colcount, sqlsock);
      for (i = 0; i < colcount; i++) {
	if (strncmp(colnames[i], "dc:", 3) == 0) {
          addChildNode(xml, dhustrdup((colnames[i] + 3)), dhustrdup(""), dhustrdup("http://purl.org/dc/elements/1.1/"));
	} else if ((sep = strchr(colnames[i], '[')) != NULL) {
          *sep = '\0';
	  *sep++;
	  sep[strlen(sep) - 1] = '\0';
          addChildNode(xml, dhustrdup((colnames[i])), dhustrdup(""), dhustrdup(sep));
	} else {
          addChildNode(xml, dhustrdup((colnames[i])), dhustrdup(""), dhustrdup("unknown"));
	}
      }
      free(colnames);
      
      moveToParent(xml);
      addChildNode(xml, strdup("status"), strdup("HTTP/1.1 200 OK"), strdup("DAV:"));
      moveToParent(xml);
      moveToParent(xml);
    }
  } else {
    // insert dummy collection record
    addChildNode(xml, strdup("response"), strdup(""), strdup("DAV:"));
    moveToFirstChild(xml);
    while (moveToNextSibling(xml) == E_OK);
      
    vstrdupcat(&href, "http://", getHostName(), "/dav/", NULL);
    addChildNode(xml, strdup("href"), href, strdup("DAV:"));
    moveToFirstChild(xml);
    addSiblingNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
    moveToNextSibling(xml);
    addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
    moveToFirstChild(xml);

    count = countStack(props);

    for (i = 0; i < count; i++) {
        pair = (Pair *) sniffNStack(props, i);
        addChildNode(xml, dhustrdup((char *)(pair->first)), dhustrdup(""), dhustrdup((char *)(pair->second)));
    }
    
    moveToParent(xml);
    addChildNode(xml, strdup("status"), strdup("HTTP/1.1 200 OK"), strdup("DAV:"));
    moveToParent(xml);
    moveToParent(xml);
  }

  if (objectID == -1 || (strcasecmp(details->type, "FOLDER") == 0)) {
     if (r->depth == -1 || depth < r->depth) {
      // load contents
      err = loadFolderContentsDB(objectID, "", timestamp, "name", &children, &numchildren, sqlsock);

      if (err == E_OK) {
        for (i = 0; i < numchildren; i++) {
          err = addEmptyObjectPropsToXMLResponse(r, children[i], timestamp, props, xml, sqlsock, depth + 1);
        }
 
        dhufree(children);
      } else if (err == NODATAFOUND) {
        err = E_OK;
      }
    }
  }
  
  if (details)
    freeObjectDetails(details);

  return err;
}

/*
 * addObjectAllPropsToXMLResponse
 *
 * Adds the specified properties to the xml response.
 */
int addObjectAllPropsToXMLResponse(webdav_request *r, int objectID, int timestamp, XMLParser *xml, void *sqlsock, int depth) {
  int err = 0, i = 0, *children = NULL, numchildren = 0, colcount = 0;
  ObjectDetails *details = NULL;
  Stack *notfound = NULL;
  char *href = NULL, **colnames = NULL;
  Pair *pair = NULL;
  
  if (objectID != -1) {
    err = getObjectDetails(objectID, &details, sqlsock);

    if (err == E_OK) {
      addChildNode(xml, strdup("response"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
      while (moveToNextSibling(xml) == E_OK);

      vstrdupcat(&href, "http://", getHostName(), "/dav/", (details->path), NULL);
      if (strcmp(details->type, "FOLDER") == 0) {
        vstrdupcat(&href, "/", NULL);
      }
      addChildNode(xml, strdup("href"), href, strdup("DAV:"));
      moveToFirstChild(xml);
      addSiblingNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
      moveToNextSibling(xml);
      addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
  
      notfound = initStack();

      // now add all the properties
      addObjectPropToXML("creationdate", "DAV:", details, xml, notfound, sqlsock);
      addObjectPropToXML("displayname", "DAV:", details, xml, notfound, sqlsock);
      addObjectPropToXML("getcontentlanguage", "DAV:", details, xml, notfound, sqlsock);
      addObjectPropToXML("getcontentlength", "DAV:", details, xml, notfound, sqlsock);
      addObjectPropToXML("getcontenttype", "DAV:", details, xml, notfound, sqlsock);
      addObjectPropToXML("modifieddate", "DAV:", details, xml, notfound, sqlsock);
      addObjectPropToXML("resourcetype", "DAV:", details, xml, notfound, sqlsock);
      addObjectPropToXML("executable", "http://apache.org/dav/props/", details, xml, notfound, sqlsock);
      addObjectPropToXML("iscollection", "DAV:", details, xml, notfound, sqlsock);
  
      // now get all the db metadata
      getAllObjectMetadata(objectID, &colnames, &colcount, sqlsock);
      for (i = 0; i < colcount; i++) {
        addObjectPropToXML(colnames[i], "http://purl.org/dc/elements/1.1/", details, xml, notfound, sqlsock);
        free(colnames[i]);
      }
      free(colnames);
      freeStack(&notfound);
      moveToParent(xml);
      addChildNode(xml, strdup("status"), strdup("HTTP/1.1 200 OK"), strdup("DAV:"));
      moveToParent(xml);
      moveToParent(xml);
    }
  } else {
    // insert dummy collection record
    addChildNode(xml, strdup("response"), strdup(""), strdup("DAV:"));
    moveToFirstChild(xml);
    while (moveToNextSibling(xml) == E_OK);
      
    vstrdupcat(&href, "http://", getHostName(), "/dav/", NULL);
    addChildNode(xml, strdup("href"), href, strdup("DAV:"));
    moveToFirstChild(xml);
    addSiblingNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
    moveToNextSibling(xml);
    addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
    moveToFirstChild(xml);

    notfound = initStack();
     
    // now add all the properties
    addRootPropToXML("creationdate", "DAV:", xml, notfound);
    addRootPropToXML("displayname", "DAV:", xml, notfound);
    addRootPropToXML("getcontentlanguage", "DAV:", xml, notfound);
    addRootPropToXML("getcontentlength", "DAV:", xml, notfound);
    addRootPropToXML("getcontenttype", "DAV:", xml, notfound);
    addRootPropToXML("modifieddate", "DAV:", xml, notfound);
    addRootPropToXML("resourcetype", "DAV:", xml, notfound);
    addRootPropToXML("executable", "http://apache.org/dav/props/", xml, notfound);
    addRootPropToXML("iscollection", "DAV:", xml, notfound);
    
    moveToParent(xml);
    addChildNode(xml, strdup("status"), strdup("HTTP/1.1 200 OK"), strdup("DAV:"));
    moveToParent(xml);
    
    if (countStack(notfound) > 0) {
      addChildNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
      while (moveToNextSibling(xml) == E_OK);
      
      addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
      while ((pair = (Pair *) popStack(notfound)) != NULL) {
          addChildNode(xml, (char *)(pair->first), strdup(""), (char *)(pair->second));
	  pair->first = NULL;
          pair->second = NULL;
          freePair(pair);
      }
      
      moveToParent(xml);
      // add 404 response
      addChildNode(xml, strdup("status"), strdup("HTTP/1.1 404 Not Found"), strdup("DAV:"));
      moveToParent(xml);
    }
    freeStack(&notfound);
    moveToParent(xml);
  }

  if (objectID == -1 || (strcasecmp(details->type, "FOLDER") == 0)) {
    if (r->depth == -1 || depth < r->depth) {
      // load contents
      err = loadFolderContentsDB(objectID, "", timestamp, "name", &children, &numchildren, sqlsock);

      if (err == E_OK) {
        for (i = 0; i < numchildren; i++) {
          err = addObjectAllPropsToXMLResponse(r, children[i], timestamp, xml, sqlsock, depth + 1);
        }
 
        dhufree(children);
      } else if (err == NODATAFOUND) {
        err = E_OK;
      }
    }
  }
  
  if (details)
    freeObjectDetails(details);

  return err;
}

/*
 * addObjectPropsToXMLResponse
 *
 * Adds the specified properties to the xml response.
 */
int addObjectPropsToXMLResponse(webdav_request *r, int objectID, int timestamp, Stack *props, XMLParser *xml, void *sqlsock, int depth) {
  int err = 0, count = 0, i = 0, *children = NULL, numchildren = 0;
  ObjectDetails *details = NULL;
  char *href = NULL, *path = NULL;
  Stack *notfound = NULL;
  Pair *pair = NULL;
    
  // workaround for stupid ms
  if (objectID != -1) {
    err = getObjectDetails(objectID, &details, sqlsock);
  }
  if ((depth != 0) || (r->depth != 1) || (r->useragent == NULL) || (strstr(r->useragent, "Microsoft Data") == NULL)) {


  if (objectID != -1) {
    if (err == E_OK) {
      addChildNode(xml, strdup("response"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
      while (moveToNextSibling(xml) == E_OK);

      path = DAVEncodeSpaces(details->path);
      vstrdupcat(&href, "http://", getHostName(), "/dav/", path, NULL);
      dhufree(path);
      if (strcmp(details->type, "FOLDER") == 0) {
        vstrdupcat(&href, "/", NULL);
      }
      addChildNode(xml, strdup("href"), href, strdup("DAV:"));
      moveToFirstChild(xml);
      addSiblingNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
      moveToNextSibling(xml);
      addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
  
      // now add all the requested properties
      count = countStack(props);

      notfound = initStack();
      
      for (i = 0; i < count; i++) {
        pair = (Pair *) sniffNStack(props, i);
        err = addObjectPropToXML((char *)(pair->first), (char *)(pair->second), details, xml, notfound, sqlsock);
        if (err != 0)
          break;
      }
  
      
      moveToParent(xml);
      addChildNode(xml, strdup("status"), strdup("HTTP/1.1 200 OK"), strdup("DAV:"));
      moveToParent(xml);
      
      if (countStack(notfound) > 0) {
        addChildNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
        moveToFirstChild(xml);
        while (moveToNextSibling(xml) == E_OK);
      
        addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
        moveToFirstChild(xml);
        while ((pair = (Pair *) popStack(notfound)) != NULL) {
            addChildNode(xml, pair->first, strdup(""), pair->second);
            pair->first = NULL;
            pair->second = NULL;
            freePair(pair);
        }
      
        moveToParent(xml);
        // add 404 response
        addChildNode(xml, strdup("status"), strdup("HTTP/1.1 404 Not Found"), strdup("DAV:"));
        moveToParent(xml);
      }
      freeStack(&notfound);
      
      
      
      moveToParent(xml);
    }
  } else {
    // insert dummy collection record
    addChildNode(xml, strdup("response"), strdup(""), strdup("DAV:"));
    moveToFirstChild(xml);
    while (moveToNextSibling(xml) == E_OK);
      
    vstrdupcat(&href, "http://", getHostName(), "/dav/", NULL);
    addChildNode(xml, strdup("href"), href, strdup("DAV:"));
    moveToFirstChild(xml);
    addSiblingNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
    moveToNextSibling(xml);
    addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
    moveToFirstChild(xml);

    count = countStack(props);
    notfound = initStack();
    
    for (i = 0; i < count; i++) {
      pair = (Pair *) sniffNStack(props, i);
      err = addRootPropToXML(pair->first, pair->second, xml, notfound);
      if (err != 0)
        break;
    }
    
    moveToParent(xml);
    addChildNode(xml, strdup("status"), strdup("HTTP/1.1 200 OK"), strdup("DAV:"));
    moveToParent(xml);
    
    if (countStack(notfound) > 0) {
      addChildNode(xml, strdup("propstat"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
      while (moveToNextSibling(xml) == E_OK);
      
      addChildNode(xml, strdup("prop"), strdup(""), strdup("DAV:"));
      moveToFirstChild(xml);
      while ((pair = (Pair *) popStack(notfound)) != NULL) {
          addChildNode(xml, pair->first, strdup(""), pair->second);
          pair->first = NULL;
          pair->second = NULL;
          freePair(pair);
      }
      
      moveToParent(xml);
      // add 404 response
      addChildNode(xml, strdup("status"), strdup("HTTP/1.1 404 Not Found"), strdup("DAV:"));
      moveToParent(xml);
    }
    freeStack(&notfound);
    
    
    moveToParent(xml);
  }
  }

  if (objectID == -1 || (strcasecmp(details->type, "FOLDER") == 0)) {
    if (r->depth == -1 || depth < r->depth) {
      // load contents
      err = loadFolderContentsDB(objectID, "", timestamp, "name", &children, &numchildren, sqlsock);

      if (err == E_OK) {
        for (i = 0; i < numchildren; i++) {
          err = addObjectPropsToXMLResponse(r, children[i], timestamp, props, xml, sqlsock, depth + 1);
        }
 
        dhufree(children);
      } else if (err == NODATAFOUND) {
        err = E_OK;
      }
    }
  }
  
  if (details)
    freeObjectDetails(details);

  return err;
}

/*
 * handlePropFindAllPropRequest
 *
 * This will handle a prop find request with a list of property
 * names to retrieve.
 */
int handlePropFindAllPropRequest(webdav_request *r, XMLParser *xml, XMLParser *response, Env *env, void *sqlsock) {
  int err = 0, timestamp = 0;
  int objectID = 0;

  // get the list of properties
  //moveToFirstChild(xml);

  // build the properties list
  // load the details of the specified object

  timestamp = time(NULL);
  
  if ((strcasecmp(r->resource, "/") == 0) || (*(r->resource) == '\0')) {
    objectID = -1;
  } else {
    err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);
  }

  // create the response

  if (err == E_OK) {
    addChildNode(response, strdup("multistatus"), strdup(""), NULL);
    addNameSpaceDecl(response, strdup("D"), strdup("DAV:"));
    addNameSpaceDecl(response, strdup("A"), strdup("http://apache.org/dav/props/"));
    addNameSpaceDecl(response, strdup("dc"), strdup("http://purl.org/dc/elements/1.1/"));
    response->current->nameSpace = dhustrdup("D");
  }
    
  // add this object to the response
  if (err == E_OK) {
    err = addObjectAllPropsToXMLResponse(r, objectID, timestamp, response, sqlsock, 0);
  }

  return err;
}

/*
 * handlePropFindPropNameRequest
 *
 * This will handle a prop find request with a list of property
 * names to retrieve.
 */
int handlePropFindPropNameRequest(webdav_request *r, XMLParser *xml, XMLParser *response, Env *env, void *sqlsock) {
  int err = 0, timestamp = 0;
  Stack *properties = NULL;
  int objectID = 0;
  Pair *pair = NULL;

  // get the list of properties
  moveToFirstChild(xml);

  // build the properties list
  properties = initStack();
  pushStack(properties, initPair(dhustrdup("creationdate"), dhustrdup("DAV:")));
  pushStack(properties, initPair(dhustrdup("getcontentlength"), dhustrdup("DAV:")));
  pushStack(properties, initPair(dhustrdup("getcontenttype"), dhustrdup("DAV:")));
  pushStack(properties, initPair(dhustrdup("getcontentlanguage"), dhustrdup("DAV:")));
  pushStack(properties, initPair(dhustrdup("getlastmodified"), dhustrdup("DAV:")));
  pushStack(properties, initPair(dhustrdup("displayname"), dhustrdup("DAV:")));
  pushStack(properties, initPair(dhustrdup("resourcetype"), dhustrdup("DAV:")));
  pushStack(properties, initPair(dhustrdup("executable"), dhustrdup("http://apache.org/dav/props/")));
  pushStack(properties, initPair(dhustrdup("iscollection"), dhustrdup("DAV:")));

  // load the details of the specified object

  timestamp = time(NULL);
  
  if ((strcasecmp(r->resource, "/") == 0) || (*(r->resource) == '\0')) {
    objectID = -1;
  } else {
    err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);
  }

  // create the response
  if (err == E_OK) {
    addChildNode(response, strdup("multistatus"), strdup(""), NULL);
    addNameSpaceDecl(response, strdup("D"), strdup("DAV:"));
    addNameSpaceDecl(response, strdup("A"), strdup("http://apache.org/dav/props/"));
    addNameSpaceDecl(response, strdup("dc"), strdup("http://purl.org/dc/elements/1.1/"));
    response->current->nameSpace = dhustrdup("D");
  }
    
  // add this object to the response
  if (err == E_OK) {
    err = addEmptyObjectPropsToXMLResponse(r, objectID, timestamp, properties, response, sqlsock, 0);
  }

  // we do not clean up the stack elements because we did not allocate them
  while ((pair = (Pair *) popStack(properties)) != NULL) {
    freePair(pair);
  }
  freeStack(&properties);
  return err;
}

/*
 * handlePropFindPropRequest
 *
 * This will handle a prop find request with a list of property
 * names to retrieve.
 */
int handlePropFindPropRequest(webdav_request *r, XMLParser *xml, XMLParser *response, Env *env, void *sqlsock) {
  int err = 0, timestamp = 0;
  Stack *properties = NULL;
  const char *name = NULL, *alias = NULL, *ns = NULL;
  int objectID = 0;
  Pair *pair = NULL;

  // get the list of properties
  moveToFirstChild(xml);

  // build the properties list
  properties = initStack();
  do {
    getCurrentTagName(xml, &name);
    getCurrentTagNameSpace(xml, &alias);
    getCurrentTagNameSpaceValue(xml, (char *) alias, &ns);
    pushStack(properties, initPair(dhustrdup((char *) name), dhustrdup((char *) ns)));
  } while (moveToNextSibling(xml) == E_OK);

  // load the details of the specified object

  timestamp = time(NULL);
  
  if ((strcasecmp(r->resource, "/") == 0) || (*(r->resource) == '\0')) {
    objectID = -1;
  } else {
    err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);
  }

  // create the response
  if (err == E_OK) {
    addChildNode(response, strdup("multistatus"), strdup(""), NULL);
    addNameSpaceDecl(response, strdup("D"), strdup("DAV:"));
    addNameSpaceDecl(response, strdup("A"), strdup("http://apache.org/dav/props/"));
    addNameSpaceDecl(response, strdup("dc"), strdup("http://purl.org/dc/elements/1.1/"));
    response->current->nameSpace = dhustrdup("D");
  }
    
  // add this object to the response
  if (err == E_OK) {
    err = addObjectPropsToXMLResponse(r, objectID, timestamp, properties, response, sqlsock, 0);
  }

  // we do not clean up the stack elements because we did not allocate them
  while ((pair = (Pair *) popStack(properties)) != NULL) {
    freePair(pair);
  }
  freeStack(&properties);
  return err;
}

/*
 * handlePropPatchSetRequest
 *
 * This will handle a prop find request with a list of property
 * names to retrieve.
 */
int handlePropPatchSetRequest(webdav_request *r, XMLParser *xml, XMLParser *response, Env *env, void *sqlsock) {
  const char *name = NULL, *value = NULL, *alias = NULL, *ns = NULL;
  int objectID = 0;
  int err = 0, timestamp = 0;
  char *pname = NULL;

  // set this metadata element
  // according to the spec, this element contains 1 prop element which is the name and value of the metadata to set.

  moveToFirstChild(xml);
  moveToFirstChild(xml);
  getCurrentTagName(xml, &name);
  getCurrentTagValue(xml, &value);
  getCurrentTagNameSpace(xml, &alias);
  getCurrentTagNameSpaceValue(xml, (char *) alias, &ns);
  moveToParent(xml);
  moveToParent(xml);

  // firstly - can't set metadata on the root folder
  if ((strcasecmp(r->resource, "/") == 0) || (*(r->resource) == '\0')) {
    // add a forbidden element
    addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
    moveToFirstChild(response);
    while (moveToNextSibling(response) == E_OK) ;
    addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
    addChildNode(response, strdup("status"), strdup("HTTP/1.1 403 Forbidden"), strdup("DAV:"));
    moveToParent(response);
    return E_OK;
  }

  // now we can begin.
  timestamp = time(NULL);
  err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);

  if ((strcasecmp(name, "getcontentlength") == 0) ||
      (strcasecmp(name, "getcontenttype") == 0) ||
      (strcasecmp(name, "getcontentlanguage") == 0) ||
      (strcasecmp(name, "getlastmodified") == 0) ||
      (strcasecmp(name, "resourcetype") == 0) ||
      (strcasecmp(name, "executable") == 0) ||
      (strcasecmp(name, "iscollection") == 0) ||
      (strcasecmp(name, "displayname") == 0) ||
      (strcasecmp(name, "creationdate") == 0)) {
    // cannot set live properties
    // add a forbidden element
    addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
    moveToFirstChild(response);
    while (moveToNextSibling(response) == E_OK) ;
    addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
    addChildNode(response, strdup("status"), strdup("HTTP/1.1 409 Conflict"), strdup("DAV:"));
    moveToParent(response);
    return E_OK;
  } else {
    if (!(userHasWriteAccess(objectID, env, sqlsock) == E_OK)) {
      addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
      moveToFirstChild(response);
      while (moveToNextSibling(response) == E_OK) ;
      addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
      addChildNode(response, strdup("status"), strdup("HTTP/1.1 403 Forbidden"), strdup("DAV:"));
      moveToParent(response);
      return E_OK;
    } else {
      // set this arbitrary dead property
      if ((ns != NULL) && (strcasecmp(ns, "http://purl.org/dc/elements/1.1/") == 0)) {
        vstrdupcat(&pname, "dc:", name, NULL);
      } else {
	if (ns != NULL && *ns != '\0') {
	  vstrdupcat(&pname, name, "[", ns, "]", NULL);
	} else {
          pname = strdup(name);
	}
      }
      err = setObjectMetadata(objectID, (char *)pname, (char *)value, sqlsock);
      dhufree(pname);
      if (err == E_OK) {
        addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
        moveToFirstChild(response);
        while (moveToNextSibling(response) == E_OK) ;
        addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
        addChildNode(response, strdup("status"), strdup("HTTP/1.1 200 OK"), strdup("DAV:"));
        moveToParent(response);
        return E_OK;
      } else {
        addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
        moveToFirstChild(response);
        while (moveToNextSibling(response) == E_OK) ;
        addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
        addChildNode(response, strdup("status"), strdup("HTTP/1.1 500 Internal Server Error"), strdup("DAV:"));
        moveToParent(response);
        return E_OK;
      }
    }
  }
}

/*
 * handlePropPatchRemoveRequest
 *
 * This will handle a prop find request with a list of property
 * names to retrieve.
 */
int handlePropPatchRemoveRequest(webdav_request *r, XMLParser *xml, XMLParser *response, Env *env, void *sqlsock) {
  const char *name = NULL, *alias = NULL, *ns = NULL;
  int objectID = 0;
  int err = 0, timestamp = 0;
  char *pname = NULL;

  // set this metadata element
  // according to the spec, this element contains 1 prop element which is the name of the metadata to remove.

  moveToFirstChild(xml);
  moveToFirstChild(xml);
  getCurrentTagName(xml, &name);
  getCurrentTagNameSpace(xml, &alias);
  getCurrentTagNameSpaceValue(xml, (char *) alias, &ns);
  moveToParent(xml);
  moveToParent(xml);

  // firstly - can't set metadata on the root folder
  if ((strcasecmp(r->resource, "/") == 0) || (*(r->resource) == '\0')) {
    // add a forbidden element
    addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
    moveToFirstChild(response);
    while (moveToNextSibling(response) == E_OK) ;
    addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
    addChildNode(response, strdup("status"), strdup("HTTP/1.1 403 Forbidden"), strdup("DAV:"));
    moveToParent(response);
    return E_OK;
  }

  // now we can begin.
  timestamp = time(NULL);
  err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);

  if ((strcasecmp(name, "getcontentlength") == 0) ||
      (strcasecmp(name, "getcontenttype") == 0) ||
      (strcasecmp(name, "getcontentlanguage") == 0) ||
      (strcasecmp(name, "getlastmodified") == 0) ||
      (strcasecmp(name, "resourcetype") == 0) ||
      (strcasecmp(name, "executable") == 0) ||
      (strcasecmp(name, "iscollection") == 0) ||
      (strcasecmp(name, "displayname") == 0) ||
      (strcasecmp(name, "creationdate") == 0)) {
    // cannot set live properties
    // add a forbidden element
    addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
    moveToFirstChild(response);
    while (moveToNextSibling(response) == E_OK) ;
    addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
    addChildNode(response, strdup("status"), strdup("HTTP/1.1 409 Conflict"), strdup("DAV:"));
    moveToParent(response);
    return E_OK;
  } else {
    if (!(userHasWriteAccess(objectID, env, sqlsock) == E_OK)) {
      addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
      moveToFirstChild(response);
      while (moveToNextSibling(response) == E_OK) ;
      addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
      addChildNode(response, strdup("status"), strdup("HTTP/1.1 403 Forbidden"), strdup("DAV:"));
      moveToParent(response);
      return E_OK;
    } else {
      // set this arbitrary dead property
      if ((ns != NULL) && (strcasecmp(ns, "http://purl.org/dc/elements/1.1/") == 0)) {
        vstrdupcat(&pname, "dc:", name, NULL);
      } else {
	if (ns != NULL && *ns != '\0') {
	  vstrdupcat(&pname, name, "[", ns, "]", NULL);
	} else {
          pname = strdup(name);
	}
      }
      err = removeObjectMetadata(objectID, (char *)pname, sqlsock);
      dhufree(pname);
      if (err == E_OK) {
        addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
        moveToFirstChild(response);
        while (moveToNextSibling(response) == E_OK) ;
        addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
        addChildNode(response, strdup("status"), strdup("HTTP/1.1 200 OK"), strdup("DAV:"));
        moveToParent(response);
        return E_OK;
      } else {
        addChildNode(response, strdup("propstat"), strdup(""), strdup("DAV:"));
        moveToFirstChild(response);
        while (moveToNextSibling(response) == E_OK) ;
        addChildNode(response, strdup("prop"), strdup(name), strdup("DAV:"));
        addChildNode(response, strdup("status"), strdup("HTTP/1.1 500 Internal Server Error"), strdup("DAV:"));
        moveToParent(response);
        return E_OK;
      }
    }
  }
}

/*
 * handlePropPatchRequest
 *
 * Handle this prop patch request.
 */
int handlePropPatchRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  int err = 0, objectID = 0, timestamp = 0;
  Env *env = NULL;
  const char *name = NULL;
  XMLParser *response = NULL;
  char *output = NULL, *headers = NULL, *len = NULL, *href = NULL, *path = NULL;
  ObjectDetails *details = NULL;
  
  env = initEnv();
  initXMLParser(&response);

  err = authoriseRequest(r, env, sockfd, sqlsock);

  if (err != 0) {
    // digest challenge sent
    return E_OK;
  }
  
  // check root element
  if (err == 0) {
    moveToRoot(xml);
    getCurrentTagName(xml, &name);
    if (!name) name = "";

    if (strcasecmp(name, "propertyupdate") != 0) {
      logError("Propfind request with wrong xml root element: %s\n", name?name:""); 
      err = HTTP_BAD_REQUEST;
    }
  }
  
  timestamp = time(NULL);
  err = isXRefValid(r->resource, timestamp, sqlsock, env, &objectID);
  if (err == 0) {
    err = getObjectDetails(objectID, &details, sqlsock);
  }

  // check first child - will determine type of proppatch
  if (err == 0) {
    moveToFirstChild(xml);
    addChildNode(response, strdup("multistatus"), strdup(""), NULL);
    addNameSpaceDecl(response, strdup("D"), strdup("DAV:"));
    addNameSpaceDecl(response, strdup("A"), strdup("http://apache.org/dav/props/"));
    addNameSpaceDecl(response, strdup("dc"), strdup("http://purl.org/dc/elements/1.1/"));
    response->current->nameSpace = dhustrdup("D");
      
    // add multistatus
    addChildNode(response, strdup("response"), strdup(""), strdup("DAV:"));
    moveToFirstChild(response);
    path = DAVEncodeSpaces(r->resource);
    href = NULL;
    vstrdupcat(&href, "http://", getHostName(), "/dav/", path, NULL);
    dhufree(path);
    if (strcmp(details->type, "FOLDER") == 0) {
      vstrdupcat(&href, "/", NULL);
    }
    addChildNode(response, strdup("href"), href, strdup("DAV:"));

    do {
      getCurrentTagName(xml, &name);
      if (!name) name = "";

      if (strcasecmp(name, "set") == 0) {
        // handle proppatch set 
        err = handlePropPatchSetRequest(r, xml, response, env, sqlsock);
      } else if (strcasecmp(name, "remove") == 0) {
        // handle proppatch remove 
        err = handlePropPatchRemoveRequest(r, xml, response, env, sqlsock);
      }

    } while (moveToNextSibling(xml) == E_OK);
  }
 
  if (err == E_OK) {
    // send response
    err = exportXML(response, &output);
  }  

  if (err == E_OK) {
    
    vstrdupcat(&headers, "HTTP/1.1 207 Multi-Status\r\n", NULL);
    vstrdupcat(&headers, "Content-Type: text/xml; charset=\"utf-8\"\r\n", NULL);
    int2Str(strlen(output), &len);
    vstrdupcat(&headers, "Content-Length: ", len, "\r\n\r\n", NULL);
    dhufree(len);

    sendData(headers, strlen(headers) * sizeof(char), sockfd);
    sendData(output, strlen(output) * sizeof(char), sockfd);


    dhufree(output);
    dhufree(headers);
  }

  if (details)
    freeObjectDetails(details);
  freeXMLParser(&response);
  freeEnv(env);
  return err;
}

/*
 * handlePropFindRequest
 *
 * Handle this prop find request.
 */
int handlePropFindRequest(webdav_request *r, XMLParser *xml, int sockfd, void *sqlsock) {
  int err = 0;
  Env *env = NULL;
  const char *name = NULL;
  XMLParser *response = NULL;
  char *output = NULL, *headers = NULL, *len = NULL;
  
  env = initEnv();
  initXMLParser(&response);

  err = authoriseRequest(r, env, sockfd, sqlsock);

  if (err != 0) {
    // digest challenge sent
    return E_OK;
  }
  
  // check root element
  if (err == 0) {
    if (r->contentlength > 0) {  
        moveToRoot(xml);
        getCurrentTagName(xml, &name);
        if (!name) name = "";

        if (strcasecmp(name, "propfind") != 0) {
            logError("Propfind request with wrong xml root element: %s\n", name?name:""); 
            err = HTTP_BAD_REQUEST;
        }
    }
    
  }

  // check first child - will determine type of propfind
  if (err == 0) {
    if (r->contentlength == 0) {
        err = handlePropFindAllPropRequest(r, xml, response, env, sqlsock);
    } else {
        moveToFirstChild(xml);
        getCurrentTagName(xml, &name);
        if (!name) name = "";
    
        if (strcasecmp(name, "prop") == 0) {
            // standard prop find with listed properties
            
            err = handlePropFindPropRequest(r, xml, response, env, sqlsock);

        } else if (strcasecmp(name, "allprop") == 0) {
            // standard all prop request
            err = handlePropFindAllPropRequest(r, xml, response, env, sqlsock);
        } else if (strcasecmp(name, "propname") == 0) {
            // standard prop name request
            err = handlePropFindPropNameRequest(r, xml, response, env, sqlsock);
        } else {
            logError("Propfind request with unknown xml element: %s\n", name?name:""); 
            err = HTTP_BAD_REQUEST;
            // unknown prop find
        }
    }
  }
 
  if (err == E_OK) {
    // send response
    err = exportXML(response, &output);
    // wtf?
  }  

  if (err == E_OK) {
    
    vstrdupcat(&headers, "HTTP/1.1 207 Multi-Status\r\n", NULL);
    vstrdupcat(&headers, "Content-Type: text/xml; charset=\"utf-8\"\r\n", NULL);
    int2Str(strlen(output), &len);
    vstrdupcat(&headers, "Content-Length: ", len, "\r\n\r\n", NULL);
    dhufree(len);

    sendData(headers, strlen(headers) * sizeof(char), sockfd);
    sendData(output, strlen(output) * sizeof(char), sockfd);


    dhufree(output);
    dhufree(headers);
  }

  if (err == INVALIDXPATH) {
    err = HTTP_CONFLICT;
  } else if (err == ACCESSDENIED) {
    err = HTTP_FORBIDDEN;
  }
  freeXMLParser(&response);
  freeEnv(env);
  return err;
}

/*
 * dispatchWebDavRequest
 *
 * Parse the xml and respond to the webdav request.
 */
int dispatchWebDavRequest(webdav_request *r, int sockfd, void *sqlsock) {
  XMLParser *xml = NULL;
  int err = 0;

  if ((*(r->xml)) == '\0' || r->method == PUT_METHOD || r->method == COPY_METHOD || r->method == MKCOL_METHOD) {
    initXMLParser(&xml);
  } else {
    if ((err = importXML(r->xml, &xml)) != E_OK) {
      logError("Could not read xml request:%s\n", r->xml);
      return HTTP_BAD_REQUEST;
    }
  }

  printWebDavRequest(r);

  switch (r->method) {
    case HEAD_METHOD:
      err = handleHeadRequest(r, xml, sockfd, sqlsock);
      break;
    case PROPFIND_METHOD:
      err = handlePropFindRequest(r, xml, sockfd, sqlsock);
      break;
    case PROPPATCH_METHOD:
      err = handlePropPatchRequest(r, xml, sockfd, sqlsock);
      break;
    case OPTIONS_METHOD:
      err = handleOptionsRequest(r, xml, sockfd, sqlsock);
      break;
    case MKCOL_METHOD:
      err = handleMkColRequest(r, xml, sockfd, sqlsock);
      break;
    case DELETE_METHOD:
      err = handleDeleteRequest(r, xml, sockfd, sqlsock);
      break;
    case PUT_METHOD:
      err = handlePutRequest(r, sockfd, sqlsock);
      break;
    case COPY_METHOD:
      err = handleCopyRequest(r, xml, sockfd, sqlsock);
      break;
    case GET_METHOD:
      err = handleGetRequest(r, xml, sockfd, sqlsock);
      break;
    case MOVE_METHOD:
      err = handleMoveRequest(r, xml, sockfd, sqlsock);
      break;
    /***
    case LOCK_METHOD:
      return "LOCK";
    case UNLOCK_METHOD:
      return "UNLOCK";
    ***/
  }

  freeXMLParser(&xml);
  return err;
}

/*
 * readBody
 *
  * Read the xml request from stdin.
 */
int readBody(int sockfd, webdav_request *webdav) {
  char *current = NULL;
  int numread = 0, length = webdav->contentlength, err = 0;


  webdav->xml = (char *) malloc(sizeof(char) * (length + 1));
  webdav->xml[length] = '\0';

  if (length > 0) {
    current = webdav->xml;
    do {
      numread = 0;
      err = readDataStatic(current, sizeof(char) * length, sockfd);
      if (err == E_OK)
          numread = length;
      length -= numread;
      current += numread;
    } while ((numread > 0) && (length > 0));
  }

  
  return E_OK;
}

/*
 * readMethod
 *
 * Read the method type and resource.
 */
int readMethod(int sockfd, webdav_request *r) {
  char buf[1024], *current = NULL, *res;
  int numread = 0, err = 0;

  current = buf;
  memset(buf, 0, sizeof(buf));

  do {
    current += numread;
    err = readDataStatic(current, sizeof(char), sockfd);
    numread = 1;
  } while (*current != '\n' && *current != '\0' && ((current - buf) < 1023));

  *current = '\0';

  current = strchr(buf, ' ');
  if (current == NULL) {
    return SYNTAXERROR;
  }
  *current = '\0';
  r->method = getMethod(buf);
 
  res = current + 1;
  current = strrchr(res, ' ');
  if (current) {
    *current = '\0';
  }

  // skip the dav folder
  if (strncmp(res, "/dav", 4) == 0) {
    res += 4;
  }
  
  r->resource = DAVEncode(res);

  return E_OK;
}

/*
 * readNextHeader
 *
 * Read the next webdav header.
 */
int readNextHeader(int sockfd, char **header, char **value) {
  char buf[1024], *current = NULL;
  int numread = 0, err = 0;

  current = buf;
  memset(buf, 0, sizeof(buf));

  err = readDataStatic(current, sizeof(char), sockfd);
  if (*current == '\n' || err != E_OK)
    return NOMOREELEMENTS;
  numread = 1;
  
  do {
    current += numread;
    err = readDataStatic(current, sizeof(char), sockfd);
    numread = 1;
  } while (*current != '\r' && *current != '\n' && *current != '\0' && ((current - buf) < 1023));

  *current = '\0';

  current = strchr(buf, ':');
  if (current == NULL)
    return SYNTAXERROR;

  *current = '\0';
  do {current ++;} while (isspace(*current));
  
  *header = strdup(buf);
  *value = strdup(current);
 
  return E_OK;
}


/*
 * readHeaders
 *
 * Read each of the request headers and add them to the 
 * webdav_request struct.
 */
int readHeaders(int sockfd, webdav_request *webdav) {
  char *header = NULL, *value = NULL, *dest = NULL;
  int err = 0;

  while ((err = readNextHeader(sockfd, &header, &value)) == E_OK) {
    if (strcasecmp(header, "CONTENT_LENGTH") == 0) {
      webdav->contentlength = strtol(value, NULL, 10);
    } else if (strcasecmp(header, "DEPTH") == 0) {
      if (strcasecmp(value, "infinity") == 0) {
        webdav->depth = -1;
      } else {
        webdav->depth = strtol(value, NULL, 10);
      }
    } else if (strcasecmp(header, "OVERWRITE") == 0) {
      if (strcasecmp(value, "t") == 0) {
        webdav->overwrite = 1;
      } else {
        webdav->overwrite = 0;
      }
    } else if (strcasecmp(header, "DESTINATION") == 0) {
      dest = strstr(value, "/dav");
      if (dest != NULL) {
        webdav->destination = DAVEncode(dest + 4);
      }
    } else if (strcasecmp(header, "Authorization") == 0) {
      webdav->authorization = strdup(value);
    } else if (strcasecmp(header, "LOCK_TOKEN") == 0) {
      webdav->locktoken = strdup(value);
    } else if (strcasecmp(header, "USER_AGENT") == 0){
      webdav->useragent = strdup(value);
    }

    dhufree(header);
    dhufree(value);
  }

  return E_OK;
}


/*
 * webdav_handler
 *
 * This is the business - handle all webdav requests.
 */
int handleWebDavRequest(int sockfd) {
  int err = 0;
  void *sqlsock = NULL;
  webdav_request *request = NULL;
  
  request = initWebDavRequest();

  readMethod(sockfd, request);
  
  if (request->method == UNKNOWN_METHOD) {
    return HTTP_METHOD_NOT_ALLOWED;
  }

  err = readHeaders(sockfd, request);
  if (err != E_OK) {
    return handleWebDavError(err, sockfd);
  }  

  err = readBody(sockfd, request);
  if (err != E_OK) {
    return handleWebDavError(err, sockfd);
  }  

  if ((sqlsock = getDBConnection()) == NULL) {
    return handleWebDavError(HTTP_INTERNAL_SERVER_ERROR, sockfd);
  }

  err = dispatchWebDavRequest(request, sockfd, sqlsock);
  if (err != E_OK) {
    closeDBConnection(sqlsock);
    return handleWebDavError(err, sockfd);
  }  
  closeDBConnection(sqlsock);
  
  // freeWebDavRequest(request);
  return E_OK;
}
