/*******************************************************************************
* api...
*
* This file has all the api functions for the script language.
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "strings.h"
#include "structs.h"
#include "parser.h"
#include "api.h"
#include "errors.h"
#include "env.h"
#include "logging.h"
#include "groups.h"
#include "users.h"
#include "package.h"
#include "objects.h"
#include "request.h"
#include "ipc.h"
#include "dbcalls.h"
#include "project.h"
#include "config.h"
#include "malloc.h"

/*********************************************************************
* callImportPackageFunction...
*
* Import an xml package.
*********************************************************************/
int callImportPackageFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *filename = NULL, *parent = NULL;
  Package *package = NULL;
  int errcode = 0, parentid = -1, access = 0;
  FileObject *file = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;

  parent = s->variableType->stringValue;
  if (strcmp(parent, "") != 0) {
    if (isXRefValid(parent, -1, info->sqlsock, info->env, &parentid) != E_OK) {
      dest->variableType->intValue = ACCESSDENIED;
      return E_OK;
    }
  }
  
  arg = arg->next;
  s = (Symbol *) arg->data;
  filename = s->variableType->stringValue;

  if (userHasWriteAccess(parentid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Import Package Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  file = getFileObject(filename, info->env);
  if (!file) {
  	errcode = importPackage(filename, &package);
  } else {
  	errcode = importPackage(file->data, &package);
  }
  if (errcode != E_OK) {
    logInfo("Internal Import Package Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  errcode = importPackageObject(parentid, package, info->env, info->sqlsock);

  logInfo("Internal Import Package Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callExportPackageFunction...
*
* Create an xml package out of an object.
*********************************************************************/
int callExportPackageFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *contents = NULL;
  int errcode = 0, objid = 0, access = 0, online = 0;
  Package *package = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->variableEnum = STRING_VARIABLE;
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Export Package Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->variableEnum = STRING_VARIABLE;
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }
  
  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Export Package Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOTONLINE));
      dest->variableType->variableEnum = STRING_VARIABLE;
      dest->variableType->stringValue = strdup("");
      return E_OK;
    }
  }

  if ((errcode = createPackageFromObject(objid, info->env, &package, info->sqlsock)) != E_OK) {
    dest->variableType->variableEnum = STRING_VARIABLE;
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  if ((errcode = exportPackage(package, &contents)) != E_OK) {
    dest->variableType->variableEnum = STRING_VARIABLE;
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  freePackage(&package);

  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = contents;

  logInfo("Export Package Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callWritelnFunction...
*
* Append these values to the output stream.
*********************************************************************/
int callWritelnFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;
  dest->variableType->intValue = 0;

  arg = list;
  s = (Symbol * ) arg->data;

  vstrdupcat(&(info->output), s->variableType->stringValue, "\n", NULL);
  return E_OK;
}

/*********************************************************************
* callWriteFunction...
*
* Append these values to the output stream.
*********************************************************************/
int callWriteFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;
  dest->variableType->intValue = 0;

  arg = list;
  s = (Symbol * ) arg->data;

  vstrdupcat(&(info->output), s->variableType->stringValue, NULL);
  return E_OK;
}

/*********************************************************************
* callIsValidFilenameFunction...
*
* Check the passed name to see if it is a valid filename.
*********************************************************************/
int callIsValidFilenameFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;

  if (isValidFilename(s->variableType->stringValue)) {
    dest->variableType->intValue = 1;
  } else {
    dest->variableType->intValue = 0;
  }
  return E_OK;
}

/*********************************************************************
* callGetFunction...
*
* Get a value from the info->env.
*********************************************************************/
int callGetFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *val = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  dest->variableType->variableEnum = STRING_VARIABLE;
  val = getEnvValue(s->variableType->stringValue, info->env);
  dest->variableType->stringValue = dhustrdup(val?val:(char *)"");
  return E_OK;
}

/*********************************************************************
* callIsVerifierFunction...
*
* Is this user in the list of verifiers for this object.
*********************************************************************/
int callIsVerifierFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *uid = NULL;
  int errcode = 0, objid = 0, verifier = 0;


  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;

  uid = getEnvValue("USERID", info->env);

  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = 0;
    return E_OK;
  }

  errcode = isVerifier(objid, strtol(uid?uid:"-2", NULL, 10), &verifier, info->sqlsock);

  dest->variableType->intValue = verifier;

  logInfo("Internal Is Verifier Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callSendEmailFunction...
*
* Send an email to someone.
*********************************************************************/
int callSendEmailFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *email = NULL, *subject = NULL, *text = NULL;
  int error = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  email = strdup(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  subject = strdup(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  text = strdup(s->variableType->stringValue);

  error = sendEmail(getEmailServer(), getEmailPort(), getHostName(), getSendmailBin(), getEmailFromAddress(), email, subject, text);
  logInfo("Send Email Request. (Success = %s, Code=%d)\n", getErrorMesg(error), error);
  
  dest->variableType->variableEnum = INT_VARIABLE;
  dest->variableType->intValue = error;
  return E_OK;
}

/*********************************************************************
* callSetFunction...
*
* Get a value from the info->env.
*********************************************************************/
int callSetFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *val = NULL, *tok = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  tok = dhustrdup(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;

  val = dhustrdup(s->variableType->stringValue);
  
  setTokenValue(tok, val, info->env);
  
  dest->variableType->variableEnum = INT_VARIABLE;
  dest->variableType->intValue = 0;
  return E_OK;
}

/*********************************************************************
* callGetMapKeysFunction...
*
* get all of the keys from this map
*********************************************************************/
int callGetMapKeysFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *input = NULL, *n = NULL;
  MapNode *node = NULL;
  int i = 0;
  
  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  input = (Symbol *) arg->data;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if (input->variableType->mapValues != NULL) {
    node = getFirstMapNode(input->variableType->mapValues);
    while (node != NULL) {
      n = (Symbol *) node->ele;
      s = (Symbol *) getSymbolArrayValue(dest->variableType->arrayValues, i);

      s->variableType->variableEnum = STRING_VARIABLE;
      s->variableType->stringValue = strdup(n->variableType->variableName);

      i++;
      node = getNextMapNode(node, input->variableType->mapValues);
    }
  }
  
  return E_OK;
}

/*********************************************************************
* callJoinFunction...
*
* split this string into an array.
*********************************************************************/
int callJoinFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  char *split = NULL, *value = NULL, *ele = NULL;
  Symbol *s = NULL, *input = NULL;
  MapNode *node = NULL;
  int i = 0;
  
  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  input = s;

  arg = arg->next;
  s = (Symbol *) arg->data;
  split = s->variableType->stringValue;
  
  node = getFirstMapNode(input->variableType->arrayValues);
  while (node != NULL) {
    s = (Symbol *) node->ele;
    if (i > 0) {
      vstrdupcat(&value, split, NULL);
    }
    ele = castSymbolToString(s);
    vstrdupcat(&value, ele, NULL);
    dhufree(ele);
    i++;
    node = getNextMapNode(node, input->variableType->arrayValues);
  }
  
  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = value;

  return E_OK;
}

/*********************************************************************
* callSplitFunction...
*
* split this string into an array.
*********************************************************************/
int callSplitFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *string = NULL, *ptr = NULL, *eptr = NULL, *value = NULL, *split = NULL;
  int i = 0;
  
  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  string = s->variableType->stringValue;

  arg = arg->next;
  s = (Symbol *) arg->data;
  split = s->variableType->stringValue;

  ptr = string;
  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  while ((eptr = strstr(ptr, split)) != NULL) {
    value = (char *) calloc(sizeof(char), (eptr - ptr + 1));
    strncpy(value, ptr, (eptr - ptr));
    s = getSymbolArrayValue(dest->variableType->arrayValues, i++);
    s->variableType->variableEnum = STRING_VARIABLE;
    s->variableType->stringValue = value;
    ptr = eptr + strlen(split);
  }
  value = (char *) calloc(sizeof(char), (strlen(ptr) + 1));
  strncpy(value, ptr, strlen(ptr));
  s = getSymbolArrayValue(dest->variableType->arrayValues, i++);
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = value;

  return E_OK;
}

/*********************************************************************
* callMapLengthFunction...
*
* Get the length of this array.
*********************************************************************/
int callMapLengthFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  MapNode *node = NULL;
  int size = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  dest->variableType->variableEnum = INT_VARIABLE;
  node = getFirstMapNode(s->variableType->mapValues);
  while (node != NULL) {
	  size++;
	  node = getNextMapNode(node, s->variableType->mapValues);
  }
  dest->variableType->intValue = size;
  return E_OK;
}

/*********************************************************************
* callArrayLengthFunction...
*
* Get the length of this array.
*********************************************************************/
int callArrayLengthFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  MapNode *node = NULL;
  int size = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  dest->variableType->variableEnum = INT_VARIABLE;
  node = getFirstMapNode(s->variableType->arrayValues);
  while (node != NULL) {
	  size++;
	  node = getNextMapNode(node, s->variableType->arrayValues);
  }
  dest->variableType->intValue = size;
  return E_OK;
}

/*********************************************************************
* callURLBaseFunction...
*
* Conditionally re-write a url.
*********************************************************************/
int callURLBaseFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *url = NULL, *newurl = NULL, *params = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  url = s->variableType->stringValue;

  if (getRewriteEnabled()) {
    newurl = strdup(url?url:"");
  } else {
    if (strncasecmp(url, "/cms/", 5) == 0) {
      vstrdupcat(&newurl, getURLBase(), (url+5), NULL);
      // will always find one
      params = strchr(newurl, '?');

      // if we find a second change it to an &
      params++;
      params = strchr(params, '?');
      
      if (params)
	*params = '&';
    } else {
      newurl = strdup(url?url:"");
    }
  }

  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = newurl;
  return E_OK;
}

/*********************************************************************
* callURLRewriteFunction...
*
* Conditionally re-write a url.
*********************************************************************/
int callURLRewriteFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *url = NULL, *newurl = NULL, *params = NULL, *output = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  url = s->variableType->stringValue;

  if (getRewriteEnabled()) {
    newurl = strdup(url?url:"");
  } else {
    if (strncasecmp(url, "/cms/", 5) == 0) {
      vstrdupcat(&newurl, getURLBase(), (url+5), NULL);
      // will always find one
      params = strchr(newurl, '?');

      // if we find a second change it to an &
      params++;
      params = strchr(params, '?');
      
      if (params)
	*params = '&';
    } else {
      newurl = strdup(url?url:"");
    }
  }
  output = info->output;

  vstrdupcat(&output, newurl, NULL);
  info->output = output;
  dhufree(newurl);

  dest->variableType->variableEnum = INT_VARIABLE;
  dest->variableType->intValue = 0;
  return E_OK;
}

/*********************************************************************
* callCSVEscapeFunction...
*
* CSVEscape a string.
*********************************************************************/
int callCSVEscapeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tok = NULL, *value = NULL, *ptr = NULL, *dst = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  tok = s->variableType->stringValue;

  value = (char *) dhumalloc(sizeof(char) * ((strlen(tok) * 2) + 1));
  dst = value;
  ptr = tok;

  while (*ptr != CNULL) {
    switch (*ptr) {
      case '"':
        *dst++ = '"';
        *dst++ = '"';
        ptr++;
        break;
      default:
        *dst++ = *ptr++;
        break;
    }
  }
  *dst = CNULL;
  
  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = value;
  return E_OK;
}

/*********************************************************************
* callCapitaliseFunction...
*
* Capitalise a string.
*********************************************************************/
int callCapitaliseFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tok = NULL, *value = NULL, *ptr = NULL, *dst = NULL;
  int inword = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  tok = s->variableType->stringValue;

  value = (char *) dhumalloc(sizeof(char) * ((strlen(tok)) + 1));
  dst = value;
  ptr = tok;

  while (*ptr != CNULL) {
    if (isalnum(*ptr) && !inword) {
	    *dst = toupper(*ptr);
	    inword = 1;
    } else if (!isalnum(*ptr)) {
	    *dst = *ptr;
	    inword = 0;
    } else {
	    *dst = *ptr;
    }
    ptr++;
    dst++;
  }
  *dst = CNULL;
  
  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = value;
  return E_OK;
}

/*********************************************************************
* callXMLEscapeFunction...
*
* XMLEscape a string.
*********************************************************************/
int callXMLEscapeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tok = NULL, *value = NULL, *ptr = NULL, *dst = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  tok = s->variableType->stringValue;

  value = (char *) dhumalloc(sizeof(char) * ((strlen(tok) * 6) + 1));
  dst = value;
  ptr = tok;

  while (*ptr != CNULL) {
    switch (*ptr) {
      case '&':
        *dst++ = '&';
        *dst++ = 'a';
        *dst++ = 'm';
        *dst++ = 'p';
        *dst++ = ';';
        ptr++;
        break;
      case '<':
        *dst++ = '&';
        *dst++ = 'l';
        *dst++ = 't';
        *dst++ = ';';
        ptr++;
        break;
      case '>':
        *dst++ = '&';
        *dst++ = 'g';
        *dst++ = 't';
        *dst++ = ';';
        ptr++;
        break;
      case '"':
        *dst++ = '&';
        *dst++ = 'q';
        *dst++ = 'u';
        *dst++ = 'o';
        *dst++ = 't';
        *dst++ = ';';
        ptr++;
        break;
      case '\'':
        *dst++ = '&';
        *dst++ = '#';
        *dst++ = '3';
        *dst++ = '9';
        *dst++ = ';';
        ptr++;
        break;
      case '%':
        *dst++ = '&';
        *dst++ = '#';
        *dst++ = '3';
        *dst++ = '7';
        *dst++ = ';';
        ptr++;
        break;
      default:
        *dst++ = *ptr++;
        break;
    }
  }
  *dst = CNULL;
  
  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = value;
  return E_OK;
}

/*********************************************************************
* callEscapeFunction...
*
* URLEncode a string.
*********************************************************************/
int callEscapeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *val = NULL, *tok = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  tok = s->variableType->stringValue;

  val = URLEncode(tok);
  
  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = val;
  return E_OK;
}

/*********************************************************************
* callGetVersionFunction...
*
* Return the version of this server.
*********************************************************************/
int callGetVersionFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = dhustrdup(VERSIONINFO);
  return E_OK;
}

/*********************************************************************
* callGetISODateFunction...
*
* Return the version of this server.
*********************************************************************/
int callGetISODateFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  char *date = NULL, *e = NULL;
  time_t t;
  LinkedList *arg = NULL;
  Symbol *s = NULL;

  arg = list;
  s = (Symbol *) arg->data;

  t = s->variableType->intValue;
  if (t <= 0) {
    t = time(NULL);
  }

  date = ctime(&t);

  e = date + strlen(date) -1;

  while (isspace(*e) && e > date) {*e = '\0'; e--;}

  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = dhustrdup(date);
  return E_OK;
}

static int randseeded = 0;

/*********************************************************************
* callRandFunction...
*
* return a random number.
*********************************************************************/
int callRandFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  int r = 0;
  time_t t;

  if (!randseeded) {
  	t = time(NULL);
  	srand(t);
	randseeded = 1;
  }
  r = rand();

  dest->variableType->variableEnum = INT_VARIABLE;
  dest->variableType->intValue = r;
  return E_OK;
}

/*********************************************************************
* callUnescapeFunction...
*
* URLEncode a string.
*********************************************************************/
int callUnescapeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *val = NULL, *tok = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  tok = s->variableType->stringValue;

  val = URLDecode(tok);
  
  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = val;
  return E_OK;
}

/*********************************************************************
* callGetErrorMessageFunction...
*
* Translate an error code into a message.
*********************************************************************/
int callGetErrorMessageFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *val = NULL;
  int errcode = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  errcode = s->variableType->intValue;

  val = dhustrdup(getErrorMesg(errcode));
  
  dest->variableType->variableEnum = STRING_VARIABLE;
  dest->variableType->stringValue = val;
  return E_OK;
}

