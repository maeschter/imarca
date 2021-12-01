/*****************************************************************************
 * kbv plugin image editor.
 * Static plugin to display images.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2017.03.17
 ****************************************************************************/
#ifndef KBVIMAGEEDITORPLUGIN_H
#define KBVIMAGEEDITORPLUGIN_H
#include <QObject>
#include <QtPlugin>
#include <QtCore>

#include <kbvPluginInterfaces.h>
#include "kbvQuestionBox.h"
#include "kbvImageEditor.h"

class KbvImageEditorPlugin : public QObject, public KbvImageEditorInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kbv.plugin.imageEditorInterface")
  Q_INTERFACES(KbvImageEditorInterface)
  
public:
  virtual ~KbvImageEditorPlugin();

  QObject* getObject() Q_DECL_OVERRIDE;
  QString pluginInfo() const Q_DECL_OVERRIDE;
  bool    showMenuItems() const Q_DECL_OVERRIDE;

  void  initialisePlugin() Q_DECL_OVERRIDE;
  void  openEditor(const QList<QPair<QString, QString> > &files) Q_DECL_OVERRIDE;
  void  openBatchEditor(const QList<QPair<QString, QString> > &files) Q_DECL_OVERRIDE;

signals:
  void  filesAltered(const QString path);
  void  noBatchProcess(const bool running);

private:
  inline void initPluginResource()    {Q_INIT_RESOURCE(editor); }
  inline void cleanupPluginResource() {Q_CLEANUP_RESOURCE(editor); }
  
  KbvImageEditor  *mEditor;
  KbvQuestionBox  mQuestionBox;

};

#endif //KBVIMAGEEDITORPLUGIN_H
/****************************************************************************/
