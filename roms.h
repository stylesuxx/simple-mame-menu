#ifndef ROMS_H
#define ROMS_H

#include <libxml/tree.h>
#include <libxml/parser.h>

static const char XPATH_YEAR[] = "/mame/game[@name='%s']/year";
static const char XPATH_DESCRIPTION[] = "/mame/game[@name='%s']/description";
static const char XPATH_MANUFACTURER[] = "/mame/game[@name='%s']/manufacturer";
static const char XPATH_BIOS[] = "/mame/game[@name='%s']/@isbios";
static const char XPATH_DEVICE[] = "/mame/game[@name='%s']/@isdevice";

static const char TEMPLATE_VERIFY[] = "mame -inipath %s -verifyroms";
static const char TEMPLATE_BUILD_XML[] = "mame -listxml > /tmp/mame.xml";

typedef struct rom_data {
  char *slug;
  char *description;
  char *year;
  char *manufacturer;
  char *state;
  int times_played;
  int bios;
  int device;
} rom_data;

int build_rom_list(const char *mame_ini_path, const char *game_xml_path);
rom_data* get_roms();
rom_data** get_all_games();

int get_rom_count();
int get_game_count();

void increase_times_played(const char *slug);

int save_rom_list(const char *path);
int load_rom_list(const char *path);
int get_available_roms(const char *verify_command);
int set_rom_info(const char *xml_path);
int add_rom(rom_data item);
void clear_roms();

#endif
