/*****************************************************************************
 * kbvCollectionView
 * This is the standard view for albums and collections
 * This view is designed to display data from album/collection database or
 * from search result.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2011.10.04
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvCollectionView.h"
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvPluginInterfaces.h"
//#include "kbvSlideView.h"
//#include "kbvSlideScene.h"
#include "kbvRenameDialog.h"
#include "kbvInformationDialog.h"

extern  KbvSetvalues            *settings;
extern  KbvRenameDialog         *renameDialog;
extern  KbvInformationDialog    *informationDialog;
extern  KbvImageEditorInterface *imageEditPlugin;

KbvCollectionView::KbvCollectionView(QWidget *parent, KbvDBInfo *databaseInfo) : QListView(parent)
{
  //Read database properties from databaseInfo
  dbName = databaseInfo->getName();
  dbType = databaseInfo->getType();
  dbIconSize = databaseInfo->getIconSize();
  dbInfo = databaseInfo;

  this->setWrapping(true);
  this->setResizeMode(QListView::Adjust);
  this->setViewMode(QListView::IconMode);   //layout from left to right
  //this->setViewMode(QListView::ListMode); //layout top down
  this->setFrameShadow (QFrame::Raised);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setTabKeyNavigation(true);

  this->setDragDropMode(QAbstractItemView::DragDrop);
  this->setDragEnabled(true);
  this->setAcceptDrops(true);
  this->setDropIndicatorShown(true);

  //Double click, Enter or Return on a selection of icons
  connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(displaySlideShow(QModelIndex)));

  //default
  clipboard = QApplication::clipboard();
  dndHandler = 0;
  collModel = 0;
  weAreVisible = false;
}

KbvCollectionView::~KbvCollectionView()
{
  //qDebug() << "KbvCollectionView::~KbvCollectionView"; //###########
}

/*************************************************************************//*!
 * Set the model by calling the base class and set a local pointer.
 */
void KbvCollectionView::setModels(KbvSortFilterModel *sortmodel, KbvCollectionModel *model)
{
  this->collModel = model;
  this->sortModel = sortmodel;

  this->sortModel->setSourceModel(collModel);
  this->setModel(sortModel);
  connect(collModel, SIGNAL(enableSorting(bool)), sortModel, SLOT(enableSorting(bool)));

}
/*************************************************************************//*!
 * Set the drag'n'drop handler for managing drops on the view.
 */
void    KbvCollectionView::setDragDropHandler(KbvCollectionDragDrop *dragDropHandler)
{
  this->dndHandler = dragDropHandler;
}
/*************************************************************************//*!
 * Toggle view mode between icon mode (default) and list mode.
 */
void    KbvCollectionView::changeViewMode(int mode)
{
  if(mode == QListView::IconMode)
    {
      this->setViewMode(QListView::IconMode);
    }
  else
    {
      this->setViewMode(QListView::ListMode);
      //this->setAlternatingRowColors(true); //not very nice
    }
  this->collModel->setViewMode(mode);
  this->updateOptions();
  //qDebug() << "kbvFileView::changeViewMode" << mode; //###########
}
/*************************************************************************//*!
 * SLOT: Read settings defined by options menu. Called by option menu and
 * after construction when the model is set to view.
 * Icon sizes are provided by the data base.
 */
void    KbvCollectionView::updateOptions()
{
  QSize grid;
  int   id;

  //qDebug() << "KbvCollectionView::updateOptions"; //###########
  id = 0;
  if (settings->showFileName)
    {
      id++;
    }
  if (settings->showFileSize)
    {
      id++;
    }
  if (settings->showImageSize)
    {
      id++;
    }
  if(viewMode() == QListView::IconMode)
    {
      grid = QSize(dbIconSize, dbIconSize);
      setGridSize(grid + QSize(10,30+id*10));
      setIconSize(grid);
    }
  else
    {
      grid = QSize(Kbv::IconList, Kbv::IconList);
      setGridSize(grid + QSize(476, 6)); //see: KbvGlobal::displayRoleData()
      setIconSize(grid);
    }
  //qDebug() << "KbvCollectionView::updateOptions gridsize" << this->gridSize(); //###########
}
/*************************************************************************//*!
 * SLOT: Set current branch when this is the database "title" and the used
 * branch is different to "branch".
 * Called by collection tree view when a branch is activated (in parallel
 * the model (readBranch) is set by same signal).
 * Branch is needed on drop on view for calling the DnD handler. All other
 * information is read from database info.
 */
