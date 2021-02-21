
#include "structs.h"
#include "syntax.h"
#include "strings.h"
#include "malloc.h"
#include "bison.h"
#include "api.h"
#include "file.h"
#include "parserstream.h"
#include "logging.h"
#include "errors.h"
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifndef MAX_VARGS
#define MAX_VARGS 1024
#endif

static int glob_variableid = 0;

int getVariableID() {
	return glob_variableid++;
}

void freeFunctionType(FunctionType *f) {
	Symbol *s = NULL;
	LinkedList *l = NULL;
	if (f != NULL) {
		if (f->functionName != NULL) {
			free(f->functionName);
		}
		if (f->nativeArgs) {
			l = f->nativeArgs;
			while (l) {
				if (l->data) {
					s = (Symbol *) l->data;
					freeSymbol(s);
				}
				l = l->next;
			}
		}
		free(f);
	}
}


void freeVariableType(VariableType *v) {
	if (v != NULL) {
		if (v->variableEnum == STRING_VARIABLE) {
			if (v->stringValue != NULL) {
				free(v->stringValue);
			}
		} else if (v->variableEnum == ARRAY_VARIABLE) {
			freeMap(v->arrayValues);
		} else if (v->variableEnum == MAP_VARIABLE) {
			freeMap(v->mapValues);
		}

		if (v->variableName != NULL) {
			free(v->variableName);
		}
		free(v);
	}
}

void freeSymbol(void *a) {
	Symbol *sa = (Symbol *) a;

	if (sa != NULL) {

		if (sa->symbolEnum == VARIABLE_SYMBOL) {
			freeVariableType(sa->variableType);
		} else if (sa->symbolEnum == FUNCTION_SYMBOL) {
			freeFunctionType(sa->functionType);
		}
		free(sa);
	}
}

int compareVariableIDs(Symbol *a, Symbol *b) {
	return b->variableType->variableID - a->variableType->variableID;
}

int compareSymbols(void *a, void *b) {
	Symbol *sa = (Symbol *) a,
	       *sb = (Symbol *) b;
	char *namea = NULL, *nameb = NULL;
	int cmp = 0;

	if (a == NULL) {
		return -1;
	}
	
	if (b == NULL) {
		return 1;
	}

	if (sa->symbolEnum == VARIABLE_SYMBOL) {
		namea = sa->variableType->variableName;
	} else {
		namea = sa->functionType->functionName;
	}

	if (sb->symbolEnum == VARIABLE_SYMBOL) {
		nameb = sb->variableType->variableName;
	} else {
		nameb = sb->functionType->functionName;
	}

	cmp = strcmp(namea?namea:"", nameb?nameb:"");

	if ((strcmp(namea?namea:"", "") == 0) && (strcmp(nameb?nameb:"", "") == 0)) {
		cmp = compareVariableIDs(a, b);
	}

	return cmp;
}

Symbol *initSymbol() {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));
	memset(s, 0, sizeof(Symbol));

	return s;
}

VariableType * copySymbolVariableType(Symbol *s);
FunctionType * copySymbolFunctionType(Symbol *s);

Symbol * copySymbol(Symbol *s) {
	Symbol *sym = NULL;

	sym = initSymbol();

	sym->symbolEnum = s->symbolEnum;

	if (sym->symbolEnum == VARIABLE_SYMBOL) {
		sym->variableType = copySymbolVariableType(s);
	} else if (sym->symbolEnum == FUNCTION_SYMBOL) {
		sym->functionType = copySymbolFunctionType(s);
	}

	return sym;
}

Map * copySymbolArrayType(Symbol *s) {
	Map *arrayValues = NULL;
	MapNode *node = NULL, *insert = NULL;
	Symbol *sym = NULL;

	arrayValues = initMap(compareSymbols, freeSymbol);

	if (s != NULL && s->symbolEnum == VARIABLE_SYMBOL) {
		if (s->variableType->variableEnum == ARRAY_VARIABLE) {
			// walk the tree
			node = getFirstMapNode(s->variableType->arrayValues);

			while (node != NULL) {
				sym = copySymbol(node->ele);
				insert = initMapNode(sym);
				insertMapValue(insert, arrayValues);
				node = getNextMapNode(node, s->variableType->arrayValues);
			}
		}
	}

	return arrayValues;
}

Map * copySymbolMapType(Symbol *s) {
	Map *mapValues = NULL;
	MapNode *node = NULL, *insert = NULL;
	Symbol *sym = NULL;

	mapValues = initMap(compareSymbols, freeSymbol);

	if (s != NULL && s->symbolEnum == VARIABLE_SYMBOL) {
		if (s->variableType->variableEnum == MAP_VARIABLE) {
			// walk the tree
			node = getFirstMapNode(s->variableType->mapValues);

			while (node != NULL) {
				sym = copySymbol(node->ele);
				insert = initMapNode(sym);
				insertMapValue(insert, mapValues);
				node = getNextMapNode(node, s->variableType->mapValues);
			}
		}
	}

	return mapValues;
}

FunctionType * copySymbolFunctionType(Symbol *s) {
	FunctionType *functionType = NULL;

	functionType = (FunctionType *) malloc(sizeof(FunctionType));
	memset(functionType, 0, sizeof(FunctionType));

	if (s && s->symbolEnum == FUNCTION_SYMBOL) {
		functionType->functionName = strdup(s->functionType->functionName);
		functionType->ptree = s->functionType->ptree;
	} else {
		functionType->functionName = strdup("");
		functionType->ptree = NULL;
	}
	return functionType;
}

VariableType * copySymbolVariableType(Symbol *s) {
	VariableType *variableType = NULL;

	variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(variableType, 0, sizeof(VariableType));
	variableType->variableEnum = UNINITIALISED_VARIABLE;
	variableType->variableID = getVariableID();

	if (s && s->symbolEnum == VARIABLE_SYMBOL) {
		variableType->variableName = strdup(s->variableType->variableName);
		if (s->variableType->variableEnum == STRING_VARIABLE) {
			variableType->variableEnum = STRING_VARIABLE;
			variableType->stringValue = strdup(s->variableType->stringValue);
		} else if (s->variableType->variableEnum == INT_VARIABLE) {
			variableType->variableEnum = INT_VARIABLE;
			variableType->intValue = s->variableType->intValue;
		} else if (s->variableType->variableEnum == FLOAT_VARIABLE) {
			variableType->variableEnum = FLOAT_VARIABLE;
			variableType->floatValue = s->variableType->floatValue;
		} else if (s->variableType->variableEnum == ARRAY_VARIABLE) {
			variableType->variableEnum = ARRAY_VARIABLE;
			variableType->arrayValues = copySymbolArrayType(s);
		} else if (s->variableType->variableEnum == MAP_VARIABLE) {
			variableType->variableEnum = MAP_VARIABLE;
			variableType->mapValues = copySymbolMapType(s);
		}
	} else {
		variableType->variableName = strdup("");
	}
	return variableType;
}


ParseTree * initParseTree(ParseEnum parseEnum) {
	ParseTree *t = NULL;
	t = (ParseTree *) malloc(sizeof(ParseTree));
	t->stringValue = NULL;
	t->parseEnum = parseEnum;
	t->firstChild = NULL;
	t->symbol = NULL;
	t->lastChild = NULL;
	t->parent = NULL;
	t->leftSibling = NULL;
	t->rightSibling = NULL;

	return t;
}

void addChildParseTree(ParseTree *parent, ParseTree *child) {
	ParseTree *t = NULL;
	child->parent = parent;
	if (parent->firstChild == NULL) {
		parent->firstChild = child;
	} else {
		t = parent->firstChild;
		while (t->rightSibling != NULL) t = t->rightSibling;
		t->rightSibling = child;
		child->leftSibling = t;
	}

	t = child;
	while (t->rightSibling != NULL) {
		t->parent = parent;
		t = t->rightSibling;
	}
	t->parent = parent;
	parent->lastChild = t;
}

void addRightSiblingParseTree(ParseTree *left, ParseTree *right) {
	ParseTree *t = NULL;
	left->rightSibling = right;
	right->leftSibling = left;

	t = right;
	while (t->rightSibling != NULL) {
		t->parent = left->parent;
		t = t->rightSibling;
	}
	t->parent = left->parent;
	if (left->parent != NULL) {
		left->parent->lastChild = t;
	}
}

ParseTree * copyParseTree(ParseTree *source) {
	ParseTree *dest = NULL;
	ParseTree *branch = NULL;

	dest = initParseTree(source->parseEnum);
	if (source->stringValue) {
		dest->stringValue = strdup(source->stringValue);
	}
	if (source->symbol) {
		dest->symbol = source->symbol;
	}
	if (source->firstChild != NULL) {
		branch = copyParseTree(source->firstChild);
		addChildParseTree(dest, branch);
	}

	if (source->rightSibling != NULL) {
		branch = copyParseTree(source->rightSibling);
		addRightSiblingParseTree(dest, branch);
	}
	return dest;
}

SymbolTable * initChildSymbolTable() {
	SymbolTable *s = NULL;

	s = (SymbolTable *) malloc(sizeof(SymbolTable));

	s->symbols = initMap(compareSymbols, freeSymbol);
	s->childSymbolTable = NULL;
	s->parentSymbolTable = NULL;

	return s;
}

void increaseScope(SymbolTable *st) {
	SymbolTable *newscope = initChildSymbolTable();
	SymbolTable *l = st;

	while (l->childSymbolTable != NULL) l = l->childSymbolTable;

	newscope->parentSymbolTable = l;
	l->childSymbolTable = newscope;
}

SymbolTable * initSymbolTable() {
	SymbolTable *s = NULL;

	s = (SymbolTable *) malloc(sizeof(SymbolTable));

	s->symbols = initMap(compareSymbols, freeSymbol);
	s->childSymbolTable = NULL;
	s->parentSymbolTable = NULL;

	registerFunctions(s);
	increaseScope(s);

	return s;
}

void freeSymbolTable(SymbolTable *s) {
	return;
	freeMap(s->symbols);
	free(s);
}


void decreaseScope(SymbolTable *st) {
	SymbolTable *l = st;
	SymbolTable *p = NULL;

	if (l->childSymbolTable == NULL) {
		// cannot go past last scope
		return;
	}
	while (l->childSymbolTable != NULL) l = l->childSymbolTable;

	p = l->parentSymbolTable;
	if (p != NULL) {
		p->childSymbolTable = NULL;
	}
	freeSymbolTable(l);
}


int symbolGetIsInt(Symbol *a) {
	if (a->symbolEnum == VARIABLE_SYMBOL) {
		return a->variableType->variableEnum == INT_VARIABLE;
	}
	return 0;
}

