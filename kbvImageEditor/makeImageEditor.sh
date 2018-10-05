#!/bin/sh
#LastChangedDate: 2018-09-20
#Build kbvImageEditor in /prd
# kbvImageEditor
#     |--src
#     |--gui
#     |--res
#     |--prd--|--debug
#             |--release
#The script must be in dir kbvImageEditor
#$$PWD means the absolute dir path of the current .pro file.

export QT_SELECT=qt5-x86_64-linux-gnu

#Use /prd as build dir
cd prd
rm -f libImageEditor.*
rm -f release/*.*
qmake ../kbvImageEditor.pro -r -spec linux-g++-64
make  -j4 -w -k -f Makefile.Release
rm -f ui_*

echo Please press any key:
read key

