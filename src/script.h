
/************************************************************************
* script.h
*
* This is the script interpreter.
************************************************************************/
#ifndef _SCRIPT_H
#define _SCRIPT_H

#include "env.h"
#include "structs.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ParseTree_ {
  struct _Token_ *token;
  struct _ParseTree_ *rightsibling, *leftchild;
  int skip, node, leftc, rights;
} ParseTree;

typedef struct _Token_ {
  char *token;
  int type, line, valtype;
  struct _Token_ *next;
  struct _ParseTree_ *ptree;
} Token;

typedef struct _Argument_ {
  char *value, *name;
  int type;
  struct _Argument_ *next;
} Argument;

typedef struct _ArgumentList_ {
  struct _Argument_ *head, *tail;
  int size;
} ArgumentList;

typedef struct _Function_ {
  char *name;
  struct _ParseTree_ *ptree;
  struct _ArgumentList_ *args;
} Function;

typedef struct _TokenList_ {
  struct _Token_ *head, *tail;
} TokenList;

typedef struct _SymbolTable_ {
  struct _map_ *symbols;
  struct _map_ *functions;
  struct _Token_ *returnValue;
  int scope;
} SymbolTable;

typedef struct _Symbol_ {
  char **values;
  int *types, scope, length;
} Symbol;

typedef struct _SymbolList_ {
  char *name;
  struct _stack_ *symbols;
} SymbolList;

enum SymbolType { INT_VARIABLE = 1,
                  STRING_VARIABLE,
                  ARRAY_VARIABLE
                };


/* ALL POSSIBLE TOKEN TYPES */
enum TokenType { START = 1,                     // 1
                 IDENTIFIER,                    // 8
                 LEFT_SQUARE_PARENTHESIS,
                 RIGHT_SQUARE_PARENTHESIS,      // 10
                 STRING_TYPE,
                 INT_TYPE,                           // 12
                 EQUALS_OPERATOR,
                 MULTIPLY_EQUALS_OPERATOR,
                 DIVIDE_EQUALS_OPERATOR,        // 15
                 MOD_EQUALS_OPERATOR,
                 ADD_EQUALS_OPERATOR,
                 SUBTRACT_EQUALS_OPERATOR,
                 ADD_OPERATOR,
                 SUBTRACT_OPERATOR,             // 20
                 DIVIDE_OPERATOR,
                 MULTIPLY_OPERATOR,
                 MOD_OPERATOR,
                 STRING_KEYWORD,
                 INT_KEYWORD,                   // 25
                 SEMICOLON_OPERATOR,
                 LEFT_PARENTHESIS,
                 RIGHT_PARENTHESIS,
                 COMMA_SEPARATOR,               // 30
                 IF_KEYWORD,
                 ELSE_KEYWORD,
                 EQUALITY_OPERATOR,
                 INEQUALITY_OPERATOR,
                 LESSTHAN_OPERATOR,
                 LESSTHANEQUAL_OPERATOR,        // 40
                 GREATERTHAN_OPERATOR,
                 GREATERTHANEQUAL_OPERATOR,     // 44
                 LEFT_SCOPE,
                 RIGHT_SCOPE,                   
                 WHILE_KEYWORD,
                 DO_KEYWORD,
                 BOOLEANOR_OPERATOR,
                 BOOLEANAND_OPERATOR,
                 COMMENT,
                 ARRAY_KEYWORD,
                 FOR_KEYWORD,
                 ARRAY,
                 BOF_TOKEN,
                 EOF_TOKEN,				
                 TYPE,
                 EXPRESSION,
                 ASSIGNMENT_OPERATOR,
                 CONSTANT,
                 ARRAY_IDENTIFIER,
                 ARITHMETIC_OPERATOR,
                 EXPRESSION_LIST,
                 COMPOUND_STATEMENT,
                 VARIABLE,
                 BEGIN_IF,
                 BEGIN_FOR,
                 BEGIN_WHILE,
                 DECL,
                 DECL_LIST,
                 FUNCTION_KEYWORD,
                 RETURN_KEYWORD,
                 BEGIN_FUNCTION,
                 VOID_KEYWORD
               };