void    KbvCollectionView::setActualBranch(QString title, QString branch)
{
  if(title == dbName)
   {
      if(this->branch != branch)
        {
          this->branch = branch;
          //qDebug() << "KbvCollectionView::setActualBranch" << branch; //###########
        }
   }
}
/*************************************************************************//*!
 * Returns the branch being shown of collection "title".
 */
QString KbvCollectionView::actualBranch(QString title)
{
  if(title == dbName)
   {
      return this->branch;
   }
  return QString("");
}
/*************************************************************************//*!
 * Drag&Drop from another application to item view.
 */
void    KbvCollectionView::dragEnterEvent(QDragEnterEvent* event)
{
  if(event->mimeData()->hasUrls() | event->mimeData()->hasText())
    {
      event->acceptProposedAction();
    }
}
/*************************************************************************//*!
 * Evaluates keyboard and determines copy or move due to modifier keys:
 * control key -> copy (default), shift key -> move.
 * Qt::ShiftModifier=0x02000000, Qt::ControlModifier=0x04000000
 * Qt::CopyAction=0x1, Qt::MoveAction=0x2, Qt::LinkAction=0x4
 */
void KbvCollectionView::dragMoveEvent(QDragMoveEvent *event)
{
  if(event->keyboardModifiers() & Qt::ShiftModifier)
    {
      event->setDropAction(Qt::MoveAction);
    }
  else
    {
      event->setDropAction(Qt::CopyAction);
    }
  event->accept();
  //qDebug() << "KbvCollectionView::dragMoveEvent action" << event->dropAction(); //###########
}
/*************************************************************************//*!
 * Drop from other application onto the visible album or collection view.\n
 * The dragged data are handled by the dndHandler. No drag&drop inside the view.
 * Qt::CopyAction=0x1, Qt::MoveAction=0x2, Qt::LinkAction=0x4
 */
void KbvCollectionView::dropEvent(QDropEvent *event)
{
  //no drag&drop inside the view
  //qDebug() << "KbvCollectionView::dropEvent source traget" <<event->source() <<this; //###########
  if (event->source() != this)
    {
      this->collModel->watchThread->stop();
      //On collections copy/move can be performed (move: shift modifier)
      //On albums copy only is permitted
      if(dbType & Kbv::TypeAlbum)
        {
          event->setDropAction(Qt::CopyAction);
        }
      event->acceptProposedAction();
      
      //qDebug() << "KbvCollectionView::dropEvent dbName branch model" << dbName << branch << collModel; //###########
      this->dndHandler->dropMimeData(event->mimeData(), event->dropAction(), branch, dbInfo, collModel);
      this->collModel->watchThread->start(branch);
    }
}

/*************************************************************************//*!
 * Drag from collection view to collection tree or dir tree or to external
 * application.\n
 * Dropped data are handled by the target. 
 * The drag is performed and when the drag object returns from internal drop
 * the model gets adjusted on drag move and the source files can be removed.
 * On drag to external targets nothing is done to the source files since drag
 * to an external app takes time and doesn't deliver an end signal.
 * This works as copy action.
 * For internal drags the mime type x-special/imarca-copied-files is added to the drag
 * object containing dbtype, dbname, dbLocation and primary keys of sender.\n
 */
