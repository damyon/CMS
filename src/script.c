#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include "win32.h"
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif
#include <ctype.h>
#include <fcntl.h>
#include "config.h"
#include "strings.h"
#include "errors.h"
#include "script.h"
#include "structs.h"
#include "package.h"
#include "objects.h"
#include "dbcalls.h"
#include "rules.h"
#include "api.h"
#include "logging.h"
#include "file.h"
#include "malloc.h"
#include "ipc.h"

#define SCRIPT_FILE_HEADER (0 << 8)

const char *FUNCTION_NAMES[] = {
                                 "write",                 // 0
                                 "writeln",               // 1
                                 "get",
                                 "isValidFilename",
                                 "set",                   // 3
                                 "login",
                                 "loadObject",            // 5
                                 "include",               // 5
                                 "getErrorMessage",
                                 "loadFolderContents",
                                 "length",
                                 "clearArray",
                                 "loadObjectDetails",     // 9
                                 "split",
                                 "join",
                                 "loadRootFolderContents",
                                 "loadObjectID",
                                 "editObjectDetails",     // 13
                                 "fileExists",
                                 "replaceFileContents",
                                 "createNewObject",       // 16
                                 "deleteObject",
                                 "searchContent",
                                 "logout",                // 19
                                 "createNewUser",
                                 "loadUserList",
                                 "loadUserDetails",       // 22
                                 "deleteUser",
                                 "editUserDetails",
                                 "createNewGroup",        // 25
                                 "loadGroupList",
                                 "loadGroupDetails",
                                 "deleteGroup",           // 28
                                 "editGroupDetails",
                                 "loadGroupMembers",
                                 "addGroupMember",        // 31
                                 "removeGroupMember",
                                 "loadPermissionList",
                                 "loadPermissionBits",    // 34
                                 "addPermission",
                                 "removePermission",
                                 "escape",                // 37
                                 "unescape",
                                 "getVersion",
                                 "setObjectMetadata",     // 40
                                 "getObjectMetadata",
                                 "getAllObjectMetadata",
                                 "removeObjectMetadata",  // 43
                                 "getISODate",
                                 "xmlEscape",
                                 "setSessionData",        // 46
                                 "getSessionData",
                                 "getAllSessionData",
                                 "removeSessionData",     // 49
                                 "loadUsersGroups",
                                 "lockObject",
                                 "unLockObject",          // 52
                                 "loadWorkflowSettings",
                                 "attachWorkflowSettings",
                                 "loadWorkflowList",      // 55
                                 "approveObject",
                                 "moveObject",
                                 "removeWorkflowSettings", // 58
                                 "replaceTextContents",
                                 "createNewTextObject",
                                 "sendEmail",             // 61
                                 "isVerifier",            
				 "exportPackage",         // 63
				 "importPackage",          // 64
                                 "loadObjectVersions",
                                 "rollbackObjectVersion",
				 "createNewFolder",
                                 "rand",
                                 "urlRewrite",
                                 "urlBase",
				 "calCreateInstance",
				 "calCreateEvent",
				 "calCreateOccurrence",
				 "calDeleteInstance",
				 "calDeleteEvent",
				 "calDeleteOccurrence",
				 "calMoveInstance",
				 "calEditOccurrence",
				 "loadCalInstanceDetails",
				 "loadCalEventDetails",
				 "loadCalOccurrenceDetails",
				 "searchCalByEvent",
				 "searchCalByDateTime",
				 "compareDate",
				 "compareTime",
				 "compareDateTime",
				 "addDate",
				 "addTime",
				 "addDateTime",
				 "currentDate",
				 "currentTime",
				 "currentDateTime",
				 "getMonthName",
				 "getMonthNameShort",
				 "getDayName",
				 "getDayNameShort",
				 "subtractDate",
				 "subtractTime",
				 "subtractDateTime",
				 "validDate",
				 "validDateTime",
				 "boardCreateInstance",
				 "boardDeleteInstance",
				 "boardMoveInstance",
				 "boardCreateTopic",
				 "boardEditTopic",
				 "boardDeleteTopic",
				 "boardCreateMessage",
				 "boardEditMessage",
				 "boardEditTopic",
				 "boardDeleteMessage",
				 "boardSearch",
				 "boardSearchTopic",
				 "boardListTopics",
				 "loadBoardInstanceDetails",
				 "loadBoardTopicDetails",
				 "loadBoardMessageDetails",
				 "boardIncrementViews",
				 "loadCalInstancePath",
				 "copyObject",
				 "capitalise",
				 "setUserMetadata",
				 "getUserMetadata",
				 "getAllUserMetadata",
				 "removeUserMetadata",
				 "csvEscape",
                                 "loadDeletedFolderContents",
                                 "loadDeletedRootFolderContents",
                                 "recoverObjectVersion",
                                 "loadVerifierComment",
                                 "loadAllVerifierComments",
                                 "addVerifierComment"
                               };

const int FUNCTION_NUMARGS[] = {
                                 -1,
                                 -1,
                                 1,
                                 1,
                                 2,
                                 2,
                                 2,
                                 1,
                                 1,
                                 6,
                                 1,
                                 1,
                                 2,
                                 3,
                                 3,
                                 5,
                                 2,
                                 4,
                                 1,
                                 4,
                                 8,
                                 1,
                                 4,
                                 0,
                                 6,
                                 4,
                                 2,
                                 1,
                                 7,
                                 2,
                                 4,
                                 2,
                                 1,
                                 3,
                                 5,
                                 2,
                                 2,
                                 5,
                                 3,
                                 3,
                                 2,
                                 1,
                                 1,
                                 0,
                                 3,
                                 3, 
                                 2,
                                 2,
                                 1,
                                 1,
                                 2,
                                 2, 
                                 1,
                                 1, 
                                 5,
                                 1,
                                 1,
                                 2,
                                 3,
                                 4,
                                 1,
                                 2,
                                 1,
                                 4,
                                 8,
                                 3,
                                 2,
				 2,
				 2,
                                 4,
                                 3,
                                 4,
                                 0,
				 1,
				 1,
				 2,
				 2,
				 9,
				 1,
				 1,
				 1,
				 2,
				 8,
				 2,
				 2,
				 2,
				 2,
				 4,
				 2,
				 2,
				 2,
				 2,
				 2,
				 2,
				 0,
				 0,
				 0,
				 1,
				 1,
				 1,
				 1,
				 2,
				 2,
				 2,
				 1,
				 1,
				 2,
				 1,
				 2,
				 6,
				 5,
				 1,
				 3,
				 2,
				 5,
				 1,
				 5,
				 2,
				 2,
				 2,
				 2,
				 2,
				 1,
				 2,
				 2,
				 1,
				 3,
				 3,
				 2,
				 2,
				 1,
				 6,
				 5,
				 3,
				 2,
				 2,
				 2
                               };

/********************************************************************
* initTokenList...
*
* dhumalloc some memory for this token list.
*********************************************************************/
TokenList *initTokenList() {
  TokenList *tokens = NULL;
 
  tokens = (TokenList *) dhumalloc(sizeof(TokenList));
  tokens->head = NULL;
  tokens->tail = NULL;
  return tokens;
}

/********************************************************************
* initArgument...
*
* dhumalloc some memory for this argument.
*********************************************************************/
Argument *initArgument(char *value, char *name, int type) {
  Argument *arg = NULL;

  arg = (Argument *) dhumalloc(sizeof(Argument));
  if (arg == NULL)
    return NULL;

  arg->value = dhustrdup(value);
  arg->name = dhustrdup(name);
  arg->type = type;
  arg->next = NULL;
  return arg;
}

/********************************************************************
* initArgumentList...
*
* dhumalloc some memory for this argument list.
*********************************************************************/
ArgumentList *initArgumentList() {
  ArgumentList *arglist = NULL;

  arglist = (ArgumentList *) dhumalloc(sizeof(ArgumentList));
  if (arglist == NULL)
    return NULL;

  arglist->head = NULL;
  arglist->tail = NULL;
  arglist->size = 0;
  return arglist;
}

/********************************************************************
* freeArgumentList...
*
* Free this entire argument list.
*********************************************************************/
void freeArgumentList(ArgumentList *list) {
  Argument *current = NULL, *next = NULL;

  if (list == NULL)
    return;

  current = list->head;
  while (current != NULL) {
    next = current->next;
    dhufree(current->value);
    dhufree(current->name);
    dhufree(current);
    current = next;
  }
  dhufree(list);
}


/********************************************************************
* addArgumentToList...
*
* Adds this argument to the argument list.
********************************************************************/
int addArgumentToList(ArgumentList *list, char *value, char *name, int type) {
  Argument *arg = NULL;

  if (list == NULL)
    return RESOURCEERROR;

  arg = initArgument(value, name, type);

  if (list->tail == NULL) {
    list->head = arg;
    list->tail = arg;
  } else {
    list->tail->next = arg;
    list->tail = arg;
  }
  list->size++;
  return E_OK;
}

/*********************************************************************
* initToken...
*
* dhumalloc some memory for this token.
*********************************************************************/
Token *initToken(char *token, int type, int line) {
  Token *tok = NULL;

  tok = (Token *) dhumalloc(sizeof(Token));
  tok->token = token;
  tok->line = line;
  tok->valtype = (type == ARRAY)?ARRAY:((type == INT_TYPE)?INT_TYPE:STRING_TYPE);
  tok->ptree = NULL;
  tok->type = type;
  tok->next = NULL;
  return tok;
}