/* FUNCTION IDS */
enum FunctionID { WRITE_FUNCTION = 0,
                  WRITELN_FUNCTION,
                  GET_FUNCTION,
                  ISVALIDFILENAME_FUNCTION,
                  SET_FUNCTION,
                  LOGIN_FUNCTION,
                  LOADOBJECT_FUNCTION,
                  INCLUDE_FUNCTION,
                  GETERRORMESSAGE_FUNCTION,
                  LOADFOLDERCONTENTS_FUNCTION,
                  LENGTH_FUNCTION,
                  CLEARARRAY_FUNCTION,
                  LOADOBJECTDETAILS_FUNCTION,
                  SPLIT_FUNCTION,
                  JOIN_FUNCTION,
                  LOADROOTFOLDERCONTENTS_FUNCTION,
                  LOADOBJECTID_FUNCTION,
                  EDITOBJECTDETAILS_FUNCTION,
                  FILEEXISTS_FUNCTION,
                  REPLACEFILECONTENTS_FUNCTION,
                  CREATENEWOBJECT_FUNCTION,
                  DELETEOBJECT_FUNCTION,
                  SEARCHCONTENT_FUNCTION,
                  LOGOUT_FUNCTION,
                  CREATENEWUSER_FUNCTION,
                  LOADUSERLIST_FUNCTION,
                  LOADUSERDETAILS_FUNCTION,
                  DELETEUSER_FUNCTION,
                  EDITUSERDETAILS_FUNCTION,
                  CREATENEWGROUP_FUNCTION,
                  LOADGROUPLIST_FUNCTION,
                  LOADGROUPDETAILS_FUNCTION,
                  DELETEGROUP_FUNCTION,
                  EDITGROUPDETAILS_FUNCTION,
                  LOADGROUPMEMBERS_FUNCTION,
                  ADDGROUPMEMBER_FUNCTION,
                  REMOVEGROUPMEMBER_FUNCTION,
                  LOADPERMISSIONLIST_FUNCTION,
                  LOADPERMISSIONBITS_FUNCTION,
                  ADDPERMISSION_FUNCTION,
                  REMOVEPERMISSION_FUNCTION,
                  ESCAPE_FUNCTION,
                  UNESCAPE_FUNCTION,
                  GETVERSION_FUNCTION,
                  SETOBJECTMETADATA_FUNCTION,
                  GETOBJECTMETADATA_FUNCTION,
                  GETALLOBJECTMETADATA_FUNCTION,
                  REMOVEOBJECTMETADATA_FUNCTION,
                  GETISODATE_FUNCTION,
                  XMLESCAPE_FUNCTION,
                  SETSESSIONDATA_FUNCTION,
                  GETSESSIONDATA_FUNCTION,
                  GETALLSESSIONDATA_FUNCTION,
                  REMOVESESSIONDATA_FUNCTION,
                  LOADUSERSGROUPS_FUNCTION,
                  LOCKOBJECT_FUNCTION,
                  UNLOCKOBJECT_FUNCTION,
                  LOADWORKFLOWSETTINGS_FUNCTION,
                  ATTACHWORKFLOWSETTINGS_FUNCTION,
                  LOADWORKFLOWLIST_FUNCTION,
                  APPROVEOBJECT_FUNCTION,
                  MOVEOBJECT_FUNCTION,
                  REMOVEWORKFLOWSETTINGS_FUNCTION,
                  REPLACETEXTCONTENTS_FUNCTION,
                  CREATENEWTEXTOBJECT_FUNCTION,
                  SENDEMAIL_FUNCTION,
                  ISVERIFIER_FUNCTION,
                  EXPORTPACKAGE_FUNCTION,
                  IMPORTPACKAGE_FUNCTION,
                  LOADOBJECTVERSIONS_FUNCTION,
                  ROLLBACKOBJECTVERSION_FUNCTION,
                  CREATENEWFOLDER_FUNCTION,
                  RAND_FUNCTION,
                  URLREWRITE_FUNCTION,
                  URLBASE_FUNCTION,
                  CALCREATEINSTANCE_FUNCTION,
                  CALCREATEEVENT_FUNCTION,
                  CALCREATEOCCURRENCE_FUNCTION,
                  CALDELETEINSTANCE_FUNCTION,
                  CALDELETEEVENT_FUNCTION,
                  CALDELETEOCCURRENCE_FUNCTION,
                  CALMOVEINSTANCE_FUNCTION,
                  CALEDITOCCURRENCE_FUNCTION,
                  LOADCALINSTANCEDETAILS_FUNCTION,
                  LOADCALEVENTDETAILS_FUNCTION,
                  LOADCALOCCURRENCEDETAILS_FUNCTION,
                  SEARCHCALBYEVENT_FUNCTION,
                  SEARCHCALBYDATETIME_FUNCTION,
                  COMPAREDATE_FUNCTION,
                  COMPARETIME_FUNCTION,
                  COMPAREDATETIME_FUNCTION,
                  ADDDATE_FUNCTION,
                  ADDTIME_FUNCTION,
                  ADDDATETIME_FUNCTION,
                  CURRENTDATE_FUNCTION,
                  CURRENTTIME_FUNCTION,
                  CURRENTDATETIME_FUNCTION,
                  GETMONTHNAME_FUNCTION,
                  GETMONTHNAMESHORT_FUNCTION,
                  GETDAYNAME_FUNCTION,
                  GETDAYNAMESHORT_FUNCTION,
                  SUBTRACTDATE_FUNCTION,
                  SUBTRACTTIME_FUNCTION,
                  SUBTRACTDATETIME_FUNCTION,
                  VALIDDATE_FUNCTION,
                  VALIDDATETIME_FUNCTION,
                  BOARDCREATEINSTANCE_FUNCTION,
                  BOARDDELETEINSTANCE_FUNCTION,
                  BOARDMOVEINSTANCE_FUNCTION,
                  BOARDCREATETOPIC_FUNCTION,
                  BOARDDELETETOPIC_FUNCTION,
                  BOARDCREATEMESSAGE_FUNCTION,
                  BOARDEDITMESSAGE_FUNCTION,
                  BOARDEDITTOPIC_FUNCTION,
                  BOARDDELETEMESSAGE_FUNCTION,
                  BOARDSEARCH_FUNCTION,
                  BOARDSEARCHTOPIC_FUNCTION,
                  BOARDLISTTOPICS_FUNCTION,
                  LOADBOARDINSTANCEDETAILS_FUNCTION,
                  LOADBOARDTOPICDETAILS_FUNCTION,
                  LOADBOARDMESSAGEDETAILS_FUNCTION,
                  BOARDINCREMENTVIEWS_FUNCTION,
                  LOADCALINSTANCEPATH_FUNCTION,
                  COPYOBJECT_FUNCTION,
                  CAPITALISE_FUNCTION,
                  SETUSERMETADATA_FUNCTION,
                  GETUSERMETADATA_FUNCTION,
                  GETALLUSERMETADATA_FUNCTION,
                  REMOVEUSERMETADATA_FUNCTION,
                  CSVESCAPE_FUNCTION,
                  LOADDELETEDFOLDERCONTENTS_FUNCTION,
                  LOADDELETEDROOTFOLDERCONTENTS_FUNCTION,
                  RECOVEROBJECTVERSION_FUNCTION,
                  LOADVERIFIERCOMMENT_FUNCTION,
                  LOADALLVERIFIERCOMMENTS_FUNCTION,
                  ADDVERIFIERCOMMENT_FUNCTION,
                  MAX_FUNCTIONS
               };

