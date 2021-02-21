/************************************************************************
* calendar.h
*
* User Management functions.
************************************************************************/
#ifndef _CALENDAR_H
#define _CALENDAR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cal_instance_ {
  int calid;
  char *objectPath;
} cal_instance;

typedef struct _cal_event_ {
  int eventid, calid;
  struct tm *created, *modified;
} cal_event;

typedef struct _cal_occurrence_ {
  int occurrenceid, eventid;
  int allday;
  char *summary, *description, *location;
  struct tm *created, *modified, *eventdate, *starttime, *endtime;
} cal_occurrence;

cal_instance * calInitInstance();
cal_event * calInitEvent();
cal_occurrence * calInitOccurrence();

void calFreeInstance(cal_instance *inst);
void calFreeEvent(cal_event *evt);
void calFreeOccurrence(cal_occurrence *occ);

#ifdef __cplusplus
}
#endif


#endif // _CALENDAR_H
