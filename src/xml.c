/*******************************************************************************
 * xml.h
 *
 * @author Damyon Wiese
 * @date 17/10/2003
 * @description This function contains the functions required to perform 
 * the reading and writing to and from xml files.
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "xml.h"
#include "malloc.h"
#include "errors.h"
#include <stdio.h>
#include <ctype.h>
#include "strings.h"
#ifdef WIN32
#include "win32.h"
#endif

/*******************************************************************************
 * functions
 ******************************************************************************/
typedef enum _TagType_ {STARTTAG, ENDTAG, FULLTAG} TagType;

/*******************************************************************************
 * getTagType
 *
 * This function examines the current tag and determines it's type.
 *
 * @param startTag -         A pointer to the start of the tag
 * @param endTag -   A pointer to the end of the tag
 * @return the tag type
 ******************************************************************************/
TagType getTagType(char *start, char *end) {
  if (end - start < 2)
    return STARTTAG;
  if (start[1] == '/')
    return ENDTAG;
  if (*(end - 2) == '/')
    return FULLTAG;
  return STARTTAG;
}

/*******************************************************************************
 * getTagValue
 *
 * This function extracts the value of the current tag.
 *
 * @param start -         A pointer to the start of the current tag.
 * @param end -   A pointer to the end of the current tag.
 * @param value - A pointer to fill with the value of the current tag.
 * @return E_OK for a win.
 ******************************************************************************/
int getTagValue(char *start, char *end, char **value) {
  char *s = NULL, *e = NULL, c = '\0', *v = NULL;
  s = end;
  e = strchr(s, '<');
  
  if (!e)
    return NODATAFOUND;
  
  c = *e;
  *e = '\0';
  v = strdup(s);
  *e = c;
  // trim witespace
  e = v;
  while (isspace(*e)) {
    e++;
  }
  if (*e == '\0')
    *v = '\0';
  *value = v;
  return E_OK;
}

/*******************************************************************************
 * addTagAttributes
 *
 * This function extracts attributes of the current tag and adds them to
 * the parse tree.
 *
 * @param parser -         A pointer to the parse tree.
 * @param start -   A pointer to the start of the current tag.
 * @param end -   A pointer to the end of the current tag.
 * @return E_OK for a win.
 ******************************************************************************/
int addTagAttributes(XMLParser *parser, char *start, char *end) {
  char *s = NULL, *e = NULL, *name = NULL, *value = NULL, c = '\0', quote = '"';

  
  // get to the start of the tag name
  s = start + 1;
  if (*s == '/') s++;
  while (isspace(*s)) s++;
  
  // skip the tag name
  while (isalnum(*s) || *s == '_' || *s == ':' || *s == '-') s++;

  // get to the start of the next attribute
  while (isspace(*s)) s++;
  
  // if this is another attribute keep going
  while (isalpha(*s)) {
    e = s;
    while (isalnum(*e) || *e == ':' || *e == '_' || *e == '-') e++;
    
    // extract the name    
    c = *e;
    *e = '\0';
    name = strdup(s);
    *e = c;
    s = e;
    // skip white space
    while (isspace(*s)) s++;
      
    // check for =
    if (*s != '=') {
      dhufree(name);
      return INCONSISTENTDATA;
    }
    s++;
    // skip white space
    while (isspace(*s)) s++;
      
    //check for "
    if (*s != '"' && *s != '\'') {
      dhufree(name);
      return INCONSISTENTDATA;
    }
    quote = *s;
    s++;
    // get closing "
    e = strchr(s, quote);
    if (e == NULL) {
      dhufree(name);
      return INCONSISTENTDATA;
    }
    // extract the value
    c = *e;
    *e = '\0';
    value = strdup(s);
    *e = c;
    s = e+1;

    if (strncmp(name, "xmlns", 5) == 0) {
      // this is a nameSpace decl
      if (*value == '\0') {
        dhufree(value);
        value = strdup("unknown");
      }
      e = strchr(name, ':');
      if (e == NULL) {
        getUniqueNameSpaceAlias(parser, &e);
        addNameSpaceDecl(parser, e, value);
        if (parser->current->nameSpace != NULL) {
          dhufree(parser->current->nameSpace);
        }
        parser->current->nameSpace = dhustrdup(e);
      } else {
        e++;
        addNameSpaceDecl(parser, strdup(e), value);
      }
      dhufree(name);
      
    } else {
      e = strchr(name, ':');
      if (e != NULL) {
        // this attribute has a namespace
        *e = '\0';
        addAttribute(parser, dhustrdup(e+1), value, name);
      } else {
        // this is an attribute with no namespace
        addAttribute(parser, name, value, NULL);
      }
    }
    while (isspace(*s)) s++;
  }
  
  return E_OK;
}

