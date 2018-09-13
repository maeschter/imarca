#!/bin/sh
#Build deban package
#2018.06.18
#The script must be in the dir <package>
# ~imarca/kbvCore/prd
# ~imarca/kbvExiv/prd
# ~imarca/package
# ~imarca/package/imarca
# ~imarca/package/imarca/debian
# ~imarca/package/imarca/usr

#working dir: ~imarca/package/imarca
echo start package update
cd imarca
pwd

echo
# application (imarca.install)
rm -f -v usr/bin/*.*
cp -a -v ../../kbvCore/prd/imarca  usr/bin

echo
# dynamic libs and plugins (imarca.install)
rm -f -v usr/lib/libKbvExiv.*
cp -a -v ../../kbvExiv/prd/libKbvExiv.so.0                       usr/lib/x86_64-linux-gnu
cp -a -v ../../kbvExiv/prd/libKbvExiv.so.0.1.0                   usr/lib/x86_64-linux-gnu

rm -f -v usr/lib/imarca/*.*
#cp -a -v ../../libDynPlugin/prd/libDynPlugin.so                 usr/lib/imarca/libDynPlugin
#cp -a -v ../../libDynPlugin/tra/libDynPlugin.de.qm              usr/lib/imarca/libDynPlugin
#cp -a -v ../../libDynPlugin/help.de/gen/libDynPlugin.de.qch     usr/lib/imarca/libDynPlugin

echo
# translations (imarca.install)
rm -f -v var/lib/imarca/*.*
cp -a -v ../../kbvTranslation/i18n/kbvCore.*.qm                  var/lib/imarca
cp -a -v ../../kbvTranslation/i18n/kbvMetadata.*.qm              var/lib/imarca
cp -a -v ../../kbvTranslation/i18n/kbvImageEditor.*.qm           var/lib/imarca

echo
# help files (imarca.install)
cp -a -v ../../kbvHelp/gen/kbvCore.*.qhc                         var/lib/imarca
cp -a -v ../../kbvHelp/gen/kbvCore.*.qch                         var/lib/imarca
cp -a -v ../../kbvHelp/gen/kbvMetadata.*.qch                     var/lib/imarca
cp -a -v ../../kbvHelp/gen/kbvImageEditor.*.qch                  var/lib/imarca

echo
echo all done
echo Please press any key:
read key

