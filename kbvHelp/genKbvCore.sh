#!/bin/sh
#Imarca compile help
#2018.06.28 help files centralised in kbvHelp
#The script must be in the dir <kbvHelp/kbvCore
#Structure:
# imarca
#   | - kbvCore
#   | - kbvImageEditor
#   | - kbvMetadata
#   | - kbvHelp
#           | - core
#           | - imageEditor
#           | - metadata
#           | - gen
#  | - test

qttooldir="/usr/lib/x86_64-linux-gnu/qt5/bin"
gendir="gen"
if [ ! -d "$gendir" ];
then
    mkdir "$gendir"
fi

cd core

$qttooldir/qcollectiongenerator kbvCore-de.qhcp -o kbvCore.de.qhc
#$qttooldir/qcollectiongenerator kbvCore-en.qhcp -o kbvCore.en.qhc

mv -f kbvCore.de.* ../gen
#mv -f kbvCore.en.* ../gen

# For test from ide QtCreator
cd ..
echo - copy to test dir
cp ./gen/kbvCore.*.qch  ../test
cp ./gen/kbvCore.*.qhc  ../test

echo Please press any key:
read key

