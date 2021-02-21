/* Advanced language parser */

%{
#include <stddef.h>
#include "parser.h"
#define YYSTYPE ParseTree *
#define YYPARSE_PARAM param
#define YYLEX_PARAM param
#include "bison.h"  /* Contains definition of `parser types'        */


void yyerror (void *param, char const *msg);
extern int yylineno;
%}

%expect 2
%error-verbose

%param { void *param }
%token FLOAT           /* Simple double precision number   */
%token INT           /* Simple double precision number   */
%token STRING        /* Any constant string   */
%left ASSIGNMENT_OP /* = += -= *= ^= /= */
%token VAR           /* a b c */
%token TYPE          /* int string float array map */
%token SEMICOLON     /* ; */
%token COMMA         /* , */
%left ADD SUBTRACT       /* + */
%left MULTIPLY DIVIDE    /* * */
%left POWER          /* ^ */
%left MOD            /* % */
%left NEGATIVE       /* - */
%left NOT 	     /* ! */
%token UNI_OPERATOR  /* ++ -- */
%token OPEN_SCOPE  /* { */
%token CLOSE_SCOPE  /* } */
%token OPEN_PAREN  /* ( */
%token CLOSE_PAREN  /* ) */
%token OPEN_ARRAY_INDEX  /* [ */
%token CLOSE_ARRAY_INDEX  /* ] */
%token FUNCTION_KEYWORD  /* function */
%token RETURN_KEYWORD  /* return */
%token IF_KEYWORD  /* if */
%token ELSE_KEYWORD  /* else */
%token WHILE_KEYWORD  /* while */
%token DO_KEYWORD  /* do */
%token FOR_KEYWORD  /* for */
%left EQUAL  /* == */
%left NOTEQUAL  /* != */
%left GREATERTHAN  /* > */
%left LESSTHAN  /* < */
%left GREATERTHANEQUAL  /* >= */
%left LESSTHANEQUAL  /* <= */
%left AND  /* && */
%left OR  /* || */

/* Grammar follows */