int symbolGetIsFloat(Symbol *a) {
	if (a->symbolEnum == VARIABLE_SYMBOL) {
		return a->variableType->variableEnum == FLOAT_VARIABLE;
	}
	return 0;
}

int symbolGetIsString(Symbol *a) {
	if (a->symbolEnum == VARIABLE_SYMBOL) {
		return a->variableType->variableEnum == STRING_VARIABLE;
	}
	return 0;
}

char *castSymbolToString(Symbol *s) {
	char numbuf[256];
	if (s == NULL)
		return strdup("");

	if (s->symbolEnum == VARIABLE_SYMBOL) {
		if (s->variableType->variableEnum == INT_VARIABLE) {
			snprintf(numbuf, 255, "%d", s->variableType->intValue);
			return strdup(numbuf);
		} else if (s->variableType->variableEnum == FLOAT_VARIABLE) {
			snprintf(numbuf, 255, "%f", s->variableType->floatValue);
			return strdup(numbuf);
		} else if (s->variableType->variableEnum == STRING_VARIABLE) {
			return strdup(s->variableType->stringValue);
		} else if (s->variableType->variableEnum == MAP_VARIABLE) {
			return strdup("<map/>");
		} else if (s->variableType->variableEnum == ARRAY_VARIABLE) {
			return strdup("<array/>");
		}
	} else if (s->symbolEnum == FUNCTION_SYMBOL) {
		return strdup("<function/>");
	} 

	// all other types cannot be cast
	return strdup("");
}

float castSymbolToFloat(Symbol *s) {
	if (s == NULL)
		return 0;

	if (s->symbolEnum == VARIABLE_SYMBOL) {
		if (s->variableType->variableEnum == INT_VARIABLE) {
			return (float) s->variableType->intValue;
		} else if (s->variableType->variableEnum == FLOAT_VARIABLE) {
			return s->variableType->floatValue;
		} else if (s->variableType->variableEnum == STRING_VARIABLE) {
			return atof(s->variableType->stringValue);
		}
	} 

	// all other types cannot be cast
	return 0;
}

int castSymbolToInt(Symbol *s) {
	if (s == NULL)
		return 0;

	if (s->symbolEnum == VARIABLE_SYMBOL) {
		if (s->variableType->variableEnum == INT_VARIABLE) {
			return s->variableType->intValue;
		} else if (s->variableType->variableEnum == FLOAT_VARIABLE) {
			return (int) floorf(s->variableType->floatValue);
		} else if (s->variableType->variableEnum == STRING_VARIABLE) {
			return strtol(s->variableType->stringValue, NULL, 10);
		}
	} 

	// all other types cannot be cast
	return 0;
}

Symbol *addSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	int ia, ib;
	float fa, fb;
	char *sa = NULL, *sb = NULL, *sc = NULL;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia + ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantFloatSymbol(fa + fb);
		} else if (symbolGetIsString(b)) {
			sa = castSymbolToString(a);
			sb = castSymbolToString(b);

			vstrdupcat(&sc, sa, sb, NULL);
			free(sa);
			free(sb);
			c = initConstantStringSymbol(sc);
			free(sc);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantFloatSymbol(fa + fb);
	} else if (symbolGetIsString(a)) {
		sa = castSymbolToString(a);
		sb = castSymbolToString(b);
		vstrdupcat(&sc, sa, sb, NULL);
		free(sa);
		free(sb);
		c = initConstantStringSymbol(sc);
		free(sc);
	}

	return c;
}

char *subtractString(char *sa, char *sb) {
	char *sc = NULL, *sd = NULL;
	// string subtraction
	//
	// abcdef - bd = acef

	sc = strdup(sa);
	while (*sc != '\0') {
		if (strchr(sb, *sc)) {
			// remove this letter from the result
			sd = sc;
			while (*sd != '\0') {
				*sd = *(sd + 1);
				sd++;
			}
			sc--;
		}
		sc++;
	}

	return sc;
}

Symbol *subtractSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	int ia, ib;
	float fa, fb;
	char *sa = NULL, *sb = NULL, *sc = NULL, *sd = NULL;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia - ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantFloatSymbol(fa - fb);
		} else if (symbolGetIsString(b)) {
			// string subtraction
			//
			// abcdef - bd = acef
			sa = castSymbolToString(a);
			sb = castSymbolToString(b);

			sc = sa;
			while (*sc != '\0') {
				if (strchr(sb, *sc)) {
					// remove this letter from the result
					sd = sc;
					while (*sd != '\0') {
						*sd = *(sd + 1);
						sd++;
					}
					sc--;
				}
				sc++;
			}

			free(sb);

			c = initConstantStringSymbol(sa);
			free(sa);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantFloatSymbol(fa - fb);
	} else if (symbolGetIsString(a)) {
		// string subtraction
		//
		// abcdef - bd = acef
		sa = castSymbolToString(a);
		sb = castSymbolToString(b);

		sc = sa;
		while (*sc != '\0') {
			if (strchr(sb, *sc)) {
				// remove this letter from the result
				sd = sc;
				while (*sd != '\0') {
					*sd = *(sd + 1);
					sd++;
				}
				sc--;
			}
			sc++;
		}

		free(sb);

		c = initConstantStringSymbol(sa);
		free(sa);
	}

	return c;
}

char *multiplyString(char *value, int times) {
	char *c = NULL;
	int i = 0;

	vstrdupcat(&c, "", NULL);
	for (i = 0; i < times; i++) {
		vstrdupcat(&c, value, NULL);
	}
	return c;
}

Symbol *multiplySymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	int ia, ib, i;
	float fa, fb;
	char *sa = NULL, *sb = NULL, *sc = NULL;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia * ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantFloatSymbol(fa * fb);
		} else if (symbolGetIsString(b)) {
			ia = castSymbolToInt(a);
			sb = castSymbolToString(b);

			for (i = 0; i < ia; i++) {
				vstrdupcat(&sc, sb, NULL);
			}
			free(sb);
			c = initConstantStringSymbol(sc);
			free(sc);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantFloatSymbol(fa * fb);
	} else if (symbolGetIsString(a)) {
		if (symbolGetIsInt(b)) {
			sa = castSymbolToString(a);
			ib = castSymbolToInt(b);

			for (i = 0; i < ib; i++) {
				vstrdupcat(&sc, sa, NULL);
			}
			free(sa);
			c = initConstantStringSymbol(sc);
			free(sc);
		} else {
			c = initConstantStringSymbol("");
		}
	}

	return c;
}

Symbol *divideSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	float fa, fb;
	int ia, ib;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			if (ib == 0) {
				return NULL;
			}

			c = initConstantIntSymbol(ia / ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);
			if (fb == 0) {
				return NULL;
			}

			c = initConstantFloatSymbol(fa / fb);
		} else {
			return NULL;
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);
		if (fb == 0) {
			return NULL;
		}

		c = initConstantFloatSymbol(fa / fb);
	} else {
		return NULL;
	}

	return c;
}

Symbol *modSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	int ia, ib;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			if (ib <= 0) {
				return NULL;
			}

			c = initConstantIntSymbol(ia % ib);
		} else if (symbolGetIsFloat(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);
			if (ib <= 0) {
				return NULL;
			}

			c = initConstantIntSymbol(ia % ib);
		} else {
			return NULL;
		}
	} else if (symbolGetIsFloat(a)) {
		ia = castSymbolToInt(a);
		ib = castSymbolToInt(b);
		if (ib <= 0) {
			return NULL;
		}

		c = initConstantIntSymbol(ia % ib);
	} else {
		return NULL;
	}

	return c;
}

Symbol *powerSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	float fa, fb;
	int ia, ib;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			if (ib == 0) {
				return NULL;
			}

			c = initConstantIntSymbol(powl(ia, ib));
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);
			if (fb == 0) {
				return NULL;
			}

			c = initConstantFloatSymbol(powf(fa, fb));
		} else {
			return NULL;
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);
		if (fb == 0) {
			return NULL;
		}

		c = initConstantFloatSymbol(powf(fa, fb));
	} else {
		return NULL;
	}

	return c;
}

Symbol *equalSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	float fa, fb;
	int ia, ib;
	char *sa, *sb;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia == ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantIntSymbol(fa == fb);
		} else {
			sa = castSymbolToString(a);
			sb = castSymbolToString(b);

			c = initConstantIntSymbol(strcmp(sa, sb) == 0);

			free(sa);
			free(sb);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantIntSymbol(fa == fb);
	} else {
		sa = castSymbolToString(a);
		sb = castSymbolToString(b);

		c = initConstantIntSymbol(strcmp(sa, sb) == 0);

		free(sa);
		free(sb);
	}

	return c;
}

Symbol *notEqualSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	float fa, fb;
	int ia, ib;
	char *sa, *sb;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia != ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantIntSymbol(fa != fb);
		} else {
			sa = castSymbolToString(a);
			sb = castSymbolToString(b);

			c = initConstantIntSymbol(strcmp(sa, sb) != 0);

			free(sa);
			free(sb);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantIntSymbol(fa != fb);
	} else {
		sa = castSymbolToString(a);
		sb = castSymbolToString(b);

		c = initConstantIntSymbol(strcmp(sa, sb) != 0);

		free(sa);
		free(sb);
	}

	return c;
}

Symbol *lessThanSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	float fa, fb;
	int ia, ib;
	char *sa, *sb;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia < ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantIntSymbol(fa < fb);
		} else {
			sa = castSymbolToString(a);
			sb = castSymbolToString(b);

			c = initConstantIntSymbol(strcmp(sa, sb) < 0);

			free(sa);
			free(sb);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantIntSymbol(fa < fb);
	} else {
		sa = castSymbolToString(a);
		sb = castSymbolToString(b);

		c = initConstantIntSymbol(strcmp(sa, sb) < 0);

		free(sa);
		free(sb);
	}

	return c;
}

Symbol *lessThanEqualSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	float fa, fb;
	int ia, ib;
	char *sa, *sb;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia <= ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantIntSymbol(fa <= fb);
		} else {
			sa = castSymbolToString(a);
			sb = castSymbolToString(b);

			c = initConstantIntSymbol(strcmp(sa, sb) <= 0);

			free(sa);
			free(sb);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantIntSymbol(fa <= fb);
	} else {
		sa = castSymbolToString(a);
		sb = castSymbolToString(b);

		c = initConstantIntSymbol(strcmp(sa, sb) <= 0);

		free(sa);
		free(sb);
	}

	return c;
}

