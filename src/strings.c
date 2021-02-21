#include <stdarg.h>
#include <stdlib.h>
#ifdef WIN32
#include "win32.h"
#include <io.h>
#else
#include <unistd.h>
#include <sys/file.h>
#define O_BINARY 0
#endif
#include "strings.h"
#include "errors.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ctype.h>
#include <time.h>
#include "malloc.h"
#include <errno.h>

/*******************************************************************************
* getTimeValue...
*
* Get the time as iso (in number).
*******************************************************************************/
int getTimeValue(char *isotime, char *ctime) {
  struct tm t;
  int i = 0;

  if (!isotime || strlen(isotime) != 14) {
    if (ctime) {
      return strtol(ctime, NULL, 10);
    } else {
      return time(NULL);
    }
  }

  for (i = 0; i < 14; i++) {
    if (!isdigit(isotime[i])) {
      return time(NULL);
    }
  }

  memset(&t, 0, sizeof(struct tm));

  t.tm_year = ((isotime[0] - '0') * 1000) + ((isotime[1] - '0') * 100) + ((isotime[2] - '0') * 10) + ((isotime[3] - '0')) - 1900;
  t.tm_mon = ((isotime[4] - '0') * 10) + ((isotime[5] - '0')) - 1;
  t.tm_mday = ((isotime[6] - '0') * 10) + ((isotime[7] - '0'));
  t.tm_hour = ((isotime[8] - '0') * 10) + ((isotime[9] - '0'));
  t.tm_min = ((isotime[10] - '0') * 10) + ((isotime[11] - '0'));
  t.tm_sec = ((isotime[12] - '0') * 10) + ((isotime[13] - '0'));
  

  return mktime(&t);
}

/*******************************************************************************
* getRand...
*
* Return a random number less than max
*******************************************************************************/
int getRand(int max) {
  return rand() % max;
}
  
/*******************************************************************************
* getISODate...
*
* Return a char buffer to the current date.
*******************************************************************************/
char *getISODate() {
  char *date = NULL, *e = NULL;
  time_t t;

  t = time(NULL);

  date = ctime(&t);

  e = date + strlen(date) -1;

  while (isspace(*e) && e > date) {*e = '\0'; e--;}

  return dhustrdup(date);
}

char *getDayStr(int day) {
	switch (day) {
		case 0:
			return "Sun";
		case 1:
			return "Mon";
		case 2:
			return "Tue";
		case 3:
			return "Wed";
		case 4:
			return "Thu";
		case 5:
			return "Fri";
		case 6:
			return "Sat";
		case 7:
			return "Sun";
		default:
			return "???";
	}
}

char *getMonthStr(int month) {
	switch (month) {
		case 0:
			return "Jan";
		case 1:
			return "Feb";
		case 2:
			return "Mar";
		case 3:
			return "Apr";
		case 4:
			return "May";
		case 5:
			return "Jun";
		case 6:
			return "Jul";
		case 7:
			return "Aug";
		case 8:
			return "Sep";
		case 9:
			return "Oct";
		case 10:
			return "Nov";
		case 11:
			return "Dec";
		case 12:
			return "Jan";
		default:
			return "???";
	}
}

/****************************
 * getISO8601...
 *
 * Return a properly formated time string
 * *************************/
char *getISO8601(int time) {
  char date[64];
  struct tm *t = NULL;
  time_t tmp = (time_t) time;

  t = gmtime(&tmp);

  sprintf(date, "%s, %.2d %s %.4d %.2d:%.2d:%.2d GMT", getDayStr(t->tm_wday), t->tm_mday, getMonthStr(t->tm_mon), t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec); 

  return dhustrdup(date);
}

/*******************************************************************************
* stripEscapes...
*
* Strip string escapes.
*******************************************************************************/
void stripEscapes(char *src) {
  char *p = src, *head = src;

  while (*p != CNULL) {
    *head = *p;
    if (*p != '\\') {
      head++;
    } else {
      if (*(p + 1) == 'r') {
        *head = '\r';
        head++;
        p++;
      } else if (*(p + 1) == 'n') {
        *head = '\n';
        head++;
        p++;
      } else if (*(p + 1) == '\\') {
        *head = '\\';
        head++;
        p++;
      }
    }
    p++;
  }
  *head = *p;
}