/*******************************************************************************
 * getTagNameSpace
 *
 * This function extracts the name of the current tag.
 *
 * @param start -         A pointer to the start of the current tag.
 * @param end -   A pointer to the end of the current tag.
 * @param name - A pointer to fill with the name of the current tag.
 * @return E_OK for a win.
 ******************************************************************************/
int getTagNameSpace(char *start, char *end, char **name) {
  char *s = NULL, *e = NULL, *n = NULL, c = '\0';
  
  s = start + 1;
  if (*s == '/') s++;
  while (isspace(*s)) s++;
  
  e = s;
  while (isalnum(*e) || *e == '_' || *e == ':' || *e == '-') e++;

  if (e == s)
    return INCONSISTENTDATA;
  c = *e;
  *e = '\0';
  n = strdup(s);
  *e = c;
  e = strchr(n, ':');
  if (e == NULL) {
    *name = NULL;
    dhufree(n);
  } else {
    *e = '\0';
    *name = n;
  }
  return E_OK;
}

/*******************************************************************************
 * getTagName
 *
 * This function extracts the name of the current tag.
 *
 * @param start -         A pointer to the start of the current tag.
 * @param end -   A pointer to the end of the current tag.
 * @param name - A pointer to fill with the name of the current tag.
 * @return E_OK for a win.
 ******************************************************************************/
int getTagName(char *start, char *end, char **name) {
  char *s = NULL, *e = NULL, *n = NULL, c = '\0';
  
  s = start + 1;
  if (*s == '/') s++;
  while (isspace(*s)) s++;
  
  e = s;
  while (isalnum(*e) || *e == '_' || *e == ':' || *e == '-') e++;

  if (e == s)
    return INCONSISTENTDATA;
  c = *e;
  *e = '\0';
  n = strdup(s);
  *e = c;

  e = strchr(n, ':');
  if (e != NULL) {
    e++;
    *name = strdup(e);
    dhufree(n);
  } else {
    *name = n;
  }
  return E_OK;
}

int findXMLDecl(char *src, char **start, char **end) {
	char *current = NULL;

	current = src;

	while (isspace(*current) && (*current != '\0')) current++;

	if (*current != '<')
		return INCONSISTENTDATA;
	*start = current;
	current++;

	if (*current != '?')
		return INCONSISTENTDATA;
	current++;
	while (isspace(*current) && (*current != '\0')) current++;

	current = strchr(current, '?');
	if (current == NULL)
		return INCONSISTENTDATA;
	current++;
	if (*current != '>')
		return INCONSISTENTDATA;
	*end = current + 1;
	return E_OK;
}

int findParentNameSpace(XMLParser *xml, char **names) {
	XMLNode *current = NULL;
	const char *ns = NULL;

	if (xml == NULL)
		return RESOURCEERROR;
	current = xml->current;

	while (current != NULL) {
		if (current->nameSpace != NULL) {
			getCurrentTagNameSpaceValue(xml, current->nameSpace, &ns);
			*names = strdup(ns);
			return E_OK;
		}
		current = current->parent;
	}
	return NODATAFOUND;
}

/*******************************************************************************
 * importXML
 *
 * This function initalise a xml parser to parse a xml char buffer.
 *
 * @param src -         A pointer to the xml char buffer
 * @param XmlParser -   A pointer to the xml parser structure
 * @return E_OK for a win.
 ******************************************************************************/
