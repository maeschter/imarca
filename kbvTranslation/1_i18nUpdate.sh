#!/bin/sh
#Imarca update translation files
##2018.06.27 translations centralised in kbvTranslation
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

$qttooldir/lupdate -no-obsolete ../kbvCore/src ../kbvCore/gui -ts kbvCore.de.ts
#$qttooldir/lupdate -no-obsolete ../kbvCore/src ../kbvCore/gui -ts kbvCore.fr.ts

$qttooldir/lupdate -no-obsolete ../kbvImageEditor/src ../kbvImageEditor/gui -ts kbvImageEditor.de.ts
#$qttooldir/lupdate -no-obsolete ../kbvImageEditor/src ../kbvImageEditor/gui -ts kbvImageEditor.fr.ts

$qttooldir/lupdate -no-obsolete ../kbvMetadata/src ../kbvMetadata/gui -ts kbvMetadata.de.ts
#$qttooldir/lupdate -no-obsolete ../kbvMetadata/src ../kbvMetadata/gui -ts kbvMetadata.fr.ts

echo Please press any key:
read key