/*******************************************************************************
* regexMatch...
*
* Does the current pattern match?
*******************************************************************************/
int regexMatch(char **thesource, char **thepattern) {
  char *source = *thesource, *pattern = *thepattern, *tmpsource = NULL, *tmppattern = NULL,
       *tmpstart = NULL, *tmpend = NULL;
  int match = 0;

  if (pattern[0] == '\\') {
    if (tolower(*source) == tolower(pattern[1])) {
      match = 1;
      source++;
    }
    pattern+= 2;
  } else {
    switch (pattern[1]) {
      case '*':
        tmpsource = source;
        tmppattern = pattern + 2;
        if (*pattern == '.') {
          match = miniRegex(tmpsource, tmppattern, &tmpstart, &tmpend);
          if (match == E_OK) {
            match = 1;
            source = tmpend;
            pattern = pattern + strlen(pattern);
          } else {
            match = 0;
          }
        } else {
          while ((tolower(*source) == tolower(*pattern) || *pattern == '.') && (*source != CNULL) && (!regexMatch(&tmpsource, &tmppattern))) {
            source++;
            tmpsource = source;
            tmppattern = pattern + 2;
          }
          match = 1;
          pattern+= 2;
        }
        break;
      case '+':
        tmpsource = source;
        tmppattern = pattern + 2;
        if (*source != CNULL) {
          if (*pattern == '.') {
            tmpsource++;
            match = miniRegex(tmpsource, tmppattern, &tmpstart, &tmpend);
            if (match == E_OK) {
              match = 1;
              source = tmpend;
              pattern = pattern + strlen(pattern);
            } else {
              match = 0;
            }
          } else {
            while ((tolower(*source) == tolower(*pattern) || *pattern == '.') && (*source != CNULL) && (!regexMatch(&tmpsource, &tmppattern))) {
              source++;
              tmpsource = source;
              tmppattern = pattern + 2;
              match = 1;
            }
            if (match)
              pattern+= 2;
          }
        }
        break;
      case '?':
        if ((tolower(*source) == tolower(*pattern) || *pattern == '.')) {
          source++;
        }
        match = 1;
        pattern+= 2;
        break;
      default:
        if ((tolower(*source) == tolower(*pattern) || *pattern == '.')) {
          pattern++;
          source++;
          match = 1;
        }
        break;
    }
  }

  *thesource = source;
  *thepattern = pattern;
  return match;
}

/*******************************************************************************
* miniRegex...
*
* Process a mini regex - this is not standards compliant
* but by reducing the unused functionality of the regex we can greatly
* increase the speed.
*
* This supports the following wildcards * + ? and .
*******************************************************************************/
int miniRegex(char *source, char *pattern, char **thestart, char **theend) {
  char *start = NULL, *end = NULL, *current = NULL, *rollback = NULL;

  if (pattern == NULL || source == NULL)
    return INVALIDREGEX;

  current = pattern;
  start = source;
  end = start;
  rollback = start;
  do {
    if (regexMatch(&end, &current)) {
      if (*current == CNULL) {
        *thestart = rollback;
        *theend = end;
        return E_OK;
      }
    } else {
      start = ++rollback;
      end = start;
      current = pattern;
    }
  } while (*start != CNULL);
  
  return REGEXNOMATCH;
}

/*******************************************************************************
* concatChars...
*
* Variatic char concatination function.
*******************************************************************************/
char *concatChars(int num , ...) {
  va_list args;
  char   argv[MAX_VARGS], *ptr = NULL;
  int i = 0;

  va_start(args, num);
  for (i = 1; i <= num; i++) {
    argv[i] = va_arg(args, int);                               
  }
  argv[i] = CNULL;

  ptr = (char *) dhumalloc(sizeof(char) * (i + 1));
  memcpy(ptr, (argv + 1), (i + 1));

  va_end(args);
  return ptr;
}

