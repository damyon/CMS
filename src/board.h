/************************************************************************
* board.h
*
* User Management functions.
************************************************************************/
#ifndef _BOARD_H
#define _BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _board_instance_ {
  int boardid;
  char *objectPath;
} board_instance;

typedef struct _board_topic_ {
  int topicid, boardid, authorid, locked, sticky, views;
  char *summary, *description;
  struct tm *created, *modified;
} board_topic;

typedef struct _board_message_ {
  int messageid, topicid, authorid;
  char *description;
  struct tm *created, *modified;
} board_message;

board_instance * boardInitInstance();
board_topic * boardInitTopic();
board_message * boardInitMessage();

void boardFreeInstance(board_instance *inst);
void boardFreeTopic(board_topic *evt);
void boardFreeMessage(board_message *occ);

#ifdef __cplusplus
}
#endif


#endif // _BOARD_H
