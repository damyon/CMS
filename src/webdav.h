/*******************************************************************************
 * webdav.h
 *
 * @author Damyon Wiese
 * @date 17/10/2003
 * @description This file handles webdav requests.
 ******************************************************************************/

// This is used to prevent this header file from being included multiple times.
#ifndef _WEBDAV_H_
#define _WEBDAV_H_

#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _MethodType_ {PROPFIND_METHOD, PROPPATCH_METHOD, MKCOL_METHOD, DELETE_METHOD, PUT_METHOD, COPY_METHOD, MOVE_METHOD, LOCK_METHOD, UNLOCK_METHOD, HEAD_METHOD, OPTIONS_METHOD, UNKNOWN_METHOD, GET_METHOD} MethodType;

typedef enum _HttpResponseCode_ {HTTP_CREATED = 201, HTTP_OK = 200, HTTP_METHOD_NOT_ALLOWED = 405, HTTP_FORBIDDEN = 403, HTTP_AUTH_REQUIRED = 401, HTTP_BAD_REQUEST = 400, HTTP_INTERNAL_SERVER_ERROR = 500, HTTP_CONFLICT = 409, HTTP_NO_CONTENT = 204, HTTP_NOT_FOUND = 404, HTTP_UNSUPPORTED_MEDIA_TYPE = 415, HTTP_PRECONDITION_FAILED = 412} HttpResponseCode;

typedef struct {
  int depth, overwrite, contentlength, method;
  char *destination, *locktoken, *xml, *resource, *authorization, *useragent;
} webdav_request;

/*
 * initWebDavRequest
 *
 * Malloc and zero this struct.
 */
webdav_request * initWebDavRequest();

/*
 * freeWebDavRequest
 *
 * Free this struct and its members.
 */
void freeWebDavRequest(webdav_request *r);

/*
 * handleError
 *
 * Return an appropriate error response.
 */
int handleWebDavError(int code, int sockfd);

/*
 * handleWebDavRequest
 *
 * Parse the xml and respond to the webdav request.
 */
int handleWebDavRequest(int sockfd);

/*
 * getWebDavMethod
 *
 * Returns an enumeration for the method based on the recongised methods.
 */
int getWebDavMethod(const char *method);


#ifdef __cplusplus
}
#endif


// This is used to prevent this header file from being included multiple times.
#endif // _WEBDAV_H
