#!/bin/sh
#update translation files
#2018.06.02 kbvDynPlugin (dynamic plugin pattern)
#The script must be in the dir libDynPlugin/tra

cd ..
#Using the *.pro-file doesn't include threads!
#/usr/lib/x86_64-linux-gnu/qt5/bin/lupdate -no-obsolete libDynPlugin.pro
/usr/lib/x86_64-linux-gnu/qt5/bin/lupdate -no-obsolete src gui -ts tra/libDynPlugin.de.ts

echo Translate with qt linguist
echo Press any key:
read key

