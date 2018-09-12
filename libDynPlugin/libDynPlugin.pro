#------------------------------------------------------------------
# libDynPlugin is a dynamic plugin pattern for testing
# $LastChangedDate: 2018-06-02 10:37:37 +0100 (Do, 15. Mär 2018) $
# $Rev: 1495 $
#  libDynPlugin       - project file
#      |- doc         - doxygen
#      |- gui         - user interfaces
#      |- help.de     - .de help files
#      |- prd         - app
#          |- debug   - debug compiler output
#          |- debug   - debug compiler output
#      |- res         - icons and other ressources
#      |- src         - source files
#      |- tra         - all translation files
# Imarca core must reside in:
#  kbvCore

# 3rd party plugins and libs must reside in their own projects
# therefore we use LIBS and INCLUDEPATH
#------------------------------------------------------------------
# $$PWD means the absolute dir path of the current .pro file.
QMAKE_CXXFLAGS += -std=c++11

TEMPLATE =  lib
TARGET   =  DynPlugin
QT      +=  gui widgets
CONFIG  +=  plugin

#Build in /prd by different make files
#use subdirs /debug and /release for moc, objects etc.
CONFIG  += debug_and_release

#include <kbvConstants.h>, <kbvPluginInterfaces.h>
INCLUDEPATH += $$PWD/../kbvCore/src


HEADERS += \ 
    src/interface.h \
    src/mainWindow.h \
    src/mainMenu.h \
    src/constants.h

SOURCES += \ 
    src/interface.cpp \
    src/mainWindow.cpp \
    src/mainMenu.cpp
           
#FORMS   +=

TRANSLATIONS += tra/libDynPlugin.de.ts

RESOURCES += res/plugin.qrc

