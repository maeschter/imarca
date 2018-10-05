#!/bin/sh
#Imarca releasee translation files
#LastChangedDate: 2018-09-20 translations centralised in kbvTranslation
#The script must be in the dir <kbvTranslation>
#Structure:
# imarca
#   | - kbvCore
#   | - kbvImageEditor
#   | - kbvMetadata
#   | - kbvTranslation
#   | - kbvHelp
#   | - test

qttooldir="/usr/lib/x86_64-linux-gnu/qt5/bin"
gendir="i18n"
if [ ! -d "$gendir" ];
then
    mkdir "$gendir"
fi

$qttooldir/lrelease  kbvCore.de.ts
#$qttooldir/lrelease  kbvCore.fr.ts

$qttooldir/lrelease  kbvImageEditor.de.ts
#$qttooldir/lrelease  kbvImageEditor.fr.ts

$qttooldir/lrelease  kbvMetadata.de.ts
#$qttooldir/lrelease  kbvMetadata.fr.ts

mv -f *.de.qm ./i18n

# For test from ide QtCreator
echo - copy to test dir
cp ./i18n/*.qm  ../test

echo Please press any key:
read key

