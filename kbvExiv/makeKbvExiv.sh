#!/bin/sh
#2018.07.08
#Build kbvExiv in /prd
# kbvExiv
#     |--src
#     |--prd--|--debug
#             |--release
#The script must be in dir kbvExiv
#$$PWD means the absolute dir path of the current .pro file.

export QT_SELECT=qt5-x86_64-linux-gnu

#Use /prd as build dir
cd prd
rm -f libKbvExiv.*
rm -f release/*.*
qmake ../kbvExiv.pro -r -spec linux-g++-64
make -j4 -w -k -f Makefile.Release

echo Please press any key:
read key