void    KbvCollectionView::mouseMoveEvent(QMouseEvent *event)
{
  KbvGeneral        generalFunc;
  QList<QPersistentModelIndex>  perIndices;
  QModelIndexList   selIndices;
  QVector<qlonglong>  primKeyList(1);
  QString           text, name, path;
  QList<QUrl>       urllist;
  QUrl              url;
  QByteArray        ba;
  int               i;
  Qt::DropAction    dropact;
  qlonglong         pk;
  bool              ok;
  
  //Dragging is performed only when
  //- the mouse left button is pressed
  //- the mouse movement is far enough from start position
  //- the selection is not empty
  if(!(event->buttons() & Qt::LeftButton))
    {
      return;
    }
  if((event->pos() - dragStartPos).manhattanLength() < QApplication::startDragDistance())
    {
      return;
    }
  selIndices = this->selectedIndexes();
  if(selIndices.isEmpty() || (dbType & Kbv::TypeAlbum))
    {
      return;
    }

  emit infoText2(QString("%1").arg(selIndices.count()));
  emit infoText3(tr("Moving / copying files. Please wait."));

  //The selection is stored as list of urls in the mime data object.
  //In addition the primary keys are stored in the mime object since this
  //enables a fast move of datasets within or between Imarca dbs.
  //map selection to model using persistent model indices
  for (int i = 0; i < selIndices.count(); ++i)
    {
      perIndices.append(sortModel->mapToSource(selIndices[i]));
    }
  //construct lists of URLs and primary keys from selected files
  primKeyList.resize(selIndices.size());
  for(i=0; i<perIndices.size(); ++i)
    {
      path = this->collModel->data(perIndices[i], Kbv::FilePathRole).toString();
      name = this->collModel->data(perIndices[i], Kbv::FileNameRole).toString();
      pk = this->collModel->data(perIndices[i], Kbv::PrimaryKeyRole).toLongLong(&ok);

      urllist.append(url.fromLocalFile(path+name));
      primKeyList.replace(i, pk);
    }
  //qDebug() << "KbvCollectionView::mouseMoveEvent urls" <<urllist; //###########

  //Mime data (type x-special/imarca-copied-files) for internal drag/drop
  //Data as strings: empty string, dbType, dbName, dbLocation and primary keys
  text = "";
  text.append(QString("\n%1").arg(dbInfo->getType(), 0, 10));
  text.append(QString("\n") + dbInfo->getName());
  text.append(QString("\n") + dbInfo->getLocation());

  for(i=0; i<primKeyList.size(); ++i)
    {
      text.append(QString("\n%1").arg(primKeyList.at(i), 0, 10));
    }
  ba = text.toUtf8();

  QMimeData *mimeData = new QMimeData;
  mimeData->setUrls(urllist);
  mimeData->setData("x-special/imarca-copied-files", ba);

  //qDebug() << "KbvCollectionView::mouseMoveEvent drag start"; //###########
  QDrag *drag = new QDrag(this);

  drag->setMimeData(mimeData);
  drag->setPixmap(generalFunc.pixMimeImage);
  //The last of the selected items determines the mime icon (image or text).
  if(QPixmap(path+name).isNull())
    {
      drag->setPixmap(generalFunc.pixMimeText);
    }
  dropact = drag->exec(Qt::CopyAction|Qt::MoveAction);
  //qDebug() << "KbvCollectionView::mouseMoveEvent drop finished" <<dropact; //###########

  //Cleanup model
  //Drag action finishes when the target has handled all files.
  //This works on internal drag which delivers the used action as well as on external
  //drag (XDND protocol), which returns allways with Qt::IgnoreAction even when a moveAction
  //has been performed.
  //Only move action requires updating the model and the database since the sources files
  //have been removed. Ignore action goes side by side with an unknown time behaviour
  //of the target app. Cleanup will be done by the watch thread.
  if(dropact == Qt::MoveAction)
    {
      //qDebug() << "KbvCollectionView::mouseMoveEvent cleanup" <<perIndices; //###########
      this->collModel->dragMoveCleanUp(perIndices);
    }
  emit infoText2("");
  emit infoText3(branch);
}
/*************************************************************************//*!
 * Drag&Drop from item view:
 * Save mouse position as starting point for dragging. Delegate key event to
 * base class to perform further mouse events.
 */
void    KbvCollectionView::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      dragStartPos = event->pos();
    }
  QListView::mousePressEvent(event);
}
/*************************************************************************//*!
 * SLOT: Key press event
 * Ctrl A key pressed: select all items in view
 * Ctrl C key pressed: copy selected items to clipboard
 * Ctrl X key pressed: move selected items to clipboard
 * Ctrl V key pressed: paste clipboard to collection branch
 */
