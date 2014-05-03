#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

int xml_open(const char *xml_path);
char* xml_get_value(char *xpath_expression);
void xml_close();

#endif