int importXML(char *src, XMLParser **xmlp) {
  char *startTag = NULL, 
       *endTag = NULL,
       *current = NULL,
       *name = NULL,
       *names = NULL,
       *value = NULL;
  int err = 0;
  TagType type = STARTTAG;
  XMLParser *parser = NULL;
  
  // safety
  if (src == NULL)
    return RESOURCEERROR;
  
  if ((err = initXMLParser(&parser)) != E_OK) {
    return err;
  }
  
  // Check for opening xml declaration
  if (findXMLDecl(src, &startTag, &endTag) != E_OK) {
    freeXMLParser(&parser);
    return INCONSISTENTDATA;
  }
  current = endTag;
  
  // Get next tag
  while (miniRegex(current, "<.+>", &startTag, &endTag) == E_OK) {
    type = getTagType(startTag, endTag);
    
    switch (type) {
      case STARTTAG:
        // get the name and value
	if ((err = getTagNameSpace(startTag, endTag, &names)) != E_OK) {
          freeXMLParser(&parser);
          return err;
	}
	if (names == NULL) {
		findParentNameSpace(parser, &names);
	}
        if ((err = getTagName(startTag, endTag, &name)) != E_OK) {
          freeXMLParser(&parser);
          return err;
        }
        if ((err = getTagValue(startTag, endTag, &value)) != E_OK) {
          freeXMLParser(&parser);
          return err;
        }
        // add the node
        if ((err = addChildNode(parser, name, value, names)) != E_OK) {
          freeXMLParser(&parser);
          return err;
        }
        // move to the node - this will fail for the root node
        moveToFirstChild(parser);
        
        // move as far right as we can go
        while (moveToNextSibling(parser) == E_OK); // Note the ;
          
        // add the attributes
        if ((err = addTagAttributes(parser, startTag, endTag)) != E_OK) {
          freeXMLParser(&parser);
          return err;
        }
        break;
      case ENDTAG:
        moveToParent(parser);
        break;
      case FULLTAG:
        // get the name
	if ((err = getTagNameSpace(startTag, endTag, &names)) != E_OK) {
          freeXMLParser(&parser);
          return err;
	}
	if (names == NULL) {
		findParentNameSpace(parser, &names);
	}
        if ((err = getTagName(startTag, endTag, &name)) != E_OK) {
	  dhufree(names);
          freeXMLParser(&parser);
          return err;
        }
        value = strdup("");
        // add the node
        if ((err = addChildNode(parser, name, value, names)) != E_OK) {
          freeXMLParser(&parser);
          return err;
        }
        
        // move to the node - this will fail for the root node
        moveToFirstChild(parser);
        
        // move as far right as we can go
        while (moveToNextSibling(parser) == E_OK); // Note the ;
        
        // add the node
        // stay where we are
        // add the attributes
        if ((err = addTagAttributes(parser, startTag, endTag)) != E_OK) {
          freeXMLParser(&parser);
          return err;
        }
        moveToParent(parser);
        
        // move as far right as we can go
        while (moveToNextSibling(parser) == E_OK); // Note the ;
        
        break;
    }
    current = endTag;
  }
  *xmlp = parser;
  return E_OK;
}

/*******************************************************************************
 * exportXMLNode
 *
 * This function constructs a char buffer of xml data from the current xml node.
 *
 * @param XmlNode -   A pointer to the xml node
 * @param output -      A pointer to the xml char buffer to be filled
 * @return E_OK for a win.
 ******************************************************************************/
int exportXMLNode(XMLNode *node, char **result, int indent) {
  char *output = *result;
  XMLNode *children = NULL;
  XMLAttribute *attr = NULL;
  XMLNameSpace *names = NULL;
  int i = 0;
  
  if (node == NULL) {
    return RESOURCEERROR;
  }
  
  // open tag
  for (i = 0; i < indent; i++) {
    vstrdupcat(&output, "  ", NULL);
  }
  vstrdupcat(&output, "<", NULL);
  if (node->nameSpace != NULL) {
    vstrdupcat(&output, node->nameSpace, ":", NULL);
  }
  vstrdupcat(&output, node->name, NULL);
  
  attr = node->attributes;
  
  while (attr) {
    if (attr->nameSpace != NULL) {
      vstrdupcat(&output, " ", attr->nameSpace, ":", attr->name, "=\"", attr->value, "\"", NULL);
    } else {
      vstrdupcat(&output, " ", attr->name, "=\"", attr->value, "\"", NULL);
    }
    attr = attr->next;
  }

  names = node->nameSpaceDecl;
  while (names) {
    vstrdupcat(&output, " xmlns:", names->alias, "=\"", names->href, "\"", NULL);
    names = names->next;
  }
  
  if (node->firstChild == NULL && strlen(node->value) == 0) {
    vstrdupcat(&output, "/>\r\n", NULL);
  } else {
    vstrdupcat(&output, ">", NULL);
    if (strlen(node->value) == 0) {
      vstrdupcat(&output, "\r\n", NULL);
    }
    if (node->firstChild) {
      children = node->firstChild;
      do {
        exportXMLNode(children, &output, indent + 1);
      } while ((children = children->rightSibling) != NULL);
    }
    if (strlen(node->value) > 0) {
      vstrdupcat(&output, node->value, NULL);
    }
    if (strlen(node->value) == 0) {
      for (i = 0; i < indent; i++) {
        vstrdupcat(&output, "  ", NULL);
      }
    }
  
    if (node->nameSpace != NULL) {
      vstrdupcat(&output, "</", node->nameSpace, ":", node->name, ">\r\n", NULL);
    } else {
      vstrdupcat(&output, "</", node->name, ">\r\n", NULL);
    }
  }
  
  *result = output;
  return E_OK;
}


