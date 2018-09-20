#------------------------------------------------------------------
# kbvExiv (libKbvExiv.so) is a dynamic qt wrapper for libExiv2
# LastChangedDate: 2018-09-20
#  kbvExiv            - project file
#      |- prd         - lib
#          |- release - compiler output
#          |- debug   - compiler output
#      |- src         - source files
#
# Imarca must reside in:
#  kbvCore
#------------------------------------------------------------------
# $$PWD means the absolute dir path of the current .pro file.
QMAKE_CXXFLAGS += -std=c++11

TARGET   = KbvExiv
TEMPLATE = lib
VERSION  = 0.1.0
DEFINES += KBVEXIV_LIBRARY

#Build by different make files in
#/prd/debug and prd/release for moc, objects etc.
CONFIG  += debug_and_release

#system libraries
LIBS    += -lexiv2

HEADERS +=  src/kbvExiv.h \
            src/kbvExivGlobal.h \
            src/kbvExivLocal.h

SOURCES +=  src/kbvExiv.cpp \
            src/kbvExivExif.cpp \
            src/kbvExivGps.cpp \
            src/kbvExivIptc.cpp \
            src/kbvExivXmp.cpp \


