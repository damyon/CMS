/*******************************************************************************
 * Base64.h
 *
 * @author Damyon Wiese
 * @date 17/10/2003
 * @description This function contains the functions required to perform 
 * the base64 encoding/decoding required for xml encoding
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "base64.h"
#include "malloc.h"
#include "errors.h"
#include <stdio.h>
/*******************************************************************************
 * Structures
 ******************************************************************************/
#define PAD -2
#define IGNORE -1

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
unsigned char encodeChar(unsigned char c) {
  if (c < 26)
    return 'A' + c;
  if (c < 52)
    return 'a' + (c - 26);
  if (c < 62)
    return '0' + (c - 52);
  if (c == 62)
    return '+';
  return '/';
}

/*******************************************************************************
 * appendData
 *
 * This function append a string to a char buffer in memory,
 * resizing the buffer if required.
 * 
 * @param src -   A null terminated string to encode.
 * @param len -   The langth of the buffer to append
 * @param buflen -   The length of the memory buffer
 * @param destlen -   The length of the data in the buffer
 * @return E_OK for a win.
 ******************************************************************************/
int appendData(char *src, int len, char **dst, int *buflen, int *used) {

  if (len == 0)
    return E_OK;
  // create new buffer
  if (*dst == NULL) {
    *buflen = 16384; 
    *dst = (char *) dhumalloc(sizeof(char) * (*buflen));
    *dst[0] = '\0';
  }

  // reallocate buffer
   
  if (len + (*used) >= (*buflen)) {
    do {
      *buflen += 16384;
    } while (len + (*used) >= (*buflen));
    *dst = (char *) dhurealloc(*dst, sizeof(char) * (*buflen));
    if (!(*dst)) {
      return RESOURCEERROR;
    }
  }

  // append

  memcpy(*dst + *used, src, len);
  *used += len;
  return E_OK;
}

/*******************************************************************************
 * appendString
 *
 * This function append a string to a char buffer in memory,
 * resizing the buffer if required.
 * 
 * @param src -   A null terminated string to encode.
 * @param buflen -   The length of the memory buffer
 * @param destlen -   The length of the data in the buffer
 * @return E_OK for a win.
 ******************************************************************************/
int appendString(char *src, char **dstp, int *buflenp, int *usedp) {
  char *dst = *dstp;
  int buflen = *buflenp,
      used = *usedp,
      retcode = 0;
  
  retcode = appendData(src, strlen(src), &dst, &buflen, &used);
  *dstp = dst;
  *buflenp = buflen;
  *usedp = used;
  return retcode;
}

/*******************************************************************************
 * base64Decode
 *
 * This function will base64 encode a string.
 * 
 * @param src -   A null terminated string to encode.
 * @param dest -   A pointer to be filled - memory will be allocated.
 * @param destlen -   A integer to be filled with the length of the decoded buffer
 * @return E_OK for a win.
 ******************************************************************************/