/*******************************************************************************
 * printXMLNode
 *
 * This function constructs a char buffer of xml data from the current xml node.
 *
 * @param XmlNode -   A pointer to the xml node
 * @param output -      A pointer to the xml char buffer to be filled
 * @return E_OK for a win.
 ******************************************************************************/
int printXMLNode(XMLNode *node, int indent) {
  XMLNode *children = NULL;
  XMLAttribute *attr = NULL;
  XMLNameSpace *names = NULL;
  int i = 0;
  
  if (node == NULL) {
    return RESOURCEERROR;
  }
  
  // open tag
  for (i = 0; i < indent; i++) {
    fprintf(stderr, "  ");
  }
  fprintf(stderr, "<%s%s%s", (node->nameSpace != NULL?node->nameSpace:""), (node->nameSpace!=NULL?":":""), node->name);
  
  attr = node->attributes;
  
  while (attr) {
    if (attr->nameSpace != NULL) {
      fprintf(stderr, " %s:%s=\"%s\"", attr->nameSpace, attr->name, attr->value);
    } else {
      fprintf(stderr, " %s=\"%s\"", attr->name, attr->value);
    }
    attr = attr->next;
  }
  
  names = node->nameSpaceDecl;
  
  while (names) {
    fprintf(stderr, " xmlns:%s=\"%s\"", names->alias, names->href);
    names = names->next;
  }
  
  if (node->firstChild == NULL && strlen(node->value) == 0) {
    fprintf(stderr, "/>\r\n");
  } else {
    fprintf(stderr, ">");
    if (strlen(node->value) == 0) {
      fprintf(stderr, "\r\n");
    }
    if (node->firstChild) {
      children = node->firstChild;
      do {
        printXMLNode(children, indent + 1);
      } while ((children = children->rightSibling) != NULL);
    }
    if (strlen(node->value) > 0) {
      fprintf(stderr, "%s", node->value);
    }
    if (strlen(node->value) == 0) {
      for (i = 0; i < indent; i++) {
        fprintf(stderr, "  ");
      }
    }
  
    if (node->nameSpace != NULL) {
      fprintf(stderr, "</%s:%s>\r\n", node->nameSpace, node->name);
    } else {
      fprintf(stderr, "</%s>\r\n", node->name);
    }
  }
  
  return E_OK;
}


/*******************************************************************************
 * printXML
 *
 * This function prints the contents of the xml parser to stdout
 *
 * @param XmlParser -   A pointer to the xml parser structure
 * @return E_OK for a win.
 ******************************************************************************/
int printXML(XMLParser *xmlp) {
  XMLNode *current = NULL;
  int err = E_OK;
  
  if (xmlp == NULL || xmlp->root == NULL)
    return RESOURCEERROR;
  
  // XML Header
  fprintf(stderr, "%s", "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n\r\n");
  
  // Root Element
  current = xmlp->root;

  if ((err = printXMLNode(current, 0)) != E_OK) {
    return err;
  }
  return E_OK;
}

/*******************************************************************************
 * exportXML
 *
 * This function constructs a char buffer of xml data from the current parse tree.
 *
 * @param XmlParser -   A pointer to the xml parser structure
 * @param output -      A pointer to the xml char buffer to be filled
 * @return E_OK for a win.
 ******************************************************************************/
