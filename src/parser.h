#ifndef _PARSER_H
#define _PARSER_H

#include "structs.h"
#include "env.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCRIPTSTARTTAG "<\\?cms"
#define SCRIPTENDTAG "?>"

typedef enum _ParseEnum_ {
	BEGIN_LEAF_TYPES, // Leaf Nodes
	FUNCTION_KEYWORD_PARSE,
	RETURN_KEYWORD_PARSE,
	BEGIN_CONTROL_TYPES,
	IF_KEYWORD_PARSE,
	ELSE_KEYWORD_PARSE,
	WHILE_KEYWORD_PARSE,
	DO_KEYWORD_PARSE,
	FOR_KEYWORD_PARSE,
	END_CONTROL_TYPES,
	SEMICOLON_PARSE,
	TYPE_PARSE,
	CONSTANT_STRING_PARSE,
	CONSTANT_INT_PARSE,
	CONSTANT_FLOAT_PARSE,
	VAR_PARSE,
	ASSIGNMENT_OPERATOR_PARSE,
	UNI_OPERATOR_PARSE,
	COMMA_PARSE,
	OPERATOR_PARSE,
	OPEN_ARRAY_INDEX_PARSE,
	CLOSE_ARRAY_INDEX_PARSE,
	OPEN_PAREN_PARSE,
	CLOSE_PAREN_PARSE,
	OPEN_SCOPE_PARSE,
	CLOSE_SCOPE_PARSE,
	END_LEAF_TYPES, // End Leaf Nodes
	BEGIN_BRANCH_TYPES, // Branch Nodes
	VARIABLE_PARSE,
	PARAM_LIST_PARSE,
	PARAM_LIST_ELE_PARSE,
	FUNCTION_DECL_PARSE,
	EXP_LIST_PARSE,
	FUNCTION_CALL_PARSE,
	BLOCK_PARSE,
	NEGATIVE_PARSE,
	NOT_PARSE,
	VAR_LIST_PARSE,
	VAR_LIST_ELE_PARSE,
	VAR_LIST_END_PARSE,
	INPUT_PARSE,
	STATEMENT_PARSE,
	EXP_PARSE,
	TYPE_DECL_PARSE,
	END_BRANCH_TYPES // End Branch Nodes
} ParseEnum;


typedef enum _SymbolEnum_ {
	FUNCTION_SYMBOL,
	VARIABLE_SYMBOL
} SymbolEnum;

typedef enum _VariableEnum_ {
	INT_VARIABLE,
	STRING_VARIABLE,
	FLOAT_VARIABLE,
	ARRAY_VARIABLE,
	MAP_VARIABLE,
	UNINITIALISED_VARIABLE
} VariableEnum;

typedef struct _LinkedList_ {
	void *data;
	struct _LinkedList_ *next;
} LinkedList;

typedef struct _ParserInfo_ {
	char errormessage[256];
	int errorcode;
	int instructionCount;
	int maxInstructions;
	char *output;
	void *sqlsock;
	Env *env;
	struct _SymbolTable_ *symbols;
} ParserInfo;

struct _Symbol_;

typedef struct _FunctionType_ {
	char *functionName;
	struct _ParseTree_ *ptree;
	int (*nativeCode)(LinkedList *args, ParserInfo *info, struct _Symbol_ *result);
	LinkedList *nativeArgs;
} FunctionType;

typedef struct _VariableType_ {
	int variableID;
	char *variableName;
	int intValue;
	float floatValue;
	char *stringValue;
	Map *arrayValues;
	Map *mapValues;
	VariableEnum variableEnum;
} VariableType;

typedef struct _Symbol_ {
	SymbolEnum symbolEnum;
	FunctionType *functionType;
	VariableType *variableType;
} Symbol;


typedef struct _SymbolTable_ {
	struct _SymbolTable_ *childSymbolTable, *parentSymbolTable;
	Map *symbols;
} SymbolTable;

typedef struct _ParseTree_ {
	ParseEnum parseEnum;
	Symbol *symbol;
	char *stringValue;
	struct _ParseTree_ *parent, *firstChild, *leftSibling, *rightSibling, *lastChild;
} ParseTree;



void appendLinkedList(LinkedList **list, void *data);

void freeLinkedList(LinkedList *list, void (*freeFunc)(void *));

SymbolTable * initSymbolTable();
void freeSymbolTable();

void putSymbol(SymbolTable *st, Symbol *s);

Symbol *getSymbol(SymbolTable *st, char *name);

void printSymbol(Symbol *s);

void printDebug();

Symbol *initConstantFloatSymbol(float value);
Symbol *initConstantFloatSymbolString(char * value);
Symbol *initConstantIntSymbol(int value);
Symbol *initConstantIntSymbolString(char * value);
Symbol *initConstantStringSymbol(char *value);

ParseTree * initParseTree(ParseEnum parseEnum);
int interpretParseTree(ParseTree *tree, ParserInfo *info);
void addChildParseTree(ParseTree *parent, ParseTree *child);
void printParseTree(ParseTree *tree);
void freeParseTree(ParseTree *tree);
void freeParseTreeNodeFunc(void *a);
char *trimquotes(char *str);
void vstrdupcat(char **str, ...);
void putSymbol(SymbolTable *st, Symbol *s);
Symbol * getSymbol(SymbolTable *st, char *name);
Symbol *initVariableStringSymbol(char *name, char *value);
Symbol *initVariableFloatSymbol(char *name, float value);
Symbol *initVariableIntSymbol(char *name, int value);
Symbol *initVariableArraySymbol(char *name);
Symbol *initVariableMapSymbol(char *name);
void printSymbolTable(SymbolTable *s);
void registerNativeFunction(SymbolTable *s, char *name, int (*nativeCode)(LinkedList *args, ParserInfo *info, Symbol *result), LinkedList *args);
void freeSymbol(void *a);
int compareSymbols(void *a, void *b);
int parseScript(char **filecontents, int *filelen, int objectid, ParserInfo *info);
char *castSymbolToString(Symbol *s);
int castSymbolToInt(Symbol *s);
float castSymbolToFloat(Symbol *s);
Symbol *getSymbolArrayValue(Map *arrayValues, int index);
Symbol *getSymbolMapValue(Map *mapValues, char *index);
void addRightSiblingParseTree(ParseTree *left, ParseTree *right);


#ifdef __cplusplus
}
#endif

#endif // _PARSER_H

