#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <dirent.h>

#include "roms.h"
#include "xmlparser.h"

rom_data *roms = NULL;
rom_data **games_ptr = NULL;

const char *game_xml = NULL;

int num_games = 0;
int num_roms = 0;
int num_allocated = 0;

int build_rom_list(const char *mame_ini_path, const char *game_xml_path)
{
    game_xml = game_xml_path;

    int length = strlen(mame_ini_path) + (strlen(TEMPLATE_VERIFY) - 2) + 1;
    char verify_command[length];
    sprintf(verify_command, TEMPLATE_VERIFY, mame_ini_path);
    verify_command[length - 1] = '\0';

    /* Try to load from custom list.
     *
     * If the custom list is not present, a new mame.xml is generated.
     * The system then builds its custom xml file with only the needed info and available romsa.
     */
    if(load_rom_list(game_xml_path) != 0) {
        get_available_roms(verify_command);    
        char *mame_xml_path = "/tmp/mame.xml";
        system(TEMPLATE_BUILD_XML);

        if(get_rom_count() > 0) {
            if(set_rom_info(mame_xml_path) != 0)
                return -1;

            save_rom_list(game_xml_path);
        }
    }

    /* Build an array of available game roms. */
    if(get_rom_count() > 0) {
        games_ptr = malloc(num_games * sizeof(rom_data *));

        int i, counter = 0;
        for(i = 0; i < num_roms; i++)
            if(roms[i].bios != 0 && roms[i].device != 0)
                games_ptr[counter++] = &roms[i];

        return 0;
    }

    return -1;
}

rom_data* get_roms() { return roms; }
rom_data** get_all_games() { return games_ptr; }

int get_game_count() { return num_games; }
int get_rom_count() { return num_roms; }

void clear_roms()
{
    int i;

    if(games_ptr != NULL) {
        free(games_ptr);
    }

    if(roms != NULL) {
        for(i = 0; i < get_rom_count(); i++) {
            free(roms[i].slug);
            free(roms[i].description);
            free(roms[i].manufacturer);
            free(roms[i].year);
            free(roms[i].state);
        }

        free(roms);
    }
}

void increase_times_played(const char *slug) {
    int i;
    for(i = 0; i < get_game_count(); i++)
        if(strcmp(games_ptr[i]->slug, slug) == 0)
            games_ptr[i]->times_played++;

    save_rom_list(game_xml);
}

int add_rom(rom_data item)
{
    if(num_roms == num_allocated) {
        if (num_allocated == 0)
            num_allocated = 50;
        else
            num_allocated += 5;

        void *_tmp = realloc(roms, (num_allocated * sizeof(rom_data)));

        if(!_tmp) {
            fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
            return(-1);
        }

        roms = (rom_data *) _tmp;
    }

    roms[num_roms] = item;
    num_roms++;

    return num_roms;
}

/* Build the array of available games.
 *
 * Let mame verify the state of the games.
 * Set slug and state of the game.
 *
 * Possible states are: good, bad and best available.
 */
int get_available_roms(const char *command)
{
    FILE *mame_output;
    mame_output = popen(command, "r");

    if(mame_output == NULL)
        return -1;

    char line[255];
    while (fgets(line, 255, mame_output) != NULL) {
        regex_t *regex = malloc(sizeof(regex_t));
        regmatch_t pmatch[10];
        int reti, len;

        reti = regcomp(regex, "^romset ([a-z0-9]*)( \\[[a-z0-9]*\\] is| is) (good|bad|best available)", REG_EXTENDED);
        if(reti) {
            fprintf(stderr, "Could not compile regex\n");
            return 1;
        }

        reti = regexec(regex, line, 10, pmatch, 0);
        if(!reti)  {
            rom_data game;

            len = pmatch[1].rm_eo - pmatch[1].rm_so;
            game.slug = malloc(len + 1);
            memcpy(game.slug, line + pmatch[1].rm_so, len);
            game.slug[len] = '\0';

            len = pmatch[3].rm_eo - pmatch[3].rm_so;
            game.state = malloc(len + 1);
            memcpy(game.state, line + pmatch[3].rm_so, len);
            game.state[len] = '\0';

            int elems;
            elems = add_rom(game);
            if(elems < 0) {
                pclose(mame_output);
                regfree(regex);
                free(regex);
                return -1;
            }
        }

        regfree(regex);
        free(regex);
    }

    pclose(mame_output);
    return 0;
}