int exportXML(XMLParser *xmlp, char **result) {
  char *output = NULL, *end = NULL;
  XMLNode *current = NULL;
  int err = E_OK;
  
  if (xmlp == NULL || xmlp->root == NULL)
    return RESOURCEERROR;
  
  // XML Header
  vstrdupcat(&output, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n\r\n", NULL);
  
  // Root Element
  current = xmlp->root;

  if ((err = exportXMLNode(current, &output, 0)) != E_OK) {
    return err;
  }

  end = output + (strlen(output) - 1);

  while (isspace(*end) && end > output) {
	  *end = '\0';
	  end--;
  }
  *result = output;

  return E_OK;
}

/*******************************************************************************
 * initXMLParser
 *
 * This function initalise an empty xml parser.
 *
 * @param XmlParser -   A pointer to the xml parser structure
 * @return E_OK for a win.
 ******************************************************************************/
int initXMLParser(XMLParser **xmlp) {
  XMLParser *p = NULL;
  
  p = (XMLParser *) dhumalloc(sizeof(XMLParser));
  if (p == NULL)
    return RESOURCEERROR;
  
  p->root = NULL;
  p->current = NULL;
  *xmlp = p;
  return E_OK;
}


/*******************************************************************************
 * initXMLAttribute
 *
 * This function initalise an empty xml attribute.
 *
 * @param XmlAttribute -   A pointer to the xml node structure
 * @return E_OK for a win.
 ******************************************************************************/
int initXMLAttribute(XMLAttribute **xmlp, char *name, char *value, char *nameSpace) {
  XMLAttribute *a = NULL;
  
  a = (XMLAttribute *) dhumalloc(sizeof(XMLAttribute));
  if (a == NULL)
    return RESOURCEERROR;
  
  a->next = NULL;
  a->name = name;
  a->value = value;
  a->nameSpace = nameSpace;
  *xmlp = a;
  return E_OK;
}

/*******************************************************************************
 * initXMLNameSpace
 *
 * This function initalise an empty xml nameSpace.
 *
 * @param XmlNameSpace -   A pointer to the xml nameSpace structure
 * @return E_OK for a win.
 ******************************************************************************/
int initXMLNameSpace(XMLNameSpace **xmlp, char *alias, char *href) {
  XMLNameSpace *n = NULL;
  
  n = (XMLNameSpace *) dhumalloc(sizeof(XMLNameSpace));
  if (n == NULL)
    return RESOURCEERROR;
  
  n->alias = alias;
  n->href = href;
  n->next = NULL;
  
  *xmlp = n;
  return E_OK;
}

/*******************************************************************************
 * initXMLNode
 *
 * This function initalise an empty xml node.
 *
 * @param XmlNode -   A pointer to the xml node structure
 * @return E_OK for a win.
 ******************************************************************************/
int initXMLNode(XMLNode **xmlp, char *name, char *value, char *nameSpace) {
  XMLNode *n = NULL;
  
  n = (XMLNode *) dhumalloc(sizeof(XMLNode));
  if (n == NULL)
    return RESOURCEERROR;
  
  n->firstChild = NULL;
  n->rightSibling = NULL;
  n->leftSibling = NULL;
  n->parent = NULL;
  
  n->name = name;
  n->value = value;
  n->nameSpace = nameSpace;
  n->nameSpaceDecl = NULL;
  n->attributes = NULL;
  *xmlp = n;
  return E_OK;
}

/*******************************************************************************
 * freeXMLNameSpace
 *
 * This function frees the memory allocated by a XML NameSpace list.
 *
 * @param attp -        A pointer the list of attributes.
 ******************************************************************************/
void freeXMLNameSpace(XMLNameSpace **attrp) {
  XMLNameSpace *attr = *attrp, 
               *next = NULL;
  
  if (attr == NULL)
    return;
  
  next = attr->next;
  
  dhufree(attr->alias);
  dhufree(attr->href);
  dhufree(attr);
  if (next)
    freeXMLNameSpace(&next);
  *attrp = attr;
}

/*******************************************************************************
 * freeXMLAttributes
 *
 * This function frees the memory allocated by a XML Attributes list.
 *
 * @param attp -        A pointer the list of attributes.
 ******************************************************************************/
void freeXMLAttribute(XMLAttribute **attrp) {
  XMLAttribute *attr = *attrp, 
               *next = NULL;
  
  if (attr == NULL)
    return;
  
  next = attr->next;
  
  dhufree(attr->name);
  dhufree(attr->value);
  dhufree(attr->nameSpace);
  dhufree(attr);
  if (next)
    freeXMLAttribute(&next);
  *attrp = attr;
}

/*******************************************************************************
 * freeXMLNode
 *
 * This function free the memory allocated by a XML Node.
 *
 * @param nodep -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
void freeXMLNode(XMLNode **nodep) {
  XMLNode *node = *nodep,
          *firstChild = NULL,
          *rightSibling = NULL;
  
  if (node == NULL)
    return;
  firstChild = node->firstChild;
  rightSibling = node->rightSibling;
  
  freeXMLAttribute(&node->attributes);
  freeXMLNameSpace(&node->nameSpaceDecl);
  dhufree(node->name);
  dhufree(node->value);
  dhufree(node->nameSpace);
  dhufree(node);
  if (firstChild)
    freeXMLNode(&firstChild);
  if (rightSibling)
    freeXMLNode(&rightSibling);
  *nodep = node;
}


/*******************************************************************************
 * freeXMLParser
 *
 * This function free the memory allocated by a XML Parser.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
void freeXMLParser(XMLParser **xmlp) {
  XMLParser *p = *xmlp;
  
  if (p == NULL)
    return;
  p->current = NULL;
  freeXMLNode(&p->root);
  dhufree(p);
  *xmlp = p;
}

/*******************************************************************************
 * getCurrentTagName
 *
 * This function will return the name of the tag that the cursor is pointing at.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int getCurrentTagName(XMLParser *xmlp, const char **name) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  
  *name = xmlp->current->name;
  return E_OK;
}

/*******************************************************************************
 * getCurrentTagNameSpace
 *
 * This function will return the nameSpace of the tag that the cursor is pointing at.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int getCurrentTagNameSpace(XMLParser *xmlp, const char **name) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  
  *name = xmlp->current->nameSpace;
  return E_OK;
}

/*******************************************************************************
 * getCurrentTagValue
 *
 * This function will return the text between the start and end of the current tag.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int getCurrentTagValue(XMLParser *xmlp, const char **value) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  
  *value = xmlp->current->value;
  return E_OK;
}

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
int getCurrentTagAttributes(XMLParser *xmlp, XMLAttribute **attr) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  
  *attr = xmlp->current->attributes;
  return E_OK;
}

/*******************************************************************************
 * getCurrentTagNameSpaces
 *
 * This function will return list of attributes for the current tag.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param attr -        A pointer to an XMLAtrribute Structure that 
 *                      will be populated with the list of attributes for the
 *                      current xml node.
 ******************************************************************************/
int getCurrentTagNameSpaces(XMLParser *xmlp, XMLNameSpace **names) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  
  *names = xmlp->current->nameSpaceDecl;
  return E_OK;
}

