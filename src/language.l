%{
#include <string.h>
#include "parser.h"
#define YY_DECL int yylex(void *param)
#define YYSTYPE ParseTree *
#define YY_INPUT(buf, retval, maxlen) (retval = getparserinput(buf, maxlen))
#include "language.tab.h"
#include "bison.h"

#define ADD_TOKEN(TYPE) \
	yylval = initParseTree((TYPE)); \
	appendLinkedList(&((ParseParam *) param)->cleanup, yylval);

int getparserinput(char *buf, int maxlen);
char *readstring(char quote, void *param);
int eatcomment(void *param);

char *glob_inputstr = NULL;

void setParserInput(char *source) {
	glob_inputstr = source;
}

void setLineNo(int lineno) {
	yylineno = lineno;
}

%}

%option yylineno

%%
for			ADD_TOKEN(FOR_KEYWORD_PARSE); return FOR_KEYWORD;
do			ADD_TOKEN(DO_KEYWORD_PARSE); return DO_KEYWORD;
while			ADD_TOKEN(WHILE_KEYWORD_PARSE); return WHILE_KEYWORD;
if			ADD_TOKEN(IF_KEYWORD_PARSE); return IF_KEYWORD;
else			ADD_TOKEN(ELSE_KEYWORD_PARSE); return ELSE_KEYWORD;
function		ADD_TOKEN(FUNCTION_KEYWORD_PARSE); return FUNCTION_KEYWORD;
return			ADD_TOKEN(RETURN_KEYWORD_PARSE); return RETURN_KEYWORD;
int			ADD_TOKEN(TYPE_PARSE); yylval->stringValue = strdup(yytext); return TYPE;
string			ADD_TOKEN(TYPE_PARSE); yylval->stringValue = strdup(yytext); return TYPE;
float			ADD_TOKEN(TYPE_PARSE); yylval->stringValue = strdup(yytext); return TYPE;
array			ADD_TOKEN(TYPE_PARSE); yylval->stringValue = strdup(yytext); return TYPE;
map			ADD_TOKEN(TYPE_PARSE); yylval->stringValue = strdup(yytext); return TYPE;
[0-9]+                  ADD_TOKEN(CONSTANT_INT_PARSE); yylval->stringValue = strdup(yytext); return INT; 
[0-9]+\.[0-9]*          ADD_TOKEN(CONSTANT_FLOAT_PARSE); yylval->stringValue = strdup(yytext); return FLOAT;
\"			ADD_TOKEN(CONSTANT_STRING_PARSE); yylval->stringValue = readstring('"', param); return STRING;
\'			ADD_TOKEN(CONSTANT_STRING_PARSE); yylval->stringValue = readstring('\'', param); return STRING;
\+=			ADD_TOKEN(ASSIGNMENT_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return ASSIGNMENT_OP;
-=			ADD_TOKEN(ASSIGNMENT_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return ASSIGNMENT_OP;
\*=			ADD_TOKEN(ASSIGNMENT_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return ASSIGNMENT_OP;
\/=			ADD_TOKEN(ASSIGNMENT_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return ASSIGNMENT_OP;
%=			ADD_TOKEN(ASSIGNMENT_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return ASSIGNMENT_OP;
\^=			ADD_TOKEN(ASSIGNMENT_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return ASSIGNMENT_OP;
=			ADD_TOKEN(ASSIGNMENT_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return ASSIGNMENT_OP;
[a-zA-Z_][a-zA-Z0-9_]*  ADD_TOKEN(VAR_PARSE); yylval->stringValue = strdup(yytext); return VAR;
;			ADD_TOKEN(SEMICOLON_PARSE); return SEMICOLON;
,			ADD_TOKEN(COMMA_PARSE); return COMMA;
--			ADD_TOKEN(UNI_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return UNI_OPERATOR;
\+\+			ADD_TOKEN(UNI_OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return UNI_OPERATOR;
\+			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return ADD;
-			ADD_TOKEN(NEGATIVE_PARSE); yylval->stringValue = strdup(yytext); return NEGATIVE;
!			ADD_TOKEN(NOT_PARSE); yylval->stringValue = strdup(yytext); return NOT;
\/			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return DIVIDE;
\*			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return MULTIPLY;
\^			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return POWER;
%			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return MOD;
==			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return EQUAL;
!=			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return NOTEQUAL;
\<			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return LESSTHAN;
>			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return GREATERTHAN;
\<=			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return GREATERTHANEQUAL;
>=			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return LESSTHANEQUAL;
\|\|			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return OR;
&&			ADD_TOKEN(OPERATOR_PARSE); yylval->stringValue = strdup(yytext); return AND;
\{			ADD_TOKEN(OPEN_SCOPE_PARSE); return OPEN_SCOPE;
\}			ADD_TOKEN(CLOSE_SCOPE_PARSE); return CLOSE_SCOPE;
\(			ADD_TOKEN(OPEN_PAREN_PARSE); return OPEN_PAREN;
\)			ADD_TOKEN(CLOSE_PAREN_PARSE); return CLOSE_PAREN;
\[			ADD_TOKEN(OPEN_ARRAY_INDEX_PARSE); return OPEN_ARRAY_INDEX;
\]			ADD_TOKEN(CLOSE_ARRAY_INDEX_PARSE); return CLOSE_ARRAY_INDEX;
\/\/.*$			
\/\*	 		{	eatcomment(param); }
[\n\t ]+			

%%

char *readstring(char quote, void *param) {
	char buf[2];
	char *i = NULL;
	char c = '\0';
	
	buf[1] = '\0';
	while (1) {
		while ( (c = input()) != quote && c != EOF && c != '\\') {
			buf[0] = c;
			vstrdupcat(&i, buf, NULL);
		}

		if (c == '\\') {
			c = input();
			if (c == 'n') {
				buf[0] = '\n';	
			} else if (c == 'r') {
				buf[0] = '\r';	
			} else if (c == 't') {
				buf[0] = '\t';	
			} else if (c == '\'') {
				buf[0] = '\'';	
			} else if (c == '\"') {
				buf[0] = '\"';	
			} else if (c == '\\') {
				buf[0] = '\\';	
			}
			vstrdupcat(&i, buf, NULL);
		} else if (c == EOF) {
			generateerror( param, "EOF in string" );
			break;
		} else {
			break;
		}
	}
	return i;
}

int eatcomment(void *param) {
	int c;

	while (1) {
		while ( (c = input()) != '*' && c != EOF )
		      ;	   /* eat up text of comment */

		if ( c == '*' ) {
		      while ( (c = input()) == '*' )
			  ; /* eat up * chars */

		      if ( c == '/' )
			  break;    /* found the end */
		      }

			if ( c == EOF )
				      {
				      generateerror( param, "EOF in comment" );
				      break;
				      }
				  }
	return 0;
}

int getparserinput(char *buf, int maxlen) {
	int n = strlen(glob_inputstr);
	if (maxlen < n) {
		n = maxlen;
	}
	memcpy(buf, glob_inputstr, n);
	glob_inputstr += n;
	return n;
}

int yywrap() {
	return 1;
}
