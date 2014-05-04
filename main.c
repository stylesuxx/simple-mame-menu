#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>
#include <regex.h>

#include "roms.h"
#include "romsorter.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

WINDOW *main_window;
MENU *game_menu;
ITEM **game_items;

WINDOW *info_window;
WINDOW *manufacturer_label;
WINDOW *year_label;
WINDOW *times_played_label;
WINDOW *game_state_label;
WINDOW *sorter_label;
WINDOW *manufacturer_value;
WINDOW *year_value;
WINDOW *times_played_value;
WINDOW *game_state_value;
WINDOW *sorter_container;
MENU *sort_menu;
ITEM **sort_items;

rom_data **games;

char *mame_ini_path = NULL;
const char *game_xml_path = NULL;

int sorting = 0, n_sorts = 0;
char *sort_menu_items[] = {"A..Z", "Z..A", "Favs", (char *) NULL};

int i = 0;
int width = 0, height = 0;
int main_window_width = 0, main_window_height = 0;
int info_window_width = 0, info_window_height = 0;

void build_main_window();
void build_info_window();
void build_game_menu();
void process_input();
void update_values(int position);

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
void set_colors();
void usage();

int main(int argc, char *argv[])
{
    if(argc < 3) {
        usage();
        return 1;
    }

    mame_ini_path = argv[1];
    game_xml_path = argv[2];

    /* Generate a list of available games and sort them ascending */
    build_rom_list(mame_ini_path, game_xml_path);
    games = get_all_games();
    if(get_game_count() < 1) {
        fprintf(stderr, "There are no games in your rom path.\nCheck that your roms are in one of the rompaths configured in mame.ini\n");
        return 1;
    }
    qsort(games, (size_t) get_game_count(), sizeof(rom_data *), rom_slug_asc);

    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_colors();

    /* Build the UI depending on size */
    getmaxyx(stdscr, height, width);
    info_window_width = 30;
    info_window_height = height;
    main_window_width = width - info_window_width;
    main_window_height = info_window_height;

    build_main_window();
    build_info_window();
    update_values(0);
    refresh();

    /* Process user input until ESC */
    process_input();

    /* Clean up */
    unpost_menu(game_menu);
    free_menu(game_menu);
    for(i = 0; i < get_game_count(); i++)
        free_item(game_items[i]);
    free(game_items);
    unpost_menu(sort_menu);
    free_menu(sort_menu);
    for(i = 0; i < 3; i++)
        free_item(sort_items[i]);
    free(sort_items);
    clear_roms();
    endwin();

    return 0;
}

void build_main_window()
{
    main_window = newwin(main_window_height, main_window_width, 0, 0);
    keypad(main_window, TRUE);

    box(main_window, 0, 0);
    print_in_middle(main_window, 1, 0, main_window_width, "Game selection", COLOR_PAIR(1));
    mvwaddch(main_window, 2, 0, ACS_LTEE);
    mvwhline(main_window, 2, 1, ACS_HLINE, width);
    mvwaddch(main_window, 2, main_window_width - 1, ACS_RTEE);

    build_game_menu();
}

void build_info_window()
{
    info_window = newwin(info_window_height, info_window_width, 0, main_window_width);
    box(info_window, 0, 0);
    print_in_middle(info_window, 1, 0, info_window_width, "Info", COLOR_PAIR(1));
    mvwaddch(info_window, 2, 0, ACS_LTEE);
    mvwhline(info_window, 2, 1, ACS_HLINE, info_window_width);
    mvwaddch(info_window, 2, info_window_width - 1, ACS_RTEE);

    manufacturer_label = newwin(1, info_window_width - 2, 3, main_window_width + 1);
    mvwprintw(manufacturer_label, 0, 0, "%s", "Manufacturer:");

    year_label = newwin(1, info_window_width - 2, 6, main_window_width + 1);
    mvwprintw(year_label, 0, 0, "%s", "Year:");

    times_played_label = newwin(1, info_window_width - 2, 9, main_window_width + 1);
    mvwprintw(times_played_label, 0, 0, "%s", "Times played:");

    game_state_label = newwin(1, info_window_width - 2, 12, main_window_width + 1);
    mvwprintw(game_state_label, 0, 0, "%s", "Game state:");

    sorter_label = newwin(1, info_window_width - 2, 15, main_window_width + 1);
    mvwprintw(sorter_label, 0, 0, "%s", "Sort by [Btn1 + <-|->]:");

    sorter_container = newwin(1, info_window_width - 2, 16, main_window_width + 1);

    manufacturer_value = newwin(1, info_window_width - 2, 4, main_window_width + 1);
    year_value = newwin(1, info_window_width - 2, 7, main_window_width + 1);
    times_played_value = newwin(1, info_window_width - 2, 10, main_window_width + 1);
    game_state_value = newwin(1, info_window_width - 2, 13, main_window_width + 1);

    /* Create horizontal sorter menu without descriptions */
    n_sorts = ARRAY_SIZE(sort_menu_items);
    sort_items = (ITEM **)calloc(n_sorts, sizeof(ITEM *));
    for(i = 0; i < n_sorts; ++i)
        sort_items[i] = new_item(sort_menu_items[i], sort_menu_items[i]);
    sort_menu = new_menu(sort_items);
    menu_opts_off(sort_menu, O_SHOWDESC);

    set_menu_win(sort_menu, sorter_container);
    set_menu_sub(sort_menu, derwin(sorter_container, 1, 5, 3, 1));
    set_menu_format(sort_menu, 1, 3);
    set_menu_mark(sort_menu, " ");

    post_menu(sort_menu);
    wrefresh(info_window);
    wrefresh(manufacturer_label);
    wrefresh(year_label);
    wrefresh(times_played_label);
    wrefresh(game_state_label);
    wrefresh(sorter_label);
    wrefresh(sorter_container);
}