/*******************************************************************************
 * getUniqueNameSpaceAlias
 *
 * This function will return a unique alias for a new nameSpace in the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param href -        The href of the nameSpace to query.
 * @param alias -       A char poiner that will be populated with the value
 ******************************************************************************/
int getUniqueNameSpaceAlias(XMLParser *xmlp, char **alias) {
  XMLNameSpace *names = NULL;
  XMLNode *current = NULL;
  int count = 0;
  char aliasbuf[256];

  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  
  current = xmlp->current;
  while (current != NULL) {
    names = current->nameSpaceDecl;
  
    while (names != NULL) {
      count++;
      names = names->next;
    }
    current = current->parent;
  }

  sprintf(aliasbuf, "ns%d", count);
  *alias = dhustrdup(aliasbuf);
  return E_OK;
}

/*******************************************************************************
 * getCurrentTagNameSpaceAlias
 *
 * This function will return the alias of the nameSpace in the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param href -        The href of the nameSpace to query.
 * @param alias -       A char poiner that will be populated with the value
 ******************************************************************************/
int getCurrentTagNameSpaceAlias(XMLParser *xmlp, char *href, const char **alias) {
  XMLNameSpace *names = NULL;
  XMLNode *current = NULL;

  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  
  current = xmlp->current;
  while (current != NULL) {
    names = current->nameSpaceDecl;
  
    while (names != NULL) {
      if (strcasecmp(names->href, href) == 0) {
        *alias = names->alias;
        return E_OK;
      }
      names = names->next;
    }
    current = current->parent;
  }
  return NODATAFOUND;
}

/*******************************************************************************
 * getCurrentTagNameSpaceValue
 *
 * This function will return the href of the named nameSpace in the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param alias -        The alias of the nameSpace to query.
 * @param href -       A char poiner that will be populated with the value
 ******************************************************************************/
