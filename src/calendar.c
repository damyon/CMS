#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "calendar.h"
#include "env.h"
#include "strings.h"
#include "config.h"
#include "errors.h"
#include "logging.h"
#include "structs.h"
#include "package.h"
#include "objects.h"
#include "users.h"
#include "dbcalls.h"
#include "malloc.h"

/*******************************************************************************
 * Initialisers - create an instance of each type of struct.
 ******************************************************************************/
cal_instance * calInitInstance() {
  cal_instance *inst = NULL;

  inst = (cal_instance *) malloc(sizeof(cal_instance));
  memset(inst, 0, sizeof(cal_instance));

  return inst;
}

cal_event * calInitEvent() {
  cal_event *event = NULL;

  event = (cal_event *) malloc(sizeof(cal_event));
  memset(event, 0, sizeof(cal_event));

  return event;
}

cal_occurrence * calInitOccurrence() {
  cal_occurrence *occurrence = NULL;

  occurrence = (cal_occurrence *) malloc(sizeof(cal_occurrence));
  memset(occurrence, 0, sizeof(cal_occurrence));

  return occurrence;
}

void calFreeInstance(cal_instance *inst) {
  if (inst != NULL) {
    if (inst->objectPath != NULL)
      dhufree(inst->objectPath);
    dhufree(inst);
  }
}

/*******************************************************************************
 * Freers - free an instance of each type of struct.
 ******************************************************************************/
void calFreeEvent(cal_event *evt) {
  if (evt != NULL) {
    if (evt->created != NULL)
      dhufree(evt->created);
    if (evt->modified != NULL)
      dhufree(evt->modified);
    dhufree(evt);
  }
}

void calFreeOccurrence(cal_occurrence *occ) {
  if (occ != NULL) {
    if (occ->created != NULL)
      dhufree(occ->created);
    if (occ->modified != NULL)
      dhufree(occ->modified);
    if (occ->summary != NULL)
      dhufree(occ->summary);
    if (occ->description != NULL)
      dhufree(occ->description);
    if (occ->location != NULL)
      dhufree(occ->location);
    if (occ->starttime != NULL)
      dhufree(occ->starttime);
    if (occ->endtime != NULL)
      dhufree(occ->endtime);
    if (occ->eventdate != NULL)
      dhufree(occ->eventdate);
    dhufree(occ);
  }
}