%%
input:  /* empty */ { 
		$$ = initParseTree(INPUT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		((ParseParam *) param)->ptree = $$;
		} 
        | input statement { 
		$$ = initParseTree(INPUT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		((ParseParam *) param)->ptree = $$;
		}
        | input block { 
		$$ = initParseTree(INPUT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		((ParseParam *) param)->ptree = $$;
		}
	| input error { ((ParseParam *) param)->lineno = yylineno; }
;

block: OPEN_SCOPE input CLOSE_SCOPE {
		$$ = initParseTree(BLOCK_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
;

statement: exp SEMICOLON	{ 
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		}
	| RETURN_KEYWORD exp SEMICOLON {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| TYPE var_list SEMICOLON {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		}
	| function_decl {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		}
	| IF_KEYWORD OPEN_PAREN exp CLOSE_PAREN statement {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		}
	| IF_KEYWORD OPEN_PAREN exp CLOSE_PAREN statement ELSE_KEYWORD statement {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		addChildParseTree($$, $7); 
		}
	| IF_KEYWORD OPEN_PAREN exp CLOSE_PAREN block {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		}
	| IF_KEYWORD OPEN_PAREN exp CLOSE_PAREN block ELSE_KEYWORD block {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		addChildParseTree($$, $7); 
		}
	| IF_KEYWORD OPEN_PAREN exp CLOSE_PAREN statement ELSE_KEYWORD block {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		addChildParseTree($$, $7); 
		}
	| IF_KEYWORD OPEN_PAREN exp CLOSE_PAREN block ELSE_KEYWORD statement {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		addChildParseTree($$, $7); 
		}
	| FOR_KEYWORD OPEN_PAREN exp SEMICOLON exp SEMICOLON exp CLOSE_PAREN block {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		addChildParseTree($$, $7); 
		addChildParseTree($$, $8); 
		addChildParseTree($$, $9); 
		}
	| FOR_KEYWORD OPEN_PAREN exp SEMICOLON exp SEMICOLON exp CLOSE_PAREN statement {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		addChildParseTree($$, $7); 
		addChildParseTree($$, $8); 
		addChildParseTree($$, $9); 
		}
	| DO_KEYWORD block WHILE_KEYWORD OPEN_PAREN exp CLOSE_PAREN SEMICOLON {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		addChildParseTree($$, $7); 
		}
	| DO_KEYWORD statement WHILE_KEYWORD OPEN_PAREN exp CLOSE_PAREN SEMICOLON {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		addChildParseTree($$, $7); 
		}
	| WHILE_KEYWORD OPEN_PAREN exp CLOSE_PAREN block {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		}
	| WHILE_KEYWORD OPEN_PAREN exp CLOSE_PAREN statement {
		$$ = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		}
;

var_list:  var_list_ele {
		$$ = initParseTree(VAR_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		}
	| var_list COMMA var_list_ele {
		$$ = initParseTree(VAR_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
;

var_list_ele: VAR {
		$$ = initParseTree(VAR_LIST_ELE_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		}
	| VAR ASSIGNMENT_OP exp {
		$$ = initParseTree(VAR_LIST_ELE_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
;


function_decl: FUNCTION_KEYWORD VAR OPEN_PAREN param_list CLOSE_PAREN block {
		$$ = initParseTree(FUNCTION_DECL_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		addChildParseTree($$, $5); 
		addChildParseTree($$, $6); 
		}
;

param_list: /* empty */ {
		$$ = initParseTree(PARAM_LIST_PARSE);
		}
	| param_list_ele {
		$$ = initParseTree(PARAM_LIST_PARSE);
		addChildParseTree($$, $1); 
		}
	| param_list COMMA param_list_ele {
		$$ = initParseTree(PARAM_LIST_PARSE); 
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
;

param_list_ele: TYPE VAR {
		$$ = initParseTree(PARAM_LIST_ELE_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1);
		addChildParseTree($$, $2);
		}
;

function_call: VAR OPEN_PAREN exp_list CLOSE_PAREN {
		$$ = initParseTree(FUNCTION_CALL_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		}
	| VAR OPEN_PAREN exp CLOSE_PAREN {
		$$ = initParseTree(FUNCTION_CALL_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		addChildParseTree($$, $4); 
		}
;

exp_list: /* empty */ {
		$$ = initParseTree(EXP_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		}
	| exp COMMA exp {
		$$ = initParseTree(EXP_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp_list COMMA exp {
		$$ = initParseTree(EXP_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
;

variable: VAR {
		$$ = initParseTree(VARIABLE_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1);
		}
	| variable OPEN_ARRAY_INDEX exp CLOSE_ARRAY_INDEX {
		$$ = initParseTree(VARIABLE_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1);
		addChildParseTree($$, $2);
		addChildParseTree($$, $3);
		addChildParseTree($$, $4);
		}
;

exp:    STRING {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		}  
	| FLOAT                { 
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		}
	| variable {
		$$ = initParseTree(EXP_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1);
		}
	| INT			{ 
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		}
	| variable ASSIGNMENT_OP exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp EQUAL exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp NOTEQUAL exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp LESSTHAN exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp LESSTHANEQUAL exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp GREATERTHAN exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp GREATERTHANEQUAL exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp ADD exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| NEGATIVE exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		}
	| NOT exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		}
	| exp NEGATIVE exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp DIVIDE exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp MULTIPLY exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp POWER exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp MOD exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp AND exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| exp OR exp {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
	| UNI_OPERATOR variable {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		}
	| variable UNI_OPERATOR {
		$$ = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		}
	| function_call {
		$$ = initParseTree(EXP_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		}
	| OPEN_PAREN exp CLOSE_PAREN {
		$$ = initParseTree(EXP_PARSE);	
		appendLinkedList(&((ParseParam *) param)->cleanup, $$);
		addChildParseTree($$, $1); 
		addChildParseTree($$, $2); 
		addChildParseTree($$, $3); 
		}
;

/* End of grammar */
%%


#include <stdio.h>

void generateerror (void *param, char const *msg) {
  snprintf (((ParseParam *) param)->errormessage, 255, "%s, Line: %d\n", msg, yylineno);
}

void yyerror (void *param, char const *msg) {
  generateerror(param, msg);
}
