#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "malloc.h"
#include "structs.h"
#include "errors.h"

/* Syntax checker structure. This manages the state machine for 
 * finding syntax errors in the input. The errors we are searching
 * for are mismatched quotes, brackets and comments.
 */

	// the states for the state machine
	typedef enum { CODE_STATE, COMMENT_STATE, STRING_STATE } SyntaxEnum;

	// found an syntax event (eg open bracket) - these are stored in the events stack.
	typedef struct _SyntaxEvent_ {
		int lineno, colno;
		char c;
	} SyntaxEvent;

	// the master state for the syntax checker
	typedef struct _Syntax_ {
		SyntaxEnum state;
		Stack *events;
		char quote;
		int singlelinecomment;
		int lineno, colno;
		char message[256];
	} Syntax;

	// run the syntax checker on the source (upto length chars)
	// and return E_OK if no errors found. If errors are found, 
	// the return will be non zero and a message will be in error.
	int checkSyntax(char *source, int length, int lineno, int colno, char **error);

	int checkSyntaxGetPosition(char *source, int length, char *filestart, char **error);
	
	int getLineNo(char *source, char *current);
#ifdef __cplusplus
}
#endif

#endif
