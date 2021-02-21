/*******************************************************************************
 * base64.h
 *
 * @author Damyon Wiese
 * @date 17/10/2003
 * @description This file sends messages to the log file. In addition, it will
 * 		monitor the size of the log file and perform automatically
 * 		rotate the log files when they reach a specified limit.
 *
 ******************************************************************************/

// This is used to prevent this header file from being included multiple times.
#ifndef _BASE64_H_
#define _BASE64_H_

#include <string.h>
#include "strings.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * base64Encode
 *
 * This function will base64 encode a string.
 *
 * @param src -         A pointer to the data to be encoded
 * @param srclen -      The length of the string to be encoded
 * @param dest -        A pointer to be filled with a null-terminated base64 string
 *                      - memory will be allocated.
 * @return 0 for a win.
 ******************************************************************************/
int base64Encode(void *src, int srclen, char **dest);

/*******************************************************************************
 * base64Decode
 *
 * This function will decode a data stream that was encoded in base64
 *
 * @param src -         A null terminated string to decode.
 * @param dest -        A pointer to be filled - memory will be allocated.
 * @param destlen -     An integer to hold the length of the decode operation.
 * @return 0 for a win.
 ******************************************************************************/
int base64Decode(char *src, char **dest, int *destlen);

#ifdef __cplusplus
}
#endif


// This is used to prevent this header file from being included multiple times.
#endif // _BASE64_H_
