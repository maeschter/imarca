#!/bin/sh
#2018.07.08
#Build Imarca in /prd
# kbvCore
#     |--src
#     |--gui
#     |--res
#     |--prd--|--debug
#     |--release
#The script must be in dir kbvCore
#$$PWD means the absolute dir path of the current .pro file.

export QT_SELECT=qt5-x86_64-linux-gnu

echo set library path for libKbvExiv.so
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/imarca/kbvExiv/prd/

echo update compile time
date +%y%m%d%H%M > ./res/other/kbv_buildtime

#Use /prd as build dir
cd prd
rm -f imarca
rm -f ui_*
rm -f release/*.*

qmake ../kbvCore.pro -r -spec linux-g++-64
make  -j4 -w -k -f Makefile.Release

echo Please press any key:
read key

