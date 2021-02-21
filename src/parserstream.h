#ifndef _PARSESTREAM_H_
#define _PARSESTREAM_H_

/* Standard #defines */
#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NodeInfo_ {
	ParseEnum parseEnum;
	int stringLength;
	int hasChild;
	int hasSibling;
} NodeInfo;

void treeCount(ParseTree *tree, int *pnumNodes, int *pstringsLength);
void saveParseTreeNode(ParseTree *tree, NodeInfo *niblock, int *pnCount, char **sblockaddress);
int saveParseTree(ParseTree *tree, char *filename);
ParseTree *initNode(NodeInfo *niblock, int *pnCount, char **sblockaddress);
void loadParseTreeNode(ParseTree *tree, NodeInfo *niblock, int *pnCount, char **sblockaddress);
int loadParseTree(char *filename, ParseTree **outtree, SymbolTable **symbols);

#ifdef __cplusplus
}
#endif

#endif // _STRINGS_H_
