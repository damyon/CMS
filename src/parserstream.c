#include "parserstream.h"
#include "parser.h"
#include "errors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*******************************************************************************
 * countParseTree...
 *
 * Counts the number the nodes in the parse tree
 ******************************************************************************/
void treeCount(ParseTree *tree, int *pnumNodes, int *pstringsLength) {

	*pnumNodes+=1;
	if (tree->stringValue) {
		*pstringsLength+=strlen(tree->stringValue) + 1; // string length + null character
	}
	if (tree->firstChild) {
		treeCount(tree->firstChild, pnumNodes, pstringsLength);
	}
	if (tree->rightSibling) {
		treeCount(tree->rightSibling, pnumNodes, pstringsLength);
	}
	
}	 

/*******************************************************************************
 * saveParseTreeNode...
 *
 * Navigates tree structure, saving tree to supplied file.
 ******************************************************************************/
void saveParseTreeNode(ParseTree *tree, NodeInfo *niblock, int *pnCount, char **sblockaddress) {

	// write node to block
	niblock[*pnCount].parseEnum = tree->parseEnum;
	// write node string to block
	if (tree->stringValue) {
		niblock[*pnCount].stringLength = strlen(tree->stringValue) + 1; // write string length to node block
		strcpy(*sblockaddress, tree->stringValue);
		strcat(*sblockaddress, "");
		*sblockaddress+=niblock[*pnCount].stringLength; // increment string block counter
	} else {
		niblock[*pnCount].stringLength = 0;
	}
	if (tree->firstChild) {
		niblock[*pnCount].hasChild = 1; // write hasChild
	} else {
		niblock[*pnCount].hasChild = 0;
	}
	if (tree->rightSibling) {
		niblock[*pnCount].hasSibling = 1; // write hasSibling
	} else {
		niblock[*pnCount].hasSibling = 0;
	}
	if (tree->firstChild) { // if child exists and no error
		*pnCount+=1; // increment node
		saveParseTreeNode(tree->firstChild, niblock, pnCount, sblockaddress); // navigate to child
	}
	if (tree->rightSibling) { // if right sibling exists and no error
		*pnCount+=1; // increment node
		saveParseTreeNode(tree->rightSibling, niblock, pnCount, sblockaddress); // navigate to right sibling
	}
	
}

/*******************************************************************************
 * saveParseTree...
 *
 * Save this parse tree to disk in a binary format optimised for reading.
 *
 * @params
 * ParseTree * tree - The parse tree to save
 * char *filename - The filename to save to.
 *
 * @return - error code (E_OK on success)
 ******************************************************************************/
int saveParseTree(ParseTree *tree, char *filename) {

	FILE *f;
	int numNodes = 0, stringsLength = 0, nCount = 0;
	int *pnumNodes, *pnCount, *pstringsLength;
	pnumNodes = &numNodes;
	pnCount = &nCount;
	pstringsLength = &stringsLength;
	NodeInfo *niblock;
	char *sblock;
	char **sblockaddress = malloc(sizeof(char **));
		
	// OPEN FILE
	if (!(f = fopen(filename, "wb"))) {
		return -1; // failed to open file
	} else { // write file
		treeCount(tree, pnumNodes, pstringsLength); // count number of nodes and symbols
		// allocate arrays
		niblock = (NodeInfo *) malloc(sizeof(NodeInfo)*numNodes);
		sblock = (char *) malloc(sizeof(char)*stringsLength);
		*sblockaddress = sblock;
		// navigate tree
		saveParseTreeNode(tree, niblock, pnCount, sblockaddress);
		*sblockaddress-=stringsLength; // reset sblock pointer to beginning of string block
		fwrite(&numNodes, sizeof(int), 1, f); // write node count
		fwrite(&stringsLength, sizeof(int), 1, f); // write length of all strings
		fwrite(niblock, sizeof(NodeInfo), numNodes, f); // write node info block
		fwrite(sblock, sizeof(char), stringsLength, f); // write strings block
		fflush(f);
		fclose(f);
		return E_OK; // wrote OK
		
	}
	
}

