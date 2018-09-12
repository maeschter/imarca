/*****************************************************************************
 * plugin constants
 * Globally used definitions, enums, constants and coefficients
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-16 21:37:52 +0100 (Fr, 16. Feb 2018) $
 * $Rev: 1451 $
 * Created: 2018.06.23
 *****************************************************************************/
#ifndef CONSTANTS_H_
#define CONSTANTS_H_
#include <QtCore>
#include <QMetaType>


namespace libDynPlugin
{
// General ***************************************************************
  #define pluginName            "dynPlugin"
  #define pluginVersion         "0.1.0"
  #define pluginAuthor          "maeschter"
  #define pluginAbstract        "Dynamic plugin as template"
  
  #define pluginInstallDir      "/usr/lib/imarca"         //libraries and plugin dirs
  #define helpInstallDir        "/var/lib/imarca"         //help and translation files
  #define configDir             "~/.config/imarca"        //configuration files

// Special****************************************************************
  #define helpNamespace         "org.kbv.dynPlugin"       //namespace of compressed help
  #define helpVirtualFolder     "help"                    //folder of compressed help

}
#endif /*CONSTANTS_H_*/
/****************************************************************************/
