#!/bin/sh
#copy dyn lib to dir "plugins" for test with QtCreator
#LastChangedDate: 2018-09-20
#The script must be in the dir kbvDynPlugin

cp -f ./prd/libDynPlugin.so              ../test/libDynPlugin
cp -f ./tra/libDynPlugin.de.qm           ../test/libDynPlugin
cp -f ./help.de/gen/libDynPlugin.de.qch  ../test/libDynPlugin