/*******************************************************************************
* vstrdupcat...
*
* Variatic string concatination function.
*******************************************************************************/
void vstrdupcat(char **str, ...) {
  va_list args;
  char   *argv[MAX_VARGS], *ptr = *str;
  long long int i = 0, len = 0,
      extralen = 0,
      increment = 8192,
      nalloc = 0;  
  int lastarg = 0;
                                                                  
  va_start(args, str);
  while (!lastarg) { 
    argv[i] = va_arg(args, char *);                               
    if ((argv[i]) && (i < (MAX_VARGS -1))) {                      
      extralen += strlen(argv[i]);
    } else {
      lastarg = 1;
    }
    i++;
  }
  
  if (ptr) {
    len = strlen(ptr);
    if ((len / increment) < ((len + extralen) / increment)) {
      // we need to realloc
      nalloc = (((len + extralen) / increment) + 1)*increment;
      ptr = (char *) dhurealloc(ptr, sizeof(char) * (nalloc));
      if (ptr == NULL)
        return;
    }
  } else {
    nalloc = ((extralen / increment) + 1)*increment;
    ptr = (char *) dhumalloc(sizeof(char) * (nalloc));
    if (ptr == NULL)
      return;
    *ptr = '\0';
  }

  i = 0;
  while (argv[i] != NULL) {
    strcat(ptr, argv[i]);
    i++;
  }

  va_end(args);
  *str = ptr;
}


/*******************************************************************************
* getNextLineFromStream...
*
* Parses the next line from a stream and returns it in a dhumalloc buffer.
*******************************************************************************/
char *getNextLineFromStream(char **sptr) {
  char *start = NULL, *ptr = *sptr, c = 0;
  
  start = ptr;
  if ((ptr = strpbrk(start, "\r\n")) != NULL) {
    c = *ptr;
    *ptr = CNULL;
    start = dhustrdup(start);
    *ptr = c;
    while (*ptr == '\r' || *ptr == '\n') ptr++;
  } else {
    start = NULL;
  }
  *sptr = ptr;
  return start;
}

