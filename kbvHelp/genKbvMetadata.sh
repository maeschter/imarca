#!/bin/sh
#Imarca compile help
#2018.06.28 help files centralised in kbvHelp
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

cd metadata

$qttooldir/qcollectiongenerator kbvMetadata-de.qhcp -o kbvMetadata.de.qhc
#$qttooldir/qcollectiongenerator kbvMetadata-en.qhcp -o kbvMetadata.en.qhc

mv -f kbvMetadata.de.* ../gen
#mv -f kbvMetadata.en.* ../gen

# For test from ide QtCreator
cd ..
echo - copy to test dir
cp ./gen/kbvMetadata.*.qch  ../test

echo Please press any key:
read key