int base64Decode(char *src, char **dest, int *destlen) {
  int len = strlen(src);
  char *b = NULL;
  int c = 0,
      cycle = 0,
      combined = 0,
      j = 0,
      i = 0,
      value = 0,
      padding = 0;

  char valueToChar[64];
  int charToValue[256];
  

  // build translate valueToChar table
  // 0..25 -> 'A'..'Z'
  for ( i=0; i<=25; i++ ) {
    valueToChar[i] = (char)('A'+i);
  }
  // 26..51 -> 'a'..'z'
  for ( i=0; i<=25; i++ ) {
    valueToChar[i+26] = (char)('a'+i);
  }
  // 52..61 -> '0'..'9'
  for ( i=0; i<=9; i++ ) {
    valueToChar[i+52] = (char)('0'+i);
  }
  valueToChar[62] = '+';
  valueToChar[63] = '/';

  // build translate charToValue table only once.
  for ( i=0; i<256; i++ ) {
    charToValue[i] = IGNORE;  // default is to ignore
  }

  for ( i=0; i<64; i++ ) {
    charToValue[(int)(valueToChar[i])] = i;
  }

  charToValue[(int)'='] = PAD;

  // allocate enough space

  b = (char *) dhumalloc(sizeof(char) * ((len / 4) * 3) + 1);
  if (b == NULL) {
    return RESOURCEERROR;
  }

  for (i = 0; i < len; i++) {
    c = src[i];
    value = (c <= 255) ? charToValue[c]:IGNORE;
    // IGNORE is used to skip this char as it is out of range.

    switch (value) {
      case IGNORE:
        break;
     
      case PAD:
        value = 0;
        // fall through
        padding++;
      
      default:
        switch (cycle) {
          case 0:
            combined = value;
            cycle = 1;
            break;
          case 1:
            combined <<=6;
            combined |= value;
            cycle = 2;
            break;
          case 2:
            combined <<=6;
            combined |= value;
            cycle = 3;
            break;
          case 3:
            combined <<= 6;
            combined |= value;
            // we have just completed a cycle of 4 chars.
            // the four 6-bit values are in combined in big-endian order
            // peel them off 8 bits at a time working lsb to msb
            // to get our original 3 8-bit bytes back

            b[j+2] = (char)combined;
            combined >>= 8;
            b[j+1] = (char)combined;
            combined >>= 8;
            b[j] = (char)combined;
            j += 3;
            cycle = 0;
            break;
        }
        break;
    }
  }

  if (cycle != 0) {
    dhufree(b);
    return INCONSISTENTDATA;
  }

  *destlen = j - padding;
  *dest = b;
  return E_OK;
}

/*******************************************************************************
 * base64Encode
 *
 * This function will base64 nncode a string.
 * 
 * @param src -   A null terminated string to encode.
 * @param dest -   A pointer to be filled - memory will be allocated.
 * @return E_OK for a win.
 ******************************************************************************/
int base64Encode(void *srcp, int srclen, char **dest) {
  char *src = (char *) srcp;
  char *dst = NULL;
  char buf[2];
  int buflen = 0, i = 0;
  int used = 0;
  
  // init
  buf[1] = '\0';
  buf[0] = '\0';
    
  //if (appendString(buf, &dst, &buflen, &used) != E_OK) {
  //  return RESOURCEERROR;
  //}
  
  if (src == NULL || srclen == 0) {
    dst = dhustrdup("");
    *dest = dst;
    return E_OK; // no data
  }
  
  for (i = 0; i < srclen; i+= 3) {
    unsigned char by1 = 0, by2 = 0, by3 = 0, by4 = 0, by5 = 0, by6 = 0, by7 = 0;

    by1 = src[i];
    if (i + 1 < srclen)
      by2 = src[i + 1];
    if (i + 2 < srclen)
      by3 = src[i + 2];
    by4 = by1 >> 2;
    by5 = ((by1 & 0x3) << 4) | (by2 >> 4);
    by6 = ((by2 & 0xf) << 2) | (by3 >> 6);
    by7 = by3 & 0x3f;
    // append to the output string
    buf[0] = encodeChar(by4);
    if (appendString(buf, &dst, &buflen, &used) != E_OK) {
      return RESOURCEERROR;
    }
    buf[0] = encodeChar(by5);
    if (appendString(buf, &dst, &buflen, &used) != E_OK) {
      return RESOURCEERROR;
    }
    
    if (i + 1 < srclen) 
      buf[0] = encodeChar(by6);
    else  
      buf[0] = '='; 
    if (appendString(buf, &dst, &buflen, &used) != E_OK) {
      return RESOURCEERROR;
    }
    if (i + 2 < srclen) 
      buf[0] = encodeChar(by7);
    else
      buf[0] = '='; 
    if (appendString(buf, &dst, &buflen, &used) != E_OK) {
      return RESOURCEERROR;
    }
  }

  if (appendData("", 1, &dst, &buflen, &used) != E_OK) {
      return RESOURCEERROR;
  }
  
  *dest = dst;
  return E_OK;
}
