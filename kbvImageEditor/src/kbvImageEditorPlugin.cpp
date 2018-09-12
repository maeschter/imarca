/*****************************************************************************
 * kbv plugin image editor.
 * Static plugin to manipulate images.
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-06-02 18:00:39 +0200 (Fr, 02. Jun 2017) $
 * $Rev: 1350 $
 * Created: 2017.03.17
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "kbvImageEditorPlugin.h"


KbvImageEditorPlugin::~KbvImageEditorPlugin()
{
  //qDebug() << "KbvImageEditorPlugin::~KbvImageEditorPlugin";//###########
  cleanupPluginResource();
  
  //do not delete the editor
  //the editor deletes itself when the window get closed;
}
/*************************************************************************//*!
 * Initialise ressources, setup gui etc.
 */
void   KbvImageEditorPlugin::initialisePlugin()
{
  initPluginResource();
  
  //qDebug() << "KbvImageEditorPlugin::initialisePlugin done"; //###########
}
/*************************************************************************//*!
 * Return a pointer to imageEditorPlugin object
 */
QObject*   KbvImageEditorPlugin::getObject()
{
  return this;
}
/*************************************************************************//*!
 * Return the plugin info
 */
QString   KbvImageEditorPlugin::pluginInfo() const
{
  return  QString("libImageEditor: manipulate images");
}
/*************************************************************************//*!
 * Show items for imageEditor in main menu "Image"
 */
bool   KbvImageEditorPlugin::showMenuItems() const
{
  return true;
}
/*************************************************************************//*!
 * Open the gui to edit images.
 * On close event all object deletions are done in the editor itself.
 */
void   KbvImageEditorPlugin::openEditor(const QList<QPair<QString, QString> > &files)
{
  QString   path, name;

  if(!files.empty())
    {
      path = files.at(0).first;
      name = files.at(0).second;
      //qDebug() << "KbvImageEditorPlugin::openEditor" <<name <<path; //###########

      mEditor = new KbvImageEditor();
      connect(mEditor,  SIGNAL(fileChanged(QString)), this, SIGNAL(filesAltered(const QString)));

      mEditor->setFiles(files);
      mEditor->loadImage(path, name);
    }
  else
    {
      //information box
      mQuestionBox.exec(QString(tr("No files selected.")), QString(tr("Image processing not possible. At least one file must be selected.")));
    }
}
/*************************************************************************//*!
 * Open the gui for batch processing images.
 * On close event all object deletions are done in the editor itself.
 */
void   KbvImageEditorPlugin::openBatchEditor(const QList<QPair<QString, QString> > &files)
{
  QString   path, name;

  if(!files.empty())
    {
      path = files.at(0).first;
      name = files.at(0).second;
      //qDebug() << "KbvImageEditorPlugin::openEditor" <<name <<path; //###########

      mEditor = new KbvImageEditor();
      connect(mEditor->batchThread,  SIGNAL(filesChanged(QString)),  this, SIGNAL(filesAltered(const QString)));
      connect(mEditor->batchThread,  SIGNAL(threadNotRunning(bool)), this, SIGNAL(noBatchProcess(const bool)));

      mEditor->setFiles(files);
      mEditor->loadImage(path, name);
      mEditor->startBatchProcess();
    }
  else
    {
      //information box
      mQuestionBox.exec(QString(tr("No files selected.")), QString(tr("Image processing not possible. At least one file must be selected.")));
    }
}

/****************************************************************************/
