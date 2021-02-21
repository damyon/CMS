/*******************************************************************************
* api...
*
* This contains the api calls for the script language.
*******************************************************************************/

#ifndef _API_H
#define _API_H

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************************************
* evalFunction... 
*
* Switch and call the right function.
*********************************************************************/
// int evalFunction(int functionid, ArgumentList *list, char **output, int *destvaltype, char **destvalue, Env *env, void *sqlsock, SymbolTable *symbols);
//
int registerFunctions(SymbolTable *symbols);

#ifdef __cplusplus
}
#endif


#endif // _API_H
