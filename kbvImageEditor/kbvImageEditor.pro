#------------------------------------------------------------------
# ImageEditor is a static lib for manipulating images
# Depends on: libKbvExiv.so
# LastChangedDate: 2018-09-20
#  kbvImageEditor     - project file
#      |- doc         - doxygen
#      |- gui         - user interfaces
#      |- prd         - lib
#          |- release - compiler output
#          |- debug   - compiler output
#      |- res         - icons and other ressources
#      |- src         - source files
# Imarca must reside in:
#  kbvCore
#------------------------------------------------------------------
# $$PWD means the absolute dir path of the current .pro file.

QMAKE_CXXFLAGS += -std=c++11

TEMPLATE =  lib
TARGET   =  ImageEditor
QT      +=  gui widgets printsupport
CONFIG  +=  plugin static

#Build in /prd by different make files
#use subdirs /debug and /release for moc, objects etc.
CONFIG  += debug_and_release
#OBJECTS_DIR  = prd
#DESTDIR      = prd
#MOC_DIR      = prd
#UI_DIR       = prd
#RCC_DIR      = prd

#include <kbvConstants.h>, <kbvPluginInterfaces.h>
INCLUDEPATH += $$PWD/../kbvCore/src

#own dynamic libs
INCLUDEPATH +=   $$PWD/../kbvExiv/src
LIBS        += -L$$PWD/../kbvExiv/prd -lKbvExiv
#other libs
LIBS        += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs

HEADERS +=  src/kbvImageEditorPlugin.h \
            src/kbvImageLabel.h \
            src/kbvImageReader.h \
            src/kbvImageEditor.h \
            src/kbvFileDialog.h \
            src/kbvRubberBand.h \
            src/kbvImageEditorUndo.h \
            src/kbvQuestionBox.h \
            src/kbvBatchDialog.h \
            src/kbvBatchProgress.h \
            src/kbvBatchThread.h

SOURCES +=  src/kbvImageEditorPlugin.cpp \
            src/kbvImageLabel.cpp \
            src/kbvImageReader.cpp \
            src/kbvImageEditor.cpp \
            src/kbvFileDialog.cpp \
            src/kbvRubberBand.cpp \
            src/kbvImageEditorUndo.cpp \
            src/kbvQuestionBox.cpp \
            src/kbvBatchDialog.cpp \
            src/kbvBatchProgress.cpp \
            src/kbvBatchThread.cpp
           
RESOURCES +=  res/editor.qrc
FORMS     +=  gui/kbvQuestionBox.ui \
              gui/kbvBatchWizard.ui \
              gui/kbvBatchProgress.ui

#TRANSLATIONS += tra/libImageEditor.de.ts