void    KbvCollectionView::keyPressEvent(QKeyEvent *event)
{
  QModelIndexList     selIndices;
  QString             str, msg1, msg2;
  int                 retval;
  QByteArray          ba;
  QStringList         mimeStrings, pathList;
  Qt::DropAction      dropAction = Qt::CopyAction;

  //qDebug() << "KbvCollectionView::keyPressEvent"<<event->key() <<weAreVisible; //###########
  //Del key pressed
  if(event->key() == Qt::Key_Delete)
    {
      msg1 = QString(tr("This removes the selected files from the data base."));
      if(dbType & Kbv::TypeAlbum)
        {
          msg2 = QString(tr("The files still remain in the file system.\n"
                            "Do you really want to remove?"));
        }
      if(dbType & Kbv::TypeCollection)
        {
          msg2 = QString(tr("The files are removed from file system also!\n"
                          "Do you really want to remove?"));
        }
      //first ask, then do it in model
      retval = informationDialog->perform(msg1, msg2, 3);
      if (retval == QMessageBox::Yes)
        {
          selIndices = this->selectedIndexes();
          for(int i=0; i<selIndices.count(); i++)
            {
              selIndices[i] = sortModel->mapToSource(selIndices[i]);
            }
          collModel->removeSelection(selIndices);
        }
    }

  //Ctrl A key pressed in mainWindow or dirView or collTreeView or here
  else if((event->key() == Qt::Key_A) && (event->modifiers() == Qt::ControlModifier) && weAreVisible)
    {
      this->selectAll();
      this->setFocus();
    }
  //Ctrl C key pressed
  else if((event->key() == Qt::Key_C) && (event->modifiers() == Qt::ControlModifier) && weAreVisible)
    {
      selIndices = this->selectedIndexes();
      for(int i=0; i<selIndices.count(); i++)
        {
          selIndices[i] = sortModel->mapToSource(selIndices[i]);
        }
      filesToClipboard(selIndices, false);
    }
  //Ctrl X key pressed
  //From albums only copy-action is permitted
  else if((event->key() == Qt::Key_X) && (event->modifiers() == Qt::ControlModifier) && weAreVisible)
    {
      selIndices = this->selectedIndexes();
      for(int i=0; i<selIndices.count(); i++)
        {
          selIndices[i] = sortModel->mapToSource(selIndices[i]);
        }
      if(dbType & Kbv::TypeAlbum)
        {
          filesToClipboard(selIndices, false);
        }
      else
        {
          filesToClipboard(selIndices, true);
        }
      //Don't remove the selection from model. The files are still present
      //hence the collectionWatcher would add them to the model again.
      //Cleanup must be done after pasting the data.
    }
  //Ctrl V key pressed
  else if((event->key() == Qt::Key_V) && (event->modifiers() == Qt::ControlModifier) && weAreVisible)
    {
      //x-special/imarca-copied-files contains:
      //from database: data as strings: action(copy/cut), dbtype, dbName, dbLocation and primary keys
      //from file system: data as strings: action(copy/cut), typeFile, source path

      this->collModel->watchThread->stop();
      emit  infoText3(QString(tr("Pasting files. Please wait.")));
      QMimeData const *mimeData = clipboard->mimeData();

      if(mimeData->hasFormat("x-special/imarca-copied-files"))
        {
          ba = mimeData->data("x-special/imarca-copied-files");
          str = QString::fromUtf8(ba.data());
          mimeStrings = str.split("\n", QString::SkipEmptyParts);

          //On collections copy/move can be performed (move: shift modifier)
          //On albums copy only is permitted
          if(dbType & Kbv::TypeAlbum)
            {
              dropAction = Qt::CopyAction;
            }
          else
            {
              dropAction = ("cut" == mimeStrings.at(0)) ? Qt::MoveAction : Qt::CopyAction;
            }
          //Paste files, update view and database and remove records in
          //source db when paste target branch is not the source branch
          pathList = globalFunc.dropExtractMimeData(mimeData);
          int n = pathList.at(0).lastIndexOf("/");
          str = pathList.at(0).left(n);             //absolute path
          //qDebug() << "kbvCollectionView::keyPressEvent ctrl-v imarca"<<mimeStrings.at(0) <<str <<this->dbInfo->getRootDir()+branch; //###########
          if(!(str.endsWith(this->dbInfo->getRootDir()+branch)))
            {
              //qDebug() << "kbvCollectionView::keyPressEvent ctrl-v imarca paste"; //###########
              this->dndHandler->dropMimeData(clipboard->mimeData(), dropAction, branch, dbInfo, collModel);
              //Remove cut files from database and clear clipboard
              if(dropAction == Qt::MoveAction)
                {
                  //qDebug() << "kbvCollectionView::keyPressEvent ctrl-v imarca cleanup"; //###########
                  this->ctrlVCleanUp(mimeStrings, pathList);
                  clipboard->clear();
                }
            }
        }
      else if(mimeData->hasFormat("x-special/gnome-copied-files"))
        {
          //copy/paste from gnome: action(cut/copy), file paths
          ba = mimeData->data("x-special/gnome-copied-files");
          str = QString::fromUtf8(ba.data());
          mimeStrings = str.split("\n", QString::SkipEmptyParts);

          //On collections copy/move can be performed (move: shift modifier)
          //On albums copy only is permitted
          if(dbType & Kbv::TypeAlbum)
            {
              dropAction = Qt::CopyAction;
            }
          else
            {
              dropAction = ("cut" == mimeStrings.at(0)) ? Qt::MoveAction : Qt::CopyAction;
            }

          //Paste files, update view and database when paste target is not the same branch
          //qDebug() << "kbvCollectionTreeView::keyPressEvent ctrl-v gnome" <<mimeStrings.at(0) <<mimeStrings.at(1); //###########
          int n = mimeStrings.at(1).lastIndexOf("/");
          str = mimeStrings.at(1).left(n);
          str.remove("file://");
          if(!(str == branch))
            {
              //qDebug() << "kbvCollectionView::keyPressEvent ctrl-v gnome paste"; //###########
              this->dndHandler->dropMimeData(clipboard->mimeData(), dropAction, branch, dbInfo, collModel);

              //Clear clipboard
              if(dropAction == Qt::MoveAction)
                {
                  //qDebug() << "kbvCollectionView::keyPressEvent ctrl-v gnome cleanup"; //###########
                  clipboard->clear();
                }
            }
        }
      emit  infoText3(branch);
      this->collModel->watchThread->start(branch);
    }

  //Delegate key event to base class only when not consumed here and view is visible
  else
    {
      if(weAreVisible)
        {
          QListView::keyPressEvent(event);
        }
    }
}
/*************************************************************************//*!
 * Copy or move selected files to clipboard. In both cases the selected files
 * remain in the source directory until they are pasted to the new target.
 * On ctrl_v the files get copied and deleted when previous action was ctrl_x.
 */
