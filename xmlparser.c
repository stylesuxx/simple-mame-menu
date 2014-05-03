#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "xmlparser.h"

xmlDocPtr xml;

int xml_open(const char *path)
{
    xml = xmlParseFile(path);
    if(xml == NULL) {
        fprintf(stderr, "%s could not be parsed.\n", path);
        return -1;
    }

    return 0;
}

void xml_close()
{
    xmlFreeDoc(xml);
}

char* xml_get_value(char *xpath_expression)
{
    xmlXPathContextPtr xpath_context;
    xmlXPathObjectPtr xpath_object;
    xmlNodePtr current_node;

    xmlChar *xml_value = NULL;
    char *value = NULL;
    int size;

    xpath_context = xmlXPathNewContext(xml);
    if(xpath_context == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context.\n");
        xmlFreeDoc(xml);
        return NULL;
    }

    xpath_object = xmlXPathEvalExpression((xmlChar *) xpath_expression, xpath_context);
    if(xpath_object == NULL) {
        fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpath_expression);
        xmlXPathFreeContext(xpath_context);
        xmlFreeDoc(xml);
        return NULL;
    }

    size = (xpath_object->nodesetval) ? xpath_object->nodesetval->nodeNr : 0;
    if(size > 0) {
        if(xpath_object->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE) {
            current_node = xpath_object->nodesetval->nodeTab[0];
            xml_value = xmlNodeListGetString(xml, current_node->xmlChildrenNode, 1);

            value = malloc(strlen((const char*) xml_value) + 1);
            strcpy(value, (const char*) xml_value);
            xmlFree(xml_value);
        }
        else {
            value = "yes";
        }
    }

    xmlXPathFreeObject(xpath_object);
    xmlXPathFreeContext(xpath_context);

    if(value == NULL)
        return "---";

    return value;

}
