#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "board.h"
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
board_instance * boardInitInstance() {
  board_instance *inst = NULL;

  inst = (board_instance *) malloc(sizeof(board_instance));
  memset(inst, 0, sizeof(board_instance));

  return inst;
}

board_topic * boardInitTopic() {
  board_topic *topic = NULL;

  topic = (board_topic *) malloc(sizeof(board_topic));
  memset(topic, 0, sizeof(board_topic));

  return topic;
}

board_message * boardInitMessage() {
  board_message *message = NULL;

  message = (board_message *) malloc(sizeof(board_message));
  memset(message, 0, sizeof(board_message));

  return message;
}

void boardFreeInstance(board_instance *inst) {
  if (inst != NULL) {
    if (inst->objectPath != NULL)
      dhufree(inst->objectPath);
    dhufree(inst);
  }
}

/*******************************************************************************
 * Freers - free an instance of each type of struct.
 ******************************************************************************/
void boardFreeTopic(board_topic *tpc) {
  if (tpc != NULL) {
    if (tpc->description != NULL)
      dhufree(tpc->description);
    if (tpc->summary != NULL)
      dhufree(tpc->summary);
    if (tpc->created != NULL)
      dhufree(tpc->created);
    if (tpc->modified != NULL)
      dhufree(tpc->modified);
    dhufree(tpc);
  }
}

void boardFreeMessage(board_message *msg) {
  if (msg != NULL) {
    if (msg->created != NULL)
      dhufree(msg->created);
    if (msg->modified != NULL)
      dhufree(msg->modified);
    if (msg->description != NULL)
      dhufree(msg->description);
    dhufree(msg);
  }
}