void  KbvCollectionView::filesToClipboard(const QModelIndexList &indexList, bool move)
{
  QString           text, name, path, gnomepaths;
  QVector<qlonglong>  primKeyList(1);
  QList<QUrl>       urllist;
  QUrl              url;
  QByteArray        ba;
  qlonglong         pk;
  bool              ok;


  if(!indexList.isEmpty())
    {
      //construct URLs from files
      primKeyList.resize(indexList.count());
      for(int i=0; i<indexList.count(); ++i)
        {
          path = this->collModel->data(indexList[i], Kbv::FilePathRole).toString();
          name = this->collModel->data(indexList[i], Kbv::FileNameRole).toString();
          pk   = this->collModel->data(indexList[i], Kbv::PrimaryKeyRole).toLongLong(&ok);

          primKeyList.replace(i, pk);
          urllist.append(url.fromLocalFile(path+name));
          gnomepaths.append("\nfile://" + path+name);
        }

      QMimeData *mimeData = new QMimeData;
      mimeData->setUrls(urllist);

      //Mime data (type x-special/imarca-copied-files) for internal copy/paste
      //Data as strings: action(copy/cut), dbtype, dbName and primary keys
      text = move ? "cut" : "copy";
      text.append(QString("\n%1").arg(dbInfo->getType(), 0, 10));
      text.append(QString("\n") + dbInfo->getName());
      text.append(QString("\n") + dbInfo->getLocation());

      for(int i=0; i<primKeyList.size(); ++i)
        {
          text.append(QString("\n%1").arg(primKeyList.at(i), 0, 10));
        }
      ba = text.toUtf8();
      mimeData->setData("x-special/imarca-copied-files", ba);
      //qDebug() << "kbvCollectionView::filesToClipboard imarca"<<text; //###########

      //Mime data (x-special/gnome-copied-files) for copy/paste to Gnome
      //Data as strings: action(copy/cut), file paths
      text = move ? "cut" : "copy";
      text.append(gnomepaths);
      ba = text.toUtf8();
      mimeData->setData("x-special/gnome-copied-files", ba);
      //qDebug() << "kbvCollectionView::filesToClipboard gnome"<<text; //###########

      clipboard->clear();
      clipboard->setMimeData(mimeData);
    }
}
/*************************************************************************//*!
* Paste has been finished in ctrl-v key event. This function removes the data
* sets in source database related to the removed source files.
* The source data sets may be members of an album or collection.
* The mimeStrings contains: action(cut), dbtype, dbName and primary keys
* The pathList contains all filepaths of the paste operation in the order of
* the primary keys in mimeList.
*/
void  KbvCollectionView::ctrlVCleanUp(const QStringList &mimeStrings, const QStringList &pathList)
{
  QSqlDatabase  db;
  QSqlQuery     query;
  QString       dbname, dbpath, stmt, str;

  //connection to source database
  dbname = mimeStrings.at(2);
  dbpath = mimeStrings.at(3);

  db = QSqlDatabase::addDatabase("QSQLITE", "cut"+dbname);
  db.setHostName("cutpaste");
  db.setDatabaseName(dbpath+dbname+QString(dbNameExt));
  //qDebug() << "kbvCollectionView::ctrlVCleanUp" <<db; //###########

  str = QString("DELETE FROM album WHERE pk = %1");
  if(db.open())
    {
      query = QSqlQuery(db);
      query.setForwardOnly(true);

      //Remove all datasets where the related files don't exist anymore
      db.transaction();
      for(int i=0; i<pathList.length(); i++)
        {
          if(!QFile::exists(pathList.at(i)))
            {
              stmt = str.arg(mimeStrings.at(i+3));
              query.exec(stmt);
              //qDebug() << "kbvCollectionView::ctrlVCleanUp" <<pathList.at(i); //###########
              //qDebug() << "kbvCollectionView::ctrlVCleanUp" <<mimeStrings.at(i+3); //###########
              query.clear();
            }
        }
      db.commit();

      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase("cut"+dbname); //close connection
    }
}

