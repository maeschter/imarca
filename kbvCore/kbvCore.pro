#---------------------------------------------------------------
# Imarca is the app for managing image collections
# kbvCore is the central software part of imarca
# LastChangedDate: 2018-09-20
#  kbvCore            - project file
#      |- doc         - doxygen
#      |- gui         - user interfaces
#      |- prd         - executable app
#          |- release - compiler output
#          |- debug   - compiler output
#      |- res         - icons and other ressources
#      |- src         - source files
#
# plugins and libs must reside in:
#  kbvImageEditor
#  kbvMetadata
#  kbvExiv
# Access with LIBS and INCLUDEPATH
#---------------------------------------------------------------
# $$PWD means the absolute dir path of the current .pro file.

#QMAKE_CXXFLAGS_RELEASE += -O1
QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app
TARGET   = imarca
QT      += core gui widgets sql
QT      += dbus printsupport help

#Build in /prd by different make files
#use subdirs /debug and /release for moc, objects etc.
CONFIG  += debug_and_release

#paths to static plugins
LIBS  += -L$$PWD/../kbvMetadata/prd -lImageMetadata
LIBS  += -L$$PWD/../kbvImageEditor/prd -lImageEditor

#own dynamic libs
INCLUDEPATH +=   $$PWD/../kbvExiv/src
LIBS        += -L$$PWD/../kbvExiv/prd -lKbvExiv  #for test with LD_LIBRARY_PATH

#system libraries
LIBS    += -ljpeg -lpng -lopencv_core -lopencv_imgproc -lopencv_imgcodecs

HEADERS +=  src/kbvPluginInterfaces.h \
            src/kbvSortFilterModel.h \
            src/kbvMain.h \
            src/kbvMainStatus.h \
            src/kbvMainMenu.h \
            src/kbvCollectionStack.h \
            src/kbvCollectionPopUp.h \
            src/kbvCollectionDragDrop.h \
            src/kbvCollectionTabs.h \
            src/kbvCollectionModel.h \
            src/kbvCollectionView.h \
            src/kbvCollectionTreeView.h \
            src/kbvCollectionTreeModel.h \
            src/kbvCollectionThread.h \
            src/kbvCollectionWatchThread.h \
            src/kbvTabBar.h \
            src/kbvTabWidget.h \
            src/kbvDirModel.h \
            src/kbvDirView.h \
            src/kbvFileTab.h \
            src/kbvFileModel.h \
            src/kbvFileModelThread.h \
            src/kbvFileWatcherThread.h \
            src/kbvFileView.h \
            src/kbvSearchModel.h \
            src/kbvSearchView.h \
            src/kbvSearchTab.h \
            src/kbvSearchThread.h \
            src/kbvDBSearchDialog.h \
            src/kbvInformationDialog.h \
            src/kbvBusyAnimation.h \
            src/kbvDBInfo.h \
            src/kbvDBOptions.h \
            src/kbvDBExport.h \
            src/kbvDBImport.h \
            src/kbvDBImExportThread.h \
            src/kbvDBImExportProgress.h \
            src/kbvRecordInfo.h \
            src/kbvHelp.h \
            src/kbvHelpInfo.h \
            src/kbvRenameDialog.h \
            src/kbvReplaceDialog.h \
            src/kbvConstants.h \
            src/kbvGeneral.h \
            src/kbvGlobal.h \
            src/kbvOptions.h \
            src/kbvSetvalues.h \
            src/kbvDBusMonitor.h \
            src/kbvHelpBrowser.h \
            src/kbvCsvChecker.h \
            src/kbvCsvCheckThread.h \
            src/kbvImageViewer.h

#    src/kbvDirWatcherThread.h \
#    src/kbvSlideScene.h \
#    src/kbvSlideView.h

SOURCES +=  src/kbvSortFilterModel.cpp \
            src/kbvMain.cpp \
            src/kbvMainStatus.cpp \
            src/kbvMainMenu.cpp \
            src/kbvCollectionStack.cpp \
            src/kbvCollectionPopUp.cpp \
            src/kbvCollectionDragDrop.cpp \
            src/kbvCollectionTabs.cpp \
            src/kbvCollectionModel.cpp \
            src/kbvCollectionView.cpp \
            src/kbvCollectionTreeView.cpp \
            src/kbvCollectionTreeModel.cpp \
            src/kbvCollectionThread.cpp \
            src/kbvCollectionWatchThread.cpp \
            src/kbvTabBar.cpp \
            src/kbvTabWidget.cpp \
            src/kbvDirModel.cpp \
            src/kbvDirView.cpp \
            src/kbvFileTab.cpp \
            src/kbvFileModel.cpp \
            src/kbvFileModelThread.cpp \
            src/kbvFileWatcherThread.cpp \
            src/kbvFileView.cpp \
            src/kbvSearchModel.cpp \
            src/kbvSearchView.cpp \
            src/kbvSearchTab.cpp \
            src/kbvSearchThread.cpp \
            src/kbvDBSearchDialog.cpp \
            src/kbvInformationDialog.cpp \
            src/kbvBusyAnimation.cpp \
            src/kbvDBInfo.cpp \
            src/kbvDBOptions.cpp \
            src/kbvDBExport.cpp \
            src/kbvDBImport.cpp \
            src/kbvDBImExportThread.cpp \
            src/kbvDBImExportProgress.cpp \
            src/kbvRecordInfo.cpp \
            src/kbvHelp.cpp \
            src/kbvHelpInfo.cpp \
            src/kbvRenameDialog.cpp \
            src/kbvReplaceDialog.cpp \
            src/kbvGeneral.cpp \
            src/kbvGlobal.cpp \
            src/kbvOptions.cpp \
            src/kbvSetvalues.cpp \
            src/main.cpp \
            src/kbvDBusMonitor.cpp \
            src/kbvHelpBrowser.cpp \
            src/kbvCsvChecker.cpp \
            src/kbvCsvCheckThread.cpp \
            src/kbvImageViewer.cpp

#    src/kbvDirWatcherThread.cpp \
#    src/kbvSlideScene.cpp \
#    src/kbvSlideView.cpp

FORMS +=    gui/kbvhelpinfo.ui \
            gui/kbvdboptions.ui \
            gui/kbvdbinfo.ui \
            gui/kbvdbexport.ui \
            gui/kbvdbimport.ui \
            gui/kbvdbimexprogress.ui \
            gui/kbvrecordinfo.ui \
            gui/kbvdbsearch.ui \
            gui/kbvanimation.ui \
            gui/kbvoptions.ui \
            gui/kbvrename.ui \
            gui/kbvreplace.ui \
            gui/kbvcvscheck.ui \

RESOURCES +=  res/kbv.qrc \
              res/other.qrc

TRANSLATIONS += tra/kbvCore.de.ts