Symbol *greaterThanSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	float fa, fb;
	int ia, ib;
	char *sa, *sb;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia > ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantIntSymbol(fa > fb);
		} else {
			sa = castSymbolToString(a);
			sb = castSymbolToString(b);

			c = initConstantIntSymbol(strcmp(sa, sb) > 0);

			free(sa);
			free(sb);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantIntSymbol(fa > fb);
	} else {
		sa = castSymbolToString(a);
		sb = castSymbolToString(b);

		c = initConstantIntSymbol(strcmp(sa, sb) > 0);

		free(sa);
		free(sb);
	}

	return c;
}

Symbol *greaterThanEqualSymbols(Symbol *a, Symbol *b) {
	Symbol *c = NULL;
	float fa, fb;
	int ia, ib;
	char *sa, *sb;

	if (symbolGetIsInt(a)) {
		if (symbolGetIsInt(b)) {
			ia = castSymbolToInt(a);
			ib = castSymbolToInt(b);

			c = initConstantIntSymbol(ia >= ib);
		} else if (symbolGetIsFloat(b)) {
			fa = castSymbolToFloat(a);
			fb = castSymbolToFloat(b);

			c = initConstantIntSymbol(fa >= fb);
		} else {
			sa = castSymbolToString(a);
			sb = castSymbolToString(b);

			c = initConstantIntSymbol(strcmp(sa, sb) >= 0);

			free(sa);
			free(sb);
		}
	} else if (symbolGetIsFloat(a)) {
		fa = castSymbolToFloat(a);
		fb = castSymbolToFloat(b);

		c = initConstantIntSymbol(fa >= fb);
	} else {
		sa = castSymbolToString(a);
		sb = castSymbolToString(b);

		c = initConstantIntSymbol(strcmp(sa, sb) >= 0);

		free(sa);
		free(sb);
	}

	return c;
}

FunctionType *initFunctionType() {
	FunctionType *functionType = NULL;

	functionType = (FunctionType *) malloc(sizeof(FunctionType));
	memset(functionType, 0, sizeof(FunctionType));
	return functionType;
}

Symbol *initConstantFloatSymbol(float value) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableName = strdup("");
	s->variableType->variableID = getVariableID();
	s->variableType->variableEnum = FLOAT_VARIABLE;
	s->variableType->floatValue = value;

	return s;
}

Symbol *initConstantFloatSymbolString(char * value) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableID = getVariableID();
	s->variableType->variableEnum = FLOAT_VARIABLE;
	s->variableType->floatValue = atof(value);

	return s;
}

Symbol *initConstantIntSymbol(int value) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableName = strdup("");
	s->variableType->variableID = getVariableID();
	s->variableType->variableEnum = INT_VARIABLE;
	s->variableType->intValue = value;

	return s;
}

Symbol *initConstantIntSymbolString(char *value) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableName = strdup("");
	s->variableType->variableID = getVariableID();
	s->variableType->variableEnum = INT_VARIABLE;
	s->variableType->intValue = strtol(value, NULL, 10);

	return s;
}

Symbol *initConstantStringSymbol(char *value) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableName = strdup("");
	s->variableType->variableID = getVariableID();
	s->variableType->variableEnum = STRING_VARIABLE;
	s->variableType->stringValue = strdup(value?value:"");

	return s;
}


Symbol *initVariableFloatSymbol(char *name, float value) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableID = getVariableID();
	s->variableType->variableName = name;
	s->variableType->variableEnum = FLOAT_VARIABLE;
	s->variableType->intValue = value;

	return s;
}

Symbol *initNativeFunctionSymbol(char *name, int (*nativeCode)(LinkedList *args, ParserInfo *info, Symbol *result), LinkedList *argsdesc) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = FUNCTION_SYMBOL;

	s->functionType = (FunctionType *) malloc(sizeof(FunctionType));
	s->functionType->functionName = strdup(name);
	s->functionType->nativeCode = nativeCode;
	s->functionType->nativeArgs = argsdesc;
	s->functionType->ptree = NULL;

	s->variableType = NULL;

	return s;

}

void registerNativeFunction(SymbolTable *symbols, char *name, int (*nativeCode)(LinkedList *args, ParserInfo *info, Symbol *result), LinkedList *argsdesc) {
	Symbol *s = initNativeFunctionSymbol(name, nativeCode, argsdesc);

	putSymbol(symbols, s);
}

Symbol *initFunctionSymbol(char *name, ParseTree *function) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = FUNCTION_SYMBOL;

	s->functionType = (FunctionType *) malloc(sizeof(FunctionType));
	s->functionType->functionName = strdup(name);
	s->functionType->ptree = copyParseTree(function);
	s->functionType->nativeCode = NULL;
	s->functionType->nativeArgs = NULL;

	s->variableType = NULL;

	return s;

}

Symbol *initVariableMapSymbol(char *name) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableID = getVariableID();
	s->variableType->variableName = name;
	s->variableType->variableEnum = MAP_VARIABLE;
	s->variableType->mapValues = initMap(compareSymbols, freeSymbol);

	return s;
}

Symbol *initVariableArraySymbol(char *name) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableID = getVariableID();
	s->variableType->variableName = name;
	s->variableType->variableEnum = ARRAY_VARIABLE;
	s->variableType->arrayValues = initMap(compareSymbols, freeSymbol);

	return s;
}

Symbol *initVariableIntSymbol(char *name, int value) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableID = getVariableID();
	s->variableType->variableName = name;
	s->variableType->variableEnum = INT_VARIABLE;
	s->variableType->intValue = value;

	return s;
}

Symbol *initVariableSymbol(char *name) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableID = getVariableID();
	s->variableType->variableName = name;
	s->variableType->variableEnum = UNINITIALISED_VARIABLE;

	return s;
}

Symbol *initVariableStringSymbol(char *name, char *value) {
	Symbol *s = NULL;
	s = (Symbol *) malloc(sizeof(Symbol));

	s->symbolEnum = VARIABLE_SYMBOL;

	s->functionType = NULL;
	s->variableType = (VariableType *) malloc(sizeof(VariableType));
	memset(s->variableType, 0, sizeof(VariableType));
	s->variableType->variableID = getVariableID();
	s->variableType->variableName = name;
	s->variableType->variableEnum = STRING_VARIABLE;
	s->variableType->stringValue = value;

	return s;
}

Symbol *getSymbolMapValue(Map *map, char *mapIndex) {
	Symbol *s = initVariableSymbol(strdup(mapIndex));
	MapNode *result = NULL;

	result = searchMap((void *) s, map);
	if (result != NULL) {
		freeSymbol(s);
		s = (Symbol *) result->ele;
		return s;
	}

	// none found, add to map and return
	result = initMapNode(s);
	insertMapValue(result, map);

	return s;
}

Symbol *getSymbolArrayValue(Map *map, int arrayIndex) {
	char numbuf[256];
	Symbol *s = NULL;
	MapNode *result = NULL;
	
	sprintf(numbuf, "%.5d", arrayIndex);
	s = initVariableSymbol(strdup(numbuf));
	result = searchMap((void *) s, map);
	if (result != NULL) {
		freeSymbol(s);
		s = (Symbol *) result->ele;
		return s;
	}

	// none found, add to map and return
	result = initMapNode(s);
	insertMapValue(result, map);
	return s;
}

// only find symbols in the current scope.
Symbol *getSymbolShallow(SymbolTable *st, char *name) {
	SymbolTable *l = st;
	Symbol *s = initVariableSymbol(strdup(name));
	MapNode *result = NULL;

	// search lowest nodes first
	while (l->childSymbolTable != NULL) { l = l->childSymbolTable; }

	result = searchMap((void *) s, l->symbols);
	if (result != NULL) {
		freeSymbol(s);
		s = (Symbol *) result->ele;
		return s;
	}
	freeSymbol(s);
	return NULL;
}

Symbol *getSymbol(SymbolTable *st, char *name) {
	SymbolTable *l = st;
	Symbol *s = initVariableSymbol(strdup(name));
	MapNode *result = NULL;

	// search lowest nodes first
	while (l->childSymbolTable != NULL) { l = l->childSymbolTable; }

	while (l != NULL) {
		result = searchMap((void *) s, l->symbols);
		if (result != NULL) {
			freeSymbol(s);
			s = (Symbol *) result->ele;
			return s;
		}
		l = l->parentSymbolTable;
	}
	freeSymbol(s);
	return NULL;
}

void putSymbol(SymbolTable *st, Symbol *s) {
	SymbolTable *l = st;
	MapNode *node = initMapNode(s);

	// insert in lowest current scope
	// unless constant (they go in top scope)
	//if ((s->symbolEnum == VARIABLE_SYMBOL) && (strlen(s->variableType->variableName) > 0)) {
	if ((s->symbolEnum == VARIABLE_SYMBOL)) {
		while (l->childSymbolTable != NULL) { l = l->childSymbolTable; }
	}

	insertMapValue(node, l->symbols);
}

void printVariableType(VariableType *v) {
	MapNode *node = NULL;
	if (v == NULL) {
		printf("<variable/>\n");
		return;
	} 
	if (v->variableEnum == STRING_VARIABLE) {
		printf("<variable type=\"string\" name=\"%s\">%s</variable>\n", v->variableName?v->variableName:"NULL", v->stringValue?v->stringValue:"");
	} else if (v->variableEnum == INT_VARIABLE) {
		printf("<variable type=\"int\" name=\"%s\">%d</variable>\n", v->variableName?v->variableName:"NULL", v->intValue);
	} else if (v->variableEnum == FLOAT_VARIABLE) {
		printf("<variable type=\"float\" name=\"%s\">%f</variable>\n", v->variableName?v->variableName:"NULL", v->floatValue);
	} else if (v->variableEnum == ARRAY_VARIABLE) {
		printf("<variable type=\"array\" name=\"%s\">\n", v->variableName?v->variableName:"NULL");
		if (v->arrayValues) {
			node = getFirstMapNode(v->arrayValues);
			while (node != NULL) {
				printSymbol((Symbol *) node->ele);
				node = getNextMapNode(node, v->arrayValues);
			}
		}
		printf("</variable>\n");
	} else if (v->variableEnum == MAP_VARIABLE) {
		printf("<variable type=\"map\" name=\"%s\">\n", v->variableName?v->variableName:"NULL");
		if (v->mapValues) {
			node = getFirstMapNode(v->mapValues);
			while (node != NULL) {
				printSymbol((Symbol *) node->ele);
				node = getNextMapNode(node, v->mapValues);
			}
		}
		printf("</variable>\n");
	} else if (v->variableEnum == UNINITIALISED_VARIABLE) {
		printf("<variable type=\"uninitialised\" name=\"%s\"/>\n", v->variableName?v->variableName:"NULL");
	}
}

