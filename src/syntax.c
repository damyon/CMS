#include "syntax.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

Syntax *initSyntax() {
	Syntax *s = (Syntax *) malloc(sizeof(Syntax));

	s->state = CODE_STATE;
	s->message[0] = '\0';
	s->lineno = 0;
	s->colno = 0;
	s->events = initStack();

	return s;
}

void freeSyntax(Syntax **syn) {
	Syntax *s = *syn;
	if (s != NULL) {
		if (s->events != NULL) {
			freeStack(&s->events);
		}
		free(s);
	}

	*syn = NULL;
}

SyntaxEvent * initSyntaxEvent(int lineno, int colno, char c) {
	SyntaxEvent *e = (SyntaxEvent *) malloc(sizeof(SyntaxEvent));

	e->lineno = lineno;
	e->colno = colno;
	e->c = c;
	return e;
}

char findClosing(char open) {

	switch (open) {
		case '(': return ')';
		case '{': return '}';
		case '[': return ']';
		default: return ' ';
	}
}

// run the syntax checker on the source (upto length chars)
// and return E_OK if no errors found. If errors are found, 
// the return will be non zero and a message will be in error.
int checkSyntax(char *source, int length, int lineno, int colno, char **error) {
	Syntax *s = initSyntax();
	SyntaxEvent *e = NULL;
	int err = E_OK, i = 0;
	char *input = source;
	s->lineno = lineno;
	s->colno = colno;

	while ((*input != '\0') && ((input - source) < length) && (err == E_OK)) {
		switch (s->state) {
			case CODE_STATE:
				// check for comments
				if (strncmp(input, "//", 2) == 0) {
					s->state = COMMENT_STATE;
					s->singlelinecomment = 1;
				} else if (strncmp(input, "/*", 2) == 0) {
					s->state = COMMENT_STATE;
					s->singlelinecomment = 0;
				// check for strings
				} else if (*input == '"') {
					s->state = STRING_STATE;
					s->quote = *input;
					e = initSyntaxEvent(s->lineno, s->colno, *input);
					pushStack(s->events, e);
				} else if (*input == '\'') {
					s->state = STRING_STATE;
					s->quote = *input;
					e = initSyntaxEvent(s->lineno, s->colno, *input);
					pushStack(s->events, e);
				// check for open brackets
				} else if (*input == '(' || *input == '{' || *input == '[') {
					e = initSyntaxEvent(s->lineno, s->colno, *input);
					pushStack(s->events, e);
				// check for close brackets
				} else if (*input == ')' || *input == '}' || *input == ']') {
					e = (SyntaxEvent *) popStack(s->events);
					if (e == NULL) {
						// found an error
						err = SYNTAXERROR;
						sprintf(s->message, "Mismatched bracket '%c' found at: line %d, col %d. Closing bracket with no opening bracket.", *input, s->lineno + 1, s->colno + 1);

					} else if (findClosing(e->c) != *input) {
						// found an error
						err = SYNTAXERROR;
						sprintf(s->message, "Mismatched bracket '%c' found at: line %d, col %d. Previous bracket was '%c' at line %d, col %d", *input, s->lineno + 1, s->colno + 1, e->c, e->lineno + 1, e->colno + 1);
						free(e);
					}
				}
				break;
			case STRING_STATE:
				if (*input == s->quote) {
					i = 0;
					while (*(input - i - 1) == '\\') {
						i++;
					}
					if ((i % 2) == 0) {
						// this is the end of the string
						e = (SyntaxEvent *) popStack(s->events);
						free(e);
						s->state = CODE_STATE;
					}
				}
				break;
			case COMMENT_STATE:
				if (s->singlelinecomment) {
					if (*input == '\n') {
						s->state = CODE_STATE;
					}
				} else {
					if (strncmp(input, "*/", 2) == 0) {
						s->state = CODE_STATE;
					}
				}
				break;
		}
		s->colno++;
		if (*input == '\n') {
			s->colno = 0;
			s->lineno++;
		}
		input++;
	}

	if (err == E_OK) {
		e = (SyntaxEvent *) popStack(s->events);
		if (e != NULL) {
			err = SYNTAXERROR;
			sprintf(s->message, "Unbalanced input character %c found at: line %d, col %d. The source of this error may be earlier in this file.", e->c, e->lineno + 1, e->colno + 1);

			free(e);
		}
	}

	if (err != E_OK) {
		*error = strdup(s->message);
	}
	freeSyntax(&s);
	return err;
}

// get the current line no from the start of the string up to current
int getLineNo(char *source, char *current) {
	int lineno = 0;

	while (source != current && *source != '\0') {
		if (*source == '\n') {
			lineno++;
		}
		source++;
	}

	return lineno;
}

// run the syntax checker but determine the current line no and col no
// before we start based on the pointer to the start of the input.
int checkSyntaxGetPosition(char *source, int length, char *filestart, char **errmsg) {
	int lineno = 0, colno = 0, err = 0;
	char *error = NULL;

	while (filestart != source && *filestart != '\0') {
		colno++;
		if (*filestart == '\n') {
			colno = 0;
			lineno++;
		}
		filestart++;
	}

	err = checkSyntax(source, length, lineno, colno, &error); 
	*errmsg = error;

	return err;
}