/*********************************************************************
* printSymbol...
*
* print this struct.
*********************************************************************/
void printSymbolList(SymbolList *s) {
  int i = 0, j = 0;
  Symbol *sym = NULL;

  if (!s) {
    fprintf(stdout, "\tSYMBOL IS NULL.\n"); 
    return;
  }

  fprintf(stdout, "\tNAME:[%s]\n", s->name);
  for (j = 0; j < countStack(s->symbols); j++) {
    sym = (Symbol *) sniffNStack(s->symbols, j);
    fprintf(stdout, "\tSCOPE:[%d]\n", sym->scope);
    fprintf(stdout, "\tLENGTH:[%d]\n", sym->length);
    for (i = 0; i < sym->length; i++) {
      fprintf(stdout, "\t\tELEMENT[%d]:TYPE:[%s]:[%s]\n", i, sym->types[i] == INT_TYPE?"INTEGER":"STRING", sym->values[i]);
    }
  }
}

/*********************************************************************
* printSymbolTable...
*
* print this struct.
*********************************************************************/
void printSymbolTable(SymbolTable *symbols) {
  MapNode *n = NULL;
  if (symbols == NULL) {
    fprintf(stdout, "NULL SYMBOL TABLE.\n"); 
  } else if (symbols->symbols->root == NULL) {
    fprintf(stdout, "EMPTY SYMBOL TABLE.\n"); 
  } else {
    n = getFirstMapNode(symbols->symbols);
    printSymbolList((SymbolList *) (n->ele));
    while ((n = getNextMapNode(n, symbols->symbols)) != NULL) {
      printSymbolList((SymbolList *) (n->ele));
    }
  }
}

/*********************************************************************
* compareFunctions...
*
* Are these objects equal?
*********************************************************************/
int compareFunctions(void *a, void *b) {
  Function *fa = (Function *) a,
           *fb = (Function *) b;

  return strcmp(fa->name, fb->name);
}

/*********************************************************************
* compareSymbolList...
*
* Are these objects equal?
*********************************************************************/
int compareSymbolList(void *a, void *b) {
  SymbolList *sa = (SymbolList *) a,
         *sb = (SymbolList *) b;

  return strcmp(sa->name, sb->name);
}

/*********************************************************************
* freeSymbol...
*
* Free this struct.
*********************************************************************/
void freeSymbol(void *symbol) {
  Symbol *s = (Symbol *) symbol;
  int i = 0;

  if (s) {
    for (i = 0; i < s->length; i++) {
      dhufree(s->values[i]);
    }
    dhufree(s->values);
    dhufree(s);
  }
}

/*********************************************************************
* freeSymbol...
*
* Free this struct.
*********************************************************************/
void freeSymbolList(void *symbol) {
  SymbolList *s = (SymbolList *) symbol;
  Symbol *sy = NULL;
  if (s) {
    while ((sy = (Symbol *) popStack(s->symbols))) { 
      freeSymbol(sy);
    }
    dhufree(s->name);
    dhufree(s);
  }
}

/*********************************************************************
* initFunction...
*
* init this struct.
*********************************************************************/
Function *initFunction(char *name, ArgumentList *args, ParseTree *ptree) {
  Function *function = NULL;

  function = (Function *) dhumalloc(sizeof(Function));
  if (function == NULL)
    return NULL;

  function->name = dhustrdup(name);
  function->ptree = copyParseTree(ptree);
  function->args = args;
  return function; 
}

/*********************************************************************
* freeFunction...
*
* Free this struct.
*********************************************************************/
void freeFunction(void *function) {
  Function *f = (Function *) function;

  if (f) {
    if (f->name)
      dhufree(f->name);
    if (f->args)
      freeArgumentList(f->args);
    freeParseTree(f->ptree);
    dhufree(f);
  }
}

/*********************************************************************
* initSymbolList...
*
* init this struct.
*********************************************************************/
SymbolList *initSymbolList() {
  SymbolList *symbols = NULL;

  symbols = (SymbolList *) dhumalloc(sizeof(SymbolList));
  if (symbols == NULL)
    return NULL;

  symbols->symbols = initStack();
  return symbols; 
}

/*********************************************************************
* initSymbolTable...
*
* init this struct.
*********************************************************************/
SymbolTable *initSymbolTable() {
  SymbolTable *symbols = NULL;

  symbols = (SymbolTable *) dhumalloc(sizeof(SymbolTable));
  if (symbols == NULL)
    return NULL;

  symbols->symbols = initMap(compareSymbolList, freeSymbolList);
  symbols->functions = initMap(compareFunctions, freeFunction);
  symbols->returnValue = NULL;
  symbols->scope = 0;
  return symbols; 
}


/*********************************************************************
* freeSymbolTable...
*
* Free this struct.
*********************************************************************/
void freeSymbolTable(SymbolTable *symbols) {

  freeMap(symbols->symbols);
  freeMap(symbols->functions);
  dhufree(symbols);
}

/*********************************************************************
* pushSymbol...
* 
* Adds a symbol to the symbol table with the current scope.
*********************************************************************/
int pushSymbol(char *name, char *value, int type, SymbolTable *symbols) {
  int index = 0, i = 0;
  Symbol *symbol = NULL;
  SymbolList *symbollist = NULL;
  MapNode *node = NULL;
  char *ptr = NULL;

  if (name == NULL || value == NULL || symbols == NULL)
    return RESOURCEERROR;

  symbol = (Symbol *) dhumalloc(sizeof(Symbol));
  if (symbol == NULL) {
    return RESOURCEERROR; 
  }
  
  ptr = strchr(name, '[');
  if (ptr) {
    *ptr = '\0';
    index = strtol(ptr + 1, NULL, 10);
  }

  symbollist = initSymbolList();
  symbollist->name = dhustrdup(name);
  if (ptr) {
    symbol->length = index + 1;
    symbol->values = (char **) dhumalloc(sizeof(char *) * symbol->length);
    symbol->values[index] = dhustrdup(value);
    symbol->types = (int *) dhumalloc(sizeof(int) * symbol->length);
    symbol->types[index] = type;
    for (i = 0; i < index; i++) {
      symbol->values[i] = dhustrdup("");
      symbol->types[i] = type;
    }
  } else {
    symbol->length = 1; 
    symbol->values = (char **) dhumalloc(sizeof(char *));
    symbol->types = (int *) dhumalloc(sizeof(int));
    symbol->types[0] = type;
    symbol->values[0] = dhustrdup(value);
  } 
  symbol->scope = symbols->scope;
  
  pushStack(symbollist->symbols, symbol);
  node = initMapNode(symbollist);
  insertMapValue(node, symbols->symbols);
  return E_OK;
}

/*********************************************************************
* forceSymbol...
* 
* Force a symbol in the symbol table.
*********************************************************************/
int forceSymbol(char *name, char *value, int type, SymbolTable *symbols) {
  int index = 0, i = 0; 
  Symbol *s = NULL;
  SymbolList key, *list = NULL;
  MapNode *node = NULL;
  char *ptr = NULL;

  if (name == NULL || value == NULL || symbols == NULL)
    return RESOURCEERROR;


  ptr = strchr(name, '[');
  if (ptr) {
    *ptr = '\0';
    index = strtol(ptr + 1, NULL, 10);
  }

  key.name = name;
  node = searchMap(&key, symbols->symbols);

  if (node) {
    list = (SymbolList *) node->ele;

    s = (Symbol *) sniffStack(list->symbols);
    if (s) {
      if (index < 0) {
        if (ptr) *ptr = '[';
        return ARRAYINDEXOUTOFBOUNDS;
      }
   
      if (s->scope == symbols->scope) {
        // set the value in this scope
        if (index >= s->length) {
          s->values = (char **) realloc(s->values, sizeof(char *) * (index + 1));
          s->types = (int *) realloc(s->types, sizeof(int) * (index + 1));
          for (i = s->length; i < index; i++) {
            s->values[i] = dhustrdup("");
            s->types[i] = type;
          }
          s->values[index] = dhustrdup(value);
          s->types[index] = type;
          s->length = index + 1;
        } else {
          dhufree(s->values[index]);
          s->values[index] = dhustrdup(value);
          s->types[index] = type;
        }
        if (ptr) *ptr = '[';
        return E_OK;
      } else {
        // add a new symbol with this scope
        s = (Symbol *) malloc(sizeof(Symbol));
  
        if (ptr) {
          s->length = index + 1;
          s->values = (char **) dhumalloc(sizeof(char *) * s->length);
          s->values[index] = dhustrdup(value);
          s->types = (int *) dhumalloc(sizeof(int) * s->length);
          s->types[index] = type;
          for (i = 0; i < index; i++) {
            s->values[i] = dhustrdup("");
            s->types[i] = type;
          }
        } else {
          s->length = 1; 
          s->values = (char **) dhumalloc(sizeof(char *));
          s->types = (int *) dhumalloc(sizeof(int));
          s->types[0] = type;
          s->values[0] = dhustrdup(value);
        } 
        s->scope = symbols->scope;
        pushStack(list->symbols, s);
        return E_OK;
      }
    }
  }
  if (ptr) *ptr = '[';
  return pushSymbol(name, value, type, symbols);
}

/*********************************************************************
* setSymbol...
* 
* Sets a symbol in the symbol table.
*********************************************************************/
int setSymbol(char *name, char *value, int type, SymbolTable *symbols) {
  int index = 0, i = 0;
  Symbol *s = NULL;
  SymbolList key, *list = NULL;
  MapNode *node = NULL;
  char *ptr = NULL;

  if (name == NULL || value == NULL || symbols == NULL) {
    return RESOURCEERROR;
  }

  ptr = strchr(name, '[');
  if (ptr) {
    *ptr = '\0';
    index = strtol(ptr + 1, NULL, 10);
  }

  key.name = name;
  node = searchMap(&key, symbols->symbols);
  if (node) {
    list = (SymbolList *) node->ele;
    s = (Symbol *) sniffStack(list->symbols);
    if (s) {
      if (index < 0) {
        *ptr = '\0';
        return ARRAYINDEXOUTOFBOUNDS;
      }
      if (index >= s->length) {
        s->values = (char **) realloc(s->values, sizeof(char *) * (index + 1));
        s->types = (int *) realloc(s->types, sizeof(int) * (index + 1));
        for (i = s->length; i < index; i++) {
          s->values[i] = dhustrdup("");
          s->types[i] = type;
        }
        s->values[index] = dhustrdup(value);
        // Never assign a type on an assignment.
        // s->types[index] = type;
        s->length = index + 1;
      } else {
        dhufree(s->values[index]);
        s->values[index] = dhustrdup(value);
        // Never assign a type on an assignment.
        //s->types[index] = type;
      }
      return E_OK;
    }
  }
  return UNDEFINEDTOKEN;
}