/*******************************************************************************
* URLEncode...
*
* Encode the value.
*******************************************************************************/
char *URLEncode(char *value) {
  int i = 0,
           j = 0,
           len = 0;
  char hex[3], *encoded = NULL;

  encoded = (char *) dhumalloc(sizeof(char) * (strlen(value)*3 + 1));
  len = strlen(value);
  hex[2] = CNULL;

  i = 0;
  while (i < len) {
    if (!isalnum(value[i])) {
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
* URLDecode...
*
* Decode the value.
*******************************************************************************/
char *URLDecode(char *value) {
  int i = 0,
           j = 0,
           len = 0,
           c = 0;
  char hex[3], *decoded = NULL;

  decoded = (char *) dhumalloc(sizeof(char) * (strlen(value) + 1));
  len = strlen(value);
  hex[2] = CNULL;

  i = 0;
  while (i < len) {
    if (value[i] == '%') {
      hex[0] = value[i + 1];
      hex[1] = value[i + 2];
      i += 2;
      c = strtol(hex, (char **) NULL, 16);
      sprintf(&decoded[j], "%c", (char) c);
    } else if (value[i] == '+') {
      decoded[j] = ' ';
    } else {
      decoded[j] = value[i];
    }
    i++; j++;
  }
  decoded[j] = CNULL;
  return decoded;
}

/******************************************************************************
* readOpenFile...
*
* given an open file this function returns
* a pointer to a newly dhumalloc piece
* of memory holding the contents of the file
* returns NULL on error.
*******************************************************************************/
char *readOpenFile(int fd, int *plen) {
  FILE     *in = NULL;
  char     *input = NULL, *current = NULL;
  int bytesread = 0, totalread = 0;
  struct stat filestats;

  if (fstat(fd, &filestats) != 0) {
    perror("readopenfile: ");
    return NULL;
  }

  in = fdopen(fd, "rb");
  if (in == NULL) {
    perror("readopenfile: ");
    return NULL;
  }

  input = (char *) calloc(sizeof(char), (filestats.st_size) + 1);

  if (input == NULL) {
    perror("readopenfile: ");
    return NULL;
  }

  input[0] = CNULL;
  current = input;

  while (totalread < filestats.st_size) {
    bytesread = fread(current, sizeof(char), (unsigned)(filestats.st_size - totalread), in);
    if (bytesread > 0) {
      totalread += bytesread;
    } else {
      if (feof(in)) {
        break;
      } else {
        perror("readopenfile: ");
        dhufree(input);
        return NULL;
      }
    }
  }

  fclose(in);
  *plen = totalread;
  return input;
}

/*******************************************************************************
* readAsciiFile...
*
* given a file name to an ASCII file, 
* will read the file contents into a NULL terminated char *.
*******************************************************************************/
char *readAsciiFile(char *filename) {
  int  fd = 0;                                                    
  int len = 0;
  char *input = NULL;

  if ((fd = open(filename, O_RDONLY|O_BINARY)) == -1)                      
    return NULL;

#ifdef WIN32
  _setmode( fd, _O_BINARY );
#endif

  input = readOpenFile(fd, &len);

  input[len] = CNULL;
  close(fd);

  return input;
}

/*******************************************************************************
* int2Bool...
*
* convert an integer into a bool.
*******************************************************************************/
void int2Bool(int i, char **str) {
  *str = (char *) dhumalloc(sizeof(char)*2);
  if (i)
    (*str)[0] = 'y';
  else
    (*str)[0] = 'n';
  (*str)[1] = '\0';
}

/*******************************************************************************
* time2Str...
*
* convert an integer into a string.
*******************************************************************************/
void time2Str(time_t i, char **str) {
  char *a = (char *) malloc(sizeof(char) * 32);

  if (i == -1)
    i = time(NULL) + 1;

  sprintf(a, "%d", (int) i);

  *str = a;
}

/*******************************************************************************
* int2Str...
*
* convert an integer into a string.
*******************************************************************************/
void int2Str(int i, char **str) {
  char *a = (char *) malloc(sizeof(char) * 32);

  sprintf(a, "%d", (int) i);

  *str = a;
}

/*******************************************************************************
* formatDateTime...
*
* Create a date time string.
*******************************************************************************/
char *formatDateTime(struct tm *t) {
  char *datetimestr = NULL;
  
  datetimestr = (char *) malloc(sizeof(char) * 64);
  sprintf(datetimestr, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

  return datetimestr;
}

/*******************************************************************************
* formatTime...
*
* Create a time string.
*******************************************************************************/
char *formatTime(struct tm *t) {
  char *timestr = NULL;
  
  timestr = (char *) malloc(sizeof(char) * 64);
  sprintf(timestr, "%.2d:%.2d:%.2d", t->tm_hour, t->tm_min, t->tm_sec);

  return timestr;
}

/*******************************************************************************
* formatDate...
*
* Create a date string.
*******************************************************************************/
char *formatDate(struct tm *t) {
  char *datestr = NULL;

  datestr = (char *) malloc(sizeof(char) * 64);
  sprintf(datestr, "%.2d-%.2d-%.2d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

  return datestr;
}

/*******************************************************************************
* parseDateTime...
*
* Create a date time struct.
*******************************************************************************/
struct tm *parseDateTime(char *datetimestr) {
  struct tm *t = NULL;
  char *c = NULL;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));

  c = datetimestr;
  t->tm_year = strtol(c, NULL, 10) - 1900;

  while (*c != '-' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_mon = strtol(c, NULL, 10) - 1;
  
  while (*c != '-' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_mday = strtol(c, NULL, 10);

  while (*c != ' ' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_hour = strtol(c, NULL, 10);

  while (*c != ':' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_min = strtol(c, NULL, 10);

  while (*c != ':' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_sec = strtol(c, NULL, 10);

  return t;
}

void printDateTime(struct tm *t) {
  fprintf(stderr, "Date Time: %d-%d-%d %d:%d:%d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

/*******************************************************************************
* parseTime...
*
* Create a time struct.
*******************************************************************************/
struct tm *parseTime(char *timestr) {
  struct tm *t = NULL;
  char *c = NULL;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));

  c = timestr;
  t->tm_hour = strtol(c, NULL, 10);

  while (*c != ':' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_min = strtol(c, NULL, 10);
  
  while (*c != ':' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_sec = strtol(c, NULL, 10);

  return t;
}

/*******************************************************************************
* parseDate...
*
* Create a date struct.
*******************************************************************************/
struct tm *parseDate(char *datestr) {
  struct tm *t = NULL;
  char *c = NULL;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));
  c = datestr;
  t->tm_year = strtol(c, NULL, 10) - 1900;

  while (*c != '-' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_mon = strtol(c, NULL, 10) - 1;

  while (*c != '-' && *c != '\0') c++;
  if (*c == '\0')
    return t;
  c++;
  t->tm_mday = strtol(c, NULL, 10);
 
  return t;
}

/*******************************************************************************
* compareTime...
*
* Compare two time structs
*******************************************************************************/
int compareTime(struct tm *a, struct tm *b) {
  if (a->tm_hour < b->tm_hour)
    return -1;
  if (a->tm_hour > b->tm_hour)
    return 1;
  if (a->tm_min < b->tm_min)
    return -1;
  if (a->tm_min > b->tm_min)
    return 1;
  if (a->tm_sec < b->tm_sec)
    return -1;
  if (a->tm_sec > b->tm_sec)
    return 1;
  return 0;
}

/*******************************************************************************
* compareDateTime...
*
* Compare two datetime structs
*******************************************************************************/
int compareDateTime(struct tm *a, struct tm *b) {
  int cmp = compareDate(a, b);
  if (cmp == 0)
    return compareTime(a, b);
  return cmp;
}

/*******************************************************************************
* compareDate...
*
* Compare two date structs
*******************************************************************************/
int compareDate(struct tm *a, struct tm *b) {
  if (a->tm_year < b->tm_year)
    return -1;
  if (a->tm_year > b->tm_year)
    return 1;
  if (a->tm_mon < b->tm_mon)
    return -1;
  if (a->tm_mon > b->tm_mon)
    return 1;
  if (a->tm_mday < b->tm_mday)
    return -1;
  if (a->tm_mday > b->tm_mday)
    return 1;
  return 0;
}

/*******************************************************************************
* addTime...
*
* Compare two time structs
*******************************************************************************/
struct tm * addTime(struct tm *a, struct tm *b) {
  struct tm *t = NULL;
  time_t ta;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));

  t->tm_sec = a->tm_sec + b->tm_sec;
  t->tm_hour = a->tm_hour + b->tm_hour;
  t->tm_min = a->tm_min + b->tm_min;

  t->tm_mday = 1;
  t->tm_year = 70;
  ta = mktime(t);
  localtime_r(&ta, t);
  
  return t;
}

/*******************************************************************************
* addDateTime...
*
* Compare two datetime structs
*******************************************************************************/
struct tm * addDateTime(struct tm *a, struct tm *b) {
  struct tm *t = NULL;
  time_t ta;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));

  t->tm_year = a->tm_year + b->tm_year;
  t->tm_mon = a->tm_mon + b->tm_mon;
  t->tm_mday = a->tm_mday + b->tm_mday;
  t->tm_sec = a->tm_sec + b->tm_sec;
  t->tm_hour = a->tm_hour + b->tm_hour;
  t->tm_min = a->tm_min + b->tm_min;

  ta = mktime(t);
  localtime_r(&ta, t);
  
  return t;
}

/*******************************************************************************
* addDate...
*
* Compare two date structs
*******************************************************************************/
struct tm * addDate(struct tm *a, struct tm *b) {
  struct tm *t = NULL;
  time_t ta;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));

  t->tm_year = a->tm_year + b->tm_year;
  t->tm_mon = a->tm_mon + b->tm_mon;
  t->tm_mday = a->tm_mday + b->tm_mday;

  ta = mktime(t);
  localtime_r(&ta, t);
  
  return t;
}

/*******************************************************************************
* subtractTime...
*
* Compare two time structs
*******************************************************************************/
struct tm * subtractTime(struct tm *a, struct tm *b) {
  struct tm *t = NULL;
  time_t ta;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));

  t->tm_sec = a->tm_sec - b->tm_sec;
  t->tm_hour = a->tm_hour - b->tm_hour;
  t->tm_min = a->tm_min - b->tm_min;

  ta = mktime(t);
  localtime_r(&ta, t);
  
  return t;
}

/*******************************************************************************
* subtractDateTime...
*
* Compare two datetime structs
*******************************************************************************/
struct tm * subtractDateTime(struct tm *a, struct tm *b) {
  struct tm *t = NULL;
  time_t ta;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));

  t->tm_year = a->tm_year - b->tm_year;
  t->tm_mon = a->tm_mon - b->tm_mon;
  t->tm_mday = a->tm_mday - b->tm_mday;
  t->tm_sec = a->tm_sec - b->tm_sec;
  t->tm_hour = a->tm_hour - b->tm_hour;
  t->tm_min = a->tm_min - b->tm_min;

  ta = mktime(t);
  localtime_r(&ta, t);
  
  return t;
}

/*******************************************************************************
* subtractDate...
*
* Compare two date structs
*******************************************************************************/
struct tm * subtractDate(struct tm *a, struct tm *b) {
  struct tm *t = NULL;
  time_t ta;

  t = (struct tm *) malloc(sizeof(struct tm));
  memset(t, 0, sizeof(struct tm));

  t->tm_year = a->tm_year - b->tm_year;
  t->tm_mon = a->tm_mon - b->tm_mon;
  t->tm_mday = a->tm_mday - b->tm_mday;
  
  ta = mktime(t);
  localtime_r(&ta, t);
  
  return t;
}