/*******************************************************************************
* runScript...
*
* Execute this block of Script.
*******************************************************************************/
int runScript(char *script, char **output, int section, int objectid, Env *env, void *sqlsock, SymbolTable *symbols);

/********************************************************************
* initArgument...
*
* Malloc some memory for this argument.
*********************************************************************/
Argument *initArgument(char *value, char *name, int type);

/********************************************************************
* initArgumentList...
*
* Malloc some memory for this argument list.
*********************************************************************/
ArgumentList *initArgumentList(void);

/********************************************************************
* freeArgumentList...
*
* Free this entire argument list.
*********************************************************************/
void freeArgumentList(ArgumentList *list);

/********************************************************************
* addArgumentToList...
*
* Adds this argument to the argument list.
********************************************************************/
int addArgumentToList(ArgumentList *list, char *value, char *name, int type);

/*********************************************************************
* interpretParseTree... 
* 
* Take a fully formed parse tree and actually run it.
*********************************************************************/
int interpretParseTree(ParseTree *current, SymbolTable *symbols, char **output, int leftmostchild, int *line, Env *env, void *sqlsock);

/*********************************************************************
* interpretParseTreeNode... 
* 
* Take a node from a fully formed parse tree and actually run it.
*********************************************************************/
int interpretParseTreeNode(ParseTree *current, SymbolTable *symbols, char **output, int leftmostchild, int *line, Env *env, void *sqlsock);