/*********************************************************************
* getSymbol...
* 
* Gets a symbol from the symbol table.
*********************************************************************/
int getSymbol(char *name, char **value, int *type, SymbolTable *symbols) {
  int index = 0;
  Symbol *s = NULL;
  SymbolList key, *list = NULL;
  MapNode *node = NULL;
  char *ptr = NULL;

  if (name == NULL || value == NULL || symbols == NULL) {
    return RESOURCEERROR;
  }

  ptr = strchr(name, '[');
  if (ptr) *ptr = '\0';

  key.name = name;
  node = searchMap(&key, symbols->symbols);
  if (node) {
    list = (SymbolList *) node->ele;
    s = (Symbol *) sniffStack(list->symbols);
    if (s) {
      if (ptr) 
        index = strtol(ptr+1, NULL, 10);

      if (index < 0) {
        if (ptr) *ptr = '[';
        return ARRAYINDEXOUTOFBOUNDS;
      }
      if (index >= s->length) {
        dhufree(*value);
        *value = dhustrdup("");
        *type = STRING_TYPE;
        return E_OK;
      }
      *type = s->types[index];
      dhufree(*value);
      *value = dhustrdup(s->values[index]);

      if (ptr) *ptr = '[';
      return E_OK;
    }
  }
  
  if (ptr) *ptr = '[';
  return UNDEFINEDTOKEN;
}

/*********************************************************************
* increaseScope...
* 
* Just increases the current scope of the symbol table.
*********************************************************************/
int increaseScope(SymbolTable *symbols) {

  symbols->scope++;
  return E_OK;
}

/*********************************************************************
* decreaseScope...
* 
* Decreases the current scope of the symbol table and
* removes all variables that were added in that higher scope.
*********************************************************************/
int decreaseScope(SymbolTable *symbols) {
  Symbol *s = NULL;
  SymbolList *list = NULL;
  MapNode *node = NULL;

  if (symbols->scope > 0) {
    symbols->scope--;

    node = getFirstMapNode(symbols->symbols);

    while (node != NULL) {
      list = (SymbolList *) node->ele;
      if (countStack(list->symbols) > 0) {
        s = (Symbol *) sniffStack(list->symbols);

        if (s) {
          if (s->scope > symbols->scope) {
            s = (Symbol *) popStack(list->symbols);
  	    freeSymbol(s);
	    if (countStack(list->symbols) <= 0) {
	      removeMapValue(node, symbols->symbols);
	    }
	    // we have to start again cause the tree rebalanced.
            node = getFirstMapNode(symbols->symbols);
          } else {
            node = getNextMapNode(node, symbols->symbols);
	  }
        }
      }
    }
  }

  return E_OK;
}

/*********************************************************************
* initParseTree...
*
* dhumalloc some memory for this node.
*********************************************************************/
ParseTree *initParseTree(Token *tok) {
  ParseTree *ptree = NULL;

  ptree = (ParseTree *) dhumalloc(sizeof(ParseTree));
  tok->ptree = ptree;
  ptree->token = tok;
  ptree->rightsibling = NULL;
  ptree->leftchild = NULL;
  ptree->skip = 0;
  ptree->node = 0;
  ptree->leftc = 0;
  ptree->rights = 0;
  return ptree;
}

/*********************************************************************
* isTerminal...
*
* Is this token a terminal token?
*********************************************************************/
int isTerminal(Token *tok) {
  if (tok == NULL)
    return 0;
  
  switch (tok->type) {
    case IDENTIFIER:
    case STRING_TYPE:
    case INT_TYPE:
    case SEMICOLON_OPERATOR:
    case ADD_OPERATOR:
    case SUBTRACT_OPERATOR:
    case MULTIPLY_OPERATOR:
    case DIVIDE_OPERATOR:
    case MOD_OPERATOR:
    case LEFT_PARENTHESIS:
    case LEFT_SQUARE_PARENTHESIS:
    case LEFT_SCOPE:
    case RIGHT_PARENTHESIS:
    case RIGHT_SQUARE_PARENTHESIS:
    case RIGHT_SCOPE:
    case EQUALS_OPERATOR:
    case MULTIPLY_EQUALS_OPERATOR:
    case DIVIDE_EQUALS_OPERATOR:
    case MOD_EQUALS_OPERATOR:
    case ADD_EQUALS_OPERATOR:
    case SUBTRACT_EQUALS_OPERATOR:
    case STRING_KEYWORD:
    case ARRAY_KEYWORD:
    case INT_KEYWORD:
    case IF_KEYWORD:
    case ELSE_KEYWORD:
    case COMMENT:
    case EOF_TOKEN:
    case BOF_TOKEN:
    case EQUALITY_OPERATOR:
    case INEQUALITY_OPERATOR:
    case LESSTHAN_OPERATOR:
    case LESSTHANEQUAL_OPERATOR:
    case GREATERTHAN_OPERATOR:
    case GREATERTHANEQUAL_OPERATOR:
    case WHILE_KEYWORD:
    case DO_KEYWORD:
    case FOR_KEYWORD:
      return 1;
      break;
    default:
      return 0;
      break;
  } 
}

/*********************************************************************
* serialiseStack...
*
* Write the contents of the stack to a buffer.
*********************************************************************/
void serialiseStack(Stack *stack, char **output) {
  int i = 0;
  Token *t = NULL;

  if (stack == NULL || countStack(stack)) 
    return;
  
  for (i = 0; i < countStack(stack); i++) {
    t = (Token *) sniffNStack(stack, i);
    vstrdupcat(output, "\nTOKEN: ", t->token, " TYPE: ", getTokenName(t->type), NULL);
  }
}

/*********************************************************************
* copyParseTree...
*
* copy the contents of the parse tree.
*********************************************************************/
ParseTree * copyParseTree(ParseTree *ptree) {
  ParseTree *dest = NULL;
  Token *tok = NULL;

  if (ptree == NULL)
    return NULL;
  tok = initToken(dhustrdup(ptree->token->token), ptree->token->type, ptree->token->line);
  tok->valtype = ptree->token->valtype;
  tok->type = ptree->token->type;

  dest = initParseTree(tok);
   
  if (ptree->leftchild)
    dest->leftchild = copyParseTree(ptree->leftchild);
  if (ptree->rightsibling)
    dest->rightsibling = copyParseTree(ptree->rightsibling);
  return dest;
}

/*********************************************************************
* freeParseTree...
*
* free the contents of the parse tree.
*********************************************************************/
void freeParseTree(ParseTree *ptree) {
  if (ptree) {
    if (ptree->leftchild)
      freeParseTree(ptree->leftchild);
    if (ptree->rightsibling)
      freeParseTree(ptree->rightsibling);
    dhufree(ptree);
  }
}

/*********************************************************************
* stripQuotes...
*
* Strip the begin and end quotes from this string.
*********************************************************************/
char *stripQuotes(char *str) {
  char *ret = NULL;
  if (*str == '\'' || *str == '"') {
    ret = (char *) dhumalloc(sizeof(char) * (strlen(str) -1));
    if (ret == NULL)
      return NULL;

    strncpy(ret, str + 1, strlen(str + 2));
    ret[strlen(str + 2)] = CNULL;
    return ret;
  } else {
    return dhustrdup(str);
  }
}
/*********************************************************************
* stripTrailingWhiteSpace...
*
* Strip the white space after this token.
*********************************************************************/
void stripTrailingWhiteSpace(char *str) {
  char *eptr = NULL;

  if (str == NULL)
    return;

  eptr = str + strlen(str) - 1;
  while (isspace(*eptr) && eptr > str) eptr--;

  *(eptr + 1) = CNULL; 
}

/*********************************************************************
* addTokenToList...
*
* Add this token to the end of this token list.
*********************************************************************/
void addTokenToList(TokenList *list,  char *token, int type, int line, int nostrip) {
  Token *last = list->tail;
  char *strippedtoken = NULL;
 
  if (type == COMMENT) {
    return;
  } else if ((type == STRING_TYPE) && (!nostrip)) {
    if (*token == '&')
      strippedtoken = dhustrdup(token + 1);
    else
      strippedtoken = stripQuotes(token);
    dhufree(token);
  } else if ((type == IDENTIFIER) && (!nostrip)) {
    strippedtoken = dhustrdup(token);
    stripTrailingWhiteSpace(strippedtoken);
  } else {
    strippedtoken = dhustrdup(token);
  }
  
  if (last == NULL) {
    list->head = initToken(strippedtoken, type, line);
    list->tail = list->head;
  } else {
    last->next = initToken(strippedtoken, type, line);
    list->tail = last->next;
  }
}

/*********************************************************************
* printTokenList...
*
* Print this token list.
*********************************************************************/
void printTokenList(TokenList *tokens) {
  Token *current = NULL;

  return;
  current = tokens->head;
  while (current != NULL) {
    fprintf(stdout, "CURRENT TOKEN: [%s] %s\n", getTokenName(current->type), current->token);
    current = current->next;
  }
  
}
/*********************************************************************
* freeTokenList...
*
* Free this token list.
*********************************************************************/
void freeToken(Token *token) {
  Token *current = NULL, *next = NULL;

  current = token;
  while (current != NULL) {
    next = current->next;
    dhufree(current->token);
    dhufree(current);
    current = next;
  }
}
/*********************************************************************
* freeTokenList...
*
* Free this token list.
*********************************************************************/
void freeTokenList(TokenList *tokens) {
  Token *current = NULL;

  if (tokens == NULL)
    return;

  current = tokens->head;
  freeToken(current);
  dhufree(tokens);
}

