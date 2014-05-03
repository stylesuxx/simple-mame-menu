#ifndef ROMS_H
#define ROMS_H

#include <libxml/tree.h>
#include <libxml/parser.h>

static const char XPATH_YEAR[] = "/mame/game[@name='%s']/year";
static const char XPATH_DESCRIPTION[] = "/mame/game[@name='%s']/description";
static const char XPATH_MANUFACTURER[] = "/mame/game[@name='%s']/manufacturer";
static const char XPATH_BIOS[] = "/mame/game[@name='%s']/@isbios";
static const char XPATH_DEVICE[] = "/mame/game[@name='%s']/@isdevice";

typedef struct rom_data {
  char *slug;
  char *description;
  char *year;
  char *manufacturer;
  char *state;
  int bios;
  int device;
} rom_data;

int build_rom_list(const char *verify_command, const char *mame_xml_path, const char *game_xml_path);
rom_data* get_roms();
rom_data* get_games();

int get_rom_count();
int get_game_count();

int save_rom_list(const char *path);
int load_rom_list(const char *path);
int get_available_roms(const char *verify_command);
int set_rom_info(const char *xml_path);
int add_rom(rom_data item);
void clear_roms();

/* Sorters */
int rom_slug_asc(const void *a, const void *b);
int rom_slug_desc(const void *a, const void *b);

#endif
