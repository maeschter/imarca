#!/bin/sh
#Imarca compile help
#2010.12.19 qt4
#2016.07.07 qt5
#2018.06.08 separated for metadata editor
#The script must be in the same dir as the *.qhcp and *.qhp source files
#Compile help project into help files  /gen/libDynPlugin.de.qhc and /gen/libDynPlugin.de.qch

gendir="gen"
if [ ! -d "$gendir" ];
then
    mkdir "$gendir"
fi
/usr/lib/x86_64-linux-gnu/qt5/bin/qcollectiongenerator libDynPlugin-de.qhcp -o gen/libDynPlugin.de.qhc

echo Please press any key:
read key

