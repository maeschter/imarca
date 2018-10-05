#!/bin/sh
#Imarca compile help
#LastChangedDate: 2018.06.28 help files centralised in kbvHelp
#The script must be in the dir <kbvHelp
#Structure:
# imarca
#   | - kbvCore
#   | - kbvImageEditor
#   | - kbvMetadata
#   | - kbvHelp
#           | - kbvCore
#           | - kbvImageEditor
#           | - kbvMetadata
#           | - gen
#  | - test

qttooldir="/usr/lib/x86_64-linux-gnu/qt5/bin"
gendir="gen"
if [ ! -d "$gendir" ];
then
    mkdir "$gendir"
fi

cd imageEditor

$qttooldir/qcollectiongenerator kbvImageEditor-de.qhcp -o kbvImageEditor.de.qhc
#$qttooldir/qcollectiongenerator kbvImageEditor-en.qhcp -o kbvImageEditor.en.qhc

mv -f kbvImageEditor.de.* ../gen
#mv -f kbvImageEditor.en.* ../gen

# For test from ide QtCreator
cd ..
echo - copy to test dir
cp ./gen/kbvImageEditor.*.qch  ../test

echo Please press any key:
read key

