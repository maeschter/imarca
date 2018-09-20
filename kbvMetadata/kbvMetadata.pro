#------------------------------------------------------------------
# ImageMetadata is a static lib for iptc exif and xmp data editing
# Depends on: libKbvExiv.so
# LastChangedDate: 2018-09-20
#  kbvmetadata        - project file
#      |- doc         - doxygen
#      |- gui         - user interfaces
#      |- prd         - lib
#          |- release - compiler output
#          |- debug   - compiler output
#      |- res         - icons and other ressources
#      |- src         - source files
# Imarca must reside in:
#  kbvCore
#
# plugins and libs must reside in their own projects
# therefore we use LIBS and INCLUDEPATH
#------------------------------------------------------------------
# $$PWD means the absolute dir path of the current .pro file.
QMAKE_CXXFLAGS += -std=c++11

TEMPLATE =  lib
TARGET   =  ImageMetadata
QT      +=  gui widgets
CONFIG  +=  plugin static

#Build in /prd by different make files
#use subdirs /debug and /release for moc, objects etc.
CONFIG  += debug_and_release

#include <kbvConstants.h>, <kbvPluginInterfaces.h>
INCLUDEPATH += $$PWD/../kbvCore/src

#own dynamic libs
INCLUDEPATH +=   $$PWD/../kbvExiv/src
LIBS        += -L$$PWD/../kbvExiv/prd -lKbvExiv

HEADERS +=  src/kbvIptcExifXmpPlugin.h \
            src/kbvImageMetaData.h \
            src/kbvCountrySelector.h

SOURCES +=  src/kbvIptcExifXmpPlugin.cpp \
            src/kbvImageMetaData.cpp \
            src/kbvImageMetaIptc.cpp \
            src/kbvImageMetaExif.cpp \
            src/kbvImageMetaXMP.cpp \
            src/kbvCountrySelector.cpp
           
FORMS   +=  gui/kbvmetadatafile.ui \
            gui/kbviptctemplatebuttons.ui \
            gui/kbvmetadatabuttons.ui \
            gui/kbviptcdata.ui \
            gui/kbvexifdata.ui \
            gui/kbvxmpdata.ui \
            gui/kbviptccontact.ui

RESOURCES += res/iptc.qrc
#TRANSLATIONS += tra/libImageMetadata.de.ts