/*********************************************************************
* freezeTokenStream...
*
* puts the tokens into a single char *
*********************************************************************/
void freezeTokenStream(TokenList *tokens, char **output) {
  Token *current = NULL;
  char *buffer = NULL;

  if (tokens == NULL) {
    *output = dhustrdup("");
    return;
  }

  current = tokens->head;
  while (current != NULL) {
    vstrdupcat(&buffer, " ", current->token, NULL);
    current = current->next;
  }
  *output = buffer;
}

/*********************************************************************
* evenEscapes...
*
* counts the number of preceding \ chars and returns 1 if they are even.
*********************************************************************/
int evenEscapes(char *pos, char *start) {
  char *current = pos;
  
  while ((current != (start + 1)) && (*(current - 1) == '\\')) 
    current--;
  return (((pos - current) % 2) == 0);
}

/*********************************************************************
* parseToken...
*
* Gets the next token from the input.
*********************************************************************/
int parseToken(char *source, char **end, char **token, int *type, int *line) {
  char *start = source, *stop = NULL, c = ' ', *ptr = NULL;

  if (start == NULL || *start == CNULL)
    return NODATAFOUND;

  while (isspace(*start)) {
    if (*start == '\n') {
      (*line)++;
    }
    start++;
  }
  if (*start == CNULL)
    return NODATAFOUND;  

  stop = start;
  // comment
  if ( (strncmp(start, "//", 2) == 0) || 
              (strncmp(start, "/*", 2) == 0) ) {
    if (strncmp(start, "//", 2) == 0) {
      stop = strpbrk(start, "\r\n");
      if (stop == NULL) {
        stop = start;
        while (*stop != CNULL) stop++;
      }
      (*line)++;
      *type = COMMENT;
    } else {
      stop = strstr(start, "*/");
     
      if (stop == NULL)
        return SYNTAXERROR;

      ptr = start;
      while (((ptr = strchr(ptr, '\n')) != NULL) && (ptr < stop)) {(*line)++; ptr++;}
      stop += 2;
      *type = COMMENT;
    }

  // either an identifier or a keyword
  } else if (isalpha(*start)) {
    // first check for keywords
    if ((strncmp(start, "string", strlen("string")) == 0) &&
               (!isalnum(start[strlen("string")]))) {
      stop += strlen("string");
      *type = STRING_KEYWORD;
    } else if ((strncmp(start, "array", strlen("array")) == 0) &&
               (!isalnum(start[strlen("array")]))) {
      stop += strlen("array");
      *type = ARRAY_KEYWORD;
    } else if ((strncmp(start, "while", strlen("while")) == 0) &&
               (!isalnum(start[strlen("while")]))) {
      stop += strlen("while");
      *type = WHILE_KEYWORD;
    } else if ((strncmp(start, "do", strlen("do")) == 0) &&
               (!isalnum(start[strlen("do")]))) {
      stop += strlen("do");
      *type = DO_KEYWORD;
    } else if ((strncmp(start, "for", strlen("for")) == 0) &&
               (!isalnum(start[strlen("for")]))) {
      stop += strlen("for");
      *type = FOR_KEYWORD;
    } else if ((strncmp(start, "if", strlen("if")) == 0) &&
               (!isalnum(start[strlen("if")]))) {
      stop += strlen("if");
      *type = IF_KEYWORD;
    } else if ((strncmp(start, "else", strlen("else")) == 0) &&
               (!isalnum(start[strlen("else")]))) {
      stop += strlen("else");
      *type = ELSE_KEYWORD;
    } else if ((strncmp(start, "int", strlen("int")) == 0) &&
               (!isalnum(start[strlen("int")]))) {
      stop += strlen("int");
      *type = INT_KEYWORD;
    } else if ((strncmp(start, "function", strlen("function")) == 0) &&
               (!isalnum(start[strlen("function")]))) {
      stop += strlen("function");
      *type = FUNCTION_KEYWORD;
    } else if ((strncmp(start, "return", strlen("return")) == 0) &&
               (!isalnum(start[strlen("return")]))) {
      stop += strlen("return");
      *type = RETURN_KEYWORD;
    } else if ((strncmp(start, "void", strlen("void")) == 0) &&
               (!isalnum(start[strlen("void")]))) {
      stop += strlen("void");
      *type = VOID_KEYWORD;
    } else {
      // identifier
      while (isalnum(*stop)) stop++;
      while (isspace(*stop)) stop++;
      *type = IDENTIFIER;
    }
  // has to be a number
  } else if (isdigit(*start)) {
    while (isdigit(*stop)) stop++;
    *type = INT_TYPE; 

  // semicolon
  } else if (*start == ';') {
    stop++;
    *type = SEMICOLON_OPERATOR;

  // equality
  } else if (strncmp(start, "==", 2) == 0) {
    stop+=2;
    *type = EQUALITY_OPERATOR;

  // inequality
  } else if (strncmp(start, "!=", 2) == 0) {
    stop+=2;
    *type = INEQUALITY_OPERATOR;

  // or
  } else if (strncmp(start, "||", 2) == 0) {
    stop+=2;
    *type = BOOLEANOR_OPERATOR;

  // and
  } else if (strncmp(start, "&&", 2) == 0) {
    stop+=2;
    *type = BOOLEANAND_OPERATOR;

  // lessthanequal
  } else if (strncmp(start, "<=", 2) == 0) {
    stop+=2;
    *type = LESSTHANEQUAL_OPERATOR;

  // greaterthanequal
  } else if (strncmp(start, ">=", 2) == 0) {
    stop+=2;
    *type = GREATERTHANEQUAL_OPERATOR;

  // multiply equals
  } else if (strncmp(start, "*=", 2) == 0) {
    stop+=2;
    *type = MULTIPLY_EQUALS_OPERATOR;

  // divide equals
  } else if (strncmp(start, "/=", 2) == 0) {
    stop+=2;
    *type = DIVIDE_EQUALS_OPERATOR;

  // mod equals
  } else if (strncmp(start, "%=", 2) == 0) {
    stop+=2;
    *type = MOD_EQUALS_OPERATOR;

  // add equals
  } else if (strncmp(start, "+=", 2) == 0) {
    stop+=2;
    *type = ADD_EQUALS_OPERATOR;

  // subtract equals
  } else if (strncmp(start, "-=", 2) == 0) {
    stop+=2;
    *type = SUBTRACT_EQUALS_OPERATOR;

  // lessthan
  } else if (*start == '<') {
    stop++;
    *type = LESSTHAN_OPERATOR;

  // greaterthan
  } else if (*start == '>') {
    stop++;
    *type = GREATERTHAN_OPERATOR;

  // equals
  } else if (*start == '=') {
    stop++;
    *type = EQUALS_OPERATOR;

  // add
  } else if (*start == '+') {
    stop++;
    *type = ADD_OPERATOR;

  // subtract
  } else if (*start == '-') {
    stop++;
    *type = SUBTRACT_OPERATOR;

  // multiply
  } else if (*start == '*') {
    stop++;
    *type = MULTIPLY_OPERATOR;

  // divide
  } else if (*start == '/') {
    stop++;
    *type = DIVIDE_OPERATOR;

  // mod
  } else if (*start == '%') {
    stop++;
    *type = MOD_OPERATOR;

  // left [ 
  } else if (*start == '[') {
    stop++;
    *type = LEFT_SQUARE_PARENTHESIS;

  // right ] 
  } else if (*start == ']') {
    stop++;
    *type = RIGHT_SQUARE_PARENTHESIS;

  // left ( 
  } else if (*start == '(') {
    stop++;
    *type = LEFT_PARENTHESIS;

  // left { 
  } else if (*start == '{') {
    stop++;
    *type = LEFT_SCOPE;

  // right ) 
  } else if (*start == ')') {
    stop++;
    *type = RIGHT_PARENTHESIS;

  // right { 
  } else if (*start == '}') {
    stop++;
    *type = RIGHT_SCOPE;

  // comma  
  } else if (*start == ',') {
    stop++;
    *type = COMMA_SEPARATOR;

  // string
  } else if (*start == '\'' || *start == '"') {
    c = *stop;
    do {stop++;} while ((c != *stop || 
                      (!evenEscapes(stop, start))) && 
                     (*stop != CNULL));
    ptr = start;
    while (((ptr = strchr(ptr, '\n')) != NULL) && (ptr < stop)) {(*line)++; ptr++;}
    if (*stop == CNULL)
      return SYNTAXERROR;
    stop++;
    *type = STRING_TYPE;
  // comment
  } else {
    return SYNTAXERROR;
  }

  *token = (char *) calloc(sizeof(char), (stop - start + 1));
  if (*token == NULL)
    return RESOURCEERROR;
  strncpy(*token, start, stop - start);
    
  if (*type == STRING_TYPE) {
    stripEscapes(*token);
  }
  *end = stop;

  return E_OK;
}

/*********************************************************************
* lexer...
*
* Parses the input into tokens.
*********************************************************************/
int lexer(char *script, char **output, TokenList **tokenlist) {
  TokenList *tokens = NULL;
  char *start = NULL, *end = NULL, *token = NULL, *buffer = NULL;
  int type = 0, retcode = 0, line = 1;

  start = script;
  tokens = initTokenList();
  
  addTokenToList(tokens, "", BOF_TOKEN, line, 0);

  while ((retcode = parseToken(start, &end, &token, &type, &line)) == E_OK) {
    addTokenToList(tokens, token, type, line, 0);
    start = end;
  }

  if (retcode != NODATAFOUND) {
    freezeTokenStream(tokens, &buffer);
    vstrdupcat(output, "[LEXER ERROR:", 
                        getErrorMesg(retcode), 
                        "]\n",
                        buffer,
                        NULL);
    dhufree(buffer);
    *tokenlist = tokens;
    return SYNTAXERROR;
  }

  addTokenToList(tokens, "", EOF_TOKEN, line, 0);

  *output = buffer;
  *tokenlist = tokens;
  return E_OK;
}