/*********************************************************************
* callRecoverObjectVersion...
*
* Recover a previous version of this deleted document.
*********************************************************************/
int callRecoverObjectVersionFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *super = NULL;
  int errcode = 0, objid = 0, access = 1, versionid = 0, index = 0;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;
  if (isDeletedXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  versionid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  index = s->variableType->intValue;

  super = getEnvValue("ISSUPERUSER", info->env);
  if (super != NULL && *super == 'y') {
	  access = 1;
  }
  
  if (!access) {
    logInfo("Internal Recover Object Version Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  } 

  errcode = rollbackObjectVersion(
			objid, 
			versionid, 
			index,
  			info->env, 
			info->sqlsock);

  dest->variableType->intValue = errcode;
  logInfo("Internal Recover Object Version Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callRollbackObjectVersion...
*
* Revert to a previous version of this document.
*********************************************************************/
int callRollbackObjectVersionFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, objid = 0, access = 1, versionid = 0, index = 0;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  versionid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  index = s->variableType->intValue;

  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK &&
      userHasReadAccess(versionid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Rollback Object Version Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  } 

  errcode = rollbackObjectVersion(
			objid, 
			versionid, 
			index,
  			info->env, 
			info->sqlsock);

  dest->variableType->intValue = errcode;
  logInfo("Internal Rollback Object Version Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callUnLockObjectFunction...
*
* Remove a lock on this object (everyone else can change it).
*********************************************************************/
int callUnLockObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *uid = NULL, *super = NULL;
  int errcode = 0, objid = 0, access = 1;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  uid = getEnvValue("USERID", info->env);
  super = getEnvValue("ISSUPERUSER", info->env);

  if ((super == NULL || *super != 'y') && isObjectLockedByUser(objid, strtol(uid?uid:"-2", NULL, 10), info->sqlsock) != E_OK) {
    access = 0;
  }

  if (!access) {
    logInfo("Internal Unlock Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  } 

  errcode = unLockObjectDB(objid, info->sqlsock);
  
  logInfo("Internal Unlock Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLockObjectFunction...
*
* Puts a lock on this object (noone else can change it).
*********************************************************************/
int callLockObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *uid = NULL;
  int errcode = 0, objid = 0, access = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  dest->variableType->variableEnum = INT_VARIABLE;

  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  uid = getEnvValue("USERID", info->env);

  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (isObjectLocked(objid, info->sqlsock) == E_OK) {
    access = 0;
  }

  if (!access) {
    logInfo("Internal Lock Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  } 

  errcode = lockObjectDB(objid, strtol(uid?uid:"-1", NULL, 10), info->sqlsock);
  
  logInfo("Internal Lock Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLogoutFunction...
*
* Deletes a valid session for this user.
*********************************************************************/
int callLogoutFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  char *session = NULL, *cookie = NULL;
  int errcode = 0;

  session = getEnvValue("CMSSESSION", info->env);
  dest->variableType->variableEnum = INT_VARIABLE;
  
  errcode = removeValidSession(session, info->sqlsock);
  if (errcode != E_OK) {
    logInfo("Logout Request. (Error = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  vstrdupcat(&cookie, "CMSSESSION=", NULL);
  setTokenValue(dhustrdup(COOKIE_DATA), cookie, info->env);
  setTokenValue(dhustrdup("USERID"), dhustrdup("-1"), info->env);
  
  logInfo("Logout Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoginFunction...
*
* Create a valid session for this user.
*********************************************************************/
int callLoginFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *user = NULL, *pass = NULL, *sess = NULL, *cookie = NULL, *fullname = NULL, *email = NULL, *usertype = NULL;
  int errcode = 0, super = 0;
  int uid = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  user = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  pass = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  
  errcode = validateUser(user, pass, &uid, &super, &fullname, &email, &usertype, info->sqlsock);
  if (errcode != E_OK) {
    logInfo("User %s Login Request. (Error = %s)\n", user, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  errcode = createValidSession(user, pass, uid, super, fullname, email, usertype, &sess, info->sqlsock, info->env);
  if (errcode != E_OK) {
    logInfo("User %s Login Request. (Error = %s)\n", user, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }
  dhufree(fullname);
  dhufree(email);
  dhufree(usertype);

  vstrdupcat(&cookie, "CMSSESSION=", sess, NULL);
  dhufree(sess);
  setTokenValue(dhustrdup(COOKIE_DATA), cookie, info->env);

  logInfo("User %s Login Request. (Success = %s)\n", user, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoadDeletedRootFolderContentsFunction...
*
* Load the list of contents in a folder that have been deleted.
*********************************************************************/
int callLoadDeletedRootFolderContentsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *filter = NULL, *sort = NULL, *super = NULL;
  int errcode = 0;
  int min = 0, max = 0, access = 0;
  int *objs = NULL, numobjs = 0, i = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  sort = s->variableType->stringValue;

  super = getEnvValue("ISSUPERUSER", info->env);

  if (super != NULL && *super == 'y') {
	  access = 1;
  }

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);
  if (!access) {
    logInfo("Internal List Deleted Root Folder Contents Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    return E_OK;
  }
  
  if ((errcode = loadDeletedFolderContents(-1, filter, min, max, sort, info->sqlsock, info->env, &objs, &numobjs)) != E_OK) {
    return E_OK;
  }

  for (i = 0; i < numobjs; i++) {
    // get the path info

    getObjectDetails(objs[i], &details, info->sqlsock);
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);

    // insert these map values
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "fileID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = objs[i];
    
    m = getSymbolMapValue(s->variableType->mapValues, "path");
    m->variableType->variableEnum = STRING_VARIABLE;
    m->variableType->stringValue = strdup(details->path);

    freeObjectDetails(details);
  }

  dhufree(objs);
  logInfo("Internal List Deleted Root Folder Contents Request. (Success = %s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadFolderContentsLengthFunction...
*
* Load the list of contents in a folder.
*********************************************************************/
int callLoadFolderContentsLengthFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *filter = NULL, *xref = NULL;
  int errcode = 0;
  int *objs = NULL, numobjs = 0, objid = 0;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = 0;
    return E_OK;
  }


  if ((errcode = loadFolderContents(objid, filter, 0, -1, "", info->sqlsock, info->env, &objs, &numobjs)) != E_OK) {
    dest->variableType->intValue = 0;
    return E_OK;
  }

  dhufree(objs);

  logInfo("Internal List Folder Contents Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = numobjs;
  return E_OK;
}

/*********************************************************************
* callLoadRootFolderContentsLengthFunction...
*
* Load the list of contents in a folder.
*********************************************************************/
int callLoadRootFolderContentsLengthFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *filter = NULL;
  int errcode = 0;
  int *objs = NULL, numobjs = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  
  if ((errcode = loadFolderContents(-1, filter, 0, -1, "", info->sqlsock, info->env, &objs, &numobjs)) != E_OK) {
    dest->variableType->intValue = 0;
    return E_OK;
  }

  dhufree(objs);

  logInfo("Internal List Root Folder Contents Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = numobjs;
  return E_OK;
}

/*********************************************************************
* callLoadRootFolderContentsFunction...
*
* Load the list of contents in a folder.
*********************************************************************/
int callLoadRootFolderContentsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *filter = NULL, *sort = NULL;
  int errcode = 0;
  int min = 0, max = 0;
  int *objs = NULL, numobjs = 0, i = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  sort = s->variableType->stringValue;
  
  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if ((errcode = loadFolderContents(-1, filter, min, max, sort, info->sqlsock, info->env, &objs, &numobjs)) != E_OK) {
    logInfo("Internal List Root Folder Contents Request. (Success = %s)\n", getErrorMesg(errcode));
    return E_OK;
  }

  for (i = 0; i < numobjs; i++) {
    getObjectDetails(objs[i], &details, info->sqlsock);
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    
    // insert these map values
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "fileID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = objs[i];
    
    m = getSymbolMapValue(s->variableType->mapValues, "path");
    m->variableType->variableEnum = STRING_VARIABLE;
    m->variableType->stringValue = strdup(details->path);

    freeObjectDetails(details);
  }

  dhufree(objs);
  logInfo("Internal List Root Folder Contents Request. (Success = %s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadPermissionBitsFunction...
*
* Loads the list of permissions attached to an object.
*********************************************************************/
int callLoadPermissionBitsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *mask = NULL;
  int errcode = 0, access = 0, objid = 0, gid = 0;

  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  gid = s->variableType->intValue;

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }
  
  if (!access) {
    logInfo("Internal Retrieve Permission Bits Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  if ((errcode = loadPermissionMask(objid, gid, info->sqlsock, &mask)) != E_OK) {
    logInfo("Internal Load Permission Mask. (Ref=%s, Success = %s)\n", xref, getErrorMesg(errcode));
    return E_OK;
  }
  
  s = getSymbolMapValue(dest->variableType->mapValues, "read");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = (mask[0] == 'r');
  s = getSymbolMapValue(dest->variableType->mapValues, "write");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = (mask[1] == 'w');
  s = getSymbolMapValue(dest->variableType->mapValues, "execute");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = (mask[2] == 'x');

  logInfo("Internal Load Permission Bits Request. (Ref=%s, Group=%d, Success=%s)\n", xref, gid, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callGetAllSessionDataFunction...
*
* Gets all the data against this object.
*********************************************************************/
int callGetAllSessionDataFunction(LinkedList *list, 
			  	  ParserInfo *info,
                                   Symbol *dest) {
  char *key = NULL, **columns = NULL, *value = NULL;
  int errcode = 0, numcols = 0, i = 0;
  Symbol *s = NULL;
  
  key = getEnvValue("CMSSESSION", info->env);

  errcode = getAllSessionData(key, &columns, &numcols, info->sqlsock);
  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  for (i = 0; i < numcols; i++) {
    s = getSymbolMapValue(dest->variableType->mapValues, columns[i]);
    s->variableType->variableEnum = STRING_VARIABLE;
    errcode = getSessionData(key, columns[i], &value, info->sqlsock);
    s->variableType->stringValue = value;


    dhufree(columns[i]);
  }
  dhufree(columns);
   
  logInfo("Internal Get Session Data Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callMoveObjectFunction...
*
* Moves this document (current user only).
*********************************************************************/
int callMoveObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, folderid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  if (*xref == CNULL) {
    folderid = -1; 
  } else {
    if (isXRefValid(xref, -1, info->sqlsock, info->env, &folderid) != E_OK) {
      dest->variableType->intValue = INVALIDXPATH;
      return E_OK;
    }
  }

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Move Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK ||
      userHasWriteAccess(folderid, info->env, info->sqlsock)) {
    logInfo("Internal Move Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }


  errcode = moveObject(objid, folderid, info->env, info->sqlsock);

  logInfo("Internal Move Object Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callRemoveNotificationSettingsFunction...
*
* Removes all notification settings from this document.
*********************************************************************/
int callRemoveNotificationSettingsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0;
  int objid = 0, access = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Remove Notification Settings Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = removeNotificationSettings(objid, info->sqlsock);

  logInfo("Internal Remove Notification Settings Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callRemoveWorkflowSettingsFunction...
*
* Removes all workflow settings from this document.
*********************************************************************/
int callRemoveWorkflowSettingsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *super = NULL;
  int errcode = 0;
  int objid = 0, access = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  super = getEnvValue("ISSUPERUSER", info->env);

  access = 1;
  if ((super == NULL || *super != 'y')) {
    access = 0;
  }

  if (!access) {
    logInfo("Internal Remove Workflow Settings Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = removeWorkflowSettings(objid, info->sqlsock);

  logInfo("Internal Remove Workflow Settings Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callApproveObjectFunction...
*
* Approves this document (current user only).
*********************************************************************/
int callApproveObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Approve Workflow Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = approveObject(objid, uid, info->sqlsock);

  logInfo("Internal Approve Workflow Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callAttachNotificationSettingsFunction...
*
* Sets the notification settings for this item.
*********************************************************************/
int callAttachNotificationSettingsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, groupid = 0;
  int objid = 0, access = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }
    
  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Set Notification Settings Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }
 
  arg = arg->next;
  s = (Symbol *) arg->data;
  groupid = s->variableType->intValue;
  
  errcode = saveNotificationSettings(objid, groupid, info->sqlsock);

  logInfo("Internal Set Notification Settings Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callAttachWorkflowSettingsFunction...
*
* Sets the workflow settings for this item.
*********************************************************************/
int callAttachWorkflowSettingsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *super = NULL;
  int errcode = 0, groupid = 0, all = 0;
  int objid = 0, access = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }
    
  super = getEnvValue("ISSUPERUSER", info->env);

  access = 1;
  if ((super == NULL || *super != 'y')) {
    access = 0;
  }


  if (!access) {
    logInfo("Internal Set Workflow Settings Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }
 
  arg = arg->next;
  s = (Symbol *) arg->data;
  groupid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  all = s->variableType->intValue;
  
  errcode = saveWorkflowSettings(objid, groupid, all, info->sqlsock);

  logInfo("Internal Set Workflow Settings Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callGetNotificationSettingsFunction...
*
* Gets the notification settings for this item.
*********************************************************************/
int callGetNotificationSettingsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, groupid = 0;
  int objid = 0, access = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;

  dest->variableType->variableEnum = INT_VARIABLE;

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = -1;
    return E_OK;
  }

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Get Notification Settings Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = -1;
    return E_OK;
  }

  errcode = loadNotificationSettings(objid, &groupid, info->sqlsock);

  if (errcode == E_OK) {
    dest->variableType->intValue = groupid;
  } else {
    dest->variableType->intValue = -1;
  }
  
  logInfo("Internal Get Workflow Settings Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadWorkflowSettingsFunction...
*
* Gets the workflow settings for this item.
*********************************************************************/
int callLoadWorkflowSettingsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, groupid = 0, all = 0;
  int objid = 0, access = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Get Workflow Settings Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  errcode = loadWorkflowSettings(objid, &groupid, &all, info->sqlsock);

  if (errcode == E_OK) {
    s = getSymbolMapValue(dest->variableType->mapValues, "groupID");
    s->variableType->variableEnum = INT_VARIABLE;
    s->variableType->intValue = groupid;
    s = getSymbolMapValue(dest->variableType->mapValues, "requiresAll");
    s->variableType->variableEnum = INT_VARIABLE;
    s->variableType->intValue = all;
  }
  
  logInfo("Internal Get Workflow Settings Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}


/*********************************************************************
* callGetSessionDataFunction...
*
* Gets this data element against this object.
*********************************************************************/
int callGetSessionDataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *key = NULL, *name = NULL, *value = NULL;
  int errcode = 0;

  arg = list;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  
  key = getEnvValue("CMSSESSION", info->env);

  errcode = getSessionData(key, name, &value, info->sqlsock);
  dest->variableType->variableEnum = STRING_VARIABLE;
  if (errcode == E_OK) {
    dest->variableType->stringValue = value;
  } else {
    dest->variableType->stringValue = strdup("");
  }
   
  logInfo("Internal Get Session Data Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callRemoveSessionDataFunction...
*
* Removes this data element against this object.
*********************************************************************/
int callRemoveSessionDataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *key = NULL, *name = NULL;
  int errcode = 0;

  arg = list;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  
  key = getEnvValue("CMSSESSION", info->env);

  errcode = removeSessionData(key, name, info->sqlsock);
   
  logInfo("Internal Remove Session Data Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->variableEnum = INT_VARIABLE;
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callSetSessionDataFunction...
*
* Sets this data element against this object.
*********************************************************************/
int callSetSessionDataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *key = NULL, *name = NULL, *value = NULL;
  int errcode = 0;

  arg = list;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  value = s->variableType->stringValue;
  
  key = getEnvValue("CMSSESSION", info->env);

  errcode = setSessionData(key, name, value, info->sqlsock);
   
  logInfo("Internal Set Session Data Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->variableEnum = INT_VARIABLE;
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callGetAllUserMetadataFunction...
*
* Gets all the metadata against this object.
*********************************************************************/
int callGetAllUserMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *uidstr = NULL, **columns = NULL, *super = NULL, *value = NULL;
  int errcode = 0, access = 0, uid = 0, thisuid = 0, numcols = 0, i = 0;

  arg = list;
  s = (Symbol *) arg->data;
  uid = s->variableType->intValue;

  super = getEnvValue("ISSUPERUSER", info->env);
  uidstr = getEnvValue("USERID", info->env);

  thisuid = strtol(uidstr?uidstr:"-2", NULL, 10);
  
  if ((super != NULL && *super == 'y') || (thisuid == uid)) {
    access = 1;
  }

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  if (!access) {
    logInfo("Internal Get All User Metadata Request. (UID = %d, Success = %s)\n", uid, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  errcode = getAllUserMetadata(uid, &columns, &numcols, info->sqlsock);
  
  for (i = 0; i < numcols; i++) {
    s = getSymbolMapValue(dest->variableType->mapValues, columns[i]);

    errcode = getUserMetadata(uid, columns[i], &value, info->sqlsock);
    if (errcode == E_OK) {
      s->variableType->stringValue = value;
    } else {
      s->variableType->stringValue = strdup("");
    }

    dhufree(columns[i]);
  }
  dhufree(columns);
   
  logInfo("Internal Get All User Metadata Request. (UID=%d, Success=%s)\n", uid, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callGetAllObjectMetadataFunction...
*
* Gets all the metadata against this object.
*********************************************************************/
int callGetAllObjectMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, **columns = NULL, *value = NULL;
  int errcode = 0, access = 0, objid = 0, numcols = 0, i = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Get Object Metadata Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  errcode = getAllObjectMetadata(objid, &columns, &numcols, info->sqlsock);
  
  for (i = 0; i < numcols; i++) {
    s = getSymbolMapValue(dest->variableType->mapValues, columns[i]);
    s->variableType->variableEnum = STRING_VARIABLE;
    errcode = getObjectMetadata(objid, columns[i], &value, info->sqlsock);
    if (errcode == 0) {
	    s->variableType->stringValue = value;
    } else {
	    s->variableType->stringValue = strdup("");
    }
    dhufree(columns[i]);
  }
  dhufree(columns);
   
  logInfo("Internal Get Object Metadata Request. (Ref=%s, Success=%s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callGetUserMetadataFunction...
*
* Gets this metadata element against this object.
*********************************************************************/
int callGetUserMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *uidstr = NULL, *name = NULL, *value = NULL, *super = NULL;
  int errcode = 0, access = 0, uid = 0, thisuid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  uid = s->variableType->intValue;

  arg = arg->next;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;

  super = getEnvValue("ISSUPERUSER", info->env);
  uidstr = getEnvValue("USERID", info->env);

  thisuid = strtol(uidstr?uidstr:"-2", NULL, 10);
  
  if ((super != NULL && *super == 'y') || (thisuid == uid)) {
    access = 1;
  }

  dest->variableType->variableEnum = STRING_VARIABLE;
  if (!access) {
    logInfo("Internal Get User Metadata Request. (UID = %d, Success = %s)\n", uid, getErrorMesg(ACCESSDENIED));
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  errcode = getUserMetadata(uid, name, &value, info->sqlsock);

  if (errcode == E_OK) {
    dest->variableType->stringValue = value;
  } else {
    dest->variableType->stringValue = strdup("");
  }
   
  logInfo("Internal Get User Metadata Request. (UID=%d, Success=%s)\n", uid, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callGetDeletedObjectMetadataFunction...
*
* Gets this metadata element against this object.
*********************************************************************/
int callGetDeletedObjectMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *name = NULL, *value = NULL;
  int errcode = 0, access = 0, objid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  objid = s->variableType->intValue;
  dest->variableType->variableEnum = STRING_VARIABLE;
  arg = arg->next;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Get Object Metadata Request. (FileID = %d, Success = %s)\n", objid, getErrorMesg(ACCESSDENIED));
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  errcode = getObjectMetadata(objid, name, &value, info->sqlsock);

  if (errcode == E_OK) {
    dest->variableType->stringValue = value;
  } else {
    dest->variableType->stringValue = strdup("");
  }
   
  logInfo("Internal Get Object Metadata Request. (FileID=%d, Success=%s)\n", objid, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callGetObjectMetadataFunction...
*
* Gets this metadata element against this object.
*********************************************************************/
int callGetObjectMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *name = NULL, *value = NULL;
  int errcode = 0, access = 0, objid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = STRING_VARIABLE;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Get Object Metadata Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  errcode = getObjectMetadata(objid, name, &value, info->sqlsock);

  if (errcode == E_OK) {
    dest->variableType->stringValue = value;
  } else {
    dest->variableType->stringValue = strdup("");
  }
   
  logInfo("Internal Get Object Metadata Request. (Ref=%s, Success=%s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callRemoveObjectMetadataFunction...
*
* Removes this metadata element against this object.
*********************************************************************/
int callRemoveObjectMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *name = NULL;
  int errcode = 0, access = 0, objid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Remove Object Metadata Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = removeObjectMetadata(objid, name, info->sqlsock);
   
  logInfo("Internal Remove Object Metadata Request. (Ref=%s, Success=%s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callRemoveUserMetadataFunction...
*
* Removes this metadata element against this object.
*********************************************************************/
int callRemoveUserMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *super = NULL, *uidstr = NULL, *name = NULL;
  int errcode = 0, access = 0, uid = 0, thisuid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  uid = s->variableType->intValue;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = arg->next;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  
  super = getEnvValue("ISSUPERUSER", info->env);
  uidstr = getEnvValue("USERID", info->env);

  thisuid = strtol(uidstr?uidstr:"-2", NULL, 10);
  
  if ((super != NULL && *super == 'y') || (thisuid == uid)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Remove User Metadata Request. (UID=%d, Success = %s)\n", uid, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = removeUserMetadata(uid, name, info->sqlsock);
   
  logInfo("Internal Remove User Metadata Request. (UID=%d, Success=%s)\n", uid, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callSetUserMetadataFunction...
*
* Sets this metadata element against this object.
*********************************************************************/
int callSetUserMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *super = NULL, *uidstr = NULL, *name = NULL, *value = NULL;
  int errcode = 0, access = 0, uid = 0, thisuid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  uid = s->variableType->intValue;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = arg->next;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  value = s->variableType->stringValue;
  
  super = getEnvValue("ISSUPERUSER", info->env);
  uidstr = getEnvValue("USERID", info->env);

  thisuid = strtol(uidstr?uidstr:"-2", NULL, 10);
  
  if ((super != NULL && *super == 'y') || (thisuid == uid)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Set User Metadata Request. (UID = %d, Success = %s)\n", uid, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = setUserMetadata(uid, name, value, info->sqlsock);
   
  logInfo("Internal Set User Metadata Request. (UID=%d, Success=%s)\n", uid, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callSetObjectMetadataFunction...
*
* Sets this metadata element against this object.
*********************************************************************/
int callSetObjectMetadataFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *name = NULL, *value = NULL;
  int errcode = 0, access = 0, objid = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  name = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  value = s->variableType->stringValue;
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Set Object Metadata Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = setObjectMetadata(objid, name, value, info->sqlsock);
   
  logInfo("Internal Set Object Metadata Request. (Ref=%s, Success=%s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoadPermissionListFunction...
*
* Loads the list of permissions attached to an object.
*********************************************************************/
int callLoadPermissionListFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *xref = NULL, *filter = NULL, *mask = NULL;
  int errcode = 0, access = 0, objid = 0, min = 0, max = 0;
  int *groups = 0, numgroups = 0, i = 0;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Permission List Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }
   
  if ((errcode = loadPermissionList(objid, filter, min, max, info->sqlsock, &groups, &numgroups)) != E_OK) {
    logInfo("Internal Load Permission List Request. (Ref=%s, Filter = %s, Success = %s)\n", xref, filter, getErrorMesg(errcode));
    return E_OK;
  }

  for (i = 0; i < numgroups; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "groupID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = groups[i];

    errcode = loadPermissionMask(objid, groups[i], info->sqlsock, &mask);
    if (errcode == E_OK) {
      m = getSymbolMapValue(s->variableType->mapValues, "read");
      m->variableType->variableEnum = INT_VARIABLE;
      m->variableType->intValue = mask[0] == 'r';
      m = getSymbolMapValue(s->variableType->mapValues, "write");
      m->variableType->variableEnum = INT_VARIABLE;
      m->variableType->intValue = mask[1] == 'w';
      m = getSymbolMapValue(s->variableType->mapValues, "execute");
      m->variableType->variableEnum = INT_VARIABLE;
      m->variableType->intValue = mask[2] == 'x';
      dhufree(mask);
    } else {
      m = getSymbolMapValue(s->variableType->mapValues, "read");
      m->variableType->variableEnum = INT_VARIABLE;
      m->variableType->intValue = 0;
      m = getSymbolMapValue(s->variableType->mapValues, "write");
      m->variableType->variableEnum = INT_VARIABLE;
      m->variableType->intValue = 0;
      m = getSymbolMapValue(s->variableType->mapValues, "execute");
      m->variableType->variableEnum = INT_VARIABLE;
      m->variableType->intValue = 0;
    }

  }

  dhufree(groups);

  logInfo("Internal Load Permission List Request. (Ref=%s, Filter=%s, Results=%d, Success=%s)\n", xref, filter, numgroups, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadUsersGroupsFunction...
*
* Load a filtered list of users groups.
*********************************************************************/
int callLoadUsersGroupsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *filter = NULL;
  int errcode = 0;
  int min = 0, max = 0, id = 0;
  int *groups = NULL, numgroups = 0, i = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  id = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  
  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if ((errcode = loadUsersGroups(id, filter, min, max, info->sqlsock, info->env, &groups, &numgroups)) != E_OK) {
    logInfo("Internal Load Group List Request. (Filter = %s, Success = %s)\n", filter, getErrorMesg(errcode));
    return E_OK;
  }

  for (i = 0; i < numgroups; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "groupID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = groups[i];
  }

  dhufree(groups);
  logInfo("Internal Load Group Members Request. (Filter = %s, Results = %d, Success = %s)\n", filter, numgroups, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadGroupMembersFunction...
*
* Load a filtered list of group members.
*********************************************************************/
int callLoadGroupMembersFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *filter = NULL;
  int errcode = 0;
  int min = 0, max = 0, id = 0;
  int *users = NULL, numusers = 0, i = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  id = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  
  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if ((errcode = loadGroupMembers(id, filter, min, max, info->sqlsock, info->env, &users, &numusers)) != E_OK) {
    logInfo("Internal Load Group List Request. (Filter = %s, Success = %s)\n", filter, getErrorMesg(errcode));
    return E_OK;
  }

  for (i = 0; i < numusers; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);

    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "userID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = users[i];
  }

  dhufree(users);
  logInfo("Internal Load Group Members Request. (Filter = %s, Results = %d, Success = %s)\n", filter, numusers, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadGroupListFunction...
*
* Load a filtered list of groups.
*********************************************************************/
int callLoadGroupListFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  Symbol *m = NULL;
  char *filter = NULL;
  int errcode = 0;
  int min = 0, max = 0;
  int *groups = NULL, numgroups = 0, i = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);
  
  if ((errcode = loadGroupList(filter, min, max, info->sqlsock, info->env, &groups, &numgroups)) != E_OK) {
    logInfo("Internal Load Group List Request. (Filter = %s, Success = %s)\n", filter, getErrorMesg(errcode));
    return E_OK;
  }

  for (i = 0; i < numgroups; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);

    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "groupID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = groups[i];
  }

  dhufree(groups);
  logInfo("Internal Load Group List Request. (Filter = %s, Results = %d, Success = %s)\n", filter, numgroups, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadUserListFunction...
*
* Load a filtered list of users.
*********************************************************************/
int callLoadUserListFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *filter = NULL;
  int errcode = 0;
  int min = 0, max = 0;
  int *users = NULL, numusers = 0, i = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);
  
  if ((errcode = loadUserList(filter, min, max, info->sqlsock, info->env, &users, &numusers)) != E_OK) {
    return E_OK;
  }

  for (i = 0; i < numusers; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);

    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "userID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = users[i];
  }

  dhufree(users);
  logInfo("Internal Load User List Request. (Filter = %s, Results = %d, Success = %s)\n", filter, numusers, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callSearchContentLengthFunction...
*
* Search the indexed documents in the system.
*********************************************************************/
int callSearchContentLengthFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *query = NULL, *folder = NULL;
  int errcode = 0;
  int *objs = NULL, *scores = NULL, numobjs = 0, folderid = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;

  s = (Symbol *) arg->data;
  query = strdup(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  folder = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;
  
  if (strcmp(folder, "") == 0 || strcmp(folder, "/") == 0) {
    folderid = -1;
  } else {
    if (isXRefValid(folder, -1, info->sqlsock, info->env, &folderid) != E_OK) {
      dest->variableType->intValue = 0;
      return E_OK;
    }
  }


  if ((errcode = searchForContent(query, 0, -1, folderid, info->sqlsock, info->env, &objs, &scores, &numobjs)) != E_OK) {
    dest->variableType->intValue = 0;
    return E_OK;
  }

  dest->variableType->intValue = numobjs;
  
  dhufree(objs);
  dhufree(scores);
  logInfo("Internal Search Request. (Query = %s, Results = %d, Success = %s)\n", query, numobjs, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callSearchContentFunction...
*
* Search the indexed documents in the system.
*********************************************************************/
int callSearchContentFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  Symbol *m = NULL;
  char *query = NULL, *folder = NULL;
  int errcode = 0;
  int min = 0, max = 0, count = 0;
  int *objs = NULL, *scores = NULL, numobjs = 0, i = 0, folderid = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;

  s = (Symbol *) arg->data;
  query = strdup(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  folder = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  
  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if (strcmp(folder, "") == 0 || strcmp(folder, "/") == 0) {
    folderid = -1;
  } else {
    if (isXRefValid(folder, -1, info->sqlsock, info->env, &folderid) != E_OK) {
      return E_OK;
    }
  }

  if ((errcode = searchForContent(query, min, max, folderid, info->sqlsock, info->env, &objs, &scores, &numobjs)) != E_OK) {
    return E_OK;
  }

  count = 0;
  for (i = 0; i < numobjs; i++) {
    if (getObjectDetails(objs[i], &details, info->sqlsock) == E_OK) {

      s = getSymbolArrayValue(dest->variableType->arrayValues, count++);

      s->variableType->variableEnum = MAP_VARIABLE;
      s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

      m = getSymbolMapValue(s->variableType->mapValues, "fileID");
      m->variableType->variableEnum = INT_VARIABLE;
      m->variableType->intValue = objs[i];
    
      m = getSymbolMapValue(s->variableType->mapValues, "score");
      m->variableType->variableEnum = INT_VARIABLE;
      m->variableType->intValue = scores[i];
    
      m = getSymbolMapValue(s->variableType->mapValues, "path");
      m->variableType->variableEnum = STRING_VARIABLE;
      m->variableType->stringValue = strdup(details->path);

      freeObjectDetails(details);
    }
  }

  dhufree(objs);
  dhufree(scores);
  logInfo("Internal Search Request. (Query = %s, Results = %d, Success = %s)\n", query, numobjs, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadWorkflowListFunction...
*
* Load the list of documents awaiting approval by this user.
*********************************************************************/
int callLoadWorkflowListFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *filter = NULL, *index = NULL;
  int errcode = 0;
  int min = 0, max = 0;
  int *objs = NULL, numobjs = 0, i = 0, userid = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;

  index = getEnvValue("userID", info->env);
  userid = strtol(index?index:"-1", NULL, 10);

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if (userid < 0) {
    logInfo("Internal List Active Workflows Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    return E_OK;
  }
  
  if ((errcode = loadWorkflowList(userid, filter, min, max, info->sqlsock, info->env, &objs, &numobjs)) != E_OK) {
    logInfo("Internal List Active Workflows Request. (Success = %s)\n", getErrorMesg(errcode));
    return E_OK;
  }

  for (i = 0; i < numobjs; i++) {
    getObjectDetails(objs[i], &details, info->sqlsock);
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);

    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "fileID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = objs[i];
    
    m = getSymbolMapValue(s->variableType->mapValues, "path");
    m->variableType->variableEnum = STRING_VARIABLE;
    m->variableType->stringValue = strdup(details->path);

    freeObjectDetails(details);
  }

  dhufree(objs);
  logInfo("Internal List Active Workflows Request. (Success = %s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadObjectVersionsFunction...
*
* Load the list of versions of a document.
*********************************************************************/
int callLoadObjectVersionsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *xref = NULL;
  int errcode = 0, objid = 0, access = 0;
  int min = 0, max = 0;
  int *objs = NULL, numobjs = 0, i = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  

  if ((userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) && 
      (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal List Version History Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  if ((errcode = loadObjectVersions(objid, min, max, info->sqlsock, info->env, &objs, &numobjs)) != E_OK) {
    logInfo("Internal List Version History Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    return E_OK;
  }

  for (i = 0; i < numobjs; i++) {
    getObjectDetails(objs[i], &details, info->sqlsock);
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);

    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "fileID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = objs[i];
    
    m = getSymbolMapValue(s->variableType->mapValues, "path");
    m->variableType->variableEnum = STRING_VARIABLE;
    m->variableType->stringValue = strdup(details->path);

    freeObjectDetails(details);
  }

  dhufree(objs);
  logInfo("Internal List Version History Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadDeletedFolderContentsFunction...
*
* Load the list of contents in a folder.
*********************************************************************/
int callLoadDeletedFolderContentsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *xref = NULL, *sort = NULL, *super = NULL, *filter = NULL;
  int errcode = 0, objid = 0, access = 0;
  int min = 0, max = 0;
  int *objs = NULL, numobjs = 0, i = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }
  arg = arg->next;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  sort = s->variableType->stringValue;
  
  super = getEnvValue("ISSUPERUSER", info->env);
  if (super != NULL && *super == 'y') {
    access = 1;	
  }

  if (!access) {
    logInfo("Internal List Deleted Folder Contents Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  if ((errcode = loadDeletedFolderContents(objid, filter, min, max, sort, info->sqlsock, info->env, &objs, &numobjs)) != E_OK) {
    return E_OK;
  }

  for (i = 0; i < numobjs; i++) {
    getObjectDetails(objs[i], &details, info->sqlsock);
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);

    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "fileID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = objs[i];
    
    m = getSymbolMapValue(s->variableType->mapValues, "path");
    m->variableType->variableEnum = STRING_VARIABLE;
    m->variableType->stringValue = strdup(details->path);

    freeObjectDetails(details);
  }

  dhufree(objs);
  logInfo("Internal List Deleted Folder Contents Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadfolderContentsFunction...
*
* Load the list of contents in a folder.
*********************************************************************/
int callLoadFolderContentsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *xref = NULL, *filter = NULL, *sort = NULL;
  int errcode = 0, objid = 0, access = 0;
  int min = 0, max = 0;
  int *objs = NULL, numobjs = 0, i = 0, online = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }
  arg = arg->next;
  s = (Symbol *) arg->data;
  filter = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  min = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  max = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  sort = s->variableType->stringValue;
  

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOTONLINE));
      return E_OK;
    }
  }

  if ((errcode = loadFolderContents(objid, filter, min, max, sort, info->sqlsock, info->env, &objs, &numobjs)) != E_OK) {
    logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    return E_OK;
  }

  for (i = 0; i < numobjs; i++) {
    getObjectDetails(objs[i], &details, info->sqlsock);
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);

    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "fileID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = objs[i];
    
    m = getSymbolMapValue(s->variableType->mapValues, "path");
    m->variableType->variableEnum = STRING_VARIABLE;
    m->variableType->stringValue = strdup(details->path);

    freeObjectDetails(details);
  }

  dhufree(objs);
  logInfo("Internal List Folder Contents Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callIncludeFunction...
*
* Include another page in this page.
*********************************************************************/
int callIncludeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *contents = NULL, *output = NULL, *thisoutput = NULL;
  int errcode = 0, objid = 0, access = 0, online = 0, timestamp = 0;
  int len = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, info->env), getEnvValue(CTIMETOK, info->env));
  if (isXRefValid(xref, timestamp, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOTONLINE));
      dest->variableType->intValue = ACCESSDENIED;
      return E_OK;
    }
  }

  thisoutput = info->output;
  info->output = output;
  
  if ((errcode = loadXRef(objid, &contents, &len, info)) != E_OK) {
    dest->variableType->intValue = errcode;
    return E_OK;
  }


  info->output = thisoutput;

  vstrdupcat(&(info->output), contents, NULL);
  
  logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoadObjectFunction...
*
* Load an object and put the contents in a variable.
*********************************************************************/
int callLoadObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *contents = NULL;
  int errcode = 0, objid = 0, access = 0, online = 0, timestamp = 0;
  int len = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, info->env), getEnvValue(CTIMETOK, info->env));

  dest->variableType->variableEnum = STRING_VARIABLE;

  if (isXRefValid(xref, timestamp, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }
  

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOTONLINE));
      dest->variableType->stringValue = strdup("");
      return E_OK;
    }
  }

  if ((errcode = loadXRef(objid, &contents, &len, info)) != E_OK) {
    logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }
  

  logInfo("Internal Retrieve Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  if (errcode == E_OK) {
    dest->variableType->stringValue = contents;
  } else {
    dest->variableType->stringValue = strdup("");
  }
  return E_OK;
}

/*********************************************************************
* callCreateNewGroupFunction...
*
* Create a new group.
*********************************************************************/
int callCreateNewGroupFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int ispublic = 0;
  char *gname = NULL, *super = NULL;
  int errcode = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;

  // ONLY SUPER USERS CAN CREATE GROUPS
  gname = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  ispublic = s->variableType->intValue;
  super = getEnvValue("ISSUPERUSER", info->env);
  
  if (super == NULL || *super != 'y') {
    logInfo("Internal Create Group Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }


  errcode = createNewGroup(gname, ispublic, info->sqlsock);

  logInfo("Internal Create Group Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCreateNewUserFunction...
*
* Create a new user.
*********************************************************************/
int callCreateNewUserFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *uname = NULL, *pword = NULL, *fname = NULL, *mysuper = NULL, *email = NULL;
  int super = 0, online = 0, errcode = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;  
  // ONLY SUPER USERS CAN CREATE USERS
  mysuper = getEnvValue("ISSUPERUSER", info->env);

  uname = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;  
  pword = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;  
  fname = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;  
  super = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;  
  online = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;  
  email = s->variableType->stringValue;
  s = (Symbol *) arg->data;  
    
  if ((mysuper == NULL || *mysuper != 'y') && (super)) {
    logInfo("Internal Create User Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = userLicenseCheck(info->sqlsock);

  if (errcode == E_OK) {
    errcode = createNewUser(uname, pword, fname, super, online, email, "INTERNAL", info->sqlsock);
  }

  logInfo("Internal Create User Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCreateNewFolderFunction...
*
* Create a new folder item.
*********************************************************************/
int callCreateNewFolderFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *title = NULL, *parent = NULL, *super = NULL;
  int errcode = 0, parentid = 0, access = 0, order = 0, ispublic = 0, issuper = 1;
  FileObject *file = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  parent = s->variableType->stringValue;
  if (strcmp(parent, "") == 0 || strcmp(parent, "/") == 0) {
    parentid = -1;
  } else {
    if (isXRefValid(parent, -1, info->sqlsock, info->env, &parentid) != E_OK) {
      dest->variableType->intValue = INVALIDXPATH;
      return E_OK;
    }
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  title = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  ispublic = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  order = s->variableType->intValue;
   
  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  if (issuper) {
    access = 1;
  } else {
    if (userHasWriteAccess(parentid, info->env, info->sqlsock) == E_OK) {
      access = 1;
    }
  }

  if (!access) {
    logInfo("Internal Create Folder Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  file = initFileObject(dhustrdup("file"),
                        dhustrdup("application/folder"),
                        dhustrdup("file.txt"),
                        dhustrdup(""),
                        0);

  errcode = createNewObject(parentid, title, ispublic, 
                                             "FOLDER", // type
                                             0, // do not index
					     "",
					     order,
                                             file,
                                             info->env,
                                             info->sqlsock);

  
  freeFileObjectList(file);

  logInfo("Internal Create Folder Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCreateNewTextObjectFunction...
*
* Create a new repository item.
*********************************************************************/
int callCreateNewTextObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *title = NULL, *contents = NULL, *parent = NULL, *type = NULL, *tplt = NULL, *super = NULL;
  int errcode = 0, parentid = 0, access = 0, order = 0, ispublic = 0, index = 0, issuper = 1;
  FileObject *file = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;

  parent = s->variableType->stringValue;
  if (strcmp(parent, "") == 0 || strcmp(parent, "/") == 0) {
    parentid = -1;
  } else {
    if (isXRefValid(parent, -1, info->sqlsock, info->env, &parentid) != E_OK) {
      dest->variableType->intValue = INVALIDXPATH;
      return E_OK;
    }
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  title = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  ispublic = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  type = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  index = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  tplt = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  order = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  contents = s->variableType->stringValue;

  if (strcmp(type?type:"", "FOLDER") == 0) {
    logInfo("Internal Create Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(INVALIDXPATH));
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  if (issuper) {
    access = 1;
  } else {
    if (userHasWriteAccess(parentid, info->env, info->sqlsock) == E_OK) {
      access = 1;
    }
  }

  if (!access) {
    logInfo("Internal Create Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  file = initFileObject(dhustrdup("file"),
                        dhustrdup("text/html"),
                        dhustrdup("file.html"),
                        dhustrdup(contents),
                        strlen(contents));

  errcode = createNewObject(parentid, title, ispublic, 
                                             type, 
                                             index, 
					     tplt,
					     order,
                                             file,
                                             info->env,
                                             info->sqlsock);

  logInfo("Internal Create Object Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCreateNewObjectFunction...
*
* Create a new repository item.
*********************************************************************/
int callCreateNewObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *title = NULL, *filename = NULL, *parent = NULL, *type = NULL, *tplt = NULL, *super = NULL;
  int errcode = 0, parentid = 0, access = 0, order = 0, ispublic = 0, index = 0, issuper = 1;
  FileObject *file = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  parent = s->variableType->stringValue;
  if (strcmp(parent, "") == 0 || strcmp(parent, "/") == 0) {
    parentid = -1;
  } else {
    if (isXRefValid(parent, -1, info->sqlsock, info->env, &parentid) != E_OK) {
      dest->variableType->intValue = INVALIDXPATH;
      return E_OK;
    }
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  title = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  ispublic = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  type = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  index = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  tplt = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  order = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  filename = s->variableType->stringValue;
  
  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  if (issuper) {
    access = 1;
  } else {
    if (userHasWriteAccess(parentid, info->env, info->sqlsock) == E_OK) {
      access = 1;
    }
  }

  if (!access) {
    logInfo("Internal Create Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  file = getFileObject(filename, info->env);
  if (!file) {
    logInfo("Internal Create Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOFILEINREQUEST));
    dest->variableType->intValue = NOFILEINREQUEST;
    return E_OK;
  }

  errcode = createNewObject(parentid, title, ispublic, 
                                             type, 
                                             index, 
 					     tplt,
					     order,
                                             file,
                                             info->env,
                                             info->sqlsock);

  logInfo("Internal Create Object Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callReplaceTextContentsFunction...
*
* Replace this file on disk.
*********************************************************************/
int callReplaceTextContentsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *contents = NULL, *tplt = NULL;
  int errcode = 0, objid = 0, access = 0, index = 0;
  FileObject *file = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }
  arg = arg->next;
  s = (Symbol *) arg->data;
  index = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  tplt = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  contents = s->variableType->stringValue;
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Replace Object Contents Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  file = initFileObject(dhustrdup("file"),
                        dhustrdup("text/html"),
                        dhustrdup("file.html"),
                        dhustrdup(contents),
                        strlen(contents));

  errcode = replaceObjectContents(objid,
                        index,
			tplt,
                        file,
                        info->env,
                        info->sqlsock);

  freeFileObjectList(file);
  logInfo("Internal Replace File Contents Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callReplaceFileContentsFunction...
*
* Replace this file on disk.
*********************************************************************/
int callReplaceFileContentsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *filename = NULL, *tplt = NULL;
  int errcode = 0, objid = 0, access = 0, index = 0;
  FileObject *file = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;  
  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }
  arg = arg->next;
  s = (Symbol *) arg->data;  
  index = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;  
  tplt = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;  
  
  filename = s->variableType->stringValue;
  
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Replace Object Contents Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  file = getFileObject(filename, info->env);
  if (!file) {
    logInfo("Internal Replace Object Contents Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOFILEINREQUEST));
    dest->variableType->intValue = NOFILEINREQUEST;
    return E_OK;
  }

  errcode = replaceObjectContents(objid, index, tplt, file, info->env, info->sqlsock);
  logInfo("Internal Replace File Contents Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  
  return E_OK;
}

/*********************************************************************
* callFileExistsFunction...
*
* Does this file exist in the info->environment?
*********************************************************************/
int callFileExistsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *filename = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  filename = s->variableType->stringValue;

  dest->variableType->variableEnum = INT_VARIABLE;
  if (fileObjectExists(filename, info->env)) {
    dest->variableType->intValue = 1;
  } else {
    dest->variableType->intValue = 0;
  }
  return E_OK;
}

/*********************************************************************
* callDeleteGroupFunction...
*
* Delete an object
*********************************************************************/
int callDeleteGroupFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *super = NULL;
  int errcode = 0, gid = 0;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  gid = s->variableType->intValue;

  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    logInfo("Internal Delete Group Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = deleteGroup(gid, info->sqlsock);

  logInfo("Internal Delete Group Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callDeleteUserFunction...
*
* Delete an object (just archives it).
*********************************************************************/
int callDeleteUserFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *super = NULL;
  int errcode = 0, uid = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data; 

  uid = s->variableType->intValue;

  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    logInfo("Internal Delete User Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = deleteUser(uid, info->sqlsock);

  logInfo("Internal Delete User Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCopyObjectFunction...
*
* Copy Object.
*********************************************************************/
int callCopyObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, objid = 0, access = 0, destid = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }
  arg = arg->next;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &destid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  if ((userHasWriteAccess(destid, info->env, info->sqlsock) == E_OK) && (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Copy Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = copyObject(objid, destid, -1, info->env, info->sqlsock);

  logInfo("Internal Copy Object Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callDeleteObjectFunction...
*
* Delete an object (just archives it).
*********************************************************************/
int callDeleteObjectFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, objid = 0, access = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Delete Object Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = deleteObject(objid, info->env, info->sqlsock);

  logInfo("Internal Delete Object Request. (Success = %s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callRemoveGroupMemberFunction...
*
* Remove a user from a group.
*********************************************************************/
int callRemoveGroupMemberFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *super = NULL;
  int errcode = 0, gid = 0, issuper = 1, uid = 0, ispublic = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

   // ONLY SUPER USERS CAN SEE OTHER USERS
  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  arg = list;
  s = (Symbol *) arg->data;
  gid = s->variableType->intValue;

  if ((isGroupPublic(gid, &ispublic, info->sqlsock) != E_OK) || !ispublic) {
    if (!issuper) {
      logInfo("Internal Remove Group Member Request. (gid = %d, Success = %s)\n", gid, getErrorMesg(ACCESSDENIED));
      dest->variableType->intValue = ACCESSDENIED;
      return E_OK;
    }
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  uid = s->variableType->intValue;
  
  errcode = removeGroupMember(gid, uid, info->sqlsock);

  logInfo("Internal Remove Group Member Request. (gid = %d, Success = %s)\n", gid, getErrorMesg(E_OK));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callRemovePermissionFunction...
*
* Remove from permission to a group.
*********************************************************************/
int callRemovePermissionFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, gid = 0, access = 0, objid = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  gid = s->variableType->intValue;

  if (!access) {
    logInfo("Internal Object Edit Permissions Request. (objid = %d, Success = %s)\n", objid, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = removeObjectPermission(objid, gid, info->sqlsock);

  logInfo("Internal Object Edit Permissions Request. (objid = %d, Success = %s)\n", objid, getErrorMesg(E_OK));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callAddPermissionFunction...
*
* Add a permission to a group.
*********************************************************************/
int callAddPermissionFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char mask[4], *xref = NULL;
  int errcode = 0, gid = 0, access = 0, objid = 0, readp = 0, writep = 0, executep = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  gid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  readp = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  writep = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  executep = s->variableType->intValue;

  sprintf(mask, "%c%c%c", readp?'r':'-', writep?'w':'-', executep?'x':'-');
  if (!access) {
    logInfo("Internal Object Edit Permissions Request. (objid = %d, Success = %s)\n", objid, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = setObjectPermission(objid, gid, mask, info->sqlsock);

  logInfo("Internal Object Edit Permissions Request. (objid = %d, Success = %s)\n", objid, getErrorMesg(E_OK));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callAddGroupMemberFunction...
*
* Add a user to a group.
*********************************************************************/
int callAddGroupMemberFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *super = NULL;
  int errcode = 0, gid = 0, issuper = 1, uid = 0, ispublic = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

   // ONLY SUPER USERS CAN EDIT PRIVATE GROUPS
  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  arg = list;
  s = (Symbol *) arg->data;
  gid = s->variableType->intValue;

  if ((isGroupPublic(gid, &ispublic, info->sqlsock) != E_OK) || !ispublic) {
    if (!issuper) {
      logInfo("Internal Add Group Member Request. (gid = %d, Success = %s)\n", gid, getErrorMesg(ACCESSDENIED));
      dest->variableType->intValue = ACCESSDENIED;
      return E_OK;
    }
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  uid = s->variableType->intValue;
  
  errcode = addGroupMember(gid, uid, info->sqlsock);

  logInfo("Internal Add Group Member Request. (gid = %d, Success = %s)\n", gid, getErrorMesg(E_OK));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callEditGroupDetailsFunction...
*
* Edit a groups details and put the return code in a variable.
*********************************************************************/
int callEditGroupDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *gname = NULL, *super = NULL;
  int errcode = 0, gid = 0, issuper = 1, ispublic = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

   // ONLY SUPER USERS CAN SEE OTHER USERS
  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  arg = list;
  s = (Symbol *) arg->data;
  gid = s->variableType->intValue;

  if (!issuper) {
    logInfo("Internal Edit Group Details Request. (gid = %d, Success = %s)\n", gid, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  gname = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  ispublic = s->variableType->intValue;
  
  errcode = editGroupDetails(gid, gname, ispublic, info->sqlsock);

  logInfo("Internal Edit Group Details Request. (gid = %d, Success = %s)\n", gid, getErrorMesg(E_OK));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callEditUserDetailsFunction...
*
* Edit a users details and put the return code in a variable.
*********************************************************************/
int callEditUserDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *uname = NULL, *upass = NULL, *fname = NULL, *thisuidstr = NULL, *email = NULL, *super = NULL;
  int errcode = 0, uid = 0, issuper = 1, thisuid = 0, online = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;

   // ONLY SUPER USERS CAN SEE OTHER USERS
  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  thisuidstr = getEnvValue("USERID", info->env);
  thisuid = strtol(thisuidstr?thisuidstr:"-1", NULL, 10);

  arg = list;
  s = (Symbol *) arg->data;
  uid = s->variableType->intValue;

  if (!issuper && uid != thisuid) {
    logInfo("Internal Edit User Details Request. (uid = %d, Success = %s)\n", uid, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  uname = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  upass = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  online = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  issuper = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  fname = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  email = s->variableType->stringValue;
  
  errcode = editUserDetails(uid, uname, upass, online, issuper, fname, email, info->sqlsock);

  logInfo("Internal Edit User Details Request. (Action User = %s, User = %d, Success = %s)\n", thisuidstr, uid, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callEditObjectDetailsFunction...
*
* Edit an objects details and put the return code in a variable.
*********************************************************************/
int callEditObjectDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *title = NULL;
  int errcode = 0, objid = 0, access = 0, order = 0, ispublic = 0;

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  title = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  ispublic = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  order = s->variableType->intValue;
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Edit Object Details Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = editObjectDetails(objid, title, ispublic, order, info->env, info->sqlsock);

  logInfo("Internal Edit Object Details Request. (Ref = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoadObjectPathFunction...
*
* Load an object details and put the contents in a variable.
*********************************************************************/
int callLoadObjectPathFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int errcode = 0, objid = 0, access = 0, online = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = STRING_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;

  objid = s->variableType->intValue;

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Path Request. (XRef = %s, Success = %s)\n", objid, getErrorMesg(ACCESSDENIED));
    dest->variableType->stringValue = strdup("");
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Path Request. (XRef = %s, Success = %s)\n", objid, getErrorMesg(NOTONLINE));
      dest->variableType->stringValue = strdup("");
      return E_OK;
    }
  }

  if ((errcode = getObjectDetails(objid, &details, info->sqlsock)) != E_OK) {
      logInfo("Internal Retrieve Path Request. (XRef = %s, Success = %s)\n", objid, getErrorMesg(errcode));
      dest->variableType->stringValue = strdup("");
      return E_OK;
  }


  dest->variableType->stringValue = strdup(details->path);
  freeObjectDetails(details);

  logInfo("Internal Retrieve Path Request. (XRef = %s, Success = %s)\n", objid, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadObjectIDFunction...
*
* Load an object details and put the contents in a variable.
*********************************************************************/
int callLoadObjectIDFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, objid = 0, access = 0, online = 0, timestamp = 0;

  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;

  xref = s->variableType->stringValue;
  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, info->env), getEnvValue(CTIMETOK, info->env));
  if (isXRefValid(xref, timestamp, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = -1;
    return E_OK;
  }

  if (userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve ID Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = -1;
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve ID Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOTONLINE));
      dest->variableType->intValue = -1;
      return E_OK;
    }
  }

  logInfo("Internal Retrieve ID Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = objid;
  return E_OK;
}

/*********************************************************************
* callLoadGroupDetailsFunction...
*
* Load an group details and put the contents in a variable.
*********************************************************************/
int callLoadGroupDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int errcode = 0, gid = 0;
  GroupDetails *details = NULL;
 
  if (list == NULL)
    return RESOURCEERROR;
  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  gid = s->variableType->intValue; 

  if ((errcode = getGroupDetails(gid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Retrieve Group Details Request. (groupID = %d, Success = %s)\n", gid, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }
  
  s = getSymbolMapValue(dest->variableType->mapValues, "groupID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->groupID, NULL, 10);
  
  s = getSymbolMapValue(dest->variableType->mapValues, "groupName");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->groupName);
  
  s = getSymbolMapValue(dest->variableType->mapValues, "isPublic");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = details->isPublic;

  freeGroupDetails(details);
  logInfo("Internal Retrieve Group Details Request. (groupID = %d, Success = %s)\n", gid, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoadUserDetailsFunction...
*
* Load an user details and put the contents in a variable.
*********************************************************************/
int callLoadUserDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *thisuidstr = NULL, *super = NULL;
  int errcode = 0, uid = 0, online = 0, thisuid = 0, issuper = 1;
  UserDetails *details = NULL;
 
   // ONLY SUPER USERS CAN SEE OTHER USERS
  super = getEnvValue("ISSUPERUSER", info->env);

  if (super == NULL || *super != 'y') {
    issuper = 0;
  }

  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  uid = s->variableType->intValue;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  thisuidstr = getEnvValue("USERID", info->env);
  thisuid = strtol(thisuidstr?thisuidstr:"-1", NULL, 10);
  
  if (thisuid != uid && !issuper && 
     ((isUserOnline(uid, &online, info->sqlsock) != E_OK) || !online)) {
    logInfo("Internal Retrieve User Details Request. (UserID = %d, Success = %s)\n", uid, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  if ((errcode = getUserDetails(uid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Retrieve User Details Request. (userID = %d, Success = %s)\n", uid, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }
  
  s = getSymbolMapValue(dest->variableType->mapValues, "userID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->userID, NULL, 10);
  
  s = getSymbolMapValue(dest->variableType->mapValues, "userName");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->userName);

  s = getSymbolMapValue(dest->variableType->mapValues, "isOnline");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = details->isOnline[0] == 'y';

  s = getSymbolMapValue(dest->variableType->mapValues, "isSuperUser");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = details->isSuperUser[0] == 'y';

  s = getSymbolMapValue(dest->variableType->mapValues, "fullName");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->fullName);

  s = getSymbolMapValue(dest->variableType->mapValues, "email");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->email);

  s = getSymbolMapValue(dest->variableType->mapValues, "userType");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->userType);

  freeUserDetails(details);
  logInfo("Internal Retrieve User Details Request. (userID = %d, Success = %s)\n", uid, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callAddVerifierCommentFunction...
*
* Load all comment details and put the contents in a variable.
*********************************************************************/
int callAddVerifierCommentFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *comment = NULL;
  int errcode = 0, access = 0, objid = 0, timestamp = 0;

  dest->variableType->variableEnum = INT_VARIABLE;
  if (list == NULL)
    return RESOURCEERROR;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, info->env), getEnvValue(CTIMETOK, info->env));
  if (isXRefValid(xref, timestamp, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  arg = arg->next;
  s = (Symbol *) arg->data;
  comment = s->variableType->stringValue;
  
  if ((userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Add Verifier Comment Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = addVerifierComment(objid, comment, info->env, info->sqlsock);

  logInfo("Internal Add Verifier Comment Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoadAllVerifierCommentsFunction...
*
* Load all comment details and put the contents in a variable.
*********************************************************************/
int callLoadAllVerifierCommentsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *xref = NULL, **columns = NULL;
  int errcode = 0, access = 0, objid = 0, online = 0, timestamp = 0, i = 0, numcols = 0;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, info->env), getEnvValue(CTIMETOK, info->env));
  if (isXRefValid(xref, timestamp, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  if ((userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) ||
	(userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Verifier Comment Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Verifier Comment Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOTONLINE));
      dest->variableType->intValue = ACCESSDENIED;
      return E_OK;
    }
  }

  if ((errcode = loadAllVerifierComments(objid, &columns, &numcols, info->sqlsock)) != E_OK) {
    logInfo("Internal Retrieve Verifier Comment Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  for (i = 0; i < numcols; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "commentID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = strtol(columns[i], NULL, 10);
  }
  dhufree(columns);
  
  logInfo("Internal Retrieve Verifier Comment Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoadVerifierCommentFunction...
*
* Load an comment details and put the contents in a variable.
*********************************************************************/
int callLoadVerifierCommentFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int errcode = 0, commentid = 0, access = 0, objid = 0, online = 0;
  VerifierComment *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  commentid = s->variableType->intValue;

  if (loadVerifierCommentObjectID(commentid, &objid, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Verifier Comment Request. (CommentID = %d, Success = %s)\n", commentid, getErrorMesg(ACCESSDENIED));
      return E_OK;
  }

  if ((userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) ||
	(userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Verifier Comment Request. (CommentID = %d, Success = %s)\n", commentid, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Verifier Comment Request. (CommentID = %d, Success = %s)\n", commentid, getErrorMesg(NOTONLINE));
      return E_OK;
    }
  }

  if ((errcode = getVerifierComment(commentid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Retrieve Verifier Comment Request. (CommentID = %d, Success = %s)\n", commentid, getErrorMesg(errcode));
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "commentID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = details->commentID;
  
  s = getSymbolMapValue(dest->variableType->mapValues, "fileID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = details->objectID;

  s = getSymbolMapValue(dest->variableType->mapValues, "userID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = details->userID;

  s = getSymbolMapValue(dest->variableType->mapValues, "created");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatDateTime(details->created);

  s = getSymbolMapValue(dest->variableType->mapValues, "comment");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->comment);

  freeVerifierComment(details);
  logInfo("Internal Retrieve Verifier Comment Request. (CommentID = %d, Success = %s)\n", commentid, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadDeletedObjectDetailsFunction...
*
* Load an object details and put the contents in a variable.
*********************************************************************/
int callLoadDeletedObjectDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int errcode = 0, objid = 0, access = 0, online = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  objid = s->variableType->intValue;

  if ((userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) ||
	(userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Details Request. (XRef = %d, Success = %s)\n", objid, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Details Request. (XRef = %d, Success = %s)\n", objid, getErrorMesg(NOTONLINE));
      return E_OK;
    }
  }

  if ((errcode = getObjectDetails(objid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Retrieve Details Request. (XRef = %d, Success = %s)\n", objid, getErrorMesg(errcode));
    return E_OK;
  }
  
  if ((errcode = getObjectPermissions(details, info->env, info->sqlsock)) != E_OK) {
    logInfo("Internal Retrieve Details Request. (XRef = %d, Success = %s)\n", objid, getErrorMesg(errcode));
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "fileID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->objectID, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "fileName");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->objectName);
  s = getSymbolMapValue(dest->variableType->mapValues, "path");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->path);
  s = getSymbolMapValue(dest->variableType->mapValues, "parentID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->parentID, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "isOnline");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->isOnline) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "type");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->type);
  s = getSymbolMapValue(dest->variableType->mapValues, "isPublic");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->isPublic) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "mimeType");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->mimeType);
  s = getSymbolMapValue(dest->variableType->mapValues, "readPermission");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->readPermission) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "writePermission");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->writePermission) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "executePermission");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->executePermission) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "lockedByUserID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->lockedByUserID, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "version");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->version, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "fileSize");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->fileSize, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "template");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->tplt);
  s = getSymbolMapValue(dest->variableType->mapValues, "relativeOrder");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->relativeOrder, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "publisherUserID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->publisherUserID, NULL, 10);

  freeObjectDetails(details);
  logInfo("Internal Retrieve Details Request. (XRef = %d, Success = %s)\n", objid, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadObjectDetailsFunction...
*
* Load an object details and put the contents in a variable.
*********************************************************************/
int callLoadObjectDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL;
  int errcode = 0, objid = 0, access = 0, online = 0, timestamp = 0;
  ObjectDetails *details = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  timestamp = getTimeValue(getEnvValue(ISOTIMETOK, info->env), getEnvValue(CTIMETOK, info->env));
  if (isXRefValid(xref, timestamp, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }

  if ((userHasReadAccess(objid, info->env, info->sqlsock) == E_OK) ||
	(userHasWriteAccess(objid, info->env, info->sqlsock) == E_OK)) {
    access = 1;
  }

  if (!access) {
    logInfo("Internal Retrieve Details Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    return E_OK;
  }

  if ((isObjectOnline(objid, &online, info->sqlsock) != E_OK) || !online) {
    if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
      logInfo("Internal Retrieve Details Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(NOTONLINE));
      return E_OK;
    }
  }

  if ((errcode = getObjectDetails(objid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Retrieve Details Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    return E_OK;
  }
  
  if ((errcode = getObjectPermissions(details, info->env, info->sqlsock)) != E_OK) {
    logInfo("Internal Retrieve Details Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "fileID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->objectID, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "fileName");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->objectName);
  s = getSymbolMapValue(dest->variableType->mapValues, "path");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->path);
  s = getSymbolMapValue(dest->variableType->mapValues, "parentID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->parentID, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "isOnline");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->isOnline) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "type");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->type);
  s = getSymbolMapValue(dest->variableType->mapValues, "isPublic");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->isPublic) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "mimeType");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->mimeType);
  s = getSymbolMapValue(dest->variableType->mapValues, "readPermission");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->readPermission) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "writePermission");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->writePermission) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "executePermission");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = *(details->executePermission) == 'y';
  s = getSymbolMapValue(dest->variableType->mapValues, "lockedByUserID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->lockedByUserID, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "version");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->version, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "fileSize");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->fileSize, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "template");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(details->tplt);
  s = getSymbolMapValue(dest->variableType->mapValues, "relativeOrder");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->relativeOrder, NULL, 10);
  s = getSymbolMapValue(dest->variableType->mapValues, "publisherUserID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = strtol(details->publisherUserID, NULL, 10);

  freeObjectDetails(details);
  logInfo("Internal Retrieve Details Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callCalCreateInstanceFunction...
*
* Create a calendar instance at this location.
*********************************************************************/
int callCalCreateInstanceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  ObjectDetails *details = NULL;
  cal_instance *instance = NULL;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  dest->variableType->variableEnum = INT_VARIABLE;

  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Create Calendar Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Create Calendar Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  if ((errcode = getObjectDetails(objid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Calendar Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  instance = calInitInstance();

  instance->objectPath = dhustrdup(details->path);
  errcode = dbCalCreateInstance(instance, info->sqlsock);

  calFreeInstance(instance);

  logInfo("Internal Create Calendar Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  freeObjectDetails(details);
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCalCreateEventFunction...
*
* Create a calendar event in this calendar
*********************************************************************/
int callCalCreateEventFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, calid = 0;
  cal_event *event = NULL;
  cal_instance *instance = NULL;

  arg = list;
  s = (Symbol *) arg->data;
  calid = s->variableType->intValue;
  dest->variableType->variableEnum = INT_VARIABLE;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Create Calendar Event Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = -1;
    return E_OK;
  }

  instance = calInitInstance();
  instance->calid = calid;

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Calendar Event Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = -1;
    calFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Create Calendar Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = -1;
    calFreeInstance(instance);
    return E_OK;
  }

  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Create Calendar Event Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = -1;
    return E_OK;
  }

  event = calInitEvent();
  event->calid = instance->calid;
  calFreeInstance(instance);

  if ((errcode = dbCalCreateEvent(event, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Calendar Event Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    calFreeEvent(event);
    dest->variableType->intValue = -1; // If >= 0, it is the eventid
    return E_OK;
  }

  dest->variableType->intValue = event->eventid;
  calFreeEvent(event);

  logInfo("Internal Create Calendar Event Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callCalCreateOccurrenceFunction...
*
* Create a calendar occurrence in this calendar
*********************************************************************/
int callCalCreateOccurrenceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *tmp = NULL, *eventdate = NULL, *starttime = NULL, *endtime = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, eventid = 0, allday = 0;
  cal_occurrence *occurrence = NULL;
  cal_event *event = NULL;
  cal_instance *instance = NULL;
  char *summary = NULL, *description = NULL, *location = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  eventid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  summary = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  description = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  location = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  eventdate = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  starttime = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  endtime = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  allday = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Create Calendar Occurrence Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  event = calInitEvent();
  event->eventid = eventid;
  if ((errcode = dbCalEventDetails(event, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Calendar Occurrence Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  instance = calInitInstance();
  instance->calid = event->calid;
  calFreeEvent(event);

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Calendar Occurrence Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Create Calendar Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    return E_OK;
  }
  calFreeInstance(instance);

  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Create Calendar Occurrence Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  occurrence = calInitOccurrence();
  occurrence->eventid = eventid;
  occurrence->summary = strdup(summary);
  occurrence->location = strdup(location);
  occurrence->description = strdup(description);
  occurrence->eventdate = parseDate(eventdate);
  occurrence->starttime = parseTime(starttime);
  occurrence->endtime = parseTime(endtime);
  occurrence->allday = allday;

		  
  if ((errcode = dbCalCreateOccurrence(occurrence, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Calendar Occurrence Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    calFreeOccurrence(occurrence);
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  dest->variableType->intValue = occurrence->occurrenceid;
  calFreeOccurrence(occurrence);

  logInfo("Internal Create Calendar Occurrence Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callCalEditOccurrenceFunction...
*
* Edit a calendar occurrence in this calendar
*********************************************************************/
int callCalEditOccurrenceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL, *eventdate = NULL, *starttime = NULL, *endtime = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, occurrenceid = 0, allday = 0;
  cal_occurrence *occurrence = NULL;
  cal_event *event = NULL;
  cal_instance *instance = NULL;
  char *summary = NULL, *description = NULL, *location = NULL;


  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;
  occurrenceid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  summary = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  description = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  location = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  eventdate = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  starttime = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  endtime = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  allday = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Edit Calendar Occurrence Request. (Success = %s - No valid session.)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  occurrence = calInitOccurrence();
  occurrence->occurrenceid = occurrenceid;
  if ((errcode = dbCalOccurrenceDetails(occurrence, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Calendar Occurrence Request. (Success = %s - Could not find occurrence.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeOccurrence(occurrence);
    return E_OK;
  }

  event = calInitEvent();
  event->eventid = occurrence->eventid;
  if ((errcode = dbCalEventDetails(event, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Calendar Occurrence Request. (Success = %s - Could not load event details.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeEvent(event);
    calFreeOccurrence(occurrence);
    return E_OK;
  }

  instance = calInitInstance();
  instance->calid = event->calid;
  calFreeEvent(event);

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Calendar Occurrence Request. (Success = %s - Could not load calendar instance.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeOccurrence(occurrence);
    calFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Edit Calendar Instance Request. (Success = %s - Could not find calendar page.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    return E_OK;
  }
  calFreeInstance(instance);

  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Edit Calendar Occurrence Request. (Success = %s - No write access.)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeOccurrence(occurrence);
    return E_OK;
  }

  dhufree(occurrence->summary);
  occurrence->summary = strdup(summary);
  dhufree(occurrence->location);
  occurrence->location = strdup(location);
  dhufree(occurrence->description);
  occurrence->description = strdup(description);
  dhufree(occurrence->eventdate);
  occurrence->eventdate = parseDate(eventdate);
  dhufree(occurrence->starttime);
  occurrence->starttime = parseTime(starttime);
  dhufree(occurrence->endtime);
  occurrence->endtime = parseTime(endtime);
  occurrence->allday = allday;

  if ((errcode = dbCalEditOccurrence(occurrence, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Calendar Occurrence Request. (Success = %s - Update error.)\n", getErrorMesg(errcode));
    calFreeOccurrence(occurrence);
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  calFreeOccurrence(occurrence);

  logInfo("Internal Edit Calendar Occurrence Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCalDeleteInstanceFunction...
*
* Delete a calendar instance at this location.
*********************************************************************/
int callCalDeleteInstanceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  cal_instance *instance = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;
  instance = calInitInstance();
  instance->calid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Delete Calendar Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Calendar Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Delete Calendar Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    return E_OK;
  }
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Delete Calendar Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeInstance(instance);
    return E_OK;
  }

  errcode = dbCalDeleteInstance(instance, info->sqlsock);

  calFreeInstance(instance);

  logInfo("Internal Delete Calendar Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCalDeleteEventFunction...
*
* Delete a calendar event
*********************************************************************/
int callCalDeleteEventFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  cal_instance *instance = NULL;
  cal_event *event = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;
  event = calInitEvent();
  event->eventid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Delete Calendar Event Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeEvent(event);
    return E_OK;
  }

  if ((errcode = dbCalEventDetails(event, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeEvent(event);
    return E_OK;
  }

  instance = calInitInstance();
  instance->calid = event->calid;
  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    calFreeEvent(event);
    return E_OK;
  }
  
  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Delete Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    calFreeEvent(event);
    return E_OK;
  }
  calFreeInstance(instance);
  
  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Delete Calendar Event Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeEvent(event);
    return E_OK;
  }

  errcode = dbCalDeleteEvent(event, info->sqlsock);

  calFreeEvent(event);

  logInfo("Internal Delete Calendar Event Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCalDeleteOccurrenceFunction...
*
* Delete a calendar event
*********************************************************************/
int callCalDeleteOccurrenceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  cal_instance *instance = NULL;
  cal_event *event = NULL;
  cal_occurrence *occurrence = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  occurrence = calInitOccurrence();
  occurrence->occurrenceid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Delete Calendar Occurrence Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeInstance(instance);
    return E_OK;
  }
  
  if ((errcode = dbCalOccurrenceDetails(occurrence, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Calendar Occurrence Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    return E_OK;
  }

  event = calInitEvent();
  event->eventid = occurrence->eventid;

  if ((errcode = dbCalEventDetails(event, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Calendar Occurrence Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    return E_OK;
  }

  instance = calInitInstance();
  instance->calid = event->calid;
  calFreeEvent(event);

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Calendar Occurrence Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    return E_OK;
  }
  
  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Delete Calendar Occurrence Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    return E_OK;
  }
  calFreeInstance(instance);
  
  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Delete Calendar Occurrence Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = dbCalDeleteOccurrence(occurrence, info->sqlsock);

  calFreeOccurrence(occurrence);

  logInfo("Internal Delete Calendar Occurrence Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callCalMoveInstanceFunction...
*
* Move a calendar instance to a new location.
*********************************************************************/
int callCalMoveInstanceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL, *newpath = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  cal_instance *instance = NULL;


  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  instance = calInitInstance();
  instance->calid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  newpath = dhustrdup(s->variableType->stringValue);

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Move Calendar Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeInstance(instance);
    dhufree(newpath);
    return E_OK;
  }

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Move Calendar Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    dhufree(newpath);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Move Calendar Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    calFreeInstance(instance);
    dhufree(newpath);
    return E_OK;
  }
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Move Calendar Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeInstance(instance);
    dhufree(newpath);
    return E_OK;
  }

  dhufree(instance->objectPath);
  instance->objectPath = newpath;
  if (dbCalMoveInstance(instance, info->sqlsock) != E_OK) {
    logInfo("Internal Move Calendar Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    calFreeInstance(instance);
    return E_OK;
  }

  calFreeInstance(instance);

  logInfo("Internal Move Calendar Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callLoadCalInstancePathFunction...
*
* Load a calendar instance path...
*********************************************************************/
int callLoadCalInstancePathFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int errcode = 0;
  int calid = 0, objid = 0;
  cal_instance *instance = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  calid = s->variableType->intValue;

  instance = calInitInstance();
  instance->calid = calid;

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->stringValue = strdup("");
    calFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->stringValue = strdup("");
    calFreeInstance(instance);
    return E_OK;
  }
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->stringValue = strdup("");
    calFreeInstance(instance);
    return E_OK;
  }

  dest->variableType->stringValue = strdup(instance->objectPath);
  calFreeInstance(instance);

  logInfo("Internal Load Calendar Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadCalInstanceDetailsFunction...
*
* Load a calendar instance details...
*********************************************************************/
int callLoadCalInstanceDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL, *xref = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  cal_instance *instance = NULL;
  ObjectDetails *details = NULL;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(INVALIDXPATH));
    return E_OK;
  }

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if ((errcode = getObjectDetails(objid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    return E_OK;
  }

  instance = calInitInstance();
  instance->objectPath = dhustrdup(details->path);

  freeObjectDetails(details);

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    return E_OK;
  }
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    calFreeInstance(instance);
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "calID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = instance->calid;
  s = getSymbolMapValue(dest->variableType->mapValues, "objectPath");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(instance->objectPath);
  calFreeInstance(instance);

  logInfo("Internal Load Calendar Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadCalEventDetailsFunction...
*
* Load a calendar event details...
*********************************************************************/
int callLoadCalEventDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, eventid = 0;
  cal_instance *instance = NULL;
  cal_event *event = NULL;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  eventid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  event = calInitEvent();
  event->eventid = eventid;

  if ((errcode = dbCalEventDetails(event, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeEvent(event);
    return E_OK;
  }

  instance = calInitInstance();
  instance->calid = event->calid;

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    calFreeEvent(event);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Load Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    calFreeEvent(event);
    return E_OK;
  }
  calFreeInstance(instance);
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Load Calendar Event Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    calFreeEvent(event);
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "calID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = event->calid;
  s = getSymbolMapValue(dest->variableType->mapValues, "eventID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = event->eventid;
  s = getSymbolMapValue(dest->variableType->mapValues, "created");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatDateTime(event->created);
  s = getSymbolMapValue(dest->variableType->mapValues, "modified");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatDateTime(event->modified);
  calFreeEvent(event);

  logInfo("Internal Load Calendar Event Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadCalOccurrenceDetailsFunction...
*
* Load a calendar occurrence details...
*********************************************************************/
int callLoadCalOccurrenceDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, occurrenceid = 0;
  cal_instance *instance = NULL;
  cal_event *event = NULL;
  cal_occurrence *occurrence = NULL;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  occurrenceid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  occurrence = calInitOccurrence();
  occurrence->occurrenceid = occurrenceid;

  if ((errcode = dbCalOccurrenceDetails(occurrence, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Calendar Occurrence Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeOccurrence(occurrence);
    return E_OK;
  }

  event = calInitEvent();
  event->eventid = occurrence->eventid;

  if ((errcode = dbCalEventDetails(event, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeOccurrence(occurrence);
    calFreeEvent(event);
    return E_OK;
  }

  instance = calInitInstance();
  instance->calid = event->calid;
  calFreeEvent(event);

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    calFreeOccurrence(occurrence);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Load Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    calFreeOccurrence(occurrence);
    return E_OK;
  }
  calFreeInstance(instance);
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Load Calendar Event Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    calFreeOccurrence(occurrence);
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "occurrenceID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = occurrence->occurrenceid;
  s = getSymbolMapValue(dest->variableType->mapValues, "eventID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = occurrence->eventid;
  s = getSymbolMapValue(dest->variableType->mapValues, "summary");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(occurrence->summary);
  s = getSymbolMapValue(dest->variableType->mapValues, "description");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(occurrence->description);
  s = getSymbolMapValue(dest->variableType->mapValues, "location");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(occurrence->location);
  s = getSymbolMapValue(dest->variableType->mapValues, "eventdate");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatDate(occurrence->eventdate);
  s = getSymbolMapValue(dest->variableType->mapValues, "starttime");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatTime(occurrence->starttime);
  s = getSymbolMapValue(dest->variableType->mapValues, "endtime");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatTime(occurrence->endtime);
  s = getSymbolMapValue(dest->variableType->mapValues, "allday");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = occurrence->allday;

  calFreeOccurrence(occurrence);

  logInfo("Internal Load Calendar Occurrence Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callSearchCalByEventFunction...
*
* Search for all occurrences of an event.
*********************************************************************/
int callSearchCalByEventFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  Symbol *m = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, eventid = 0, count = 0, i = 0;
  cal_instance *instance = NULL;
  cal_event *event = NULL;
  cal_occurrence **occurrences = NULL;


  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  eventid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  event = calInitEvent();
  event->eventid = eventid;

  if ((errcode = dbCalEventDetails(event, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeEvent(event);
    return E_OK;
  }

  instance = calInitInstance();
  instance->calid = event->calid;

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    calFreeEvent(event);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    calFreeEvent(event);
    return E_OK;
  }
  calFreeInstance(instance);
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    calFreeEvent(event);
    return E_OK;
  }

  if ((errcode = dbCalSearchEvent(event, &count, &occurrences, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeEvent(event);
    return E_OK;
  }

  for (i = 0; i < count; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "occurrenceID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = occurrences[i]->occurrenceid;
    calFreeOccurrence(occurrences[i]);
  }

  dhufree(occurrences);
  calFreeEvent(event);

  logInfo("Internal Search Calendar Occurrence Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callSearchCalByDateTimeFunction...
*
* Search for all occurrences of an event.
*********************************************************************/
int callSearchCalByDateTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, id = 0, count = 0, i = 0;
  cal_instance *instance = NULL;
  cal_occurrence **occurrences = NULL;
  struct tm *start = NULL, *end = NULL;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  id = s->variableType->intValue;

  arg = arg->next;
  s = (Symbol *) arg->data;
  start = parseDateTime(s->variableType->stringValue);

  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseDateTime(s->variableType->stringValue);

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  instance = calInitInstance();
  instance->calid = id;

  if ((errcode = dbCalInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    dhufree(start);
    dhufree(end);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    dhufree(start);
    dhufree(end);
    return E_OK;
  }
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    calFreeInstance(instance);
    dhufree(start);
    dhufree(end);
    return E_OK;
  }

  if ((errcode = dbCalSearchTime(instance, start, end, &count, &occurrences, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Calendar Event Request. (Success = %s)\n", getErrorMesg(errcode));
    calFreeInstance(instance);
    dhufree(start);
    dhufree(end);
    return E_OK;
  }
  dhufree(start);
  dhufree(end);
  calFreeInstance(instance);

  for (i = 0; i < count; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "occurrenceID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = occurrences[i]->occurrenceid;
    calFreeOccurrence(occurrences[i]);
  }

  dhufree(occurrences);

  logInfo("Internal Search Calendar Occurrence Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callCompareDateFunction...
*
* Compare 2 dates.
*********************************************************************/
int callCompareDateFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int cmp = 0;
  struct tm *start = NULL, *end = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;
  start = parseDate(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseDate(s->variableType->stringValue);

  cmp = compareDate(start, end);
  dhufree(start);
  dhufree(end);

  dest->variableType->intValue = cmp;
  return E_OK;
}

/*********************************************************************
* callCompareTimeFunction...
*
* Compare 2 dates.
*********************************************************************/
int callCompareTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int cmp = 0;
  struct tm *start = NULL, *end = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  start = parseTime(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseTime(s->variableType->stringValue);

  cmp = compareTime(start, end);
  dhufree(start);
  dhufree(end);

  dest->variableType->intValue = cmp;
  return E_OK;
}

/*********************************************************************
* callCompareDateTimeFunction...
*
* Compare 2 dates.
*********************************************************************/
int callCompareDateTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int cmp = 0;
  struct tm *start = NULL, *end = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  start = parseDateTime(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseDateTime(s->variableType->stringValue);

  cmp = compareDateTime(start, end);
  dhufree(start);
  dhufree(end);

  dest->variableType->intValue = cmp;
  return E_OK;
}

/*********************************************************************
* callSubtractDateFunction...
*
* Subtract 2 dates.
*********************************************************************/
int callSubtractDateFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  struct tm *start = NULL, *end = NULL, *sum = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;
  arg = list;
  s = (Symbol *) arg->data;
  start = parseDate(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseDate(s->variableType->stringValue);

  sum = subtractDate(start, end);

  dest->variableType->stringValue = formatDate(sum);
  dhufree(start);
  dhufree(end);
  dhufree(sum);

  return E_OK;
}

/*********************************************************************
* callSubtractTimeFunction...
*
* Subtract 2 dates.
*********************************************************************/
int callSubtractTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  struct tm *start = NULL, *end = NULL, *sum = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  start = parseTime(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseTime(s->variableType->stringValue);

  sum = subtractTime(start, end);

  dest->variableType->stringValue = formatTime(sum);
  dhufree(start);
  dhufree(end);
  dhufree(sum);

  return E_OK;
}

/*********************************************************************
* callSubtractDateTimeFunction...
*
* Subtract 2 dates.
*********************************************************************/
int callSubtractDateTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  struct tm *start = NULL, *end = NULL, *sum = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  start = parseDateTime(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseDateTime(s->variableType->stringValue);

  sum = subtractDateTime(start, end);

  dest->variableType->stringValue = formatDateTime(sum);
  dhufree(start);
  dhufree(end);
  dhufree(sum);

  return E_OK;
}

/*********************************************************************
* callAddDateFunction...
*
* Add 2 dates.
*********************************************************************/
int callAddDateFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  struct tm *start = NULL, *end = NULL, *sum = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  start = parseDate(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseDate(s->variableType->stringValue);

  sum = addDate(start, end);
  dest->variableType->stringValue = formatDate(sum);
  
  dhufree(start);
  dhufree(end);
  dhufree(sum);

  return E_OK;
}

/*********************************************************************
* callAddTimeFunction...
*
* Add 2 dates.
*********************************************************************/
int callAddTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  struct tm *start = NULL, *end = NULL, *sum = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  start = parseTime(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseTime(s->variableType->stringValue);

  sum = addTime(start, end);

  dest->variableType->stringValue = formatTime(sum);
  dhufree(start);
  dhufree(end);
  dhufree(sum);

  return E_OK;
}

/*********************************************************************
* callAddDateTimeFunction...
*
* Add 2 dates.
*********************************************************************/
int callAddDateTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  struct tm *start = NULL, *end = NULL, *sum = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  start = parseDateTime(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseDateTime(s->variableType->stringValue);

  sum = addDateTime(start, end);

  dest->variableType->stringValue = formatDateTime(sum);
  dhufree(start);
  dhufree(end);
  dhufree(sum);

  return E_OK;
}

/*********************************************************************
* callGetMonthNameShortFunction...
*
* GetMonthNameShortFunction 2 dates.
*********************************************************************/
int callGetMonthNameShortFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int mon = 0;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  mon = s->variableType->intValue;

  switch (mon) {
  	case 1:
  		dest->variableType->stringValue = dhustrdup("Jan");
		break;
  	case 2:
  		dest->variableType->stringValue = dhustrdup("Feb");
		break;
  	case 3:
  		dest->variableType->stringValue = dhustrdup("Mar");
		break;
  	case 4:
  		dest->variableType->stringValue = dhustrdup("Apr");
		break;
  	case 5:
  		dest->variableType->stringValue = dhustrdup("May");
		break;
  	case 6:
  		dest->variableType->stringValue = dhustrdup("Jun");
		break;
  	case 7:
  		dest->variableType->stringValue = dhustrdup("Jul");
		break;
  	case 8:
  		dest->variableType->stringValue = dhustrdup("Aug");
		break;
  	case 9:
  		dest->variableType->stringValue = dhustrdup("Sep");
		break;
  	case 10:
  		dest->variableType->stringValue = dhustrdup("Oct");
		break;
  	case 11:
  		dest->variableType->stringValue = dhustrdup("Nov");
		break;
  	case 12:
  		dest->variableType->stringValue = dhustrdup("Dec");
		break;
	default:
  		dest->variableType->stringValue = dhustrdup("---");
		break;
  }

  return E_OK;
}

/*********************************************************************
* callGetDayNameFunction...
*
* GetDayNameFunction 2 dates.
*********************************************************************/
int callGetDayNameFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *dates = NULL;
  struct tm *day = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  dates = s->variableType->stringValue;

  day = parseDate(dates);

  mktime(day);
  switch (day->tm_wday) {
  	case 0:
  		dest->variableType->stringValue = dhustrdup("Sunday");
		break;
  	case 1:
  		dest->variableType->stringValue = dhustrdup("Monday");
		break;
  	case 2:
  		dest->variableType->stringValue = dhustrdup("Tuesday");
		break;
  	case 3:
  		dest->variableType->stringValue = dhustrdup("Wednesday");
		break;
  	case 4:
  		dest->variableType->stringValue = dhustrdup("Thursday");
		break;
  	case 5:
  		dest->variableType->stringValue = dhustrdup("Friday");
		break;
  	case 6:
  		dest->variableType->stringValue = dhustrdup("Saturday");
		break;
	default:
  		dest->variableType->stringValue = dhustrdup("Unknown");
		break;
  }
  dhufree(day);

  return E_OK;
}

/*********************************************************************
* callGetDayNameShortFunction...
*
* GetDayNameShortFunction 2 dates.
*********************************************************************/
int callGetDayNameShortFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *dates = NULL;
  struct tm *day = NULL;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  dates = s->variableType->stringValue;

  day = parseDate(dates);

  mktime(day);
  switch (day->tm_wday) {
  	case 0:
  		dest->variableType->stringValue = dhustrdup("Sun");
		break;
  	case 1:
  		dest->variableType->stringValue = dhustrdup("Mon");
		break;
  	case 2:
  		dest->variableType->stringValue = dhustrdup("Tue");
		break;
  	case 3:
  		dest->variableType->stringValue = dhustrdup("Wed");
		break;
  	case 4:
  		dest->variableType->stringValue = dhustrdup("Thu");
		break;
  	case 5:
  		dest->variableType->stringValue = dhustrdup("Fri");
		break;
  	case 6:
  		dest->variableType->stringValue = dhustrdup("Sat");
		break;
	default:
  		dest->variableType->stringValue = dhustrdup("---");
		break;
  }
  dhufree(day);

  return E_OK;
}

/*********************************************************************
* callValidDateFunction...
*
*********************************************************************/
int callValidDateFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *dates = NULL;
  struct tm *day = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  dates = s->variableType->stringValue;

  day = parseDate(dates);

  if (mktime(day) == -1) {
    dest->variableType->intValue = 0;
  } else {
    dest->variableType->intValue = 1;
  }
  dhufree(day);

  return E_OK;
}

/*********************************************************************
* callValidDateTimeFunction...
*
*********************************************************************/
int callValidDateTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *dates = NULL;
  struct tm *day = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  dates = s->variableType->stringValue;

  day = parseDateTime(dates);

  if (mktime(day) == -1) {
    dest->variableType->intValue = 0;
  } else {
    dest->variableType->intValue = 1;
  }
  dhufree(day);

  return E_OK;
}


/*********************************************************************
* callGetMonthNameFunction...
*
* GetMonthNameFunction 2 dates.
*********************************************************************/
int callGetMonthNameFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  int mon = 0;

  dest->variableType->variableEnum = STRING_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  mon = s->variableType->intValue;
  
  switch (mon) {
  	case 1:
  		dest->variableType->stringValue = dhustrdup("January");
		break;
  	case 2:
  		dest->variableType->stringValue = dhustrdup("February");
		break;
  	case 3:
  		dest->variableType->stringValue = dhustrdup("March");
		break;
  	case 4:
  		dest->variableType->stringValue = dhustrdup("April");
		break;
  	case 5:
  		dest->variableType->stringValue = dhustrdup("May");
		break;
  	case 6:
  		dest->variableType->stringValue = dhustrdup("June");
		break;
  	case 7:
  		dest->variableType->stringValue = dhustrdup("July");
		break;
  	case 8:
  		dest->variableType->stringValue = dhustrdup("August");
		break;
  	case 9:
  		dest->variableType->stringValue = dhustrdup("September");
		break;
  	case 10:
  		dest->variableType->stringValue = dhustrdup("October");
		break;
  	case 11:
  		dest->variableType->stringValue = dhustrdup("November");
		break;
  	case 12:
  		dest->variableType->stringValue = dhustrdup("December");
		break;
	default:
  		dest->variableType->stringValue = dhustrdup("Unknown");
		break;
  }

  return E_OK;
}

/*********************************************************************
* callCurrentDateTimeFunction...
*
*********************************************************************/
int callCurrentDateTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  struct tm now;
  time_t secs;

  dest->variableType->variableEnum = STRING_VARIABLE;

  time(&secs);
  localtime_r(&secs, &now);

  dest->variableType->stringValue = formatDateTime(&now);

  return E_OK;
}

/*********************************************************************
* callCurrentTimeFunction...
*
*********************************************************************/
int callCurrentTimeFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  struct tm now;
  time_t secs;

  dest->variableType->variableEnum = STRING_VARIABLE;

  time(&secs);
  localtime_r(&secs, &now);

  dest->variableType->stringValue = formatTime(&now);

  return E_OK;
}

/*********************************************************************
* callCurrentDateFunction...
*
*********************************************************************/
int callCurrentDateFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  struct tm now;
  time_t secs;

  dest->variableType->variableEnum = STRING_VARIABLE;

  time(&secs);
  localtime_r(&secs, &now);

  dest->variableType->stringValue = formatDate(&now);

  return E_OK;
}

// Board Functions
/*********************************************************************
* callBoardCreateInstanceFunction...
*
* Create a board instance at this location.
*********************************************************************/
int callBoardCreateInstanceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  ObjectDetails *details = NULL;
  board_instance *instance = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    dest->variableType->intValue = INVALIDXPATH;
    return E_OK;
  }

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Create Board Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Create Board Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  if ((errcode = getObjectDetails(objid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Board Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  instance = boardInitInstance();

  instance->objectPath = dhustrdup(details->path);
  errcode = dbBoardCreateInstance(instance, info->sqlsock);

  dest->variableType->intValue = instance->boardid;
  boardFreeInstance(instance);

  logInfo("Internal Create Board Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  freeObjectDetails(details);
  return E_OK;
}

/*********************************************************************
* callBoardDeleteInstanceFunction...
*
* Delete a board instance at this location.
*********************************************************************/
int callBoardDeleteInstanceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  board_instance *instance = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  instance = boardInitInstance();
  instance->boardid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Delete Board Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Board Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Delete Board Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    return E_OK;
  }
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Delete Board Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeInstance(instance);
    return E_OK;
  }

  errcode = dbBoardDeleteInstance(instance, info->sqlsock);

  boardFreeInstance(instance);

  logInfo("Internal Delete Board Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callBoardMoveInstanceFunction...
*
* Move a board instance to a new location.
*********************************************************************/
int callBoardMoveInstanceFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL, *newpath = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  board_instance *instance = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;
  
  arg = list;
  s = (Symbol *) arg->data;
  instance = boardInitInstance();
  instance->boardid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  newpath = dhustrdup(s->variableType->stringValue);

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Move Board Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeInstance(instance);
    dhufree(newpath);
    return E_OK;
  }

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Move Board Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    dhufree(newpath);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Move Board Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    dhufree(newpath);
    return E_OK;
  }
  
  if (userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Move Board Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeInstance(instance);
    dhufree(newpath);
    return E_OK;
  }

  dhufree(instance->objectPath);
  instance->objectPath = newpath;
  if (dbBoardMoveInstance(instance, info->sqlsock) != E_OK) {
    logInfo("Internal Move Board Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeInstance(instance);
    return E_OK;
  }

  boardFreeInstance(instance);

  logInfo("Internal Move Board Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callBoardCreateTopicFunction...
*
* Create a topic in this board
*********************************************************************/
int callBoardCreateTopicFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *tmp = NULL, *summary = NULL, *description = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, boardid = 0, locked = 0, sticky = 0;
  board_topic *topic = NULL;
  board_instance *instance = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  boardid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  summary = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  description = s->variableType->stringValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  locked = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  sticky = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Create Board Topic Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  instance = boardInitInstance();
  instance->boardid = boardid;

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Board Topic Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Create Board Topic Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    return E_OK;
  }

  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Create Board Topic Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  topic = boardInitTopic();
  topic->boardid = instance->boardid;
  topic->summary = dhustrdup(summary);
  topic->description = dhustrdup(description);
  topic->authorid = uid;
  topic->locked = locked;
  topic->sticky = sticky;
  boardFreeInstance(instance);

  if ((errcode = dbBoardCreateTopic(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Board Topic Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    boardFreeTopic(topic);
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  dest->variableType->intValue = topic->topicid;
  boardFreeTopic(topic);

  logInfo("Internal Create Board Topic Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callBoardDeleteTopicFunction...
*
* Delete a board topic
*********************************************************************/
int callBoardDeleteTopicFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  board_instance *instance = NULL;
  board_topic *topic = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  topic = boardInitTopic();
  topic->topicid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Delete Board Topic Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeTopic(topic);
    return E_OK;
  }

  if ((errcode = dbBoardTopicDetails(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Board Topic Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeTopic(topic);
    return E_OK;
  }

  instance = boardInitInstance();
  instance->boardid = topic->boardid;
  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Board Topic Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    boardFreeTopic(topic);
    return E_OK;
  }
  
  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Delete Board Topic Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    boardFreeTopic(topic);
    return E_OK;
  }
  boardFreeInstance(instance);
  
  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Delete Board Topic Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }
  
  if ((userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) &&
      (topic->authorid != uid)) {
    logInfo("Internal Delete Board Topic Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  errcode = dbBoardDeleteTopic(topic, info->sqlsock);

  boardFreeTopic(topic);

  logInfo("Internal Delete Board Topic Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callBoardCreateMessageFunction...
*
* Create a board message in this board
*********************************************************************/
int callBoardCreateMessageFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *xref = NULL, *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, topicid = 0;
  board_message *message = NULL;
  board_topic *topic = NULL;
  board_instance *instance = NULL;
  char *description = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  topicid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  description = dhustrdup(s->variableType->stringValue);

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Create Board Message Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  topic = boardInitTopic();
  topic->topicid = topicid;
  if ((errcode = dbBoardTopicDetails(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Board Message Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeTopic(topic);
    return E_OK;
  }

  instance = boardInitInstance();
  instance->boardid = topic->boardid;
  boardFreeTopic(topic);

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Board Message Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Create Board Message Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    return E_OK;
  }
  boardFreeInstance(instance);

  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Create Board Message Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  message = boardInitMessage();
  message->topicid = topicid;
  message->description = description;
  message->authorid = uid;

  if ((errcode = dbBoardCreateMessage(message, info->sqlsock)) != E_OK) {
    logInfo("Internal Create Board Message Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    boardFreeMessage(message);
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  dest->variableType->intValue = message->messageid;
  boardFreeMessage(message);

  logInfo("Internal Create Board Message Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callBoardEditMessageFunction...
*
* Edit a message in this board
*********************************************************************/
int callBoardEditMessageFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, messageid = 0;
  board_message *message = NULL;
  board_topic *topic = NULL;
  board_instance *instance = NULL;
  char *description = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  messageid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  description = dhustrdup(s->variableType->stringValue);

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Edit Board Message Request. (Success = %s - No valid session.)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  message = boardInitMessage();
  message->messageid = messageid;
  if ((errcode = dbBoardMessageDetails(message, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Board Message Request. (Success = %s - Could not find message.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeMessage(message);
    return E_OK;
  }

  topic = boardInitTopic();
  topic->topicid = message->topicid;
  if ((errcode = dbBoardTopicDetails(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Board Message Request. (Success = %s - Could not load topic details.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeTopic(topic);
    boardFreeMessage(message);
    return E_OK;
  }

  instance = boardInitInstance();
  instance->boardid = topic->boardid;
  boardFreeTopic(topic);

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Board Message Request. (Success = %s - Could not load board instance.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeMessage(message);
    boardFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Edit Board Message Request. (Success = %s - Could not find board page.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    return E_OK;
  }
  boardFreeInstance(instance);

  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Edit Board Message Request. (Success = %s - No write access.)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeMessage(message);
    return E_OK;
  }

  if ((userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) && 
	  (message->authorid != uid)) {
    logInfo("Internal Edit Board Message Request. (Success = %s - No write access.)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeMessage(message);
    return E_OK;
  }

  dhufree(message->description);
  message->description = description;

  if ((errcode = dbBoardEditMessage(message, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Board Message Request. (Success = %s - Update error.)\n", getErrorMesg(errcode));
    boardFreeMessage(message);
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  boardFreeMessage(message);

  logInfo("Internal Edit Board Message Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callBoardEditTopicFunction...
*
* Edit a message in this board
*********************************************************************/
int callBoardEditTopicFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, topicid = 0, locked = 0, sticky = 0;
  board_topic *topic = NULL;
  board_instance *instance = NULL;
  char *description = NULL, *summary = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  topicid = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  summary = dhustrdup(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  description = dhustrdup(s->variableType->stringValue);
  arg = arg->next;
  s = (Symbol *) arg->data;
  locked = s->variableType->intValue;
  arg = arg->next;
  s = (Symbol *) arg->data;
  sticky = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Edit Board Topic Request. (Success = %s - No valid session.)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    return E_OK;
  }

  topic = boardInitTopic();
  topic->topicid = topicid;
  if ((errcode = dbBoardTopicDetails(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Board Topic Request. (Success = %s - Could not find topic.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeTopic(topic);
    return E_OK;
  }

  instance = boardInitInstance();
  instance->boardid = topic->boardid;

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Board Topic Request. (Success = %s - Could not load board instance.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeTopic(topic);
    boardFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Edit Board Topic Request. (Success = %s - Could not find board page.)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    boardFreeTopic(topic);
    return E_OK;
  }
  boardFreeInstance(instance);

  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Edit Board Topic Request. (Success = %s - No write access.)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeTopic(topic);
    return E_OK;
  }

  if ((userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) && 
	  (topic->authorid != uid)) {
    logInfo("Internal Edit Board Topic Request. (Success = %s - No write access.)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeTopic(topic);
    return E_OK;
  }

  dhufree(topic->description);
  topic->description = description;
  dhufree(topic->summary);
  topic->summary = summary;
  topic->locked = locked;
  topic->sticky = sticky;

  if ((errcode = dbBoardEditTopic(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Edit Board Topic Request. (Success = %s - Update error.)\n", getErrorMesg(errcode));
    boardFreeTopic(topic);
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  boardFreeTopic(topic);

  logInfo("Internal Edit Board Topic Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callBoardDeleteMessageFunction...
*
* Delete a board message
*********************************************************************/
int callBoardDeleteMessageFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  board_instance *instance = NULL;
  board_topic *topic = NULL;
  board_message *message = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  message = boardInitMessage();
  message->messageid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if (uid < 0) {
    logInfo("Internal Delete Board Message Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeMessage(message);
    return E_OK;
  }

  if ((errcode = dbBoardMessageDetails(message, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Board Message Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeMessage(message);
    return E_OK;
  }
  
  topic = boardInitTopic();
  topic->topicid = message->topicid;
  if ((errcode = dbBoardTopicDetails(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Board Message Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeMessage(message);
    boardFreeTopic(topic);
    return E_OK;
  }

  instance = boardInitInstance();
  instance->boardid = topic->boardid;
  boardFreeTopic(topic);

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Delete Board Message Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    boardFreeMessage(message);
    return E_OK;
  }
  
  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Delete Board Message Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    boardFreeMessage(message);
    return E_OK;
  }
  boardFreeInstance(instance);
  
  if (userHasExecuteAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Delete Board Message Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeMessage(message);
    return E_OK;
  }
  
  if ((userHasWriteAccess(objid, info->env, info->sqlsock) != E_OK) &&
      (message->authorid != uid)) {
    logInfo("Internal Delete Board Message Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeMessage(message);
    return E_OK;
  }

  errcode = dbBoardDeleteMessage(message, info->sqlsock);

  boardFreeMessage(message);

  logInfo("Internal Delete Board Message Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callBoardSearchFunction...
*
* Search for messages
*********************************************************************/
int callBoardSearchFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *tmp = NULL, *terms = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, id = 0, count = 0, i = 0;
  board_instance *instance = NULL;
  board_message **messages = NULL;
  struct tm *start = NULL, *end = NULL;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  id = s->variableType->intValue;

  arg = arg->next;
  s = (Symbol *) arg->data;
  start = parseDateTime(s->variableType->stringValue);

  arg = arg->next;
  s = (Symbol *) arg->data;
  end = parseDateTime(s->variableType->stringValue);

  arg = arg->next;
  s = (Symbol *) arg->data;
  terms = strdup(s->variableType->stringValue);

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  instance = boardInitInstance();
  instance->boardid = id;

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    dhufree(start);
    dhufree(end);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    dhufree(start);
    dhufree(end);
    return E_OK;
  }
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    dest->variableType->intValue = ACCESSDENIED;
    boardFreeInstance(instance);
    dhufree(start);
    dhufree(end);
    return E_OK;
  }

  if ((errcode = dbBoardSearch(instance, start, end, terms, &count, &messages, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    dest->variableType->intValue = errcode;
    boardFreeInstance(instance);
    dhufree(start);
    dhufree(end);
    return E_OK;
  }
  dhufree(start);
  dhufree(end);
  boardFreeInstance(instance);

  for (i = 0; i < count; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "messageID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = messages[i]->messageid;
    boardFreeMessage(messages[i]);
  }

  dhufree(messages);

  logInfo("Internal Search Board Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callBoardSearchTopicFunction...
*
* Search for messages
*********************************************************************/
int callBoardSearchTopicFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, id = 0, count = 0, i = 0;
  board_instance *instance = NULL;
  board_message **messages = NULL;
  board_topic *topic = NULL;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  id = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  topic = boardInitTopic();
  topic->topicid = id;
  
  if ((errcode = dbBoardTopicDetails(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeTopic(topic);
    return E_OK;
  }
  
  instance = boardInitInstance();
  instance->boardid = topic->boardid;

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    boardFreeTopic(topic);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    boardFreeTopic(topic);
    return E_OK;
  }
  boardFreeInstance(instance);
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    boardFreeTopic(topic);
    return E_OK;
  }

  if ((errcode = dbBoardSearchTopic(topic, &count, &messages, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeTopic(topic);
    return E_OK;
  }
  boardFreeTopic(topic);

  for (i = 0; i < count; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "messageID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = messages[i]->messageid;
    boardFreeMessage(messages[i]);
  }

  dhufree(messages);

  logInfo("Internal Search Board Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

/*********************************************************************
* callBoardListTopicsFunction...
*
* Search for topics
*********************************************************************/
int callBoardListTopicsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL, *m = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, id = 0, count = 0, i = 0;
  board_instance *instance = NULL;
  board_topic **topics = NULL;

  dest->variableType->variableEnum = ARRAY_VARIABLE;
  dest->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  id = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  instance = boardInitInstance();
  instance->boardid = id;

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    return E_OK;
  }
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    boardFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = dbBoardListTopics(instance, &count, &topics, info->sqlsock)) != E_OK) {
    logInfo("Internal Search Board Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    return E_OK;
  }
  boardFreeInstance(instance);

  for (i = 0; i < count; i++) {
    s = getSymbolArrayValue(dest->variableType->arrayValues, i);
    s->variableType->variableEnum = MAP_VARIABLE;
    s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

    m = getSymbolMapValue(s->variableType->mapValues, "topicID");
    m->variableType->variableEnum = INT_VARIABLE;
    m->variableType->intValue = topics[i]->topicid;
    boardFreeTopic(topics[i]);
  }

  dhufree(topics);

  logInfo("Internal Search Board Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadBoardInstanceDetailsFunction...
*
* Load a board instance details...
*********************************************************************/
int callLoadBoardInstanceDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL, *xref = NULL;
  int errcode = 0;
  int objid = 0, uid = 0;
  board_instance *instance = NULL;
  ObjectDetails *details = NULL;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  xref = s->variableType->stringValue;
  if (isXRefValid(xref, -1, info->sqlsock, info->env, &objid) != E_OK) {
    return E_OK;
  }

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  if ((errcode = getObjectDetails(objid, &details, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Board Instance Request. (XRef = %s, Success = %s)\n", xref, getErrorMesg(errcode));
    return E_OK;
  }

  instance = boardInitInstance();
  instance->objectPath = dhustrdup(details->path);

  freeObjectDetails(details);

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Board Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Load Board Instance Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    return E_OK;
  }
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Load Calendar Instance Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    boardFreeInstance(instance);
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "boardID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = instance->boardid;
  s = getSymbolMapValue(dest->variableType->mapValues, "objectPath");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(instance->objectPath);

  boardFreeInstance(instance);

  logInfo("Internal Load Board Instance Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadBoardTopicDetailsFunction...
*
* Load a topic details...
*********************************************************************/
int callLoadBoardTopicDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, topicid = 0;
  board_instance *instance = NULL;
  board_topic *topic = NULL;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  topicid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  topic = boardInitTopic();
  topic->topicid = topicid;

  if ((errcode = dbBoardTopicDetails(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Board Topic Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeTopic(topic);
    return E_OK;
  }

  instance = boardInitInstance();
  instance->boardid = topic->boardid;

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Board Topic Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    boardFreeTopic(topic);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Load Board Topic Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    boardFreeTopic(topic);
    return E_OK;
  }
  boardFreeInstance(instance);
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Load Board Topic Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    boardFreeTopic(topic);
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "boardID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = topic->boardid;
  s = getSymbolMapValue(dest->variableType->mapValues, "topicID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = topic->topicid;
  s = getSymbolMapValue(dest->variableType->mapValues, "summary");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(topic->summary);
  s = getSymbolMapValue(dest->variableType->mapValues, "description");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(topic->description);
  s = getSymbolMapValue(dest->variableType->mapValues, "authorID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = topic->authorid;
  s = getSymbolMapValue(dest->variableType->mapValues, "locked");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = topic->locked;
  s = getSymbolMapValue(dest->variableType->mapValues, "sticky");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = topic->sticky;
  s = getSymbolMapValue(dest->variableType->mapValues, "created");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatDateTime(topic->created);
  s = getSymbolMapValue(dest->variableType->mapValues, "modified");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatDateTime(topic->modified);
  s = getSymbolMapValue(dest->variableType->mapValues, "views");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = topic->views;
  boardFreeTopic(topic);

  logInfo("Internal Load Board Topic Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callLoadBoardMessageDetailsFunction...
*
* Load a message details...
*********************************************************************/
int callLoadBoardMessageDetailsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  LinkedList *arg = NULL;
  Symbol *s = NULL;
  char *tmp = NULL;
  int errcode = 0;
  int objid = 0, uid = 0, messageid = 0;
  board_instance *instance = NULL;
  board_topic *topic = NULL;
  board_message *message = NULL;

  dest->variableType->variableEnum = MAP_VARIABLE;
  dest->variableType->mapValues = initMap(compareSymbols, freeSymbol);

  arg = list;
  s = (Symbol *) arg->data;
  messageid = s->variableType->intValue;

  tmp = getEnvValue("userID", info->env);
  uid = strtol(tmp?tmp:"-1", NULL, 10);

  message = boardInitMessage();
  message->messageid = messageid;
  
  if ((errcode = dbBoardMessageDetails(message, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Board Message Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeMessage(message);
    return E_OK;
  }
  
  topic = boardInitTopic();
  topic->topicid = message->topicid;

  if ((errcode = dbBoardTopicDetails(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Board Message Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeTopic(topic);
    boardFreeMessage(message);
    return E_OK;
  }

  instance = boardInitInstance();
  instance->boardid = topic->boardid;
  boardFreeTopic(topic);

  if ((errcode = dbBoardInstanceDetails(instance, info->sqlsock)) != E_OK) {
    logInfo("Internal Load Board Message Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    boardFreeMessage(message);
    return E_OK;
  }

  if ((errcode = isXRefValid(instance->objectPath, -1, info->sqlsock, info->env, &objid)) != E_OK) {
    logInfo("Internal Load Board Message Request. (Success = %s)\n", getErrorMesg(errcode));
    boardFreeInstance(instance);
    boardFreeMessage(message);
    return E_OK;
  }
  boardFreeInstance(instance);
  
  if (userHasReadAccess(objid, info->env, info->sqlsock) != E_OK) {
    logInfo("Internal Load Board Message Request. (Success = %s)\n", getErrorMesg(ACCESSDENIED));
    boardFreeMessage(message);
    return E_OK;
  }

  s = getSymbolMapValue(dest->variableType->mapValues, "messageID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = message->messageid;
  s = getSymbolMapValue(dest->variableType->mapValues, "topicID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = message->topicid;
  s = getSymbolMapValue(dest->variableType->mapValues, "description");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = strdup(message->description);
  s = getSymbolMapValue(dest->variableType->mapValues, "authorID");
  s->variableType->variableEnum = INT_VARIABLE;
  s->variableType->intValue = message->authorid;
  s = getSymbolMapValue(dest->variableType->mapValues, "created");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatDateTime(message->created);
  s = getSymbolMapValue(dest->variableType->mapValues, "modified");
  s->variableType->variableEnum = STRING_VARIABLE;
  s->variableType->stringValue = formatDateTime(message->modified);
  boardFreeMessage(message);

  logInfo("Internal Load Board Message Request. (Success=%s)\n", getErrorMesg(errcode));
  return E_OK;
}

/*********************************************************************
* callBoardIncrementViewsFunction...
*
* Increment the view count for this topic.
*********************************************************************/
int callBoardIncrementViewsFunction(LinkedList *list, ParserInfo *info, Symbol *dest) {
  Symbol *s = NULL;
  LinkedList *arg = NULL;
  int errcode = 0;
  int topicid = 0;
  board_topic *topic = NULL;

  dest->variableType->variableEnum = INT_VARIABLE;

  arg = list;
  s = (Symbol *) arg->data;
  topicid = s->variableType->intValue;

  topic = boardInitTopic();
  topic->topicid = topicid;

  if ((errcode = dbBoardIncrementViews(topic, info->sqlsock)) != E_OK) {
    logInfo("Internal Board Increment View Request. (Success = %s - Update error.)\n", getErrorMesg(errcode));
    boardFreeTopic(topic);
    dest->variableType->intValue = errcode;
    return E_OK;
  }

  boardFreeTopic(topic);

  logInfo("Internal Board Increment View Request. (Success=%s)\n", getErrorMesg(errcode));
  dest->variableType->intValue = errcode;
  return E_OK;
}

int registerFunctions(SymbolTable *symbols) {
	LinkedList *args = NULL;

	args = NULL;
	appendLinkedList(&args, initVariableMapSymbol(strdup("")));
	registerNativeFunction(symbols, "getMapKeys", callGetMapKeysFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "write", callWriteFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "writeln", callWritelnFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "get", callGetFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "isValidFilename", callIsValidFilenameFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "set", callSetFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "login", callLoginFunction, args);
	
	args = NULL;
	registerNativeFunction(symbols, "logout", callLogoutFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "urlRewrite", callURLRewriteFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "include", callIncludeFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getFile", callLoadObjectFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "exportPackage", callExportPackageFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "importPackage", callImportPackageFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getErrorMessage", callGetErrorMessageFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getRootFolderContents", callLoadRootFolderContentsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getRootFolderContentsLength", callLoadRootFolderContentsLengthFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getFolderContentsLength", callLoadFolderContentsLengthFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getDeletedRootFolderContents", callLoadDeletedRootFolderContentsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getDeletedFolderContents", callLoadDeletedFolderContentsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getFolderContents", callLoadFolderContentsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getFileVersions", callLoadObjectVersionsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableArraySymbol(strdup("")));
	registerNativeFunction(symbols, "arrayLength", callArrayLengthFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableMapSymbol(strdup("")));
	registerNativeFunction(symbols, "mapLength", callMapLengthFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getFileDetails", callLoadObjectDetailsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getDeletedFileDetails", callLoadDeletedObjectDetailsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getVerifierComment", callLoadVerifierCommentFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getAllVerifierComments", callLoadAllVerifierCommentsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableArraySymbol(strdup("")));
	registerNativeFunction(symbols, "join", callJoinFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "split", callSplitFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getFileID", callLoadObjectIDFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getFilePath", callLoadObjectPathFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "editFileDetails", callEditObjectDetailsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "uploadFileExists", callFileExistsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "replaceFileContents", callReplaceFileContentsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "replaceFileText", callReplaceTextContentsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "rollbackFileVersion", callRollbackObjectVersionFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "recoverFileVersion", callRecoverObjectVersionFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "createNewFolder", callCreateNewFolderFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "createNewFile", callCreateNewObjectFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "createNewTextFile", callCreateNewTextObjectFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "deleteFile", callDeleteObjectFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "search", callSearchContentFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "searchLength", callSearchContentLengthFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "createNewUser", callCreateNewUserFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getUserList", callLoadUserListFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getUserDetails", callLoadUserDetailsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "deleteUser", callDeleteUserFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "editUserDetails", callEditUserDetailsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "createNewGroup", callCreateNewGroupFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getGroupList", callLoadGroupListFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getGroupDetails", callLoadGroupDetailsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "deleteGroup", callDeleteGroupFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "editGroupDetails", callEditGroupDetailsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getGroupMembers", callLoadGroupMembersFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "addGroupMember", callAddGroupMemberFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "removeGroupMember", callRemoveGroupMemberFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getPermissionList", callLoadPermissionListFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getPermissionBits", callLoadPermissionBitsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "addPermission", callAddPermissionFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "removePermission", callRemovePermissionFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "escape", callEscapeFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "unescape", callUnescapeFunction, args);

	args = NULL;
	registerNativeFunction(symbols, "getVersion", callGetVersionFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "setUserMetadata", callSetUserMetadataFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getUserMetadata", callGetUserMetadataFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getAllUserMetadata", callGetAllUserMetadataFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "removeUserMetadata", callRemoveUserMetadataFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "setFileMetadata", callSetObjectMetadataFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getFileMetadata", callGetObjectMetadataFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getDeletedFileMetadata", callGetDeletedObjectMetadataFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getAllFileMetadata", callGetAllObjectMetadataFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "removeFileMetadata", callRemoveObjectMetadataFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getISODate", callGetISODateFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "csvEscape", callCSVEscapeFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "capitalise", callCapitaliseFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "xmlEscape", callXMLEscapeFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "urlBase", callURLBaseFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "setSessionData", callSetSessionDataFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getSessionData", callGetSessionDataFunction, args);

	args = NULL;
	registerNativeFunction(symbols, "getAllSessionData", callGetAllSessionDataFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "removeSessionData", callRemoveSessionDataFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getUsersGroups", callLoadUsersGroupsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "lockFile", callLockObjectFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "unlockFile", callUnLockObjectFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getWorkflowSettings", callLoadWorkflowSettingsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getNotificationSettings", callGetNotificationSettingsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "attachWorkflowSettings", callAttachWorkflowSettingsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "attachNotificationSettings", callAttachNotificationSettingsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getWorkflowList", callLoadWorkflowListFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "approveFile", callApproveObjectFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "moveFile", callMoveObjectFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "removeWorkflowSettings", callRemoveWorkflowSettingsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "removeNotificationSettings", callRemoveNotificationSettingsFunction, args);

	args = NULL;
	registerNativeFunction(symbols, "rand", callRandFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "sendEmail", callSendEmailFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "isVerifier", callIsVerifierFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "copyFile", callCopyObjectFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "addVerifierComment", callAddVerifierCommentFunction, args);


    // These are all calendar functions
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "calCreateInstance", callCalCreateInstanceFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calCreateEvent", callCalCreateEventFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calCreateOccurrence", callCalCreateOccurrenceFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calDeleteInstance", callCalDeleteInstanceFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calDeleteEvent", callCalDeleteEventFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calDeleteOccurrence", callCalDeleteOccurrenceFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "calMoveInstance", callCalMoveInstanceFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calEditOccurrence", callCalEditOccurrenceFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "calGetInstanceDetails", callLoadCalInstanceDetailsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calGetInstancePath", callLoadCalInstancePathFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calGetEventDetails", callLoadCalEventDetailsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calGetOccurrenceDetails", callLoadCalOccurrenceDetailsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "calSearchByEvent", callSearchCalByEventFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "calSearchByDateTime", callSearchCalByDateTimeFunction, args);

    // These are date functions
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "compareDate", callCompareDateFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "compareTime", callCompareTimeFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "compareDateTime", callCompareDateTimeFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "addDate", callAddDateFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "addTime", callAddTimeFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "addDateTime", callAddDateTimeFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "subtractDate", callSubtractDateFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "subtractTime", callSubtractTimeFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "subtractDateTime", callSubtractDateTimeFunction, args);

	args = NULL;
	registerNativeFunction(symbols, "currentDate", callCurrentDateFunction, args);
	
	args = NULL;
	registerNativeFunction(symbols, "currentTime", callCurrentTimeFunction, args);

	args = NULL;
	registerNativeFunction(symbols, "currentDateTime", callCurrentDateTimeFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getMonthName", callGetMonthNameFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "getMonthNameShort", callGetMonthNameShortFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getDayName", callGetDayNameFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "getDayNameShort", callGetDayNameShortFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "validDate", callValidDateFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "validDateTime", callValidDateTimeFunction, args);

    // These are board functions 
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "boardCreateInstance", callBoardCreateInstanceFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardDeleteInstance", callBoardDeleteInstanceFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "boardMoveInstance", callBoardMoveInstanceFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardCreateTopic", callBoardCreateTopicFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardEditTopic", callBoardEditTopicFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardDeleteTopic", callBoardDeleteTopicFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "boardCreateMessage", callBoardCreateMessageFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "boardEditMessage", callBoardEditMessageFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardDeleteMessage", callBoardDeleteMessageFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "boardSearch", callBoardSearchFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardSearchTopic", callBoardSearchTopicFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardListTopics", callBoardListTopicsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableStringSymbol(strdup(""), strdup("")));
	registerNativeFunction(symbols, "boardGetInstanceDetails", callLoadBoardInstanceDetailsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardGetTopicDetails", callLoadBoardTopicDetailsFunction, args);
	
	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardGetMessageDetails", callLoadBoardMessageDetailsFunction, args);

	args = NULL;
	appendLinkedList(&args, initVariableIntSymbol(strdup(""), 0));
	registerNativeFunction(symbols, "boardIncrementViews", callBoardIncrementViewsFunction, args);

	return E_OK;
}
