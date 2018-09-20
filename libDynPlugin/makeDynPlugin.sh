#!/bin/sh
#LastChangedDate: 2018-09-20
#Build libDynPlugin in /prd
#Build dirs:
# libDynPlugin  
#          |--gui
#          |--help.de
#          |--prd--|--debug
#                  |--release
#          |--res
#          |--src
#          |--tra
#The script must be in dir libDynPlugin
#$$PWD means the absolute dir path of the current .pro file.

export QT_SELECT=qt5-x86_64-linux-gnu

#Use /prd as build dir
cd prd
rm -f libDynPlugin.*
qmake ../libDynPlugin.pro -r -spec linux-g++-64
make  -j4 -w -k -f Makefile.Release
rm -f ui_*

echo Please press any key:
read key

