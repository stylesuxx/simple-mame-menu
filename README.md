# simple mame menu

A simple ncurses based frontend for mame.

## Build

    qmake
    make

## Usage

    Usage: ./mame_menu PATH_TO_INIS GAME_XML_PATH

## Controlls

The menu can easyli be controlled with the same keys as the default mame config for player 1.

    up/down:                choose game
    left/right:             scroll page up/down
    CTRL/BTN1 + left/right: change sort
    SELECT/ENTER/BTN3:      start game
    ESC:                    exit
