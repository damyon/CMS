/*******************************************************************************
* rules.h...
*
* Defines all the reduction rules for the parser.
*******************************************************************************/

#ifndef _RULES_H
#define _RULES_H

#ifdef __cplusplus
extern "C" {
#endif

/**** structs ****/
enum RepeatValues {
                    ONEONLY,
                    ONEORMORE,
                    ZEROORMORE,
                    ZEROORONE 
                  };

typedef struct _RulePhrase_ {
  int repeats;
  int reducetype;
  char *rule;
} RulePhrase;

/*********************************************************************
* printReductionRules...
*
* Print the full list of reduction rules to stdout.
*********************************************************************/
void printReductionRules(Map *rules);

/*********************************************************************
* buildStackStr...
*
* Builds a regexable string representation of the stack.
*********************************************************************/
char *buildStackStr(Stack *stack, int pos);

/*********************************************************************
* initRulePhrase...
*
* Create an empty rule phrase.
*********************************************************************/
RulePhrase *initRulePhrase(int reducetype, int repeats, char *rule);

/*********************************************************************
* freeRulePhrase...
*
* Release the memory for this rule phrase.
*********************************************************************/
void freeRulePhrase(void *thephrase); 

/*********************************************************************
* cmpRulePhrase...
*
* compare two reduction rules.
*********************************************************************/
int cmpRulePhrase(void *ptra, void *ptrb);

/*********************************************************************
* initReductionRules...
*
* Return a structure that represents our entire set of reduction rules.
*********************************************************************/
int initReductionRules(Map *rules);

/*********************************************************************
* matchRule...
*   
* Search for a match to this rule set on the stack.
*********************************************************************/
int matchRule(RulePhrase *rule, Stack *stack, int *start, int *end);

/*********************************************************************
* matchPartialRule...
*
* Does this stack partially match the rule set? - ie
* could it match a different rule if there were more tokens.
*********************************************************************/
int matchPartialRule(RulePhrase *rule, Stack *stack);

/*********************************************************************
* printStack...
*
* Prints the contents of each token on the stack.
*********************************************************************/
void printStack(Stack *stack);

/*********************************************************************
* getTokenName...
*
* Get the name of this token type.
*********************************************************************/
char *getTokenName(int i);

/*********************************************************************
* printRule...
*
* For debugging - print this rule to stdout.
*********************************************************************/
void printRule(RulePhrase *rule);

#ifdef __cplusplus
}
#endif


#endif // _RULES_H