int set_rom_info(const char *mame_xml_path)
{
    if(xml_open(mame_xml_path) != 0)
        return 1;

    int i, length;
    num_games = 0;
    for(i = 0; i < get_rom_count(); i++) {
        char *year, *manufacturer, *description, *bios, *device;

        length = strlen(roms[i].slug) + (strlen(XPATH_YEAR) - 2) + 1;
        year = malloc(length);
        sprintf(year, XPATH_YEAR, roms[i].slug);
        year[length - 1] = '\0';

        length = strlen(roms[i].slug) + (strlen(XPATH_MANUFACTURER) - 2) + 1;
        manufacturer = malloc(length);
        sprintf(manufacturer, XPATH_MANUFACTURER, roms[i].slug);
        manufacturer[length - 1] = '\0';

        length = strlen(roms[i].slug) + (strlen(XPATH_DESCRIPTION) - 2) + 1;
        description = malloc(length);
        sprintf(description, XPATH_DESCRIPTION, roms[i].slug);
        description[length - 1] = '\0';

        length = strlen(roms[i].slug) + (strlen(XPATH_BIOS) - 2) + 1;
        bios = malloc(length);
        sprintf(bios, XPATH_BIOS, roms[i].slug);
        bios[length - 1] = '\0';

        length = strlen(roms[i].slug) + (strlen(XPATH_DEVICE) - 2) + 1;
        device = malloc(length);
        sprintf(device, XPATH_DEVICE, roms[i].slug);
        device[length - 1] = '\0';

        roms[i].description = xml_get_value(description);
        roms[i].year = xml_get_value(year);
        roms[i].manufacturer = xml_get_value(manufacturer);
        roms[i].bios = strcmp("yes", xml_get_value(bios));
        roms[i].device = strcmp("yes", xml_get_value(device));
        roms[i].times_played = 0;

        if(roms[i].bios != 0 && roms[i].device != 0)
            num_games++;

        free(year);
        free(manufacturer);
        free(description);
        free(bios);
        free(device);
    }

    xml_close();

    return 0;
}

int save_rom_list(const char *path)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL, node = NULL;
    int i;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "mame");
    xmlDocSetRootElement(doc, root_node);

    for(i = 0; i < get_rom_count(); i++) {
        node = xmlNewChild(root_node, NULL, (xmlChar *) "game", NULL);
        xmlNewProp(node, (xmlChar *) "name", (xmlChar *) roms[i].slug);

        if(roms[i].bios == 0)
            xmlNewProp(node, (xmlChar *) "isbios", (xmlChar *) "yes");

        if(roms[i].device == 0)
            xmlNewProp(node, (xmlChar *) "isdevice", (xmlChar *) "yes");

        xmlNewChild(node, NULL, (xmlChar *) "description", (xmlChar *) roms[i].description);
        xmlNewChild(node, NULL, (xmlChar *) "manufacturer", (xmlChar *) roms[i].manufacturer);
        xmlNewChild(node, NULL, (xmlChar *) "year", (xmlChar *) roms[i].year);
        xmlNewChild(node, NULL, (xmlChar *) "state", (xmlChar *) roms[i].state);

        char buffer[10];
        snprintf(buffer, 10, "%d", roms[i].times_played);

        xmlNewChild(node, NULL, (xmlChar *) "times_played", (xmlChar *) buffer);
    }

    if(xmlSaveFormatFileEnc(path, doc, "UTF-8", 1) == -1)
        fprintf(stderr, "Could not write game xml to %s\n", path);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}

int load_rom_list(const char *path)
{
    xmlDoc *doc = NULL;
    xmlNodePtr current, properties;
    int elems;

    doc = xmlReadFile(path, NULL, 0);

    if(doc == NULL) {
        fprintf(stderr, "Could not load game xml from %s\n", path);
        return -1;
    }

    current = xmlDocGetRootElement(doc);
    if(current == NULL) {
        fprintf(stderr, "Game xml is empty.\n");
        xmlFreeDoc(doc);
        return -1;
    }

    if(xmlStrcmp(current->name, (const xmlChar *) "mame")) {
        fprintf(stderr, "Game xml is malformatted, root node != mame\n");
        xmlFreeDoc(doc);
        return -1;
    }

    current = current->xmlChildrenNode;
    while(current != NULL) {
        if((!xmlStrcmp(current->name, (const xmlChar *) "game"))) {
            rom_data rom;
            xmlChar *bios, *device;

            rom.slug = (char *) xmlGetProp(current, (xmlChar *) "name");

            bios = xmlGetProp(current, (xmlChar *) "isbios");
            if(xmlStrcmp(bios, (xmlChar *) "yes") == 0)
                rom.bios =  0;
            else
                rom.bios = -1;

            device = xmlGetProp(current, (xmlChar *) "isdevice");
            if(xmlStrcmp(device, (xmlChar *) "yes") == 0)
                rom.device = 0;
            else
                rom.device = -1;

            properties = current->xmlChildrenNode;
            while(properties != NULL) {
                if((!xmlStrcmp(properties->name, (const xmlChar *) "year")))
                    rom.year = (char *) xmlNodeGetContent(properties);

                if((!xmlStrcmp(properties->name, (const xmlChar *) "state")))
                    rom.state = (char *) xmlNodeGetContent(properties);

                if((!xmlStrcmp(properties->name, (const xmlChar *) "manufacturer")))
                    rom.manufacturer = (char *) xmlNodeGetContent(properties);

                if((!xmlStrcmp(properties->name, (const xmlChar *) "description")))
                    rom.description = (char *) xmlNodeGetContent(properties);


                if((!xmlStrcmp(properties->name, (const xmlChar *) "times_played"))) {
                    char *played = (char *) xmlNodeGetContent(properties);
                    rom.times_played = strtol(played, NULL, 10);
                    free(played);
                }

                properties = properties->next;
            }

            free(bios);
            free(device);

            elems = add_rom(rom);
            if(elems < 0) {
                xmlFreeDoc(doc);
                return -1;
            }

            if(rom.bios != 0 && rom.device != 0)
                num_games++;
        }

        current = current->next;
    }

    xmlFreeDoc(doc);

    return 0;
}