/*********************************************************************
* printParseTree...
*
* Print the contents of the parse tree.
*********************************************************************/
void printParseTree(ParseTree *ptree, int indent) {
  int i = 0;

  if (ptree == NULL) {
    fprintf(stderr, "NULL PARSE TREE\n"); 
    return;
  }
  for (i = 0; i < indent; i++) {
    fprintf(stderr, " ");
  } 
  fprintf(stderr, "TOKEN: %s TYPE: %s VALTYPE: %s LINE: %d \n", ptree->token->token?ptree->token->token:"NULL", getTokenName(ptree->token->type), getTokenName(ptree->token->valtype), ptree->token->line);
  if (ptree->leftchild)
    printParseTree(ptree->leftchild, indent + 1);
  if (ptree->rightsibling)
    printParseTree(ptree->rightsibling, indent);
}

/*********************************************************************
* lookAheadCmp...
*
* Comparison function for the look ahead binary search.
*********************************************************************/
int lookAheadCmp(char *rule, char *search) {
  int i = 0, lena = strlen(rule), lenb = strlen(search), cmp = 0;


  for (i = 0; i < lena && i < lenb; i++) {
    cmp = rule[i] - search[i];
    if (cmp < 0)
      return -1;
    if (cmp > 0)
      return 1;
  }

  if (lena >= lenb)
    return 0;

  return lenb - lena;
}
/*********************************************************************
* searchNodeLookAhead...
*
* Binary search that will return partial matches
*********************************************************************/
MapNode *searchNodeLookAhead(MapNode *current, char *search) {
  int cmp = 0;
  RulePhrase *rule = NULL;

  if (current == NULL)
    return NULL;

  rule = (RulePhrase *) current->ele;
  cmp = lookAheadCmp(rule->rule, search);

  if (cmp > 0) {
    if (!current->left->left)
      return NULL;
    return searchNodeLookAhead(current->left, search);
  } else if (cmp < 0) {
    if (!current->right->right)
      return NULL;
    return searchNodeLookAhead(current->right, search);
  }

 // printf("Found look ahead:");
 // printRule(rule);
  //printf("\n");
  return current;
}

/*********************************************************************
* searchLookAhead...
*
* Binary search that will return partial matches
*********************************************************************/
int searchLookAhead(Map *ruleset, char *search) {
  MapNode *result = NULL;

  result = searchNodeLookAhead(ruleset->root, search);
  if (result) {
    return E_OK;
  }

  return NOMATCH;
}

/*********************************************************************
* reduceStack...
*
* Apply a reduction rule to the stack.
*********************************************************************/
int reduceStack(Stack *stack, Map *ruleset, Token *lookahead) {
  /* I need to write a set of routines in rules.c that I can use to 
   * match the stack and reduce it.
   * Once I have written that bit - I can add the code so that whenever
   * something is pushed onto the stack or reduced, that information is
   * captured in the parse tree.
   */
  RulePhrase *rule = NULL;
  int len = 0, i = 0, result = 0;
  ParseTree *node = NULL, *pop = NULL;
  Token *tok = NULL, *token = NULL;
  MapNode *match = NULL;
  char *search = NULL, *matchRule = NULL, buf[2], *p = NULL;
  
  if (ruleset == NULL)
    return RESOURCEERROR; 

  search = buildStackStr(stack, 0);
  rule = initRulePhrase(START, ONEONLY, search);
  for (i = strlen(search); (i > 0) && match == NULL; i--) {
    rule->rule = search + strlen(search) - i;
    //printf("Search for match...");
   // printRule(rule);
    //printf("\n");
    match = searchMap(rule, ruleset);
  }


  if (!match) {
    rule->rule = search;
    freeRulePhrase(rule);
    return NOMATCH;
  }
  vstrdupcat(&matchRule, rule->rule, NULL);
  rule->rule = search;
  freeRulePhrase(rule);

  rule = (RulePhrase *) match->ele;
  //printf("MATCH: ");
 // printRule(rule);
 // printf("\n");
 // printStack(stack);

  // If we look ahead and find a partial or full match, do not reduce
  if (lookahead) {
    buf[0] = lookahead->type;
    buf[1] = '\0';
    vstrdupcat(&matchRule, buf, NULL);

    p = matchRule;

    while (p[0] != '\0' && p[1] != '\0') {
      result = searchLookAhead(ruleset, p);

      if (result == E_OK || result == PARTIALMATCH) {
        // get more tokens!
        return NOMATCH;
      }
      p++;
    }
    dhufree(matchRule);

  }
  
  len = strlen(rule->rule);
  pop = ((Token *)(sniffNStack(stack, len - 1)))->ptree;
  tok = initToken(dhustrdup(""), rule->reducetype, pop->token->line );
  node = initParseTree(tok);
  // set the new node to be the parent of the nodes we are popping
  node->leftchild = pop;
  for (i = 0; i < len; i++) {
    token = (Token *) popStack(stack);

    if (i != len - 1)
      ((Token *)(sniffStack(stack)))->ptree->rightsibling = token->ptree;
    
  }
  pushStack(stack, tok);
  return E_OK;
}


/*********************************************************************
* buildParseTree...
*
* Stage 2 - build a valid parse tree to execute the script.
*********************************************************************/
int buildParseTree(TokenList *tokens, char **output, ParseTree **ptree) {
  int retcode = E_OK;
  Stack *stack = NULL;
  Token *token = NULL;
  Map *reductionrules = NULL;
  ParseTree *node = NULL;
  char numbuf[128];

  // Until we have only the start symbol in the stack and there are no
  // more tokens we proceed like this.
  // A First examine the stack.
  // If we can reduce the stack
  //   reduce the stack
  // else 
  //   push the next input token onto the stack

  
  reductionrules = initMap(cmpRulePhrase, freeRulePhrase);

  if ((retcode = initReductionRules(reductionrules)) != E_OK)
    return retcode;

  stack = initStack();
  if (stack == NULL)
    return RESOURCEERROR;
  
  if (tokens == NULL)
    return RESOURCEERROR;

  token = tokens->head;
  if (token == NULL)
    return RESOURCEERROR;

  node = initParseTree(token);
  
  pushStack(stack, token);
  token = token->next;

  while ((token != NULL) || (reduceStack(stack, reductionrules, token) == E_OK)) {
    // NOTE THE SEMI COLON!
    if (reduceStack(stack, reductionrules, token) == E_OK) {
      while (reduceStack(stack, reductionrules, token) == E_OK);
    }
    
    if (token) {
      node = initParseTree(token);
      pushStack(stack, token);
      token = token->next;  
    }
  }

  // free the reduction rules
  freeMap(reductionrules);

  token = (Token *) sniffStack(stack);
  if (token == NULL) {
    vstrdupcat(output, "<!-- FATAL SYNTAX ERROR -->", NULL);
    return SYNTAXERROR;
  } else if (token->type != START || countStack(stack) != 1) {
    // printStack(stack);
    sprintf(numbuf, "%d", token->line);
    vstrdupcat(output, "<!-- FATAL SYNTAX ERROR ON LINE: ", numbuf, "-->", NULL);
    freeParseTree(token->ptree);
    return SYNTAXERROR;
  }
  *ptree = token->ptree;
  
  freeStack(&stack);

  return E_OK;
}


/*********************************************************************
* countSiblings... 
* 
* How many siblings at this level of the parse tree?
*********************************************************************/
int countSiblings(ParseTree *start) {
  ParseTree *current = start;
  int numsiblings = 0;

  while (current != NULL) {current = current->rightsibling; numsiblings++;}

  return numsiblings;
}

/*********************************************************************
* evalIntegerArithmetic... 
* 
* Perform this integer operation.
*********************************************************************/
int evalIntegerArithmetic(char *intA, char *oper, char *intB, char **result) {
  int A = 0, B = 0, C = 0;

  A = strtol(intA, NULL, 10);
  B = strtol(intB, NULL, 10);

  switch(*oper) {
    case '+':
      C = A + B;
      break;
    case '-':
      C = A - B;
      break;
    case '*':
      C = A * B;
      break;
    case '/':
      if (B == 0)
        return DIVIDEBYZERO;
      C = A / B;
      break;
    case '%':
      C = A % B;
      break;
    case '=':
      C = A == B;
      break;
    case '!':
      C = A != B;
      break;
    case '<':
      if (oper[1] == '=')
        C = A <= B;
      else
        C = A < B;
      break;
    case '>':
      if (oper[1] == '=')
        C = A >= B;
      else
        C = A > B;
      break;
    case '&':
      C = (A != 0) && (B != 0);
      break;
    case '|':
      C = (A != 0) || (B != 0);
      break;
    default:
      return INVALIDOPERATOR;
  }

  int2Str(C, result);
  return E_OK;
}

/*********************************************************************
* evalStringArithmetic... 
* 
* Perform this string operation.
*********************************************************************/
int evalStringArithmetic(char *strA, char *oper, char *strB, char **result, int *type) {
  char *strC = NULL;
  int len = 0;
  
  switch(*oper) {
    case '+':
      vstrdupcat(&strC, strA, strB, NULL);
      *type = STRING_TYPE;
      break;
    case '=':
      if (strcmp(strA, strB) == 0)
        vstrdupcat(&strC, "1", NULL);
      else
        vstrdupcat(&strC, "0", NULL);
      *type = INT_TYPE;
      break;
    case '!':
      if (strcmp(strA, strB) == 0)
        vstrdupcat(&strC, "0", NULL);
      else
        vstrdupcat(&strC, "1", NULL);
      *type = INT_TYPE;
      break;
    case '-':
      if (strcmp(strB, strA + strlen(strA) - strlen(strB)) == 0) {
        len = strlen(strA) - strlen(strB);
        strC = (char *) dhumalloc(sizeof(char) * (len + 1));
        strncpy(strC, strA, len);
        strC[len] = CNULL;
      } else {
        strC = dhustrdup(strA);
      }
      *type = STRING_TYPE;
      break;

    default:
      return INVALIDOPERATOR;
  }
  *result = strC;
  return E_OK;
}

