#!/bin/sh
#2018.07.08
#Build libImageMetadata in /prd
# kbvMetadata
#     |--src
#     |--gui
#     |--res
#     |--prd--|--debug
#             |--release
#The script must be in dir kbvMetadata
#$$PWD means the absolute dir path of the current .pro file.

export QT_SELECT=qt5-x86_64-linux-gnu

#Use /prd as build dir
cd prd
rm -f libImageMetadata.*
rm -f release/*.*

qmake ../kbvMetadata.pro -r -spec linux-g++-64
make  -j4 -w -k -f Makefile.Release
rm -f ui_*

echo Please press any key:
read key

