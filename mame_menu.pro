TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/include/libxml2
LIBS += -I/usr/include/ -lxml2
LIBS += -lncurses
LIBS += -lmenu

SOURCES += main.c \
    xmlparser.c \
    roms.c

HEADERS += \
    xmlparser.h \
    roms.h

