#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include "win32.h"
#else
#include <unistd.h>
#endif
#include <ctype.h>
#include "strings.h"
#include "errors.h"
#include "structs.h"
#include "rules.h"
#include "logging.h"
#include "malloc.h"

const char *TOKEN_NAMES[] = { 
                              "",                               // META CHARACTERS
                              "<start>",                        // 1
                              "<identifier>",                   // 8
                              "[",                              // 9
                              "]",                              // 10
                              "<string>",                       // 11
                              "<int>",                          // 12
                              "=",                              // 13
                              "*=",                             // 14
                              "/=",                             // 15
                              "%=",                             // 16
                              "+=",                             // 17
                              "-=",                             // 18
                              "+",                              // 19
                              "-",                              // 20
                              "/",                              // 21
                              "*",                              // 22
                              "%",                              // 23
                              "string",                         // 24
                              "int",                            // 25
                              ";",                              // 26
                              "(",                              // 28
                              ")",                              // 29
                              ",",                              // 30
                              "if",                             // 32
                              "else",                           // 33
                              "==",                             // 37
                              "!=",                             // 38
                              "<",                              // 39
                              "<=",                             // 40
                              ">",                              // 41
                              ">=",                             // 44
                              "{",                              // 45
                              "}",                              // 47
                              "while",                          // 48
                              "do",                          // 48
                              "||",                             // 50
                              "&&",                             // 51
                              "comment",                        // 52
                              "array",                          // 53
                              "for",                            // 55
                              "<array>",
                              "<bof-token>",                    
                              "<eof-token>",                    
                              "<type>",
                              "<expression>",
                              "<assignment-operator>",
                              "<constant>",
                              "<array-identifier>",
                              "<arithmetic-operator>",
                              "<expression-list>",
                              "<compound-statement>",
                              "<variable>",
                              "<begin-if>",
                              "<begin-for>",
                              "<begin-while>",
                              "<decl>",
                              "<decl-list>",
                              "function",
                              "return",
                              "<begin-function>",
                              "void"
                            };

/*********************************************************************
* initRulePhrase...
*
* Create an empty rule phrase.
*********************************************************************/
RulePhrase *initRulePhrase(int reducetype, int repeats, char *rule) {
  RulePhrase *phrase = NULL;

  phrase = (RulePhrase *) dhumalloc(sizeof(RulePhrase));
  if (phrase == NULL)
    return NULL;

  phrase->repeats = repeats;
  phrase->rule = rule;
  phrase->reducetype = reducetype;
  return phrase;
}

/*********************************************************************
* freeRulePhrase...
*
* Release the memory for this rule phrase.
*********************************************************************/
void freeRulePhrase(void *thephrase) {
  RulePhrase *phrase = (RulePhrase *) thephrase;

  if (phrase) {
    if (phrase->rule != NULL) {
      dhufree(phrase->rule);
    }
    dhufree(phrase);
  }
}

/*********************************************************************
* cmpRulePhrase...
*
* compare two reduction rules.
*********************************************************************/
int cmpRulePhrase(void *ptra, void *ptrb) {
  RulePhrase *a = (RulePhrase *) ptra,
             *b = (RulePhrase *) ptrb;
  int i = 0, lena = strlen(a->rule), lenb = strlen(b->rule), cmp = 0;

  //printf("cmpRulePhrase(");
 // printRule(a);
 // printf("==");
 // printRule(b);
 // printf(")\n");
  for (i = 0; i < lena && i < lenb; i++) {
    cmp = a->rule[i] - b->rule[i];
    if (cmp != 0)
      return cmp;
  }

  return lenb - lena;
}

/*********************************************************************
* printReductionRules...
*
* Print the full list of reduction rules to stdout.
*********************************************************************/
void printReductionRules(Map *rules) {
  MapNode *node = NULL;
  RulePhrase *rule = NULL;

  node = getFirstMapNode(rules);

  printf("Reduction Rules...\n");
  while (node != NULL) {
    rule = (RulePhrase *) node->ele;
    printRule(rule);
    printf("\n");
    node = getNextMapNode(node, rules);
  }

}