/*********************************************************************
* setSymbol...                   
*                                
* Sets a symbol in the symbol table.
*********************************************************************/
int setSymbol(char *name, char *value, int type, SymbolTable *symbols);

/*********************************************************************
* forceSymbol...
*
* Force a symbol in the symbol table.
*********************************************************************/
int forceSymbol(char *name, char *value, int type, SymbolTable *symbols);

/*********************************************************************
* getSymbol...
*
* Gets a symbol from the symbol table.
*********************************************************************/
int getSymbol(char *name, char **value, int *type, SymbolTable *symbols);

/*********************************************************************
* getFunctionID...               
*                                
* Resolve this function name to an id
*********************************************************************/
int getFunctionID(char *funcname);
                                 
/*********************************************************************
* getFunctionNumArgs...          
*                                
* How many arguments are allowed for this function?
*********************************************************************/
int getFunctionNumArgs(int functionid);

/*********************************************************************
* initSymbolTable...             
*                                
* init this struct.              
*********************************************************************/
SymbolTable *initSymbolTable(void);

/*********************************************************************
* freeSymbolTable...             
*                                
* Free this struct.              
*********************************************************************/
void freeSymbolTable(SymbolTable *symbols);

/*********************************************************************
* freeSymbol...
*
* Free this struct.
*********************************************************************/
void freeSymbol(void *symbol);

/*********************************************************************
* lexer...                       
*                                
* Parses the input into tokens.  
*********************************************************************/
int lexer(char *script, char **output, TokenList **tokenlist);

/*********************************************************************
* printTokenList...
*
* Print this token list.
*********************************************************************/
void printTokenList(TokenList *tokens);

/*********************************************************************
* buildParseTree...              
*                                
* Stage 2 - build a valid parse tree to execute the script.
*********************************************************************/
int buildParseTree(TokenList *tokens, char **output, ParseTree **ptree);

/*********************************************************************
* freeTokenList...               
*                                
* Free this token list.          
*********************************************************************/
void freeTokenList(TokenList *tokens);

/*********************************************************************
* freeParseTree...               
*                                
* free the contents of the parse tree.
*********************************************************************/
void freeParseTree(ParseTree *ptree);

/*********************************************************************
* printParseTree...               
*                                
* print the contents of the parse tree.
*********************************************************************/
void printParseTree(ParseTree *ptree, int indent);

/*********************************************************************
* copyParseTree...               
*                                
* copy the contents of the parse tree.
*********************************************************************/
ParseTree * copyParseTree(ParseTree *ptree);

/*********************************************************************
* writeParseTree...              
*                                
* Write the parse tree for this section to disk.
*********************************************************************/
int writeParseTree(int section, int objectid, ParseTree *ptree);

/*********************************************************************
* clearArray...
*
* Resets an array in the symbol table to have a length of 0.
*********************************************************************/
int clearArray(char *name, SymbolTable *symbols);

#ifdef __cplusplus
}
#endif


#endif // _SCRIPT_H
