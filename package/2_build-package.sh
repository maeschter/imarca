#!/bin/sh
#Build deban package
#LastChangedDate: 2018.09.20

#working dir: imarca
cd imarca
dpkg-buildpackage -us -uc

echo aufräumen
#rm -rf debian/imarca

echo Fertig
echo Bitte Taste drücken
read taste