/*************************************************************************//*!
 * Return a list of model indicees of selected items. The indicees are not
 * mapped to the data model.
 */
QModelIndexList&  KbvCollectionView::getSelectedItems()
{
  selectedItems = this->selectionModel()->selectedIndexes();
  return  selectedItems;
}
/*************************************************************************//*!
 * Return the data for given index and role.
 */
QVariant    KbvCollectionView::getItemData(const QModelIndex &index, int role)
{
  QModelIndex idx = this->sortModel->mapToSource(index);
  return  this->collModel->data(idx, role);
}
/*************************************************************************//*!
 * SLOT: Slide show when main menu View|Slide show is activated or F8 pressed
 * or return key pressed or double click.
 * If nothing is selected, all items of the current directory are displayed.
 * If a sole item is selected the slide show starts from this item.
 */
void  KbvCollectionView::displaySlideShow(QModelIndex idx)
{
  //return key pressed or double click
  Q_UNUSED(idx)
  this->displaySlideShow();
}
void  KbvCollectionView::displaySlideShow()
{
  //main menu View|Slide show activated or F8 pressed
  //only if this tab is visible
  if(weAreVisible)
    {
      globalFunc.displayImages(this, collModel, sortModel);
      //globalFunc.displaySlideShow(this, collModel, sortModel);
    }
}
/*************************************************************************//*!
 * Slot: called by stacked widget when the related tab becomes visible.
 */
void  KbvCollectionView::tabIsVisible(bool visible)
{
  weAreVisible = visible;
  if(visible)
    {
      this->setFocus();
    }
}
/*************************************************************************//*!
 * SLOT: Single or multiple rename via menu in main window.
 */
void  KbvCollectionView::renameFromMenu()
{
  QModelIndexList selIndices;
  QString         name1, name2;

  //only if this tab is visible
  if(weAreVisible)
    {
      selIndices = this->selectedIndexes();
      for(int i=0; i<selIndices.count();i++)
        {
          selIndices[i] = sortModel->mapToSource(selIndices[i]);
        }

      emit infoText2(QString("%1").arg(selIndices.count()));
      emit infoText3(tr("Renaming. Please wait."));

      if (selIndices.count() > 1)
        {
          //Multiple rename. Send the names of the first two selected files to
          //dialog for name preview.
          name1 = this->collModel->data(selIndices[0], Kbv::FileNameRole).toString();
          name2 = this->collModel->data(selIndices[1], Kbv::FileNameRole).toString();
          if (renameDialog->perform(name1, name2, true) == QDialog::Accepted)
            {
              this->collModel->multipleRename(selIndices,
                      renameDialog->prefix, renameDialog->suffix, renameDialog->extension,
                      renameDialog->startValue, renameDialog->stepValue,
                      renameDialog->numerals, renameDialog->combination);
            }
        }
      if (selIndices.count() == 1)
        {
          //Single rename. Send the name of the selected file to dialog.
          name1 = this->collModel->data(selIndices[0], Kbv::FileNameRole).toString();
          name2 = "";
          if (renameDialog->perform(name1, name2, false) == QDialog::Accepted)
            {
              this->collModel->singleRename(selIndices,
                    renameDialog->prefix, renameDialog->suffix, renameDialog->extension,
                    renameDialog->combination);
            }
        }
      this->clearSelection();
      emit infoText2("");
      emit infoText3(branch);
    }
}
/****************************************************************************/