void printFunctionType(FunctionType *f) {
	printf("<function name=\"%s\"/>\n", f->functionName);
}

void printMapSymbol(void *ele) {
	Symbol *s = (Symbol *) ele;

	if (s != NULL) {
		printSymbol(s);
	} else {
		printf("NULL\n");
	}
}

void printSymbolTable(SymbolTable *s) {
	SymbolTable *l = s;
	MapNode *node = NULL;
	int scope = 0;

	while (l != NULL) {
		scope++;
		printf("printSymbolTable(%d) {\n", scope);

		node = getFirstMapNode(l->symbols);

		while (node != NULL) {
			printSymbol(node->ele);
			node = getNextMapNode(node, l->symbols);
		}

		l = l->childSymbolTable;

		printf("}\n");
	}
}

void printSymbol(Symbol *s) {
	if (s == NULL) {
		printf("NULL\n");
	} else {
		if (s->symbolEnum == VARIABLE_SYMBOL) {
			printVariableType(s->variableType);
		} else if (s->symbolEnum == FUNCTION_SYMBOL) {
			printFunctionType(s->functionType);
		} else {
			printf("INVALID SYMBOL\n");
		}
	}
}

void printDebug() {
	printf("<debug/>\n");
}

char *getParseEnumDesc(ParseTree *tree) {
	if (tree == NULL) {
		return "NULL";
	}
	switch(tree->parseEnum) {
		case VARIABLE_PARSE:
			return "Variable";
		case PARAM_LIST_PARSE:
			return "Param List";
		case PARAM_LIST_ELE_PARSE:
			return "Param List Ele";
		case FUNCTION_DECL_PARSE:
			return "Function Decl";
		case IF_KEYWORD_PARSE:
			return "if";
		case ELSE_KEYWORD_PARSE:
			return "else";
		case DO_KEYWORD_PARSE:
			return "do";
		case FOR_KEYWORD_PARSE:
			return "for";
		case WHILE_KEYWORD_PARSE:
			return "while";
		case RETURN_KEYWORD_PARSE:
			return "return";
		case FUNCTION_KEYWORD_PARSE:
			return "function";
		case EXP_LIST_PARSE:
			return "Exp List";
		case OPEN_ARRAY_INDEX_PARSE:
			return "[";
		case CLOSE_ARRAY_INDEX_PARSE:
			return "]";
		case OPEN_PAREN_PARSE:
			return "(";
		case CLOSE_PAREN_PARSE:
			return ")";
		case FUNCTION_CALL_PARSE:
			return "Function Call";
		case BLOCK_PARSE:
			return "Block";
		case CLOSE_SCOPE_PARSE:
			return "}";
		case OPEN_SCOPE_PARSE:
			return "{";
		case NOT_PARSE:
			return "!";
		case NEGATIVE_PARSE:
			return "-";
		case COMMA_PARSE:
			return ",";
		case VAR_LIST_PARSE:
			return "Var List";
		case VAR_LIST_ELE_PARSE:
			return "Var List Ele";
		case VAR_LIST_END_PARSE:
			return "Var List End";
		case UNI_OPERATOR_PARSE:
			return "Uni Operator";
		case OPERATOR_PARSE:
			return "Operator";
		case SEMICOLON_PARSE:
			return ";";
		case INPUT_PARSE:
			return "Input";
		case STATEMENT_PARSE:
			return "Statement";
		case EXP_PARSE:
			return "Exp";
		case CONSTANT_STRING_PARSE:
			return "Constant String";
		case CONSTANT_INT_PARSE:
			return "Constant Int";
		case CONSTANT_FLOAT_PARSE:
			return "Constant Float";
		case TYPE_PARSE:
			return "Type";
		case TYPE_DECL_PARSE:
			return "Type Decl";
		case VAR_PARSE:
			return "Var";
		case ASSIGNMENT_OPERATOR_PARSE:
			return "Assignment Operator";
		default: 
			return "Unknown";
	}
}

char *getParseValueDesc(ParseTree *tree) {
	return tree->stringValue?tree->stringValue:"";
}

void printIndent(int i) {
	while (i-- > 0) {
		printf(" ");
	}
}

void printIndentedParseTree(ParseTree *tree, int indent) {
	printIndent(indent);
	if (!tree->firstChild) {
		printf("<node type=\"%s\" value=\"%s\" parent=\"%s\"/>\n", getParseEnumDesc(tree), getParseValueDesc(tree), tree->parent?"NOT NULL":"NULL");
	} else {
		printf("<node type=\"%s\" value=\"%s\" parent=\"%s\">\n", getParseEnumDesc(tree), getParseValueDesc(tree), tree->parent?"NOT NULL":"NULL");
		printIndentedParseTree(tree->firstChild, indent + 1);
		printIndent(indent);
		printf("</node>\n");
	}
	if (tree->rightSibling) {
		printIndentedParseTree(tree->rightSibling, indent);
	}
}

void freeParseTreeNode(ParseTree *tree) {
	if (tree) {
		if (tree->stringValue)
			free(tree->stringValue);
		free(tree);
	}
}

void freeParseTreeNodeFunc(void *a) {
	freeParseTreeNode((ParseTree *) a);
}

void freeParseTree(ParseTree *tree) {
	ParseTree *child, *next;
	if (tree) {
		if (tree->leftSibling == NULL && tree->parent != NULL) {
			tree->parent->firstChild = NULL;
		}
		if (tree->rightSibling == NULL && tree->parent != NULL) {
			tree->parent->lastChild = NULL;
		}
		if (tree->leftSibling != NULL) {
			tree->leftSibling->rightSibling = tree->rightSibling;
		}
		if (tree->rightSibling != NULL) {
			tree->rightSibling->leftSibling = tree->leftSibling;
		}
		child = tree->firstChild;
		while (child != NULL) {
			next = child->rightSibling;
			freeParseTree(child);
			child = next;
		}
		if (tree->stringValue != NULL)
			free(tree->stringValue);
		free(tree);
	}
}

void printParseTree(ParseTree *tree) {
	printf("PARSE TREE\n");
	printIndentedParseTree(tree, 0);	
}

char *trimquotes(char *str) {
	char *p = str + (strlen(str) - 1);
	*p = '\0';
	return strdup(str + 1);
}

void freeLinkedList(LinkedList *list, void (*freeFunc)(void *)) {
	LinkedList *l = list, *next = NULL;

	while (l != NULL) {
		if (l->data != NULL) {
			freeFunc(l->data);
			l->data = NULL;
		}
		next = l->next;
		free(l);
		l = next;
	}
}


/**
 * Add this element to the start of this list (faster).
 * Also - this makes this more of a stack but that is OK.
 */
void appendLinkedList(LinkedList **list, void *data) {
	LinkedList *l = (LinkedList *) malloc(sizeof(LinkedList)),
		   *c = *list;

	if (c == NULL) {
	  l->data = data;
	  l->next = NULL;
	  *list = l;
	} else {
		while (c->next != NULL) c = c->next;
		l->data = data;
		l->next = NULL;
		c->next = l;
	}
}


/**
 * Run the compiled parse tree.
 */
