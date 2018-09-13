#!/bin/sh
#Build deban package
#2018.06.18

#working dir: imarca
cd imarca
dpkg-buildpackage -us -uc

echo aufräumen
#rm -rf debian/imarca

echo Fertig
echo Bitte Taste drücken
read taste

