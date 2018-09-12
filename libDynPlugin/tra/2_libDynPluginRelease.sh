#!/bin/sh
#release translation files
#2018.06.02 kbvDynPlugin (dynamic plugin pattern)
#The script must be in the dir libDynPlugin/tra

cd ..
/usr/lib/x86_64-linux-gnu/qt5/bin/lrelease libDynPlugin.pro

echo Please press any key:
read key