int interpretParseTree(ParseTree *tree, ParserInfo *parserInfo) {
	int err = 0, arrayIndex = 0, intValue = 0, i = 0;
	char *mapIndex = NULL, *stringValue = NULL, *stringValue2 = NULL;
	float floatValue = 0.0f;
	VariableEnum varType = STRING_VARIABLE;
	ParseTree *current = NULL, *param = NULL, *paramvalue = NULL, *func = NULL;
	Symbol *symbol = NULL, *expValue = NULL, *arg = NULL;
	LinkedList *symbollist = NULL, *listele = NULL, *arglist = NULL;

	parserInfo->instructionCount++;
	if (parserInfo->instructionCount >= parserInfo->maxInstructions) {
		snprintf(parserInfo->errormessage, 255, "Instruction limit reached. Execution terminated.");
		parserInfo->errorcode = 1;
		return E_PARSE_ERROR;
	}

	if (!tree) {
		snprintf(parserInfo->errormessage, 255, "Encountered NULL Parse Tree.");
		parserInfo->errorcode = 1;
		return E_PARSE_ERROR;
	}

	if (tree->parseEnum > BEGIN_BRANCH_TYPES && tree->parseEnum < END_BRANCH_TYPES) {
		// this is a branch node
		if (tree->parseEnum == EXP_PARSE) {
			// This is an exp expression
			// there are various types - which we need to
			// determine which this is
			// The types are:
			// exp: int
			//    | float
			//    | string
			//    | variable
			//    | exp operator exp
			//    | unioperator variable
			//    | variable unioperator
			//    | func_call
			//    | ( exp )
			//    | - exp
			//    | variable assignmentoperator exp
			// 
			// of these only the variable rules share the first token
			// these rules all have separate second tokens so we can determine the
			// rule from the first 2 tokens.
			current = tree->firstChild;

			if (current->parseEnum == CONSTANT_INT_PARSE ||
				current->parseEnum == CONSTANT_STRING_PARSE ||
				current->parseEnum == CONSTANT_FLOAT_PARSE) {
				// all one node rules
				err = interpretParseTree(current, parserInfo);
				if (err != 0) {
					return err;
				}
				tree->symbol = current->symbol;
			} else if (current->parseEnum == FUNCTION_CALL_PARSE) {
				// func_call
				// 	|----var
				// 	|----(
				// 	|----exp (or exp_list)
				// 	|----)
				//
				//

				// first get the function from the symbol table
				symbol = getSymbol(parserInfo->symbols, current->firstChild->stringValue);

				if (symbol == NULL) {
					snprintf(parserInfo->errormessage, 255, "Undefined function: %s.", current->firstChild->stringValue);
					return E_PARSE_ERROR;
				}

				if (symbol->symbolEnum != FUNCTION_SYMBOL) {
					snprintf(parserInfo->errormessage, 255, "Undefined function: %s.", current->firstChild->stringValue);
					return E_PARSE_ERROR;
				}

				// now create a variable for each of the parameters
				if (symbol->functionType->ptree != NULL) {
					increaseScope(parserInfo->symbols);

					func = copyParseTree(symbol->functionType->ptree);

					param = func->firstChild;
					param = param->rightSibling->rightSibling->rightSibling;

					while ((param != NULL) && (param->parseEnum == PARAM_LIST_PARSE)) {
						param = param->firstChild;
					}
					// value is in 
					paramvalue = current->firstChild->rightSibling->rightSibling;
					while ((paramvalue != NULL) && (paramvalue->parseEnum == EXP_LIST_PARSE)) {
						paramvalue = paramvalue->firstChild;
					}
					

					while ((param != NULL) && (param->parseEnum == PARAM_LIST_ELE_PARSE)) {
						// need the type, name and value of this variable
						if (strcmp(param->firstChild->stringValue, "int") == 0) {
							varType = INT_VARIABLE;
						} else if (strcmp(param->firstChild->stringValue, "string") == 0) {
							varType = STRING_VARIABLE;
						} else if (strcmp(param->firstChild->stringValue, "float") == 0) {
							varType = FLOAT_VARIABLE;
						} else if (strcmp(param->firstChild->stringValue, "array") == 0) {
							varType = ARRAY_VARIABLE;
						} else if (strcmp(param->firstChild->stringValue, "map") == 0) {
							varType = MAP_VARIABLE;
						}

						// name is in param->firstChild->rightSibling->stringValue
						// interpret this param value
						err = interpretParseTree(paramvalue, parserInfo);
						if (err != 0) {
							decreaseScope(parserInfo->symbols);
							return err;
						}

						switch (varType) {
							case INT_VARIABLE:
								symbol = initVariableIntSymbol(strdup(param->firstChild->rightSibling->stringValue), castSymbolToInt(paramvalue->symbol));
								break;
							case STRING_VARIABLE:
								symbol = initVariableStringSymbol(strdup(param->firstChild->rightSibling->stringValue), castSymbolToString(paramvalue->symbol));
								break;
							case FLOAT_VARIABLE:
								symbol = initVariableFloatSymbol(strdup(param->firstChild->rightSibling->stringValue), castSymbolToFloat(paramvalue->symbol));
								break;
							case ARRAY_VARIABLE:
								symbol = initVariableArraySymbol(strdup(param->firstChild->rightSibling->stringValue));
								symbol->variableType->arrayValues = copySymbolArrayType(paramvalue->symbol);
								break;
							case MAP_VARIABLE:
								symbol = initVariableMapSymbol(strdup(param->firstChild->rightSibling->stringValue));
								symbol->variableType->mapValues = copySymbolMapType(paramvalue->symbol);
								break;
							case UNINITIALISED_VARIABLE:
								break;
						}
						appendLinkedList(&symbollist, symbol);

						// move to next param decl and value

						if ((param->parent != NULL) && (param->parent->rightSibling != NULL) && (param->parent->rightSibling->parseEnum == COMMA_PARSE)) {
							param = param->parent->rightSibling->rightSibling;
						} else {
							param = NULL;
						}


						if (paramvalue->rightSibling != NULL && paramvalue->rightSibling->parseEnum == COMMA_PARSE) {
							paramvalue = paramvalue->rightSibling->rightSibling;
						} else if (paramvalue->parent != NULL && paramvalue->parent->rightSibling != NULL && paramvalue->parent->rightSibling->parseEnum == COMMA_PARSE) {
							paramvalue = paramvalue->parent->rightSibling->rightSibling;
						} else {
							paramvalue = NULL;
						}
					}

					while (symbollist != NULL) {
						symbol = (Symbol *) symbollist->data;
						putSymbol(parserInfo->symbols, symbol);
						listele = symbollist;
						symbollist = symbollist->next;
						free(listele);
					}
					symbollist = NULL;

					// added all local variables
					//
					// execute the block

					err = interpretParseTree(func->lastChild, parserInfo);
					if (err != 0 && err != E_FUNCTION_RETURN) {
						decreaseScope(parserInfo->symbols);
						return err;
					}
				
					if (err == E_FUNCTION_RETURN) {
						err = 0;
					}
					decreaseScope(parserInfo->symbols);

					if (func->symbol != NULL && func->symbol->symbolEnum != FUNCTION_SYMBOL) {
						tree->symbol = func->symbol;
						tree->symbol->symbolEnum = VARIABLE_SYMBOL;
						if (tree->symbol->variableType->variableName) {
							free(tree->symbol->variableType->variableName);
						}
						tree->symbol->variableType->variableName = strdup("");
						putSymbol(parserInfo->symbols, tree->symbol);
					} else {
						tree->symbol = initConstantIntSymbol(0);
						putSymbol(parserInfo->symbols, tree->symbol);
					}
					freeParseTree(func);
				} else {
					// run native code
					arglist = symbol->functionType->nativeArgs;

					paramvalue = current->firstChild->rightSibling->rightSibling;
					while (paramvalue && (paramvalue->parseEnum == EXP_LIST_PARSE)) {
						paramvalue = paramvalue->firstChild;
					}
					symbollist = NULL;

					i = 0;
					while (arglist != NULL) {
						arg = (Symbol *) arglist->data;
						varType = arg->variableType->variableEnum;

						// check for values
						if (paramvalue == NULL) {
							snprintf(parserInfo->errormessage, 255, "Not enough arguments for function: %s.", symbol->functionType->functionName);
							decreaseScope(parserInfo->symbols);
							return E_PARSE_ERROR;
						}

						// evalulate the argument
						err = interpretParseTree(paramvalue, parserInfo);
						if (err != 0) {
							decreaseScope(parserInfo->symbols);
							return err;
						}

						// Create the arg
						switch (varType) {
							case INT_VARIABLE:
								expValue = initVariableIntSymbol(strdup(arg->variableType->variableName), castSymbolToInt(paramvalue->symbol));
								break;
							case STRING_VARIABLE:
								expValue = initVariableStringSymbol(strdup(arg->variableType->variableName), castSymbolToString(paramvalue->symbol));
								break;
							case FLOAT_VARIABLE:
								expValue = initVariableFloatSymbol(strdup(arg->variableType->variableName), castSymbolToFloat(paramvalue->symbol));
								break;
							case ARRAY_VARIABLE:
								expValue = initVariableArraySymbol(strdup(arg->variableType->variableName));
								expValue->variableType->arrayValues = copySymbolArrayType(paramvalue->symbol);
								break;
							case MAP_VARIABLE:
								expValue = initVariableMapSymbol(strdup(arg->variableType->variableName));
								expValue->variableType->mapValues = copySymbolMapType(paramvalue->symbol);
								break;
							case UNINITIALISED_VARIABLE:
								break;
						}
						
						appendLinkedList(&symbollist, expValue);

						// get the next arg
						arglist = arglist->next;

						// get the next param value
						if (paramvalue->rightSibling != NULL && paramvalue->rightSibling->parseEnum == COMMA_PARSE) {
							paramvalue = paramvalue->rightSibling->rightSibling;
						} else if (paramvalue->parent != NULL && paramvalue->parent->rightSibling != NULL && paramvalue->parent->rightSibling->parseEnum == COMMA_PARSE) {
							paramvalue = paramvalue->parent->rightSibling->rightSibling;
						} else {
							paramvalue = NULL;
						}
					}

					// ok all arguments are in symbollist
					//
					expValue = initConstantIntSymbol(0);

					if (symbol->functionType->nativeCode(symbollist, parserInfo, expValue) != E_OK) {
						snprintf(parserInfo->errormessage, 255, "An error occurred in native function: %s", symbol->functionType->functionName);
						decreaseScope(parserInfo->symbols);
						return E_PARSE_ERROR;
					}

					freeLinkedList(symbollist, freeSymbol);

					tree->symbol = expValue;
					tree->symbol->symbolEnum = VARIABLE_SYMBOL;
					if (tree->symbol->variableType->variableName) {
						free(tree->symbol->variableType->variableName);
					}
					tree->symbol->variableType->variableName = strdup("");
					putSymbol(parserInfo->symbols, tree->symbol);

				}

			} else if (current->parseEnum == EXP_PARSE) {
				// exp operator exp
				err = interpretParseTree(current, parserInfo);
				if (err != 0) {
					return err;
				}

				// conditional execution of the right hand side
				if ((strcmp(current->rightSibling->stringValue, "&&") != 0) && (strcmp(current->rightSibling->stringValue, "||") != 0)) {
					err = interpretParseTree(current->rightSibling->rightSibling, parserInfo);
					if (err != 0) {
						return err;
					}
				}

				// do the operation
				if (strcmp(current->rightSibling->stringValue, "+") == 0) {
					tree->symbol = addSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "-") == 0) {
					tree->symbol = subtractSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "*") == 0) {
					tree->symbol = multiplySymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "/") == 0) {
					tree->symbol = divideSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					if (tree->symbol == NULL) {
						snprintf(parserInfo->errormessage, 255, "Divide by zero error.");
						return E_PARSE_ERROR;
					}
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "%") == 0) {
					tree->symbol = modSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					if (tree->symbol == NULL) {
						snprintf(parserInfo->errormessage, 255, "Attempt to mod invalid symbols.");
						return E_PARSE_ERROR;
					}
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "^") == 0) {
					tree->symbol = powerSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, ">") == 0) {
					tree->symbol = greaterThanSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, ">=") == 0) {
					tree->symbol = greaterThanEqualSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "<") == 0) {
					tree->symbol = lessThanSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "<=") == 0) {
					tree->symbol = lessThanEqualSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "&&") == 0) {
					// conditional execution of the right hand side
					if (castSymbolToInt(current->symbol)) {
						err = interpretParseTree(current->rightSibling->rightSibling, parserInfo);
						if (err != 0) {
							return err;
						}
						if (castSymbolToInt(current->rightSibling->rightSibling->symbol)) {
							tree->symbol = initConstantIntSymbol(1);
						} else {
							tree->symbol = initConstantIntSymbol(0);
						}

					} else {
						tree->symbol = initConstantIntSymbol(0);
					}
					if (tree->symbol == NULL) {
						snprintf(parserInfo->errormessage, 255, "Attempt to and invalid symbols.");
						return E_PARSE_ERROR;
					}
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "||") == 0) {
					// conditional execution of the right hand side
					if (!castSymbolToInt(current->symbol)) {
						err = interpretParseTree(current->rightSibling->rightSibling, parserInfo);
						if (err != 0) {
							return err;
						}
						if (!castSymbolToInt(current->rightSibling->rightSibling->symbol)) {
							tree->symbol = initConstantIntSymbol(0);
						} else {
							tree->symbol = initConstantIntSymbol(1);
						}
					} else {
						tree->symbol = initConstantIntSymbol(1);
					}
					if (tree->symbol == NULL) {
						snprintf(parserInfo->errormessage, 255, "Attempt to or invalid symbols.");
						return E_PARSE_ERROR;
					}
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "==") == 0) {
					tree->symbol = equalSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (strcmp(current->rightSibling->stringValue, "!=") == 0) {
					tree->symbol = notEqualSymbols(current->symbol, current->rightSibling->rightSibling->symbol);
					putSymbol(parserInfo->symbols, tree->symbol);
				}
			} else if (current->parseEnum == UNI_OPERATOR_PARSE) {
				if (strcmp(current->stringValue, "++") == 0) {
					current = current->rightSibling;
					err = interpretParseTree(current, parserInfo);
					if (err != 0) {
						return err;
					}
					
					if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
						symbol = current->symbol;
						symbol->variableType->intValue++;
						tree->symbol = initConstantIntSymbol(castSymbolToInt(current->symbol));
						putSymbol(parserInfo->symbols, tree->symbol);
					} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
						symbol = current->symbol;
						symbol->variableType->floatValue += 1.0f;
						tree->symbol = initConstantFloatSymbol(castSymbolToFloat(current->symbol));
						putSymbol(parserInfo->symbols, tree->symbol);
					} else {
						snprintf(parserInfo->errormessage, 255, "'++' operator can only be used on floats or ints.");
						return E_PARSE_ERROR;
					}

				} else if (strcmp(current->stringValue, "--") == 0) {
					current = current->rightSibling;
					err = interpretParseTree(current, parserInfo);
					if (err != 0) {
						return err;
					}
					
					if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
						symbol = current->symbol;
						symbol->variableType->intValue--;
						tree->symbol = initConstantIntSymbol(castSymbolToInt(current->symbol));
						putSymbol(parserInfo->symbols, tree->symbol);
					} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
						symbol = current->symbol;
						symbol->variableType->floatValue -= 1.0f;
						tree->symbol = initConstantFloatSymbol(castSymbolToFloat(current->symbol));
						putSymbol(parserInfo->symbols, tree->symbol);
					} else {
						snprintf(parserInfo->errormessage, 255, "'--' operator can only be used on floats or ints.");
						return E_PARSE_ERROR;
					}

				}
			} else if (current->parseEnum == OPEN_PAREN_PARSE) {
				current = current->rightSibling;
				err = interpretParseTree(current, parserInfo);
				if (err != 0) {
					return err;
				}
				tree->symbol = current->symbol;

			} else if (current->parseEnum == NEGATIVE_PARSE) {
				current = current->rightSibling;
				err = interpretParseTree(current, parserInfo);
				if (err != 0) {
					return err;
				}
					
				if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
					symbol = current->symbol;
					symbol->variableType->intValue = - symbol->variableType->intValue;
					tree->symbol = initConstantIntSymbol(castSymbolToInt(current->symbol));
					putSymbol(parserInfo->symbols, tree->symbol);
				} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
					symbol = current->symbol;
					symbol->variableType->floatValue = - symbol->variableType->floatValue;
					tree->symbol = initConstantFloatSymbol(castSymbolToFloat(current->symbol));
					putSymbol(parserInfo->symbols, tree->symbol);
				} else {
					snprintf(parserInfo->errormessage, 255, "'-' operator can only be used on floats or ints.");
					return E_PARSE_ERROR;
				}

			} else if (current->parseEnum == VARIABLE_PARSE) {
				err = interpretParseTree(current, parserInfo);
				if (err != 0) {
					return err;
				}

				if (current->rightSibling != NULL && current->rightSibling->parseEnum == ASSIGNMENT_OPERATOR_PARSE && current->rightSibling->rightSibling != NULL) {
					// this is an assignment

					err = interpretParseTree(current->rightSibling->rightSibling, parserInfo);
					if (err != 0) {
						return err;
					}

					if (strcmp(current->rightSibling->stringValue, "=") == 0) {
						if (current->symbol->variableType->variableEnum == STRING_VARIABLE) {
							free(current->symbol->variableType->stringValue);
							current->symbol->variableType->stringValue = castSymbolToString(current->rightSibling->rightSibling->symbol);
						} else if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							current->symbol->variableType->intValue = castSymbolToInt(current->rightSibling->rightSibling->symbol);
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							current->symbol->variableType->floatValue = castSymbolToFloat(current->rightSibling->rightSibling->symbol);
						} else if (current->symbol->variableType->variableEnum == ARRAY_VARIABLE) {
							freeMap(current->symbol->variableType->arrayValues);
							current->symbol->variableType->arrayValues = copySymbolArrayType(current->rightSibling->rightSibling->symbol);
						} else if (current->symbol->variableType->variableEnum == MAP_VARIABLE) {
							freeMap(current->symbol->variableType->mapValues);
							current->symbol->variableType->mapValues = copySymbolMapType(current->rightSibling->rightSibling->symbol);
						} else if (current->symbol->variableType->variableEnum == UNINITIALISED_VARIABLE) {
							mapIndex = strdup(current->symbol->variableType->variableName);
							freeVariableType(current->symbol->variableType);
							current->symbol->variableType = copySymbolVariableType(current->rightSibling->rightSibling->symbol);
							free(current->symbol->variableType->variableName);
							current->symbol->variableType->variableName = mapIndex;
						}
					} else if (strcmp(current->rightSibling->stringValue, "*=") == 0) {
						if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							current->symbol->variableType->intValue = castSymbolToInt(current->symbol) * castSymbolToInt(current->rightSibling->rightSibling->symbol);
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							current->symbol->variableType->floatValue = castSymbolToFloat(current->symbol) * castSymbolToFloat(current->rightSibling->rightSibling->symbol);
						} else if (current->symbol->variableType->variableEnum == STRING_VARIABLE) {
							stringValue = multiplyString(current->symbol->variableType->stringValue, castSymbolToInt(current->rightSibling->rightSibling->symbol));
							free(current->symbol->variableType->stringValue);
							current->symbol->variableType->stringValue = stringValue;
						} else {
							snprintf(parserInfo->errormessage, 255, "Attempt to multiply array variable (%s).", getParseEnumDesc(current));
							return E_PARSE_ERROR;
						}
					} else if (strcmp(current->rightSibling->stringValue, "/=") == 0) {
						if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							intValue = castSymbolToInt(current->rightSibling->rightSibling->symbol);
							if (intValue == 0) {
								snprintf(parserInfo->errormessage, 255, "Divide by zero error.");
								return E_PARSE_ERROR;
							}
							current->symbol->variableType->intValue = castSymbolToInt(current->symbol) / intValue;
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							floatValue = castSymbolToFloat(current->rightSibling->rightSibling->symbol);
							if (floatValue == 0.0f) {
								snprintf(parserInfo->errormessage, 255, "Divide by zero error.");
								return E_PARSE_ERROR;
							}
							current->symbol->variableType->floatValue = castSymbolToFloat(current->symbol) / floatValue;
						} else {
							snprintf(parserInfo->errormessage, 255, "Attempt to divide non numeric variable (%s).", getParseEnumDesc(current));
							return E_PARSE_ERROR;
						}
					} else if (strcmp(current->rightSibling->stringValue, "+=") == 0) {
						if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							intValue = castSymbolToInt(current->rightSibling->rightSibling->symbol);
							current->symbol->variableType->intValue = castSymbolToInt(current->symbol) + intValue;
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							floatValue = castSymbolToFloat(current->rightSibling->rightSibling->symbol);
							current->symbol->variableType->floatValue = castSymbolToFloat(current->symbol) + floatValue;
						} else if (current->symbol->variableType->variableEnum == STRING_VARIABLE) {
							stringValue = castSymbolToString(current->symbol);
							stringValue2 = castSymbolToString(current->rightSibling->rightSibling->symbol);
							free(current->symbol->variableType->stringValue);
							current->symbol->variableType->stringValue = NULL;
							vstrdupcat(&(current->symbol->variableType->stringValue), stringValue, stringValue2, NULL);
							free(stringValue);
							free(stringValue2);
						} else {
							snprintf(parserInfo->errormessage, 255, "Attempt to add array variable (%s).", getParseEnumDesc(current));
							return E_PARSE_ERROR;
						}
					} else if (strcmp(current->rightSibling->stringValue, "-=") == 0) {
						if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							intValue = castSymbolToInt(current->rightSibling->rightSibling->symbol);
							current->symbol->variableType->intValue = castSymbolToInt(current->symbol) - intValue;
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							floatValue = castSymbolToFloat(current->rightSibling->rightSibling->symbol);
							current->symbol->variableType->floatValue = castSymbolToFloat(current->symbol) - floatValue;
						} else if (current->symbol->variableType->variableEnum == STRING_VARIABLE) {
							stringValue = castSymbolToString(current->symbol);
							stringValue2 = castSymbolToString(current->rightSibling->rightSibling->symbol);
							free(current->symbol->variableType->stringValue);
							current->symbol->variableType->stringValue = subtractString(stringValue, stringValue2);
							free(stringValue);
							free(stringValue2);
						} else {
							snprintf(parserInfo->errormessage, 255, "Attempt to add array variable (%s).", getParseEnumDesc(current));
							return E_PARSE_ERROR;
						}
					} else if (strcmp(current->rightSibling->stringValue, "%=") == 0) {
						if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							intValue = castSymbolToInt(current->rightSibling->rightSibling->symbol);
							current->symbol->variableType->intValue = castSymbolToInt(current->symbol) % intValue;
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							intValue = castSymbolToFloat(current->rightSibling->rightSibling->symbol);
							current->symbol->variableType->floatValue = castSymbolToInt(current->symbol) % intValue;
						} else {
							snprintf(parserInfo->errormessage, 255, "Attempt to mod non numeric variable (%s).", getParseEnumDesc(current));
							return E_PARSE_ERROR;
						}
					} else if (strcmp(current->rightSibling->stringValue, "^=") == 0) {
						if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							intValue = castSymbolToInt(current->rightSibling->rightSibling->symbol);
							if (intValue == 0) {
								snprintf(parserInfo->errormessage, 255, "Divide by zero error.");
								return E_PARSE_ERROR;
							}
							current->symbol->variableType->intValue = powl(castSymbolToInt(current->symbol), intValue);
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							floatValue = castSymbolToFloat(current->rightSibling->rightSibling->symbol);
							if (floatValue == 0.0f) {
								snprintf(parserInfo->errormessage, 255, "Divide by zero error.");
								return E_PARSE_ERROR;
							}
							current->symbol->variableType->floatValue = powf(castSymbolToFloat(current->symbol), floatValue);
						} else {
							snprintf(parserInfo->errormessage, 255, "Attempt to power non numeric variable (%s).", getParseEnumDesc(current));
							return E_PARSE_ERROR;
						}
					}

					tree->symbol = copySymbol(current->symbol);
					tree->symbol->symbolEnum = VARIABLE_SYMBOL;
					if (tree->symbol->variableType->variableName) {
						free(tree->symbol->variableType->variableName);
					}
					tree->symbol->variableType->variableName = strdup("");
					putSymbol(parserInfo->symbols, tree->symbol);

				} else if (current->rightSibling != NULL && current->rightSibling->parseEnum == UNI_OPERATOR_PARSE) {
					if (strcmp(current->rightSibling->stringValue, "++") == 0) {
						if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							symbol = current->symbol;
							tree->symbol = initConstantIntSymbol(castSymbolToInt(current->symbol));
							putSymbol(parserInfo->symbols, tree->symbol);
							symbol->variableType->intValue++;
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							symbol = current->symbol;
							tree->symbol = initConstantFloatSymbol(castSymbolToFloat(current->symbol));
							putSymbol(parserInfo->symbols, tree->symbol);
							symbol->variableType->floatValue += 1.0f;
						} else {
							snprintf(parserInfo->errormessage, 255, "'++' operator can only be used on floats or ints.");
							return E_PARSE_ERROR;
						}
						
					} else if (strcmp(current->rightSibling->stringValue, "--") == 0) {
						if (current->symbol->variableType->variableEnum == INT_VARIABLE) {
							symbol = current->symbol;
							tree->symbol = initConstantIntSymbol(castSymbolToInt(current->symbol));
							putSymbol(parserInfo->symbols, tree->symbol);
							symbol->variableType->intValue--;
						} else if (current->symbol->variableType->variableEnum == FLOAT_VARIABLE) {
							symbol = current->symbol;
							tree->symbol = initConstantFloatSymbol(castSymbolToFloat(current->symbol));
							putSymbol(parserInfo->symbols, tree->symbol);
							symbol->variableType->floatValue -= 1.0f;
						} else {
							snprintf(parserInfo->errormessage, 255, "'--' operator can only be used on floats or ints.");
							return E_PARSE_ERROR;
						}
					}
				} else {
					tree->symbol = copySymbol(current->symbol);
					tree->symbol->symbolEnum = VARIABLE_SYMBOL;
					if (tree->symbol->variableType->variableName) {
						free(tree->symbol->variableType->variableName);
					}
					tree->symbol->variableType->variableName = strdup("");
					putSymbol(parserInfo->symbols, tree->symbol);
				}
			}

			
		} else if (tree->parseEnum == BLOCK_PARSE) {
			if (tree->firstChild && tree->firstChild->rightSibling) {
				// execute first statement
				increaseScope(parserInfo->symbols);
				err = interpretParseTree(tree->firstChild->rightSibling, parserInfo);
				if (err != 0) {
					decreaseScope(parserInfo->symbols);
					return err;
				}
				decreaseScope(parserInfo->symbols);
			}
		} else if (tree->parseEnum == STATEMENT_PARSE || tree->parseEnum == INPUT_PARSE) {
			if (tree->firstChild) {
				// execute any lower nodes
				err = interpretParseTree(tree->firstChild, parserInfo);
				if (err != 0) {
					return err;
				}
				
			}
			if (tree->rightSibling) {
				// execute any right hand nodes
				err = interpretParseTree(tree->rightSibling, parserInfo);
				if (err != 0) {
					return err;
				}
			}
		} else if (tree->parseEnum == VARIABLE_PARSE) {
			// get this variable from the symbol table	
			err = interpretParseTree(tree->firstChild, parserInfo);
			if (err != 0) {
				return err;
			}
			tree->symbol = tree->firstChild->symbol;
			// first check if this is an array or a map

			if (tree->rightSibling && tree->rightSibling->parseEnum == OPEN_ARRAY_INDEX_PARSE) {
				if (tree->symbol->variableType->variableEnum == ARRAY_VARIABLE) {
					err = interpretParseTree(tree->rightSibling->rightSibling, parserInfo);
					if (err != 0) {
						return err;
					}

					arrayIndex = castSymbolToInt(tree->rightSibling->rightSibling->symbol);

					// get symbol
					tree->symbol = getSymbolArrayValue(tree->symbol->variableType->arrayValues, arrayIndex);
					
					if (tree->symbol == NULL) {
						snprintf(parserInfo->errormessage, 255, "An undefined array value was referenced (%d).", arrayIndex);
						return E_PARSE_ERROR;
					}
				
					if (tree->symbol->symbolEnum != VARIABLE_SYMBOL || tree->symbol->variableType->variableEnum == UNINITIALISED_VARIABLE) {
						snprintf(parserInfo->errormessage, 255, "An uninitialised array value was referenced (%d).", arrayIndex);
						return E_PARSE_ERROR;
					}
				} else if (tree->symbol->variableType->variableEnum == MAP_VARIABLE) {
					err = interpretParseTree(tree->rightSibling->rightSibling, parserInfo);
					if (err != 0) {
						return err;
					}

					mapIndex = castSymbolToString(tree->rightSibling->rightSibling->symbol);

					// get symbol
					tree->symbol = getSymbolMapValue(tree->symbol->variableType->mapValues, mapIndex);
				
					if (tree->symbol == NULL) {
						snprintf(parserInfo->errormessage, 255, "An undefined map value was referenced (%s).", mapIndex);
						return E_PARSE_ERROR;
					}
				
					if (tree->symbol->symbolEnum != VARIABLE_SYMBOL || tree->symbol->variableType->variableEnum == UNINITIALISED_VARIABLE) {
						snprintf(parserInfo->errormessage, 255, "An uninitialised map value was referenced (%s).", mapIndex);
						return E_PARSE_ERROR;
					}

				} else {
					snprintf(parserInfo->errormessage, 255, "Attempt to access simple variable (%s) as collection type.", getParseEnumDesc(current));
					return E_PARSE_ERROR;
				}
			}

		} else if (tree->parseEnum == FUNCTION_DECL_PARSE) {
			// basically, we just push this symbol into the symbol table 
			tree->symbol = initFunctionSymbol(tree->firstChild->rightSibling->stringValue, tree);
			putSymbol(parserInfo->symbols, tree->symbol);
		} else {
			if (tree->firstChild) {
				// execute any lower nodes
				err = interpretParseTree(tree->firstChild, parserInfo);
				if (err != 0) {
					return err;
				}
				// copy the value from the lower node
				tree->symbol = tree->firstChild->symbol;

			}
		}
	} else if (tree->parseEnum > BEGIN_LEAF_TYPES && tree->parseEnum < END_LEAF_TYPES) {
		// this is a leaf node
		if (tree->parseEnum > BEGIN_CONTROL_TYPES && tree->parseEnum < END_CONTROL_TYPES) {
			//
			if (tree->parseEnum == IF_KEYWORD_PARSE) {
				current = tree->rightSibling->rightSibling;	
				err = interpretParseTree(current, parserInfo);
				if (err != 0) {
					return err;
				}

				if (castSymbolToInt(current->symbol)) {
					current = current->rightSibling->rightSibling;
					err = interpretParseTree(current, parserInfo);
					if (err != 0) {
						return err;
					}
				} else {
					if ((current->rightSibling->rightSibling->rightSibling != NULL) && (current->rightSibling->rightSibling->rightSibling->parseEnum == ELSE_KEYWORD_PARSE)) {
						current = current->rightSibling->rightSibling->rightSibling->rightSibling;
						err = interpretParseTree(current, parserInfo);
						if (err != 0) {
							return err;
						}
					}
				}
				
			} else if (tree->parseEnum == WHILE_KEYWORD_PARSE) {
				// while ( exp ) block
				// execute the exp
				err = interpretParseTree(tree->rightSibling->rightSibling, parserInfo);
				if (err != 0) {
					return err;
				}

				current = tree->rightSibling->rightSibling->rightSibling->rightSibling;
				// if the exp is true
				while (castSymbolToInt(tree->rightSibling->rightSibling->symbol)) {
					// execute the block
					err = interpretParseTree(current, parserInfo);
					if (err != 0) {
						return err;
					}
					// reexecute the exp
					err = interpretParseTree(tree->rightSibling->rightSibling, parserInfo);
					if (err != 0) {
						return err;
					}
				}
			} else if (tree->parseEnum == DO_KEYWORD_PARSE) {
				// do block while ( exp );
				// execute the block
				err = interpretParseTree(tree->rightSibling, parserInfo);
				if (err != 0) {
					return err;
				}

				current = tree->rightSibling->rightSibling->rightSibling->rightSibling;
				// execute the exp
				err = interpretParseTree(current, parserInfo);
				if (err != 0) {
					return err;
				}

				// if the exp is true
				while (castSymbolToInt(current->symbol)) {
					// execute the block
					err = interpretParseTree(tree->rightSibling, parserInfo);
					if (err != 0) {
						return err;
					}
					// reexecute the exp
					err = interpretParseTree(current, parserInfo);
					if (err != 0) {
						return err;
					}
				}
			} else if (tree->parseEnum == FOR_KEYWORD_PARSE) {
				// for ( exp ; exp ; exp ) block
				// execute the first exp
				err = interpretParseTree(tree->rightSibling->rightSibling, parserInfo);
				if (err != 0) {
					return err;
				}

				// execute the second exp
				err = interpretParseTree(tree->rightSibling->rightSibling->rightSibling->rightSibling, parserInfo);
				if (err != 0) {
					return err;
				}

				// evaluate the second exp
				current = tree->rightSibling->rightSibling->rightSibling->rightSibling->rightSibling->rightSibling->rightSibling->rightSibling;
				while (castSymbolToInt(tree->rightSibling->rightSibling->rightSibling->rightSibling->symbol)) {
					// evaluate the block
					err = interpretParseTree(current, parserInfo);
					if (err != 0) {
						return err;
					}

					// evaluate the third exp
					err = interpretParseTree(tree->rightSibling->rightSibling->rightSibling->rightSibling->rightSibling->rightSibling, parserInfo);
					if (err != 0) {
						return err;
					}

					// re-evaluate the second exp
					err = interpretParseTree(tree->rightSibling->rightSibling->rightSibling->rightSibling, parserInfo);
					if (err != 0) {
						return err;
					}
				}



			}

		} else {
			if (tree->parseEnum == CONSTANT_INT_PARSE) {
				// This is a constant
				tree->symbol = initConstantIntSymbolString(tree->stringValue);
				putSymbol(parserInfo->symbols, tree->symbol);
			} else if (tree->parseEnum == CONSTANT_FLOAT_PARSE) {
				// This is a constant
				tree->symbol = initConstantFloatSymbolString(tree->stringValue);
				putSymbol(parserInfo->symbols, tree->symbol);
			} else if (tree->parseEnum == CONSTANT_STRING_PARSE) {
				// This is a constant
				tree->symbol = initConstantStringSymbol(tree->stringValue);
				putSymbol(parserInfo->symbols, tree->symbol);
			} else if (tree->parseEnum == VAR_PARSE) {
				// get this symbol from the symbol table
				tree->symbol = getSymbol(parserInfo->symbols, tree->stringValue);
				

				if (tree->symbol == NULL) {
					snprintf(parserInfo->errormessage, 255, "An undefined variable was referenced (%s).", tree->stringValue);
					return E_PARSE_ERROR;
				}
				
				if (tree->symbol->symbolEnum != VARIABLE_SYMBOL || tree->symbol->variableType->variableEnum == UNINITIALISED_VARIABLE) {
					snprintf(parserInfo->errormessage, 255, "An uninitialised variable was referenced (%s).", tree->stringValue);
					return E_PARSE_ERROR;
				}

			} else if (tree->parseEnum == TYPE_PARSE) {
				// This is a type declaration
				// the symbol is NULL but we need to walk the list and add
				// the variables
				//
				if (strcmp(tree->stringValue, "int") == 0) {
					varType = INT_VARIABLE;
				} else if (strcmp(tree->stringValue, "string") == 0) {
					varType = STRING_VARIABLE;
				} else if (strcmp(tree->stringValue, "float") == 0) {
					varType = FLOAT_VARIABLE;
				} else if (strcmp(tree->stringValue, "array") == 0) {
					varType = ARRAY_VARIABLE;
				} else if (strcmp(tree->stringValue, "map") == 0) {
					varType = MAP_VARIABLE;
				} else {
					varType = UNINITIALISED_VARIABLE;
				}

				current = tree->rightSibling;
				if (!current || current->parseEnum != VAR_LIST_PARSE) {
					snprintf(parserInfo->errormessage, 255, "Expected VAR_LIST_PARSE got %s.", getParseEnumDesc(current));
					parserInfo->errorcode = 1;
					return E_PARSE_ERROR;
				}

				// first var is deepest
				while ((current != NULL) && (current->parseEnum == VAR_LIST_PARSE)) {
					current = current->firstChild;
				}

				// while current != NULL
				while (current != NULL) {
					// add variable
					expValue = NULL;
					if (current->firstChild->rightSibling && 
							current->firstChild->rightSibling->rightSibling) {
						err = interpretParseTree(current->firstChild->rightSibling->rightSibling, parserInfo);
						if (err != 0) {
							return err;
						}
						expValue = current->firstChild->rightSibling->rightSibling->symbol;
					}

					symbol = getSymbolShallow(parserInfo->symbols, current->firstChild->stringValue);
					if (symbol == NULL) {
						symbol = initVariableIntSymbol(strdup(current->firstChild->stringValue), 0);
						putSymbol(parserInfo->symbols, symbol);
					}
					if (symbol->symbolEnum == VARIABLE_SYMBOL) {
						if (symbol->variableType->variableEnum == STRING_VARIABLE) {
							dhufree(symbol->variableType->stringValue);
							symbol->variableType->stringValue = NULL;
						} else if (symbol->variableType->variableEnum == ARRAY_VARIABLE) {
							freeMap(symbol->variableType->arrayValues);
							symbol->variableType->arrayValues = NULL;
						} else if (symbol->variableType->variableEnum == MAP_VARIABLE) {
							freeMap(symbol->variableType->mapValues);
							symbol->variableType->mapValues = NULL;
						}
					} else {
						snprintf(parserInfo->errormessage, 255, "Cannot create variable - name clashes with existing function %s.", current->firstChild->stringValue);
						parserInfo->errorcode = 1;
						return E_PARSE_ERROR;
					}

					switch(varType) {
						case INT_VARIABLE: 
							symbol->variableType->variableEnum = INT_VARIABLE;
							symbol->variableType->intValue = castSymbolToInt(expValue);
							break;
						case FLOAT_VARIABLE: 
							symbol->variableType->variableEnum = FLOAT_VARIABLE;
							symbol->variableType->floatValue = castSymbolToFloat(expValue);
							break;
						case STRING_VARIABLE: 
							symbol->variableType->variableEnum = STRING_VARIABLE;
							symbol->variableType->stringValue = castSymbolToString(expValue);
							break;
						case ARRAY_VARIABLE: 
							symbol->variableType->variableEnum = ARRAY_VARIABLE;
							symbol->variableType->arrayValues = copySymbolArrayType(expValue);
							break;
						case MAP_VARIABLE: 
							symbol->variableType->variableEnum = MAP_VARIABLE;
							symbol->variableType->mapValues = copySymbolMapType(expValue);
							break;
						default:
							symbol = initVariableSymbol(strdup(current->firstChild->stringValue));
							break;
					}

					// find next var
					current = current->parent;
					if (current->rightSibling != NULL 
							&& current->rightSibling->parseEnum == COMMA_PARSE
							&& current->rightSibling->rightSibling
							&& current->rightSibling->rightSibling->parseEnum == VAR_LIST_ELE_PARSE) {
						current = current->rightSibling->rightSibling;
					} else {
						current = NULL;
					}
				}

				// End of type decl
			} else if (tree->parseEnum == RETURN_KEYWORD_PARSE) {
				// return exp ;
				// evaluate exp;
				err = interpretParseTree(tree->rightSibling, parserInfo);
				if (err != 0 && err != E_FUNCTION_RETURN) {
					return err;
				}

				// walk up the tree looking for functions, copy this symbol to that point of the tree
				current = tree;
				while (current != NULL && current->parseEnum != FUNCTION_DECL_PARSE) {
					current = current->parent;
					while (current->leftSibling != NULL) current = current->leftSibling;
				}

				if (current == NULL) {
					snprintf(parserInfo->errormessage, 255, "Return keyword can only be used from within a function.");
					return E_PARSE_ERROR;
				}

				current->symbol = copySymbol(tree->rightSibling->symbol);
				current->symbol->symbolEnum = VARIABLE_SYMBOL;
				if (current->symbol->variableType->variableName) {
					free(current->symbol->variableType->variableName);
				}
				current->symbol->variableType->variableName = strdup("");
				return E_FUNCTION_RETURN;
			}
		}
	}	
	return E_OK;
}