int getCurrentTagNameSpaceValue(XMLParser *xmlp, char *alias, const char **href) {
  XMLNameSpace *names = NULL;
  XMLNode *current = NULL;

  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL || alias == NULL)
    return RESOURCEERROR;
  
  current = xmlp->current;
  while (current != NULL) {
    names = current->nameSpaceDecl;
  
    while (names != NULL) {
      if ((names->alias != NULL) && (strcasecmp(names->alias, alias) == 0)) {
        *href = names->href;
        return E_OK;
      }
      names = names->next;
    }
    current = current->parent;
  }
  return NODATAFOUND;
}

/*******************************************************************************
 * getCurrentTagAttributeValue
 *
 * This function will return the value of the named attribute in the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the attribute to query.
 * @param value -       A char poiner that will be populated with the value
 ******************************************************************************/
int getCurrentTagAttributeValue(XMLParser *xmlp, char *name, char *namespace, const char **value) {
  XMLAttribute *attr = NULL;
  const char *alias = NULL;

  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  
  attr = xmlp->current->attributes;

  if (namespace != NULL) {
    getCurrentTagNameSpaceAlias(xmlp, namespace, &alias);
  }
  
  while (attr != NULL) {
    if (namespace == NULL) {
      if ((strcasecmp(attr->name, name) == 0)) {
        *value = attr->value;
        return E_OK;
      }
    } else {
      if ((strcasecmp(attr->name, name) == 0)) {
        if (attr->nameSpace != NULL && alias != NULL && (strcmp(attr->nameSpace, alias) == 0)) {
          *value = attr->value;
          return E_OK;
        }
      }
    }
    attr = attr->next;
  }
  return NODATAFOUND;
}

/*******************************************************************************
 * moveToNextSibling
 *
 * This function will move the cursor to the next sibling of the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int moveToNextSibling(XMLParser *xmlp) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  if (xmlp->current->rightSibling == NULL)
    return NODATAFOUND;
  xmlp->current = xmlp->current->rightSibling;
  return E_OK;
}

/*******************************************************************************
 * moveToFirstChild
 *
 * This function will move the cursor to the first child of the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int moveToFirstChild(XMLParser *xmlp) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  if (xmlp->current->firstChild == NULL)
    return NODATAFOUND;
  xmlp->current = xmlp->current->firstChild;
  return E_OK;
}

/*******************************************************************************
 * moveToParent
 *
 * This function will move the cursor to the parent of the current tag
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int moveToParent(XMLParser *xmlp) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  if (xmlp->current == NULL)
    return RESOURCEERROR;
  if (xmlp->current->parent == NULL)
    return NODATAFOUND;
  xmlp->current = xmlp->current->parent;
  return E_OK;
}

/*******************************************************************************
 * addChildNode
 *
 * This function will insert a new xml node as a child of the current node
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the new node to insert
 * @param value -       The value of the new node to insert
 ******************************************************************************/
int addChildNode(XMLParser *xmlp, char *name, char *value, char *nameSpace) {
  XMLNode *child = NULL, *insert = NULL;
  int err = E_OK;
  char *nsalias = NULL;
  
  // safety
  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (name == NULL || value == NULL)
    return RESOURCEERROR;
  
  // allocate the structure
  if ((err = initXMLNode(&child, name, value, NULL)) != E_OK)
    return err;

  if (xmlp->current) {
    // insert this node as a child of the current node.
    insert = xmlp->current;
    if (insert->firstChild == NULL) {
      insert->firstChild = child;
      child->parent = insert;
    } else {
      insert = insert->firstChild;
      while (insert->rightSibling != NULL) {
        insert = insert->rightSibling;
      }
      insert->rightSibling = child;
      child->leftSibling = insert;
      child->parent = insert->parent;
    }
  } else {
    // insert this node as the root
    xmlp->root = child;
    xmlp->current = child;
  }

  insert = xmlp->current;
  xmlp->current = child;

  if (nameSpace != NULL) {
    if (getCurrentTagNameSpaceAlias(xmlp, nameSpace, (const char **) & nsalias) == E_OK) {
      child->nameSpace = dhustrdup(nsalias);
      dhufree(nameSpace);
    } else {
      getUniqueNameSpaceAlias(xmlp, &nsalias);
      addNameSpaceDecl(xmlp, nsalias, nameSpace);
      child->nameSpace = dhustrdup(nsalias);
    }
  }

  xmlp->current = insert;
  
  return E_OK;
}

/*******************************************************************************
 * addSiblingNode
 *
 * This function will insert a new xml node as a sibling of the current node
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the new node to insert
 * @param value -       The value of the new node to insert
 ******************************************************************************/