/*********************************************************************
* getFunctionID... 
* 
* Resolve this function name to an id
*********************************************************************/
int getFunctionID(char *funcname) {
  int i = 0, functionid = -1;
  for (i = 0; i < MAX_FUNCTIONS; i++) {
    if (strcmp(FUNCTION_NAMES[i], funcname?funcname:"") == 0) {
      functionid = i;
    }
  }
  return functionid;
}

/*********************************************************************
* getFunctionNumArgs... 
* 
* How many arguments are allowed for this function?
*********************************************************************/
int getFunctionNumArgs(int functionid) {
  return FUNCTION_NUMARGS[functionid];
}

/*********************************************************************
* evaluatesTrue... 
* 
* Is this true?
*********************************************************************/
int evalBoolean(char *value, int type) {
  if (type == STRING_TYPE) {
    return strcmp(value, "");
  } else {
    return strcmp(value, "0");
  }
}

/*********************************************************************
* evalExpression... 
* 
* Evaluate this sequence of tokens and put the resulting token into dest.
*********************************************************************/
int evalExpression(ParseTree *expr, ParseTree *dest, SymbolTable *symbols, char **output, int *line, Env *env, void *sqlsock) {
  ParseTree *current = NULL, *tmp = NULL;
  int retcode = 0, functionid = 0, numargs = 0, type = 0, stype = 0;
  ArgumentList *list = NULL;
  char *value = NULL, *oper = NULL, *svalue = NULL, *name = NULL, *UNDEFINED = "<undefined>";
  MapNode *node = NULL;
  Function *function = NULL;
  Argument *arg = NULL, *argdef = NULL;

  // printParseTree(expr, 5);

  current = expr;
  if (current->token)
    *line = current->token->line;

  if (current->token->type == TYPE) {
    if (current->rightsibling == NULL ||
        current->leftchild == NULL ||
        current->rightsibling->token->type != IDENTIFIER) {
        return INVALIDNUMARGS;
    }
    switch (current->leftchild->token->type) {
      case STRING_KEYWORD:
        type = STRING_TYPE;
        break;
      case INT_KEYWORD:
        type = INT_TYPE;
        break;
      case ARRAY_KEYWORD:
        type = ARRAY;
        break;
      default:
        type = STRING_TYPE;
        break;
    }
    retcode = forceSymbol(current->rightsibling->token->token, "", type, symbols);
    if (retcode != E_OK)
      return retcode;
  } else if (current->token->type == BEGIN_FOR && 
             current->rightsibling && 
             current->rightsibling->token->type == EXPRESSION && 
             current->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->rightsibling->token->type == EXPRESSION && 
             current->rightsibling->rightsibling->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->token->type == EXPRESSION && 
             current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling ) {

    // for (A; b; c)  d
    if ((retcode = interpretParseTreeNode(current->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
      return retcode;
        
    // for (a; B; c) d
    if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
      return retcode;
        
    // for (a; B; c)  d
    while (evalBoolean(current->rightsibling->rightsibling->rightsibling->token->token, current->rightsibling->rightsibling->rightsibling->token->valtype)) {
      // eval subtree
      // for (a; b; c)  D

      if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
        return retcode;
        
      // for (a; b; C)  d
      if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
        return retcode;
        
      // for (a; B; c)  d
      if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
        return retcode;
          
    }
    current->rightsibling->skip = 1;
    current->rightsibling->rightsibling->rightsibling->skip = 1;
    current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->skip = 1;
    current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->skip = 1;

    
  } else if (current->token->type == BEGIN_WHILE && 
             current->rightsibling && 
             current->rightsibling->token->type == EXPRESSION && 
             current->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->token->type == RIGHT_PARENTHESIS &&
             current->rightsibling->rightsibling->rightsibling) {
    if ((retcode = interpretParseTreeNode(current->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
      return retcode;
    while (evalBoolean(current->rightsibling->token->token, current->rightsibling->token->valtype)) {
      // eval subtree
      if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
        return retcode;
        
      if ((retcode = interpretParseTreeNode(current->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
        return retcode;
    }
    current->rightsibling->skip = 1;

  } else if (current->token->type == DO_KEYWORD && 
             current->rightsibling && 
             current->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->token->type == BEGIN_WHILE &&
             current->rightsibling->rightsibling->rightsibling) {

    // DO IT NOW
    if ((retcode = interpretParseTreeNode(current->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
      return retcode;

    // EVALUATE THE CONDITION
    if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
      return retcode;

    while (evalBoolean(current->rightsibling->rightsibling->rightsibling->token->token, current->rightsibling->rightsibling->rightsibling->token->valtype)) {
      
      // DO IT NOW
      if ((retcode = interpretParseTreeNode(current->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
        return retcode;

      // EVALUATE THE CONDITION
      if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
        return retcode;

    }
    current->rightsibling->rightsibling->rightsibling->skip = 1;

  } else if (current->token->type == BEGIN_IF && 
             current->rightsibling && 
             current->rightsibling->token->type == EXPRESSION && 
             current->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->token->type == RIGHT_PARENTHESIS &&
             current->rightsibling->rightsibling->rightsibling) {

    if (evalBoolean(current->rightsibling->token->token, current->rightsibling->token->valtype)) {
      // eval subtree
      if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK) {
        return retcode;
      } 
    } else {
      if (current->rightsibling->rightsibling->rightsibling->rightsibling &&
          current->rightsibling->rightsibling->rightsibling->rightsibling->token->type == ELSE_KEYWORD &&
          current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling) {
        // eval subtree
        if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK) {
          return retcode;
        } 
      } else if (current->rightsibling->rightsibling->rightsibling->rightsibling &&
          current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling &&
          current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->token->type == ELSE_KEYWORD &&
          current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling) {
        if ((retcode = interpretParseTreeNode(current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK) {
          return retcode;
        } 
      }
    } 
    
  } else if (current->token->type == LEFT_PARENTHESIS && 
             current->rightsibling && 
             current->rightsibling->token->type == EXPRESSION && 
             current->rightsibling->rightsibling && 
             current->rightsibling->rightsibling->token->type == RIGHT_PARENTHESIS) {
    dest->token->valtype = current->rightsibling->token->valtype;
    dhufree(dest->token->token);
    dest->token->token = dhustrdup(current->rightsibling->token->token);
    
  } else if (current->token->type == ARRAY_IDENTIFIER) {
    dest->token->valtype = current->token->valtype;
    dhufree(dest->token->token);
    vstrdupcat(&(dest->token->token), current->leftchild->token->token, "[", current->leftchild->rightsibling->rightsibling->token->token, "]", NULL);
  } else if (current->token->type == VARIABLE &&
         current->rightsibling != NULL &&
         (current->rightsibling->token->type == EQUALS_OPERATOR ||
         current->rightsibling->token->type == ADD_EQUALS_OPERATOR ||
         current->rightsibling->token->type == SUBTRACT_EQUALS_OPERATOR ||
         current->rightsibling->token->type == MULTIPLY_EQUALS_OPERATOR ||
         current->rightsibling->token->type == DIVIDE_EQUALS_OPERATOR ||
         current->rightsibling->token->type == MOD_EQUALS_OPERATOR) &&
         current->rightsibling->rightsibling != NULL) {

    if (strcmp(current->rightsibling->token->token, "=") == 0) {
      if (current->leftchild->token->type == ARRAY_IDENTIFIER) {
        value = dhustrdup(current->rightsibling->rightsibling->token->token); 
        type = STRING_TYPE;
      } else {
        if ((retcode = getSymbol(current->token->token, &svalue, &type, symbols)) != E_OK) {
          dhufree(oper);
          return retcode;
        }
        value = dhustrdup(current->rightsibling->rightsibling->token->token); 
      }
    } else {
      oper = (char *) dhumalloc(sizeof(char) * (2));
      oper[0] = *(current->rightsibling->token->token);
      oper[1] = CNULL;
      if ((retcode = getSymbol(current->token->token, &svalue, &stype, symbols)) != E_OK) {
        dhufree(oper);
        dhufree(value);
        return retcode;
      }
      if (stype == INT_TYPE) {
        if ((retcode = evalIntegerArithmetic(svalue, oper, current->rightsibling->rightsibling->token->token, &value)) != E_OK) {
          dhufree(oper);
          dhufree(value);
          dhufree(svalue);
          return retcode;
        }
        type = INT_TYPE;
      } else {
        if ((retcode = evalStringArithmetic(svalue, oper, current->rightsibling->rightsibling->token->token, &value, &type)) != E_OK) {
          dhufree(oper);
          dhufree(value);
          dhufree(svalue);
          return retcode;
        }
      }
    }
    if ((retcode = setSymbol(current->token->token, 
                             value,
                             type, 
                             symbols)) != E_OK) {
      dhufree(oper);
      dhufree(value);
      dhufree(svalue);
      return retcode;
    }
    dhufree(oper);
    dhufree(value);
    dhufree(svalue);
  } else if (current->token->type == EXPRESSION &&
             current->rightsibling && current->rightsibling->rightsibling &&
             (current->rightsibling->token->type == ADD_OPERATOR ||
              current->rightsibling->token->type == SUBTRACT_OPERATOR ||
              current->rightsibling->token->type == MULTIPLY_OPERATOR ||
              current->rightsibling->token->type == DIVIDE_OPERATOR ||
              current->rightsibling->token->type == EQUALITY_OPERATOR ||
              current->rightsibling->token->type == INEQUALITY_OPERATOR ||
              current->rightsibling->token->type == LESSTHAN_OPERATOR ||
              current->rightsibling->token->type == GREATERTHAN_OPERATOR ||
              current->rightsibling->token->type == LESSTHANEQUAL_OPERATOR ||
              current->rightsibling->token->type == GREATERTHANEQUAL_OPERATOR ||
              current->rightsibling->token->type == MOD_OPERATOR ||
              current->rightsibling->token->type == BOOLEANAND_OPERATOR ||
              current->rightsibling->token->type == BOOLEANOR_OPERATOR) && 
              current->rightsibling->rightsibling->token->type == EXPRESSION) {
    if (current->token->valtype == STRING_TYPE || current->rightsibling->rightsibling->token->valtype == STRING_TYPE) {
      // string operations
      dhufree(dest->token->token);
      if ((retcode = evalStringArithmetic(current->token->token, 
                                          current->rightsibling->token->token, 
                                          current->rightsibling->rightsibling->token->token, 
                                          &dest->token->token, &dest->token->valtype)) != E_OK) {
        return retcode;
      }
    } else {
      // integer operations
      dest->token->valtype = INT_TYPE;
      dhufree(dest->token->token);
      if ((retcode = evalIntegerArithmetic(current->token->token, 
                                           current->rightsibling->token->token, 
                                           current->rightsibling->rightsibling->token->token, 
                                           &dest->token->token)) != E_OK) {
        return retcode;
      }
    }
  } else if (current->token->type == BEGIN_FUNCTION && current->rightsibling && current->leftchild ) {
    // build the argument list
    list = initArgumentList();
    tmp = current->rightsibling;
    while (tmp && tmp->token->type == DECL_LIST) {
      tmp = tmp->leftchild;
      if (tmp && tmp->token->type == DECL && tmp->leftchild->leftchild && tmp->leftchild->rightsibling) {
        name = tmp->leftchild->rightsibling->token->token;
      
        switch (tmp->leftchild->leftchild->token->type) {
          case STRING_KEYWORD:
            type = STRING_TYPE;
            break;
          case INT_KEYWORD:
            type = INT_TYPE;
            break;
          case ARRAY_KEYWORD:
            type = ARRAY;
            break;
          default:
            type = STRING_TYPE;
            break;
        }

        addArgumentToList(list, "", name, type);
        if (tmp->rightsibling) {
          tmp = tmp->rightsibling->rightsibling;
        }
      }
    }
    if (tmp && tmp->token->type == DECL && tmp->leftchild && tmp->leftchild->leftchild && tmp->leftchild->rightsibling) {
      name = tmp->leftchild->rightsibling->token->token;

      switch (tmp->leftchild->leftchild->token->type) {
        case STRING_KEYWORD:
          type = STRING_TYPE;
          break;
        case INT_KEYWORD:
          type = INT_TYPE;
          break;
        case ARRAY_KEYWORD:
          type = ARRAY;
          break;
        default:
          type = STRING_TYPE;
          break;
      }

      addArgumentToList(list, "", name, type);
    }
    
    // Create a new function from this node
    function = initFunction(current->leftchild->rightsibling->token->token, list, current->rightsibling->rightsibling->rightsibling);

    node = searchMap(function, symbols->functions);
    if (!node) {
      node = initMapNode(function);
      insertMapValue(node, symbols->functions);
    }
  } else if (current->token->type == RETURN_KEYWORD && current->rightsibling) { // RETURN A VALUE
    dest->token->valtype = current->rightsibling->token->valtype;
    dhufree(dest->token->token);
    dest->token->token = dhustrdup(current->rightsibling->token->token);

    symbols->returnValue = initToken(dhustrdup(current->rightsibling->token->token), 
                                     current->rightsibling->token->valtype, 
                                     current->rightsibling->token->line);
    
    return FUNCTIONRETURN;
  } else if (current->token->type == IDENTIFIER && current->rightsibling &&
             current->rightsibling->token->type == LEFT_PARENTHESIS) { // THE FUNCTION PARSER
    tmp = current->rightsibling->rightsibling;

    // build the argument list
    list = initArgumentList();

    while (tmp->token->type == EXPRESSION_LIST) {
      tmp = tmp->leftchild;
      if (tmp->token->type == EXPRESSION) {
        name = UNDEFINED;
        if (tmp->leftchild && tmp->leftchild->token->type == VARIABLE && tmp->leftchild->leftchild) {
          name = tmp->leftchild->leftchild->token->token;
        }

        addArgumentToList(list, tmp->token->token, name, tmp->token->valtype);
      }
      tmp = tmp->rightsibling->rightsibling;
    }
    if (tmp->token->type == EXPRESSION) {
      name = UNDEFINED;
      if (tmp->leftchild && tmp->leftchild->token->type == VARIABLE && tmp->leftchild->leftchild) {
        name = tmp->leftchild->leftchild->token->token;
      }

      addArgumentToList(list, tmp->token->token, name, tmp->token->valtype);
    }

    // call the function
    functionid = getFunctionID(current->token->token);
    if (functionid == -1) {
      // This is a special case - look for the user defined function
      function = initFunction(current->token->token, NULL, NULL);
      node = searchMap(function, symbols->functions);
      freeFunction(function);
      if (node) {
        function = (Function *) node->ele;

        if (list->size != function->args->size) {
          freeArgumentList(list);
          return INVALIDNUMARGS;
        }

        arg = list->head;
        argdef = function->args->head;

        increaseScope(symbols);
        while ((arg != NULL) && (argdef != NULL)) {
          retcode = forceSymbol(argdef->name, arg->value, argdef->type, symbols);
          arg = arg->next;
          argdef = argdef->next;
        }

        //printParseTree(function->ptree, 5);
        retcode = interpretParseTreeNode(function->ptree, symbols, output, 0, line, env, sqlsock);
        if (retcode != E_OK && retcode != FUNCTIONRETURN ) {
          return retcode;
        } 
        decreaseScope(symbols);
        freeArgumentList(list);
        if (symbols->returnValue) {
          dest->token->valtype = symbols->returnValue->valtype;
          dhufree(dest->token->token);
          dest->token->token = symbols->returnValue->token;
          symbols->returnValue->token = NULL;
          freeToken(symbols->returnValue);
          symbols->returnValue = NULL;
        }
        return E_OK;
      } else {
        freeArgumentList(list);
        return FUNCTIONUNDEFINED;
      }
    }
    numargs = getFunctionNumArgs(functionid);

    if (numargs > 0) {
      if (numargs != list->size) {
        freeArgumentList(list);
        return INVALIDNUMARGS;
      }
    }

    // return the result
    if ((retcode = evalFunction(functionid, list, output, &(dest->token->valtype), &(dest->token->token), env, sqlsock, symbols)) != E_OK) {
      freeArgumentList(list);
      return retcode;
    }

    freeArgumentList(list);


  } else if (current->token->type == VARIABLE) { 
    if ((retcode = getSymbol(current->token->token, &(dest->token->token), &(dest->token->valtype), symbols)) != E_OK) {
      return retcode;
    }
  } else {
    dest->token->valtype = current->token->valtype;
    dhufree(dest->token->token);
    dest->token->token = dhustrdup(current->token->token);
  } 
  
  return E_OK;
}

/*********************************************************************
* interpretParseTree... 
* 
* Take a single node of a fully formed parse tree and actually run it.
*********************************************************************/
int interpretParseTreeNode(ParseTree *current, SymbolTable *symbols, char **output, int leftmostchild, int *line, Env *env, void *sqlsock) {
  int retcode = 0;

  if (current == NULL)
    return RESOURCEERROR;

  if (current->token)
    *line = current->token->line;

  if (current->leftchild) 
    if ((retcode = interpretParseTree(current->leftchild, symbols, output, 1, line, env, sqlsock)) != E_OK)
      return retcode;

  if (current->leftchild)
    if ((retcode = evalExpression(current->leftchild, current, symbols, output, line, env, sqlsock)) != E_OK)
      return retcode;

  return E_OK;
}

/*********************************************************************
* interpretParseTree... 
* 
* Take a fully formed parse tree and actually run it.
*********************************************************************/
int interpretParseTree(ParseTree *current, SymbolTable *symbols, char **output, int leftmostchild, int *line, Env *env, void *sqlsock) {
  int retcode = 0;

  if (current == NULL)
    return RESOURCEERROR;

  if (current->token)
    *line = current->token->line;

  if (current->skip)
    return E_OK;

  // certain execution paths should not be run until later.
  if (current->token->type == BEGIN_IF) {
    current->rightsibling->rightsibling->rightsibling->skip = 1;
    if (current->rightsibling->rightsibling->rightsibling->rightsibling &&
        current->rightsibling->rightsibling->rightsibling->rightsibling->token->type == ELSE_KEYWORD ) {
      current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->skip = 1;
    }
  }
  if (current->token->type == BEGIN_FOR) {
    current->rightsibling->skip = 1;
    current->rightsibling->rightsibling->rightsibling->skip = 1;
    current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->skip = 1;
    current->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->rightsibling->skip = 1;
  }
  if (current->token->type == BEGIN_FUNCTION) {
    current->rightsibling->rightsibling->rightsibling->skip = 1;
  }

  if (current->leftchild) 
    if ((retcode = interpretParseTree(current->leftchild, symbols, output, 1, line, env, sqlsock)) != E_OK)
      return retcode;

  if (current->rightsibling)
    if ((retcode = interpretParseTree(current->rightsibling, symbols, output, 0, line, env, sqlsock)) != E_OK)
      return retcode;

  if (current->leftchild)
    if ((retcode = evalExpression(current->leftchild, current, symbols, output, line, env, sqlsock)) != E_OK)
      return retcode;

  return E_OK;
}

/*********************************************************************
* countParseTree...
*
* Walk through the binary tree - counting nodes and setting indexes
*********************************************************************/
void countParseTree(ParseTree *node, int *count) {
  if (node == NULL || count == NULL)
    return;

  node->node = (*count)++;
  countParseTree(node->leftchild, count);
  countParseTree(node->rightsibling, count);
}

#define TOKEN_SEP         "TOKEN:"
#define TYPE_SEP          "TYPE:"
#define LINE_SEP          "LINE:"
#define LEFTCHILD_SEP     "LEFT:"
#define RIGHTSIBLING_SEP  "RIGHT:"

/*********************************************************************
* countParseNodeLen...
*
* Recursive length routine.
*********************************************************************/
int countParseNodeLen(ParseTree *node, int *len) {
  int retcode = 0;
  if (node == NULL)
    return E_OK;

  *len += strlen(TOKEN_SEP) + 8 + strlen(node->token->token) + strlen(TYPE_SEP) + 32 + strlen(LINE_SEP) + 32 + strlen(LEFTCHILD_SEP) + 32 + strlen(RIGHTSIBLING_SEP) + 32;
 
  if ((retcode = countParseNodeLen(node->leftchild, len)) != E_OK) {
    return retcode;
  }
  if ((retcode = countParseNodeLen(node->rightsibling, len)) != E_OK) {
    return retcode;
  }

  return E_OK;
}

/*********************************************************************
* serialiseParseNode...
*
* Recursive serialise routine.
*********************************************************************/
int serialiseParseNode(ParseTree *node, int fd) {
  int b = 0;
  int retcode = 0;
  if (node == NULL)
    return E_OK;

  // send the length of the token
  b = strlen(node->token->token); 
  writeFileData(&b, sizeof(int), fd);

  // send the token value
  writeFileData(node->token->token, sizeof(char) * b, fd);
  
  // type is one int
  b = node->token->type;
  writeFileData(&b, sizeof(int), fd);

  // line is one int
  b = node->token->line;
  writeFileData(&b, sizeof(int), fd);
  
  // left child
  if (node->leftchild)
    b = node->leftchild->node;
  else
    b = -1;
  writeFileData(&b, sizeof(int), fd);
  
  // right sibling
  if (node->rightsibling)
    b = node->rightsibling->node;
  else
    b = -1;
  writeFileData(&b, sizeof(int), fd);
  
  if ((retcode = serialiseParseNode(node->leftchild, fd)) != E_OK) {
    return retcode;
  }
  if ((retcode = serialiseParseNode(node->rightsibling, fd)) != E_OK) {
    return retcode;
  }

  return E_OK;
}

/*********************************************************************
* serialiseParseTree...
*
* Convert this structure into a form that can be dumped to disk.
*********************************************************************/
int serialiseParseTree(ParseTree *ptree, int fd) {
  int count = 0;
  int id = SCRIPT_FILE_HEADER;
  // We need to write a list of these records 
  // TOKEN:[256]TYPE:[32]LINE:[32]LEFT:[32]RIGHT:[32]

  // We can do better
  // first - write a header (USED TO VALIDATE THIS IS A VALID SCRIPT NODE)
  writeFileData(&id, sizeof(int), fd);
  

  // Next send the total number of nodes in the tree
  countParseTree(ptree, &count);
  writeFileData(&count, sizeof(int), fd);

  // Now send the list of nodes
  serialiseParseNode(ptree, fd);

  return E_OK;
}

/*********************************************************************
* unserialiseParseTree...
*
* Take this stream and convert it inot the internal representation 
* of a parse tree.
*********************************************************************/
int unserialiseParseTree(int fd, ParseTree **ptree, TokenList **tokens) {
  int count = 0, i = 0, id = 0;
  char *token = NULL;
  ParseTree **ptrs = NULL;
  TokenList *toks = NULL;
  int type = 0, line = 0, left = 0, right = 0, len = 0;
  int err = 0;

  if ((err = readFileData(&id, sizeof(int), fd)) != E_OK) {
    return err;
  }

  if (id != SCRIPT_FILE_HEADER) {
    return FILEREADERROR;
  }

  // Read the number of rows
  if ((err = readFileData(&count, sizeof(int), fd)) != E_OK) {
    return err;
  }

  ptrs = (ParseTree **) dhumalloc(sizeof(ParseTree *) * (count));
  toks = initTokenList();
  for (i = 0; i < count; i++) {
    // Read the length of the token
    if ((err = readFileData(&len, sizeof(int), fd)) != E_OK) {
      return err;
    }
    // Read the token
    token = (char *) dhumalloc(sizeof(char) * len + 1);
    if ((err = readFileData(token, sizeof(char) * len, fd)) != E_OK) {
      return err;
    }
    token[len] = CNULL;

    // Read the type
    if ((err = readFileData(&type, sizeof(int), fd)) != E_OK) {
      return err;
    }

    // Read the line
    if ((err = readFileData(&line, sizeof(int), fd)) != E_OK) {
      return err;
    }

    // Read the left
    if ((err = readFileData(&left, sizeof(int), fd)) != E_OK) {
      return err;
    }

    // Read the right
    if ((err = readFileData(&right, sizeof(int), fd)) != E_OK) {
      return err;
    }

    addTokenToList(toks, token, type, line, 1);
    dhufree(token);
    ptrs[i] = initParseTree(toks->tail);
    ptrs[i]->leftc = left;
    ptrs[i]->rights = right;
  }
  for (i = 0; i < count; i++) {
    if (ptrs[i]->leftc >= 0)
      ptrs[i]->leftchild = ptrs[ptrs[i]->leftc];
    if (ptrs[i]->rights >= 0)
      ptrs[i]->rightsibling = ptrs[ptrs[i]->rights];
  }
  *ptree = ptrs[0];
  dhufree(ptrs);
  
  *tokens = toks;
  return E_OK;
}

/*********************************************************************
* loadParseTree...
*
* Load the parse tree for this section from disk.
*********************************************************************/
int loadParseTree(int section, int objectid, ParseTree **ptree, TokenList **tokens) {
  char *filepath = NULL;
  int retcode = 0;
  int fd = 0;

  if ((retcode = generateScriptPath(objectid, section, &filepath)) != E_OK) {
    return retcode;
  }

  if ((fd = open(filepath, O_RDONLY|O_BINARY)) == -1) {
    return FILEREADERROR;
  }

#ifdef WIN32
  _setmode( fd, _O_BINARY );
#endif

  if ((retcode = unserialiseParseTree(fd, ptree, tokens)) != E_OK) {
    dhufree(filepath);
    return retcode;
  }

  dhufree(filepath);
  close(fd);

  return E_OK;
}

/*********************************************************************
* writeParseTree...
*
* Write the parse tree for this section to disk.
*********************************************************************/
int writeParseTree(int section, int objectid, ParseTree *ptree) {
  char *filecontents = NULL, *filepath = NULL;
  int retcode = 0;
  int fd = 0;

  if ((retcode = generateScriptPath(objectid, section, &filepath)) != E_OK) {
    return retcode;
  }

  if ((fd = open(filepath, O_RDWR|O_CREAT|O_BINARY)) == -1) {
    dhufree(filepath);
    return FILEWRITEERROR; 
  }

#ifdef WIN32
  _setmode( fd, _O_BINARY );
#endif

  if ((retcode = serialiseParseTree(ptree, fd)) != E_OK) {
    dhufree(filepath);
    return retcode;
  }

  close(fd);

  dhufree(filepath);
  dhufree(filecontents);
    return retcode;
  return E_OK;
}

/*********************************************************************
* runScript...
*
* This runs the script specifed by script and
* returns the output in output.
* It generally will not return errors but embed them
* in the output.
*
* This involves these steps.
* 1) tokenise the stream.
* 2) build a parse tree from the tokens.
* 3) run a semantic checker on the parse tree.
* 4) step through the parse tree executing each statement.
*
* step 4 will require a symbol table.
*********************************************************************/
int runScript(char *script, char **output, int section, int objectid, Env *env, void *sqlsock, SymbolTable *symbols) {
  TokenList *tokens = NULL;
  ParseTree *ptree = NULL;
  ObjectDetails *details = NULL;
  int retcode = 0, line = 1;
  char *buf = NULL;

  
  // LOAD THE PARSE TREE IF IT HAS BEEN COMPILED BEFORE
  if (loadParseTree(section, objectid, &ptree, &tokens) != E_OK) {

    retcode = lexer(script, output, &tokens);
    if (retcode != E_OK) {
      return E_OK;
    }

    retcode = buildParseTree(tokens, output, &ptree);
    if (retcode != E_OK) {
      freeTokenList(tokens);
      return E_OK;
    }

    writeParseTree(section, objectid, ptree);
  }

  if (symbols == NULL) {
    retcode = RESOURCEERROR;
  } else {
    line = 1;

    
    if ((retcode = getObjectDetails(objectid, &details, sqlsock)) != E_OK) {
      vstrdupcat(output, "[INTERPRETER ERROR:", 
                         getErrorMesg(retcode), 
                         "]\n",
                         NULL);
      return E_OK;
    }

    retcode = forceSymbol(dhustrdup("this"), dhustrdup(details->path), STRING_TYPE, symbols);
    
    retcode = interpretParseTree(ptree, symbols, output, 1, &line, env, sqlsock);

    if (retcode != E_OK) {
      int2Str(line, &buf);
      vstrdupcat(output, "[INTERPRETER ERROR:", 
                         getErrorMesg(retcode), 
                         " LINE:", buf, ", FILE:", details->path, "]\n",
                         NULL);
      dhufree(buf);
      freeObjectDetails(details);
      return E_OK;
    }
    freeObjectDetails(details);
  }
 
  freeTokenList(tokens);
  freeParseTree(ptree);
  
  return retcode;
}
