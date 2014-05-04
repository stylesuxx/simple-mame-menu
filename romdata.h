#ifndef ROMDATA_H
#define ROMDATA_H

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

#endif