void build_game_menu()
{
    switch(sorting) {
        case 0:
            qsort(games, (size_t) get_game_count(), sizeof(rom_data *), rom_slug_asc);
            break;
        case 1:
            qsort(games, (size_t) get_game_count(), sizeof(rom_data *), rom_slug_desc);
            break;
        case 2:
            qsort(games, (size_t) get_game_count(), sizeof(rom_data *), rom_played_desc);
            break;
    }

    if(game_menu != NULL) {
        unpost_menu(game_menu);
        free_menu(game_menu);
        for(i = 0; i < get_game_count() + 1 ; i++)
            free_item(game_items[i]);

        free(game_items);
    }

    game_items = malloc((get_game_count() + 1) * sizeof(ITEM *));
    for(i = 0; i < get_game_count(); i++)
        game_items[i] = new_item(games[i]->slug, games[i]->description);
    game_items[get_game_count()] = NULL;

    game_menu = new_menu(game_items);
    set_menu_win(game_menu, main_window);
    set_menu_sub(game_menu, derwin(main_window, main_window_height - 4, main_window_width - 2, 3, 1));
    set_menu_format(game_menu, main_window_height - 4, 1);
    set_menu_mark(game_menu, " > ");
    post_menu(game_menu);

    refresh();
}

void process_input()
{
    int user_input;
    char buffer[256];
    int current = item_index(current_item(game_menu));
    while((user_input = wgetch(main_window)) != 27) {
        switch(user_input) {
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
            case 0x22F:
                menu_driver(sort_menu, REQ_RIGHT_ITEM);
                sorting ++;
                if(sorting > 2)
                    sorting = 2;

                build_game_menu();
                break;
            case 0x220:
                menu_driver(sort_menu, REQ_LEFT_ITEM);
                sorting --;
                if(sorting < 0)
                    sorting = 0;

                build_game_menu();
                break;

            case 0x20:
            case 0x0A:
                increase_times_played(games[current]->slug);
                sprintf(buffer, "mame -inipath %s %s > /dev/null 2>&1", mame_ini_path, games[current]->slug);
                system(buffer);
                break;

            /*
            default:
                fprintf(stderr, "Charcter pressed is '%d'", user_input);
                break;
            */
        }

        current = item_index(current_item(game_menu));
        update_values(current);
    }
}

/* Update all necesary info when game selection changes */
void update_values(int position)
{
    werase(manufacturer_value);
    werase(year_value);
    werase(times_played_value);
    werase(game_state_value);

    mvwprintw(manufacturer_value, 0, 0, "%s", games[position]->manufacturer);
    mvwprintw(year_value, 0, 0, "%s", games[position]->year);
    mvwprintw(times_played_value, 0, 0, "%d", games[position]->times_played);
    mvwprintw(game_state_value, 0, 0, "%s", games[position]->state);

    wrefresh(manufacturer_value);
    wrefresh(year_value);
    wrefresh(times_played_value);
    wrefresh(game_state_value);

    wrefresh(main_window);
    wrefresh(sorter_container);
}

void set_colors()
{
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
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
    printf("Usage: ./mame_menu PATH_TO_INIS GAME_XML_PATH\n");
}
