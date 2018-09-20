/*****************************************************************************
 * kbv plugin for image meta data.
 * Plugin for displaying and editing image meta data IPTC, EXIF and XMP
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2017.03.17
 ****************************************************************************/
#ifndef KBVIPTCEXIFXMPPLUGIN_H
#define KBVIPTCEXIFXMPPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <QtCore>
#include <kbvConstants.h>
#include <kbvPluginInterfaces.h>
#include "kbvImageMetaData.h"

class KbvIptcExifXmpPlugin : public QObject, public KbvIptcExifXmpInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kbv.plugin.iptcExifXmpInterface")
  Q_INTERFACES(KbvIptcExifXmpInterface)
  
public:
  virtual ~KbvIptcExifXmpPlugin();
  QString pluginInfo() const Q_DECL_OVERRIDE;
  bool    showMenuItems() const Q_DECL_OVERRIDE;
  QString getIptcInfo(QString path) const Q_DECL_OVERRIDE;
  QString getExifInfo(QString path) const Q_DECL_OVERRIDE;
  

  void  initialisePlugin() Q_DECL_OVERRIDE;
  void  openEditor(const QList<QPair<QString, QString> > &files) Q_DECL_OVERRIDE;
  void  openIptcTemplate() Q_DECL_OVERRIDE;

private:
  inline void initPluginResource()    {Q_INIT_RESOURCE(iptc); }
  inline void cleanupPluginResource() {Q_CLEANUP_RESOURCE(iptc); }
  
  KbvImageMetaData  *imageMetadata;
};

#endif //KBVIPTCEXIFXMPPLUGIN_H
/****************************************************************************/
