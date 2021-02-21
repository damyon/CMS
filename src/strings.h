#ifndef _STRINGS_H_
#define _STRINGS_H_

#include <time.h>
/* Standard #defines */

#ifdef __cplusplus
extern "C" {
#endif


#ifndef CNULL
#define CNULL '\0'
#endif

#ifndef MAX_VARGS
#define MAX_VARGS 100                                             
#endif 

#ifndef n_free
#define n_free(x) if((x) != NULL){free((void *) x);x=NULL;}
#endif

/*******************************************************************************
* getISO8601...
*
* Get the time as iso (as string).
*******************************************************************************/
char *getISO8601(int time);

/*******************************************************************************
* getTimeValue...
*
* Get the time as iso (in number).
*******************************************************************************/
int getTimeValue(char *isotime, char *ctime);

/*******************************************************************************
* getISODate...
*
* Return a char buffer to the current date.
*******************************************************************************/
char *getISODate(void);

/*******************************************************************************
* vstrdupcat...
*
* Variatic string concatination function.
*******************************************************************************/
void vstrdupcat(char **str, ...);

/*******************************************************************************
* concatChars...
*       
* Variatic char concatination function.
*******************************************************************************/
char *concatChars(int num , ...);

/*******************************************************************************
* getNextLineFromStream...
*
* Parses the next line from a stream and returns it in a malloced buffer.
*******************************************************************************/
char *getNextLineFromStream(char **ptr);
 
/*******************************************************************************
* URLDecode...
*
* URL Decode a string
*******************************************************************************/
char *URLDecode(char *value);

/*******************************************************************************
* URLEncode...
*
* URL Encode a string
*******************************************************************************/
char *URLEncode(char *value);

/******************************************************************************
* readOpenFile...
*
* given an open file this function returns
* a pointer to a newly malloced piece
* of memory holding the contents of the file
* returns NULL on error.
*******************************************************************************/
char *readOpenFile(int fd, int *plen);

/*******************************************************************************
* readAsciiFile...
*
* given a file name to an ASCII file,
* will read the file contents into a NULL terminated char *.
*******************************************************************************/
char *readAsciiFile(char *filename);

/*******************************************************************************
* miniRegex...
*
* Process a mini regex - this is not standards compliant
* but by reducing the unused functionality of the regex we can greatly
* increase the speed.
*     
* This supports the following wildcards * + ? and .
*******************************************************************************/
int miniRegex(char *source, char *pattern, char **thestart, char **theend);

/*******************************************************************************
* time2Str...
*
* convert an integer into a string.
*******************************************************************************/
void time2Str(time_t i, char **str);


/*******************************************************************************
* int2Str...
*
* convert an integer into a string.
*******************************************************************************/
void int2Str(int i, char **str);

/*******************************************************************************
* int2Bool...
*
* convert an integer into a 'y'/'n' string.
*******************************************************************************/
void int2Bool(int i, char **str);

/*******************************************************************************
* getRand...
*
* return a random number less than max
*******************************************************************************/
int getRand(int max);

/*******************************************************************************
* stripEscapes...
*
* strip escapes (\) from the string
*******************************************************************************/
void stripEscapes(char *src);

/*******************************************************************************
* formatDateTime...
*
* Create a date time string.
*******************************************************************************/
char *formatDateTime(struct tm *t);

/*******************************************************************************
* formatTime...
*
* Create a time string.
*******************************************************************************/
char *formatTime(struct tm *t);

/*******************************************************************************
* formatDate...
*
* Create a date string.
*******************************************************************************/
char *formatDate(struct tm *t);

/*******************************************************************************
* parseDateTime...
*
* Create a date time struct.
*******************************************************************************/
struct tm *parseDateTime(char *datetimestr);

/*******************************************************************************
* parseTime...
*
* Create a time struct.
*******************************************************************************/
struct tm *parseTime(char *timestr);

/*******************************************************************************
* parseDate...
*
* Create a date struct.
*******************************************************************************/
struct tm *parseDate(char *datestr);

/*******************************************************************************
* compareTime...
*
* Compare two time structs
*******************************************************************************/
int compareTime(struct tm *a, struct tm *b);

/*******************************************************************************
* compareDateTime...
*
* Compare two datetime structs
*******************************************************************************/
int compareDateTime(struct tm *a, struct tm *b);

/*******************************************************************************
* compareDate...
*
* Compare two date structs
*******************************************************************************/
int compareDate(struct tm *a, struct tm *b);

/*******************************************************************************
* addTime...
*
* Add two time structs
*******************************************************************************/
struct tm * addTime(struct tm *a, struct tm *b);

/*******************************************************************************
* addDateTime...
*
* Add two datetime structs
*******************************************************************************/
struct tm * addDateTime(struct tm *a, struct tm *b);

/*******************************************************************************
* addDate...
*
* Add two date structs
*******************************************************************************/
struct tm * addDate(struct tm *a, struct tm *b);

/*******************************************************************************
* subtractTime...
*
* Compare two time structs
*******************************************************************************/
struct tm * subtractTime(struct tm *a, struct tm *b);

/*******************************************************************************
* subtractDateTime...
*
* Compare two datetime structs
*******************************************************************************/
struct tm * subtractDateTime(struct tm *a, struct tm *b);

/*******************************************************************************
* subtractDate...
*
* Compare two date structs
*******************************************************************************/
struct tm * subtractDate(struct tm *a, struct tm *b);

void printDateTime(struct tm *t);
#ifdef __cplusplus
}
#endif

#endif // _STRINGS_H_