/*******************************************************************************
* parseNextSection...
*
* Process the next section of script.
*******************************************************************************/
char *parseNextSection(char *start, char **source, int section, int objectid, ParserInfo *info) {
  char *output = NULL, *current = NULL, *openstart = NULL,
       *openend = NULL, *closestart = NULL, *closeend = NULL, *filename = NULL, *msg = NULL;
  int i = section, errcode = E_OK;
  ParseParam param;

  current = *source;
  if (current == NULL || *current == '\0')
    return NULL;

  while (miniRegex(current, SCRIPTSTARTTAG, &openstart, &openend) == E_OK) {
    if (miniRegex(openend, SCRIPTENDTAG, &closestart, &closeend) != E_OK) {
      current = openend + 1;
    } else {
      setLineNo(getLineNo(start, openstart) + 1);
      *openstart = '\0';
      vstrdupcat(&output, current, NULL);
      current = openend + 1;
      *openstart = '<';

#ifdef COMPILE_PARSE_TREE
      generateScriptPath(objectid, i, &filename);

      if (loadParseTree(filename, &(param.ptree), &(info->symbols)) != E_OK) {
        param.ptree = NULL;
      }
#endif

      if (param.ptree == NULL) {
        // run the parser
        memset(&param, 0, sizeof(ParseParam));
        setParserInput(current);
        *closestart = '\0';
        // parse the input and put the parse tree in param.ptree
        errcode = yyparse(&param);
        *closestart = '?';

	if (errcode == E_OK) {
      	  // save the parse tree for later
#ifdef COMPILE_PARSE_TREE
          if (saveParseTree(param.ptree, filename) != E_OK) {
            logError("Script error: Could not compile script to %s.\n", filename);
	  }
	  free(filename);
#endif
	} else {
          // run a syntax checker on the input
	  errcode = checkSyntaxGetPosition(current, closestart - current, start, &msg);
	  if (errcode != E_OK) {
            vstrdupcat(&output, "<!-- Parse error: (", param.errormessage, ") more info (", msg, ") -->", NULL);
	    free(msg);
	  } else {
            vstrdupcat(&output, "<!-- Parse error: (", param.errormessage, ") -->", NULL);
	  }
	}
      }      

      info->output = output;
      if (errcode == E_OK) {
        if ((errcode = interpretParseTree(param.ptree, info)) != E_OK) {
	  // run a syntax checker
	  errcode = checkSyntaxGetPosition(current, closestart - current, start, &msg);
	  if (errcode != E_OK) {
            logError("Script error: (%s) more info (%s)\n", info->errormessage, msg);
            vstrdupcat(&output, "<!-- Runtime error: (", info->errormessage, ") more info (", msg, ") -->", NULL);
	    free(msg);
	  } else {
            logError("Script error: (%s)\n", info->errormessage);
            vstrdupcat(&output, "<!-- Runtime error: (", info->errormessage, ") -->", NULL);
	  }
        }
        freeParseTree(param.ptree);

      }
      // reset any errors
      errcode = E_OK;
      output = info->output;
      current = closeend;
      i++;
    }
  }

  vstrdupcat(&output, current, NULL);
  *source = NULL;
  return output;
}