/*******************************************************************************
 * initNode...
 *
 * Takes the node from the NodeInfo array and attempts to place it in the tree.
 *
 ******************************************************************************/
ParseTree *initNode(NodeInfo *niblock, int *pnCount, char **sblockaddress) {

	ParseTree *t = NULL;
	int length;

	t = initParseTree(niblock[*pnCount].parseEnum); // create new tree
	length = niblock[*pnCount].stringLength;
	if (length) { // initialise stringValue if non-zero length
		//t->stringValue = *sblockaddress;
		t->stringValue = (char *) calloc(length, sizeof(char));
		strcpy(t->stringValue,*sblockaddress);
		*sblockaddress+=length; // initialise next string pointer
	}
	return t;
}
 

/*******************************************************************************
 * loadParseTreeNode...
 *
 * Takes nodes from the node information block and builds a parse tree.
 * Links up strings to nodes from string block
 ******************************************************************************/
void loadParseTreeNode(ParseTree *tree, NodeInfo *niblock, int *pnCount, char **sblockaddress) {

	ParseTree *child = NULL, *sibling = NULL;
	int hasChild, hasSibling;

	hasChild = niblock[*pnCount].hasChild;
	hasSibling = niblock[*pnCount].hasSibling;
	
	if (hasChild) { // pull out left child
		*pnCount+=1;
		child = initNode(niblock, pnCount, sblockaddress);
		addChildParseTree(tree, child); // add to tree as child of current node
		loadParseTreeNode(child, niblock, pnCount, sblockaddress);
	}
	if (hasSibling) { // pull out right sibling
		*pnCount+=1;
		sibling = initNode(niblock, pnCount, sblockaddress);
		addRightSiblingParseTree(tree, sibling); // add to tree as right sibling of current node
		loadParseTreeNode(sibling, niblock, pnCount, sblockaddress);
	}
	
}

/*******************************************************************************
 * loadParseTree...
 *
 * Load this parse tree from disk in a binary format optimised for reading.
 *
 * @params
 * char *filename - The filename to load from.
 * ParseTree ** tree - The new parse tree.
 * SymbolTable ** symbols - A symbol table containing any constant symbols referred
 *                          to by the parse tree.
 *
 * @return - error code (E_OK on success)
 ******************************************************************************/
int loadParseTree(char *filename, ParseTree **outtree, SymbolTable **symbols) {

	ParseTree *tree = NULL;
	SymbolTable *st = *symbols;
	int numNodes = 0, stringsLength = 0, nCount = 0;
	int *pnumNodes, *pnCount, *pstringsLength;
	pnumNodes = &numNodes;
	pnCount = &nCount;
	pstringsLength = &stringsLength;
	FILE *f;
	NodeInfo *niblock;
	char *sblock;
	char **sblockaddress = malloc(sizeof(char **));
	
	// OPEN FILE
	if (!(f = fopen(filename, "rb"))) { // open file for reading
		return -1; // failed to open file for reading
	} else {
		fread(&numNodes, sizeof(int), 1, f); // read in number of nodes
		fread(&stringsLength, sizeof(int), 1, f); // read in length of string block
		// allocate arrays
		niblock = (NodeInfo *) malloc(sizeof(NodeInfo)*numNodes);
		sblock = (char *) malloc(sizeof(char)*stringsLength);
		*sblockaddress = sblock;
		// read in arrays
		fread(niblock, sizeof(NodeInfo), numNodes, f);
		fread(sblock, sizeof(char), stringsLength, f);
		fflush(f);
		fclose(f);
		// create new tree with first node
		tree = initNode(niblock, pnCount, sblockaddress);
		// add branches to parse tree
		loadParseTreeNode(tree, niblock, pnCount, sblockaddress);
		*outtree = tree;
		*symbols = st;
		return E_OK;
	} 
	
}
