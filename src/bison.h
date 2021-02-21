#ifndef _BISON_H
#define _BISON_H

#include "parser.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ParseParam_ {
	ParseTree *ptree;
	char errormessage[255];
	LinkedList *cleanup;
	int lineno;
} ParseParam;

void setParserInput(char *input);
void setLineNo(int lineno);
int getparserinput(char *buf, int maxlen);
int yyparse (void *param);
int yylex(void *param);
void yyrestart( FILE *input_file );
void generateerror(void *param, const char *message);

#ifdef __cplusplus
}
#endif

#endif // _BISON_H