/*******************************************************************************
* parseScript...
*
* Process any  script tags in the source.
*******************************************************************************/
int parseScript(char **filecontents, int *filelen, int objectid, ParserInfo *info) {
  char *ptr = NULL,
       *section = NULL,
       *output = NULL;
  Queue *queue = NULL;
  char *current = NULL;
  int totallen = 0, i = 0;
  output = (char *) dhumalloc(sizeof(char) * (*filelen + 1));

  memcpy(output, *filecontents, *filelen);
  output[*filelen] = '\0';

  queue = initQueue();
  ptr = output;
  while ((section = parseNextSection(output, &ptr, i++, objectid, info)) != NULL) {
    pushQueue(queue, section);
  }
  dhufree(output);

  totallen = 1;
  for (i = 0; i < countQueue(queue); i++) {
    current = (char *) sniffNQueue(queue, i);
    totallen += strlen(current?current:"");
  }

  dhufree(*filecontents);
  output = (char *) dhumalloc(sizeof(char) * (totallen));

  totallen = 0;
  while ((section = (char *) popQueue(queue)) != NULL) {
    memcpy(output + totallen, section, strlen(section));
    totallen += strlen(section);
    dhufree(section);
  }
  output[totallen] = '\0';
  freeQueue(&queue);
  *filelen = totallen;
  *filecontents = output;
  return E_OK;
}

