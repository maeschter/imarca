/*****************************************************************************
 * Dynamic plugin interface
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-06-02 10:47:16 +0200 (So, 02. Apr 2017) $
 * $Rev: 1234 $
 * Created: 2018.06.06
 ****************************************************************************/
#ifndef INTERFACE_H
#define INTERFACE_H
#include <QObject>
#include <QtPlugin>
#include <QtCore>
#include <kbvPluginInterfaces.h>
#include "mainWindow.h"


class Interface : public QObject, public KbvDynamicPluginInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kbv.plugin.dynamicInterface")
  Q_INTERFACES(KbvDynamicPluginInterface)
  
public:
  virtual ~Interface() Q_DECL_OVERRIDE;
  void          initialisePlugin() Q_DECL_OVERRIDE;
  KbvDynamicPluginInterface::PLUGIN_INFO   getPluginInfo() const Q_DECL_OVERRIDE;
  QMenu*        getPluginMenu() const Q_DECL_OVERRIDE;

  MainWindow      *mainWin;
  //QTranslator   *translator;

private:
  inline void initPluginResource()    {Q_INIT_RESOURCE(dynplugin); }
  inline void cleanupPluginResource() {Q_CLEANUP_RESOURCE(dynplugin); }

  KbvDynamicPluginInterface::PLUGIN_INFO    pluginInfo;
};

#endif //INTERFACE_H
/****************************************************************************/