int addSiblingNode(XMLParser *xmlp, char *name, char *value, char *nameSpace) {
  XMLNode *sibling = NULL, *insert = NULL;
  int err = E_OK;
  char *nsalias = NULL;
  
  // safety
  if (xmlp == NULL)
    return RESOURCEERROR;
  
  if (name == NULL || value == NULL)
    return RESOURCEERROR;
  
  // allocate the structure
  if ((err = initXMLNode(&sibling, name, value, nameSpace)) != E_OK)
    return err;
  
  if (xmlp->current) {
    // insert this node as a sibling of the current node.
    insert = xmlp->current;
    while (insert->rightSibling != NULL) {
      insert = insert->rightSibling;
    }
    insert->rightSibling = sibling;
    sibling->leftSibling = insert;
    sibling->parent = insert->parent;
  } else {
    // insert this node as the root
    xmlp->root = sibling;
    xmlp->current = sibling;
  }
  
  insert = xmlp->current;
  xmlp->current = sibling;

  if (nameSpace != NULL) {
    if (getCurrentTagNameSpaceAlias(xmlp, nameSpace, (const char **) & nsalias) == E_OK) {
      sibling->nameSpace = dhustrdup(nsalias);
      dhufree(nameSpace);
    } else {
      getUniqueNameSpaceAlias(xmlp, &nsalias);
      addNameSpaceDecl(xmlp, nsalias, nameSpace);
      sibling->nameSpace = dhustrdup(nsalias);
    }
  }

  xmlp->current = insert;
  
  return E_OK;
}

/*******************************************************************************
 * addNameSpaceDecl
 *
 * This function will insert a new NameSpace for the current node.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the new NameSpace to insert
 * @param value -       The value of the new NameSpace to insert
 ******************************************************************************/
int addNameSpaceDecl(XMLParser *xmlp, char *alias, char *href) {
  XMLNameSpace *names = NULL, *last = NULL;
  int err = E_OK;
  
  // safety
  if (xmlp == NULL || xmlp->current == NULL)
    return RESOURCEERROR;
  
  if (alias == NULL || href == NULL)
    return RESOURCEERROR;
  
  // allocate the struct
  if ((err = initXMLNameSpace(&last, alias, href) != E_OK)) {
    return err;
  }
  
  // insert it
  names = xmlp->current->nameSpaceDecl;
  if (!names) {
    xmlp->current->nameSpaceDecl = last;
  } else {
    while (names->next != NULL) {
      names = names->next;
    }
    names->next = last;
  }
  
  return E_OK;
}

/*******************************************************************************
 * addAttribute
 *
 * This function will insert a new attribute for the current node.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 * @param name -        The name of the new node to insert
 * @param value -       The value of the new node to insert
 ******************************************************************************/
int addAttribute(XMLParser *xmlp, char *name, char *value, char *nameSpace) {
  XMLAttribute *attr = NULL, *last = NULL;
  int err = E_OK;
  char *nsalias = NULL;
  
  // safety
  if (xmlp == NULL || xmlp->current == NULL)
    return RESOURCEERROR;
  
  if (name == NULL || value == NULL)
    return RESOURCEERROR;
  
  // allocate the struct
  if ((err = initXMLAttribute(&last, name, value, NULL) != E_OK)) {
    return err;
  }
  
  // insert it
  attr = xmlp->current->attributes;
  if (!attr) {
    xmlp->current->attributes = last;
  } else {
    while (attr->next != NULL) {
      attr = attr->next;
    }
    attr->next = last;
  }
  
  if (nameSpace != NULL) {
    if (getCurrentTagNameSpaceAlias(xmlp, nameSpace, (const char **) & nsalias) == E_OK) {
      last->nameSpace = dhustrdup(nsalias);
      dhufree(nameSpace);
    } else {
      getUniqueNameSpaceAlias(xmlp, &nsalias);
      addNameSpaceDecl(xmlp, nsalias, nameSpace);
      last->nameSpace = dhustrdup(nsalias);
    }
  }
  
  return E_OK;
}

/*******************************************************************************
 * moveToRoot
 *
 * This function will move the cursor to the root of the xml tree.
 *
 * @param xmlp -        A pointer to a currently allocated xml parser.
 ******************************************************************************/
int moveToRoot(XMLParser *xmlp) {
  if (xmlp == NULL)
    return RESOURCEERROR;
  if (xmlp->root == NULL)
    return RESOURCEERROR;
  xmlp->current = xmlp->root;
  return E_OK;
}
