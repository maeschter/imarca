/*****************************************************************************
 * kbv plugin interfaces.
 * Interface declarations for static and dynamic plugins
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 15:27:08 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1470 $
 * Created: 2017.03.17
 ****************************************************************************/
#ifndef KBVPLUGININTERFACES_H_
#define KBVPLUGININTERFACES_H_

#include <QtCore>
#include <QtWidgets>
#include <QtPlugin>

/*****************************************************************************
 * Static plugin libImageMetadata
 * Interface for manipulating image meta data IPTC, EXIF and XMP
 */
class KbvIptcExifXmpInterface
{
  public:
    virtual ~KbvIptcExifXmpInterface() {}
    
    virtual QString pluginInfo() const = 0;
    virtual bool    showMenuItems() const = 0;
    virtual QString getIptcInfo(QString path) const = 0;
    virtual QString getExifInfo(QString path) const = 0;

    virtual void  initialisePlugin() = 0;
    virtual void  openEditor(const QList<QPair<QString, QString> > &files) = 0;
    virtual void  openIptcTemplate() = 0;
};

Q_DECLARE_INTERFACE(KbvIptcExifXmpInterface, "org.kbv.plugin.iptcExifXmpInterface")

/*****************************************************************************
 * Static plugin libImageEditor
 * Interface for manipulating images using openCV
 */
class KbvImageEditorInterface
{
  public:
    virtual ~KbvImageEditorInterface() {}
    
    virtual QObject* getObject() = 0;
    virtual QString pluginInfo() const = 0;
    virtual bool  showMenuItems() const = 0;
    virtual void  initialisePlugin() = 0;

    virtual void  openEditor(const QList<QPair<QString, QString> > &files) = 0;
    virtual void  openBatchEditor(const QList<QPair<QString, QString> > &files) = 0;

    //signals in plugin: filesChanged(const QString &path)
};

Q_DECLARE_INTERFACE(KbvImageEditorInterface, "org.kbv.plugin.imageEditorInterface")

/*****************************************************************************
 * Dynamic plugin interface
 * Interface for all dynamic plugins
 */
class KbvDynamicPluginInterface
{
public:
  struct  PLUGIN_INFO
    {
      QString    name;         //plugin name
      QString    version;      //plugin version x.y.z
      QString    author;       //person or company
      QString    abstract;     //short description
    };

  virtual ~KbvDynamicPluginInterface() {}

  virtual void          initialisePlugin() = 0;
  virtual PLUGIN_INFO   getPluginInfo() const = 0;
  virtual QMenu*        getPluginMenu() const = 0;
};

Q_DECLARE_INTERFACE(KbvDynamicPluginInterface, "org.kbv.plugin.dynamicInterface")

#endif // KBVPLUGININTERFACES
/****************************************************************************/
