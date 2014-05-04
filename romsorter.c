#include <string.h>

#include "romsorter.h"
#include "roms.h"

int rom_slug_asc(const void *a, const void *b)
{
    const rom_data *g1 = *(rom_data **) a;
    const rom_data *g2 = *(rom_data **) b;

    return strcmp(g1->slug, g2->slug);
}


int rom_slug_desc(const void *a, const void *b)
{
    const rom_data *g1 = *(rom_data **) a;
    const rom_data *g2 = *(rom_data **) b;

    return strcmp(g2->slug, g1->slug);
}

int rom_times_played(const void *a, const void *b)
{
    const rom_data *g1 = *(rom_data **) a;
    const rom_data *g2 = *(rom_data **) b;

    return (g2->times_played - g1->times_played);
}