/*********************************************************************
* initReductionRules...
*
* Return a structure that represents our entire set of reduction rules.
*********************************************************************/
int initReductionRules(Map *rules) {
  RulePhrase *phrase = NULL; 
  MapNode *node = NULL;
  char *source = NULL;
  
  if (!rules)
    return RESOURCEERROR;

  // <start> ::= <bof_token> 
  source = concatChars(1, BOF_TOKEN);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <start> ::= <eof_token>
  source = concatChars(1, EOF_TOKEN);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <begin-function> ::= function <identifier> 
  source = concatChars(3, FUNCTION_KEYWORD, IDENTIFIER, LEFT_PARENTHESIS);
  phrase = initRulePhrase(BEGIN_FUNCTION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-function> <decl-list> ) <start>
  source = concatChars(4, BEGIN_FUNCTION, DECL_LIST, RIGHT_PARENTHESIS, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <start> ::= <begin-function> void ) <start>
  source = concatChars(4, BEGIN_FUNCTION, VOID_KEYWORD, RIGHT_PARENTHESIS, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <start> ::= <begin-function> <decl> ) <start>
  source = concatChars(4, BEGIN_FUNCTION, DECL, RIGHT_PARENTHESIS, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-function> ) <start>
  source = concatChars(3, BEGIN_FUNCTION, RIGHT_PARENTHESIS, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= return <expression>
  source = concatChars(2, RETURN_KEYWORD, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <start> <expression> ;
  source = concatChars(3, START, EXPRESSION, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <start> ::= <decl> ; 
  source = concatChars(2, DECL, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <start> ::= <start> <start>
  source = concatChars(2, START, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <variable> ::= <identifier>
  source = concatChars(1, IDENTIFIER);
  phrase = initRulePhrase(VARIABLE, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <expression> ::= <variable>
  source = concatChars(1, VARIABLE);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <decl> ::= <type> <identifier>
  source = concatChars(2, TYPE, IDENTIFIER);
  phrase = initRulePhrase(DECL, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <decl-list> ::= <decl> , <decl>
  source = concatChars(3, DECL, COMMA_SEPARATOR, DECL);
  phrase = initRulePhrase(DECL_LIST, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <decl-list> ::= <decl> , <decl-list>
  source = concatChars(3, DECL, COMMA_SEPARATOR, DECL_LIST);
  phrase = initRulePhrase(DECL_LIST, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <type> ::= <string-keyword>
  source = concatChars(1, STRING_KEYWORD);
  phrase = initRulePhrase(TYPE, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <type> ::= <array-keyword>
  source = concatChars(1, ARRAY_KEYWORD);
  phrase = initRulePhrase(TYPE, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <type> ::= <int-keyword>
  source = concatChars(1, INT_KEYWORD);
  phrase = initRulePhrase(TYPE, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <expression> ::= <variable> = <expression>
  source = concatChars(3, VARIABLE, EQUALS_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <variable> *= <expression>
  source = concatChars(3, VARIABLE, MULTIPLY_EQUALS_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <expression> ::= <variable> /= <expression>
  source = concatChars(3, VARIABLE, DIVIDE_EQUALS_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <expression> ::= <variable> %= <expression>
  source = concatChars(3, VARIABLE, MOD_EQUALS_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <variable> += <expression>
  source = concatChars(3, VARIABLE, ADD_EQUALS_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <variable> -= <expression>
  source = concatChars(3, VARIABLE, SUBTRACT_EQUALS_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <constant>
  source = concatChars(1, CONSTANT);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <constant> ::= <string>
  source = concatChars(1, STRING_TYPE);
  phrase = initRulePhrase(CONSTANT, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <constant> ::= <int>
  source = concatChars(1, INT_TYPE);
  phrase = initRulePhrase(CONSTANT, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <expression> ::= <identifier> ( )
  source = concatChars(3, IDENTIFIER, LEFT_PARENTHESIS, RIGHT_PARENTHESIS);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <identifier> ( <expression> )
  source = concatChars(4, IDENTIFIER, LEFT_PARENTHESIS, EXPRESSION, RIGHT_PARENTHESIS);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <expression> ::= <identifier> ( <expression_list> )
  source = concatChars(4, IDENTIFIER, LEFT_PARENTHESIS, EXPRESSION_LIST, RIGHT_PARENTHESIS);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <expression-list> ::= <expression> , <expression>
  source = concatChars(3, EXPRESSION, COMMA_SEPARATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION_LIST, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <expression-list> ::= <expression> , <expression_list>
  source = concatChars(3, EXPRESSION, COMMA_SEPARATOR, EXPRESSION_LIST);
  phrase = initRulePhrase(EXPRESSION_LIST, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <array-identifier> ::= <identifier> [ <expression> ] 
  source = concatChars(4, IDENTIFIER, LEFT_SQUARE_PARENTHESIS, EXPRESSION, RIGHT_SQUARE_PARENTHESIS);
  phrase = initRulePhrase(ARRAY_IDENTIFIER, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <variable> ::= <array-identifier>
  source = concatChars(1, ARRAY_IDENTIFIER);
  phrase = initRulePhrase(VARIABLE, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> != <expression>
  source = concatChars(3, EXPRESSION, INEQUALITY_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> == <expression>
  source = concatChars(3, EXPRESSION, EQUALITY_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> + <expression>
  source = concatChars(3, EXPRESSION, ADD_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> - <expression>
  source = concatChars(3, EXPRESSION, SUBTRACT_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> * <expression>
  source = concatChars(3, EXPRESSION, MULTIPLY_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> / <expression>
  source = concatChars(3, EXPRESSION, DIVIDE_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> % <expression>
  source = concatChars(3, EXPRESSION, MOD_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> && <expression>
  source = concatChars(3, EXPRESSION, BOOLEANAND_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> || <expression>
  source = concatChars(3, EXPRESSION, BOOLEANOR_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> < <expression>
  source = concatChars(3, EXPRESSION, LESSTHAN_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> <= <expression>
  source = concatChars(3, EXPRESSION, LESSTHANEQUAL_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> > <expression>
  source = concatChars(3, EXPRESSION, GREATERTHAN_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <expression> ::= <expression> >= <expression>
  source = concatChars(3, EXPRESSION, GREATERTHANEQUAL_OPERATOR, EXPRESSION);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // grouping
  source = concatChars(3, LEFT_PARENTHESIS, EXPRESSION, RIGHT_PARENTHESIS);
  phrase = initRulePhrase(EXPRESSION, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
 
  // code blocks
  source = concatChars(2, LEFT_SCOPE, START);
  phrase = initRulePhrase(COMPOUND_STATEMENT, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // code blocks
  source = concatChars(3, LEFT_SCOPE, EXPRESSION, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(COMPOUND_STATEMENT, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // code blocks
  source = concatChars(2, LEFT_SCOPE, EXPRESSION);
  phrase = initRulePhrase(COMPOUND_STATEMENT, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // code blocks
  source = concatChars(2, COMPOUND_STATEMENT, EXPRESSION);
  phrase = initRulePhrase(COMPOUND_STATEMENT, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // code blocks
  source = concatChars(3, COMPOUND_STATEMENT, EXPRESSION, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(COMPOUND_STATEMENT, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // code blocks
  source = concatChars(2, COMPOUND_STATEMENT, START);
  phrase = initRulePhrase(COMPOUND_STATEMENT, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // code blocks
  source = concatChars(2, COMPOUND_STATEMENT, RIGHT_SCOPE);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // for loop
  source = concatChars(2, FOR_KEYWORD, LEFT_PARENTHESIS);
  phrase = initRulePhrase(BEGIN_FOR, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <begin-while> ::= while (
  source = concatChars(2, WHILE_KEYWORD, LEFT_PARENTHESIS);
  phrase = initRulePhrase(BEGIN_WHILE, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <begin-if> ::= if (
  source = concatChars(2, IF_KEYWORD, LEFT_PARENTHESIS);
  phrase = initRulePhrase(BEGIN_IF, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-if> <expression> ) <expression> ;
  source = concatChars(5, BEGIN_IF, EXPRESSION, RIGHT_PARENTHESIS, EXPRESSION, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);
  
  // <start> ::= <begin-if> <expression> ) <start>
  source = concatChars(4, BEGIN_IF, EXPRESSION, RIGHT_PARENTHESIS, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-if> <expression> ) <start> else <expression> ;
  source = concatChars(7, BEGIN_IF, EXPRESSION, RIGHT_PARENTHESIS, START, ELSE_KEYWORD, EXPRESSION, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-if> <expression> ) <start> else <start>
  source = concatChars(6, BEGIN_IF, EXPRESSION, RIGHT_PARENTHESIS, START, ELSE_KEYWORD, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-if> <expression> ) <expression> ; else <expression> ;
  source = concatChars(8, BEGIN_IF, EXPRESSION, RIGHT_PARENTHESIS, EXPRESSION, SEMICOLON_OPERATOR, ELSE_KEYWORD, EXPRESSION, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-if> <expression> ) <expression> ; else <start>
  source = concatChars(7, BEGIN_IF, EXPRESSION, RIGHT_PARENTHESIS, EXPRESSION, SEMICOLON_OPERATOR, ELSE_KEYWORD, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-while> <expression> ) <expression> ;
  source = concatChars(5, BEGIN_WHILE, EXPRESSION, RIGHT_PARENTHESIS, EXPRESSION, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-while> <expression> ) <start> 
  source = concatChars(4, BEGIN_WHILE, EXPRESSION, RIGHT_PARENTHESIS, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <do-keyword> <start> <begin-while> <expression> ) ;
  source = concatChars(6, DO_KEYWORD, START, BEGIN_WHILE, EXPRESSION, RIGHT_PARENTHESIS, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <do-keyword> <expression> ; <begin-while> <expression> ) ;
  source = concatChars(7, DO_KEYWORD, EXPRESSION, SEMICOLON_OPERATOR, BEGIN_WHILE, EXPRESSION, RIGHT_PARENTHESIS, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-for> <expression> ; <expression> ; <expression> ) <expression> ;
  source = concatChars(9, BEGIN_FOR, EXPRESSION, SEMICOLON_OPERATOR, EXPRESSION, SEMICOLON_OPERATOR, EXPRESSION, RIGHT_PARENTHESIS, EXPRESSION, SEMICOLON_OPERATOR);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  // <start> ::= <begin-for> <expression> ; <expression> ; <expression> ) <start>
  source = concatChars(8, BEGIN_FOR, EXPRESSION, SEMICOLON_OPERATOR, EXPRESSION, SEMICOLON_OPERATOR, EXPRESSION, RIGHT_PARENTHESIS, START);
  phrase = initRulePhrase(START, ONEONLY, source);
  node = initMapNode(phrase);
  insertMapValue(node, rules);

  return E_OK;
}

/*********************************************************************
* buildStackStr...
*
* Builds a regexable string representation of the stack.
*********************************************************************/
char *buildStackStr(Stack *stack, int pos) {
  char *ptr = NULL;
  int i = 0;

  if ((stack == NULL) || (stack->size == 0) || (pos >= stack->size))
    return NULL;

  ptr = (char *) dhumalloc(sizeof(char) * (stack->size - pos + 4)); // We include room to grow
  if (ptr == NULL)
    return NULL;

  for (i = pos; i < stack->size; i++) {
    ptr[i - pos] = (char)((Token *)stack->ele[i])->type;
  }  
  ptr[i - pos] = CNULL;

  return ptr;
}

/*********************************************************************
* printOOBData...
*
* This string does not contain printable characters so translate, then print it.
*********************************************************************/
void printOOBString(char *str) {
  char *ptr = NULL;

  if (str == NULL) {
    printf("NULL");
    return;
  }

  ptr = str;
  while (*ptr != CNULL) printf(" %s", TOKEN_NAMES[(int)(*(ptr++))]); 
  printf(" ");
}

/*********************************************************************
* matchPhrase...
*
* Compares the types in the phrase to the types on the stack.
*********************************************************************/
int matchPhrase(RulePhrase *phrase, Stack *stack, int pos, int *end) {
  char *stackstr = NULL;
  char *prule = NULL;
  char *rstart = NULL, *rend = NULL;
  int i = 0;

  if (phrase == NULL || stack == NULL || (stack->size == 0))
    return RESOURCEERROR;

  stackstr = buildStackStr(stack, pos);
  if (stackstr == NULL)
    return RESOURCEERROR;
  
  //printf("matchPhrase(");
  //printOOBString(phrase->rule);
  //printf(", ");
  //printOOBString(stackstr);
  //printf(")\n");
  prule = dhustrdup(phrase->rule);
  if (prule == NULL)
    return RESOURCEERROR;

  for (i = strlen(prule) - 1; i > 0; i--) {
    prule[i] = CNULL;
    if ((miniRegex(stackstr, prule, &rstart, &rend)) == E_OK) {
      if (pos + (rend - rstart) == (stack->size)) {
        return PARTIALMATCH;
      }
    }
  }

  if ((miniRegex(stackstr, phrase->rule, &rstart, &rend)) == E_OK) {
    if ((rstart == stackstr) && (rstart != rend)) {
      *end = pos + (rend - rstart);
      dhufree(stackstr);
      return E_OK;
    }
  }


  dhufree(stackstr);
  return NOMATCH;
}

/*********************************************************************
* applyRuleToStack...
*
* Does this rule apply to this point in the stack (Recursive)
*********************************************************************/
int applyRuleToStack(RulePhrase *phrase, Stack *stack, int pos, int *end, int flag) {
  int ret = 0;

  // BASE CASE
  if (phrase == NULL) {
    return E_OK;
  }
  //printf("applyRuleToStack(");
  //printRule(phrase);
 // printf(", %d)\n", pos);
 // printStack(stack);

  if (pos >= stack->size) {
    //printf("return PARTIALMATCH;\n");
    return PARTIALMATCH;
  }

  ret = matchPhrase(phrase, stack, pos, end);

  return ret;
}

/*********************************************************************
* printRule...
*
* For debugging - print this rule to stdout.
*********************************************************************/
void printRule(RulePhrase *rule) {
  unsigned int i = 0;
  switch (rule->repeats) {
    case ZEROORMORE:
      printf(" { ");
      for (i = 0; i < strlen(rule->rule); i++) {
        printf("%s", TOKEN_NAMES[(int)rule->rule[i]]);
      }
      printf(" }* ");
      break;
    case ZEROORONE:
      printf(" { ");
      for (i = 0; i < strlen(rule->rule); i++) {
        printf("%s", TOKEN_NAMES[(int)rule->rule[i]]);
      }
      printf(" }? ");
      break;
    case ONEORMORE:
      printf(" { ");
      for (i = 0; i < strlen(rule->rule); i++) {
        printf("%s", TOKEN_NAMES[(int)rule->rule[i]]);
      }
      printf(" }+ ");
      break;
    case ONEONLY:
      printf(" ");
      for (i = 0; i < strlen(rule->rule); i++) {
        printf("%s", TOKEN_NAMES[(int)rule->rule[i]]);
      }
      printf(" ");
      break;
  }

  printf(" ==> %s", TOKEN_NAMES[rule->reducetype]);
}

/*********************************************************************
* matchPartialRule...
*
* Does this stack partially match the rule set? - ie
* could it match a different rule if there were more tokens.
*********************************************************************/
int matchPartialRule(RulePhrase *rule, Stack *stack) {
  int pos = 0, send = 0, start = 0, end = 0;

  if (rule == NULL || stack == NULL || (stack->size == 0))
    return RESOURCEERROR;

  while (pos != stack->size) {
    start = pos;
    send = end;
    if ((applyRuleToStack(rule, stack, pos, &end, 0) == PARTIALMATCH)) {
      return PARTIALMATCH;
    }
    end = send;
    pos++;
  }
  
  return NOMATCH;
}

/*********************************************************************
* matchRule...
*
* Search for a match to this rule set on the stack.
*********************************************************************/
int matchRule(RulePhrase *rule, Stack *stack, int *start, int *end) {
  int pos = 0, send = 0;

  if (rule == NULL || stack == NULL || (stack->size == 0))
    return RESOURCEERROR;

  pos = stack->size - 1;
  while (pos >= 0) {
    *start = pos;
    send = *end;
    if ((applyRuleToStack(rule, stack, pos, end, 0) == E_OK) && (*start != *end)) {
      return E_OK;
    }
    *end = send;
    pos--;
  }
  
  return NOMATCH;
}

/*********************************************************************
* getTokenName...
*
* Get the name of this token type.
*********************************************************************/
char *getTokenName(int i) {
  return (char *)(TOKEN_NAMES[i]);
}

/*********************************************************************
* printStack...
*
* Prints the contents of each token on the stack.
*********************************************************************/
void printStack(Stack *stack) {
  int i = 0;
  Token *token = NULL;

  //return;
  if (stack == NULL)
    return;

  printf("---------DUMP-STACK-------------------------------\n\n");
  for (i = 0; i < stack->size; i++) {
    token = (Token *) stack->ele[i];
    printf("%s\t\t%s\n", TOKEN_NAMES[token->type], token->token);
  }
  printf("\n\n");
}

