/*******************************************************************************
 * xml.h
 *
 * @author Damyon Wiese
 * @date 17/10/2003
 * @description This file sends messages to the log file. In addition, it will
 * 		monitor the size of the log file and perform automatically
 * 		rotate the log files when they reach a specified limit.
 *
 ******************************************************************************/

// This is used to prevent this header file from being included multiple times.
#ifndef _XML_H_
#define _XML_H_

#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _XMLAttribute_ {
  char *name;
  char *value;
  char *nameSpace;
  struct _XMLAttribute_ *next;
} XMLAttribute;

typedef struct _XMLNameSpace_ {
  char *alias;
  char *href;
  struct _XMLNameSpace_ *next;
} XMLNameSpace;

typedef struct _XMLNode_ {
  char *name;
  char *value;
  char *nameSpace;
  XMLNameSpace *nameSpaceDecl;
  XMLAttribute *attributes;
	struct _XMLNode_ *firstChild;
	struct _XMLNode_ *rightSibling;
	struct _XMLNode_ *parent;
	struct _XMLNode_ *leftSibling;
} XMLNode;

typedef struct _XMLParser_ {
  XMLNode *root;
  XMLNode *current;
} XMLParser;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * importXML
 *
 * This function initalise a xml parser to parse a xml char buffer.
 *
 * @param src -         A pointer to the xml char buffer
 * @param XmlParser -   A pointer to the xml parser structure
 * @return NOERROR for a win.
 ******************************************************************************/
int importXML(char *src, XMLParser **xmlp);

/*******************************************************************************
 * exportXML
 *
 * This function constructs a char buffer of xml data from the current parse tree.
 *
 * @param XmlParser -   A pointer to the xml parser structure
 * @param output -      A pointer to the xml char buffer to be filled
 * @return NOERROR for a win.
 ******************************************************************************/
int exportXML(XMLParser *xmlp, char **output);

/*******************************************************************************
 * printXML
 *
 * This function prints the contents of the xml parser to stdout
 *
 * @param XmlParser -   A pointer to the xml parser structure
 * @return NOERROR for a win.
 ******************************************************************************/
int printXML(XMLParser *xmlp);

/*******************************************************************************
 * initXMLParser
 *
 * This function initalise an empty xml parser.
 *
 * @param XmlParser -   A pointer to the xml parser structure
 * @return NOERROR for a win.
 ******************************************************************************/
int initXMLParser(XMLParser **xmlp);

/*******************************************************************************
 * freeXMLParser
 *
 * This function free the memory allocated by a XML Parser.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
void freeXMLParser(XMLParser **xmlp);

/*******************************************************************************
 * getCurrentTagName
 *
 * This function will return the name of the tag that the cursor is pointing at.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int getCurrentTagName(XMLParser *xmlp, const char **name);

/*******************************************************************************
 * getCurrentTagValue
 *
 * This function will return the text between the start and end of the current tag.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int getCurrentTagValue(XMLParser *xmlp, const char **value);

/*******************************************************************************
 * getCurrentTagAttributes
 *
 * This function will return list of attributes for the current tag.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param attr -        A pointer to an XMLAtrribute Structure that 
 *                      will be populated with the list of attributes for the
 *                      current xml node.
 ******************************************************************************/
int getCurrentTagAttributes(XMLParser *xmlp, XMLAttribute **attr);

/*******************************************************************************
 * getCurrentTagAttributeValue
 *
 * This function will return the value of the named attribute in the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the attribute to query.
 * @param value -       A char poiner that will be populated with the value
 ******************************************************************************/
int getCurrentTagAttributeValue(XMLParser *xmlp, char *name, char *nameSpace, const char **value);

/*******************************************************************************
 * moveToNextSibling
 *
 * This function will move the cursor to the next sibling of the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int moveToNextSibling(XMLParser *xmlp);

/*******************************************************************************
 * moveToFirstChild
 *
 * This function will move the cursor to the first child of the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int moveToFirstChild(XMLParser *xmlp);

/*******************************************************************************
 * moveToParent
 *
 * This function will move the cursor to the parent of the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int moveToParent(XMLParser *xmlp);

/*******************************************************************************
 * addChildNode
 *
 * This function will insert a new xml node as a child of the current node
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the new node to insert
 * @param value -       The value of the new node to insert
 ******************************************************************************/
int addChildNode(XMLParser *xmlp, char *name, char *value, char *nameSpace);

/*******************************************************************************
 * addSiblingNode
 *
 * This function will insert a new xml node as a sibling of the current node
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the new node to insert
 * @param value -       The value of the new node to insert
 ******************************************************************************/
int addSiblingNode(XMLParser *xmlp, char *name, char *value, char *nameSpace);

/*******************************************************************************
 * addAttribute
 *
 * This function will insert a new attribute for the current node.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the new node to insert
 * @param value -       The value of the new node to insert
 ******************************************************************************/
int addAttribute(XMLParser *xmlp, char *name, char *value, char *nameSpace);

/*******************************************************************************
 * moveToRoot
 *
 * This function will move the cursor to the root of the xml tree.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int moveToRoot(XMLParser *xmlp);

/*******************************************************************************
 * addNameSpaceDecl
 *
 * This function will insert a new NameSpace for the current node.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the new NameSpace to insert
 * @param value -       The value of the new NameSpace to insert
 ******************************************************************************/
int addNameSpaceDecl(XMLParser *xmlp, char *alias, char *href);

/*******************************************************************************
 * getCurrentTagNameSpaceAlias
 *
 * This function will return the alias of the nameSpace in the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param href -        The href of the nameSpace to query.
 * @param alias -       A char poiner that will be populated with the value
 ******************************************************************************/
int getCurrentTagNameSpaceAlias(XMLParser *xmlp, char *href, const char **alias);

/*******************************************************************************
 * getUniqueNameSpaceAlias
 *
 * This function will return a unique alias for a new nameSpace in the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param href -        The href of the nameSpace to query.
 * @param alias -       A char poiner that will be populated with the value
 ******************************************************************************/
int getUniqueNameSpaceAlias(XMLParser *xmlp, char **alias);

/*******************************************************************************
 * getCurrentTagNameSpaceValue
 *
 * This function will return the href of the named nameSpace in the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param alias -        The alias of the nameSpace to query.
 * @param href -       A char poiner that will be populated with the value
 ******************************************************************************/
int getCurrentTagNameSpaceValue(XMLParser *xmlp, char *alias, const char **href);

/*******************************************************************************
 * getCurrentTagNameSpace
 *
 * This function will return the nameSpace of the tag that the cursor is pointing at.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int getCurrentTagNameSpace(XMLParser *xmlp, const char **name);

#ifdef __cplusplus
}
#endif


// This is used to prevent this header file from being included multiple times.
#endif // _XML_H

