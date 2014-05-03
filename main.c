#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>

#include "roms.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD   4

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
void usage();

int main(int argc, char *argv[])
{
    if(argc < 4) {
        usage();
        return 1;
    }

    char *rom_path = argv[1]; //"/home/babs/Roms/";
    char *mame_xml_path = argv[2]; //"testing/mame.xml";
    const char *game_xml_path = argv[3]; //"testing/custom_1.xml";

    ITEM **my_items;
    int c;
    WINDOW *main_window;
    MENU *game_menu;

    WINDOW *info_window;
    WINDOW *manufacturer_label;
    WINDOW *manufacturer_value;
    WINDOW *year_label;
    WINDOW *year_value;
    WINDOW *platform_label;
    WINDOW *platform_value;

    int i;
    int width = 0, height = 0;
    int main_window_width = 0, main_window_height = 0;
    int info_window_width = 0, info_window_height = 0;

    char *verify_command_template = "mame -rompath %s -verifyroms";
    int length = strlen(rom_path) + (strlen(verify_command_template) - 2) + 1;
    char verify_command[length];
    sprintf(verify_command, verify_command_template, rom_path);
    verify_command[length - 1] = '\0';

    /* Generate a list of available games and sort them ascending */
    rom_data *games;
    build_rom_list(verify_command, mame_xml_path, game_xml_path);
    games = get_games();

    if(get_game_count() < 1) {
        fprintf(stderr, "There are no games in your rom path...\n");
        return 1;
    }
    qsort(games, (size_t) get_game_count(), sizeof(rom_data), rom_slug_asc);
    //return -1;

    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    getmaxyx(stdscr, height, width);

    info_window_width = 30;
    info_window_height = height;
    main_window_width = width - info_window_width;
    main_window_height = info_window_height;

    /* Create items */
    my_items = malloc(get_game_count() * sizeof(ITEM *));
    for(i = 0; i < get_game_count(); i++) {
        my_items[i] = new_item(games[i].slug, games[i].description);
    }

    /* Crate the game menu */
    game_menu = new_menu(my_items);

    /* Create the window to be associated with the menu */
    main_window = newwin(main_window_height, main_window_width, 0, 0);
    keypad(main_window, TRUE);

    info_window = newwin(info_window_height, info_window_width, 0, main_window_width);
    box(info_window, 0, 0);
    print_in_middle(info_window, 1, 0, info_window_width, "Info", COLOR_PAIR(1));
    mvwaddch(info_window, 2, 0, ACS_LTEE);
    mvwhline(info_window, 2, 1, ACS_HLINE, info_window_width);
    mvwaddch(info_window, 2, info_window_width - 1, ACS_RTEE);

    manufacturer_label = newwin(1, info_window_width - 2, 3, main_window_width + 1);
    mvwprintw(manufacturer_label, 0, 0, "%s", "Manufacturer:");

    manufacturer_value = newwin(1, info_window_width - 2, 4, main_window_width + 1);

    year_label = newwin(1, info_window_width - 2, 6, main_window_width + 1);
    mvwprintw(year_label, 0, 0, "%s", "Year:");

    year_value = newwin(1, info_window_width - 2, 7, main_window_width + 1);

    platform_label = newwin(1, info_window_width - 2, 9, main_window_width + 1);
    mvwprintw(platform_label, 0, 0, "%s", "Platform:");

    platform_value = newwin(1, info_window_width - 2, 10, main_window_width + 1);

    mvwprintw(manufacturer_value, 0, 0, "%s", games[0].manufacturer);
    mvwprintw(year_value, 0, 0, "%s", games[0].year);
    //mvwprintw(platform_value, 0, 0, "%s", games[0].platform);

    /* Set main window and sub window */
    set_menu_win(game_menu, main_window);
    set_menu_sub(game_menu, derwin(main_window, main_window_height - 4, main_window_width - 2, 3, 1));
    set_menu_format(game_menu, main_window_height - 4, 1);

    /* Set menu mark to the string " > " */
    set_menu_mark(game_menu, " > ");

    /* Print a border around the main window and print a title */
    box(main_window, 0, 0);
    print_in_middle(main_window, 1, 0, main_window_width, "Game selection", COLOR_PAIR(1));
    mvwaddch(main_window, 2, 0, ACS_LTEE);
    mvwhline(main_window, 2, 1, ACS_HLINE, width);
    mvwaddch(main_window, 2, main_window_width - 1, ACS_RTEE);

    /* Post the menu */
    post_menu(game_menu);
    wrefresh(main_window);

    wrefresh(info_window);
    wrefresh(manufacturer_label);
    wrefresh(year_label);
    wrefresh(platform_label);

    wrefresh(manufacturer_value);
    wrefresh(year_value);
    //wrefresh(platform_value);

    refresh();

    while((c = wgetch(main_window)) != KEY_BACKSPACE) {
        switch(c) {
            case KEY_DOWN:
                menu_driver(game_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(game_menu, REQ_UP_ITEM);
                break;
            case KEY_RIGHT:
                menu_driver(game_menu, REQ_SCR_DPAGE);
                break;
            case KEY_LEFT:
                menu_driver(game_menu, REQ_SCR_UPAGE);
                break;
            case 0xA:
                break;
        }

        int current = item_index(current_item(game_menu));

        werase(manufacturer_value);
        werase(year_value);
        werase(platform_value);
        mvwprintw(manufacturer_value, 0, 0, "%s", games[current].manufacturer);
        mvwprintw(year_value, 0, 0, "%s", games[current].year);
        //mvwprintw(platform_value, 0, 0, "%s", games[current].platform);

        wrefresh(manufacturer_value);
        wrefresh(year_value);
        //wrefresh(platform_value);
        wrefresh(main_window);
    }

    /* Unpost and free all the memory taken up */
    unpost_menu(game_menu);
    free_menu(game_menu);

    for(i = 0; i < get_game_count(); i++)
        free_item(my_items[i]);

    free(my_items);
    clear_roms();

    endwin();

    return 0;
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{   int length, x, y;
    float temp;

    if(win == NULL)
        win = stdscr;
    getyx(win, y, x);
    if(startx != 0)
        x = startx;
    if(starty != 0)
        y = starty;
    if(width == 0)
        width = 80;

    length = strlen(string);
    temp = (width - length)/ 2;
    x = startx + (int)temp;
    wattron(win, color);
    mvwprintw(win, y, x, "%s", string);
    wattroff(win, color);
    refresh();
}

void usage() {
    printf("Usage: ./mame_menu ROMPATH MAME_XML_PATH GAME_XML_PATH\n");
}
