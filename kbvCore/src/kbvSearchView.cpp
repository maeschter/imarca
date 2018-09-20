/*****************************************************************************
 * kbvSearchView
 * This is the standard view for search results on collections.
 * This view is supporting drag only.
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2011.10.04
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include <QtGui>
#include "kbvSearchView.h"
#include "kbvConstants.h"
#include "kbvPluginInterfaces.h"
//#include "kbvSlideView.h"
//#include "kbvSlideScene.h"
#include "kbvRenameDialog.h"
#include "kbvInformationDialog.h"

extern  KbvRenameDialog         *renameDialog;
extern  KbvInformationDialog    *informationDialog;
//extern  KbvImageEditorInterface *imageEditPlugin;

KbvSearchView::KbvSearchView(QWidget *parent) : QListView(parent)
{
  this->setWrapping(true);
  this->setResizeMode(QListView::Adjust);
  this->setViewMode(QListView::IconMode);   //layout from left to right
  this->setFrameShadow (QFrame::Raised);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setTabKeyNavigation(true);

  this->setDragDropMode(QAbstractItemView::DragDrop);
  this->setDragEnabled(true);
  this->setAcceptDrops(false);
  this->setDropIndicatorShown(false);

  //default
  clipboard = QApplication::clipboard();
  weAreVisible = false;
  mode = QListView::IconMode;

  //Double click, Enter or Return on a selection of icons
  connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(displaySlideShow(QModelIndex)));
}

KbvSearchView::~KbvSearchView()
{
  //qDebug() << "KbvSearchView::~KbvSearchView"; //###########
  delete    this->searchModel;
  delete    this->sortModel;
}
/*************************************************************************//*!
 * Set pointers to both models.
 */
void    KbvSearchView::setModels(KbvSortFilterModel *sortmodel, KbvSearchModel *model)
{
  this->searchModel = model;
  this->sortModel = sortmodel;

  this->sortModel->setSourceModel(searchModel);
  this->setModel(sortModel);
}
/*************************************************************************//*!
 * Set a pointer to the database info object (database settings).
 */
void    KbvSearchView::setDatabaseInfo(KbvDBInfo *dbInfo)
{
  dbName = dbInfo->getName();
  dbType = dbInfo->getType();
  dbIconSize = dbInfo->getIconSize();
  dbLocation = dbInfo->getLocation();
  //qDebug() << "KbvSearchView::setDatabaseInfo" <<dbName <<dbType <<dbIconSize; //###########
}
/*************************************************************************//*!
 * Set visibility flag when the tab becomes visible.
 */
void    KbvSearchView::setVisibleFlag(bool visible)
{
  this->weAreVisible = visible;
}
/*************************************************************************//*!
 * Update parameters due to the data base settings. This slot is called when
 * the database has been opened or when the view changes from icon to list
 * mode and vice versa. The settings are required for displaying the
 * database and record info.
 */
void    KbvSearchView::updateGridSize()
{
  QSize grid;
  int   id;

  id = 3;

  if(viewMode() == QListView::IconMode)
    {
      grid = QSize(dbIconSize, dbIconSize);
      setGridSize(grid + QSize(10, 30+id*10));
      setIconSize(grid);
    }
  else
    {
      grid = QSize(Kbv::IconList, Kbv::IconList);
      setGridSize(grid + QSize(476, 6)); //see: KbvGlobal::displayRoleData()
      setIconSize(grid);
    }
  //qDebug() << "KbvSearchView::updateGridSize()" << grid; //###########
}
/*************************************************************************//*!
 * Toggle view mode between icon mode (default) and list mode.
 */
void KbvSearchView::changeViewMode(int mode)
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
  this->updateGridSize();
  this->searchModel->setViewMode(mode);
  //qDebug() << "KbvSearchView::changeViewMode" << mode; //###########
}
/*************************************************************************//*!
 * Drag from search view to collection tree or dir tree or to external
 * application.\n
 * Dropped data are handled by the target. 
 * The drag is performed and when the drag object returns from internal drop
 * the model gets adjusted on drag move and the source files can be removed.
 * For internal drags the mime type x-special/imarca-copied-files is added to the drag
 * object containing dbtype and dbname of sender.
 */
void    KbvSearchView::mouseMoveEvent(QMouseEvent *event)
{
  QList<QPersistentModelIndex>  perIndices;
  QModelIndexList   selIndices;
  QVector<qlonglong>  primKeyList(1);
  QString           text, name, path;
  QList<QUrl>       urllist;
  QUrl              url;
  QByteArray        ba;
  qlonglong         pk;
  int               i, dropact=Qt::IgnoreAction;
  bool              ok;

  //Dragging is performed only when
  //- the mouse left button is pressed
  //- the mouse movement is far enough from start position
  //- the selection is not empty
  if (!(event->buttons() & Qt::LeftButton))
    {
      return;
    }
  if ((event->pos() - dragStartPos).manhattanLength() < QApplication::startDragDistance())
    {
      return;
    }
  selIndices = this->selectedIndexes();
  if(selIndices.isEmpty())
    {
      return;
    }
  //qDebug() << "KbvSearchView::mouseMoveEvent drag start"; //###########

  emit infoText2(QString("%1").arg(selIndices.count(), 6));
  emit infoText3(tr("Moving / copying. Please wait."));

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
      path = this->searchModel->data(perIndices[i], Kbv::FilePathRole).toString();
      name = this->searchModel->data(perIndices[i], Kbv::FileNameRole).toString();
      pk   = this->searchModel->data(perIndices[i], Kbv::PrimaryKeyRole).toLongLong(&ok);

      urllist.append(url.fromLocalFile(path+name));
      primKeyList.replace(i, pk);
      //qDebug() << "KbvSearchView::mouseMoveEvent urls" <<urllist; //###########
    }

  //Mime data (type x-special/imarca-copied-files) for internal drag/drop
  //Data as strings: empty string, dbType, dbName, dbLocation and primary keys
  text = "";
  text.append(QString("\n%1").arg(dbType, 0, 10));
  text.append(QString("\n") + dbName);
  text.append(QString("\n") + dbLocation);
  for(i=0; i<primKeyList.size(); ++i)
    {
      text.append(QString("\n%1").arg(primKeyList.at(i), 0, 10));
    }
  ba = text.toUtf8();

  QMimeData *mimeData = new QMimeData;
  mimeData->setUrls(urllist);
  mimeData->setData("x-special/imarca-copied-files", ba);

  QDrag *drag = new QDrag(this);
  
  drag->setMimeData(mimeData);
  drag->setPixmap(generalFunc.pixMimeImage);
  //The last of the selected items determines the mime icon (image or text).
  if(QPixmap(path+name).isNull())
    {
      drag->setPixmap(generalFunc.pixMimeText);
    }
  dropact = drag->exec(Qt::CopyAction|Qt::MoveAction);

  //Cleanup model
  //Drag action finishes when the target has handled all files.
  //This works on internal drag which delivers the used action as well as on external
  //drag (XDND protocol), which returns allways with Qt::IgnoreAction even when a moveAction
  //has been performed.
  //Only move action requires updating the searchModel and searchDatabase since the
  //source files already have been removed.
  if(dropact == Qt::MoveAction)
    {
      this->searchModel->dragMoveCleanUp(perIndices);
    }
  emit infoText2("");
  emit infoText3("");
  //qDebug() << "KbvSearchView::mouseMoveEvent drag finished" <<dropact; //###########
}
/*************************************************************************//*!
 * Drag from search view:
 * Save mouse position as starting point for dragging. Delegate key event to
 * base class to perform further mouse events.
 */
void    KbvSearchView::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      dragStartPos = event->pos();
    }
  QListView::mousePressEvent(event);
}
/*************************************************************************//*!
 * SLOT: Key press event
 * Delete key pressed: delete items in model, move them to waste bin
 * Ctrl A key pressed: select all items in view
 * Ctrl C key pressed: copy selected items to clipboard
 * Ctrl X key pressed: move selected items to clipboard
 * Note: on ctrl-x the files are removed from source model
 * but remain in file system and database until ctrl-v
 */
void    KbvSearchView::keyPressEvent(QKeyEvent *event)
{
  QModelIndexList   selIndices;
  QString   msg1, msg2;
  int       retval;

  //qDebug() << "KbvSearchView::keyPressEvent"<<event->key() <<weAreVisible; //###########
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
      informationDialog->setCaptionAndIcon(msg1, msg2, 3);
      retval = informationDialog->exec();
      if (retval == QMessageBox::Yes)
        {
          selIndices = this->selectedIndexes();
          for(int i=0; i<selIndices.count(); i++)
            {
              selIndices[i] = sortModel->mapToSource(selIndices[i]);
            }
          this->searchModel->removeSelection(selIndices);
        }
    }

  //Ctrl A key pressed
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
      //Cleanup the search model since the ctrl-v action doesn't have access.
      //and there is no searchWatcher to do this after removing the files.
      //Cleaning the source database and model is done on ctrl-v.
      this->searchModel->ctrlXCleanUp(selIndices);
    }

  //Delegate key event to base class when not used and view is visible
  else
    {
      if(weAreVisible)
        {
          QListView::keyPressEvent (event);
        }
    }
}
/*************************************************************************//*!
 * Copy or move selected files to clipboard. In both cases the selected files
 * remain in the source directory until they are pasted to the new target.
 */
void  KbvSearchView::filesToClipboard(const QModelIndexList &indexList, bool move)
{
  QString             text, name, path, gnomepaths;
  QVector<qlonglong>  primKeyList(1);
  QList<QUrl>         urllist;
  QUrl                url;
  QByteArray          ba;
  qlonglong           pk;
  bool                ok=false;


  if(!indexList.isEmpty())
    {
      //construct URLs from files
      primKeyList.resize(indexList.count());
      for(int i=0; i<indexList.count(); ++i)
        {
          path = this->searchModel->data(indexList[i], Kbv::FilePathRole).toString();
          name = this->searchModel->data(indexList[i], Kbv::FileNameRole).toString();
          pk   = this->searchModel->data(indexList[i], Kbv::PrimaryKeyRole).toLongLong(&ok);

          primKeyList.replace(i, pk);
          urllist.append(url.fromLocalFile(path+name));
          gnomepaths.append("\nfile://" + path+name);
        }

      QMimeData *mimeData = new QMimeData;
      mimeData->setUrls(urllist);

      //Mime data (type x-special/imarca-copied-files) for internal copy/paste
      //Data as strings: action(copy/cut), dbtype, dbName, dbLocation and primary keys
      text = move ? "cut" : "copy";
      text.append(QString("\n%1").arg(dbType, 0, 10));
      text.append(QString("\n") + dbName);
      text.append(QString("\n") + dbLocation);
      for(int i=0; i<primKeyList.size(); ++i)
        {
          text.append(QString("\n%1").arg(primKeyList.at(i), 0, 10));
        }
      ba = text.toUtf8();
      mimeData->setData("x-special/imarca-copied-files", ba);
      //qDebug() << "kbvSearchView::filesToClipboard x-special/imarca-copied-files"<<text; //###########

      //Mime data (x-special/gnome-copied-files) for copy/paste to Gnome
      //action(copy/cut), file paths
      text = move ? "cut" : "copy";
      text.append(gnomepaths);
      ba = text.toUtf8();
      mimeData->setData("x-special/gnome-copied-files", ba);
      //qDebug() << "kbvSearchView::filesToClipboard x-special/gnome-copied-files"<<text; //###########

      clipboard->clear();
      clipboard->setMimeData(mimeData);
    }
}

/*************************************************************************//*!
 * Return a list of model indicees of selected items. The indicees are not
 * mapped to the data model.
 */
QModelIndexList&  KbvSearchView::getSelectedItems()
{
  selectedItems = this->selectionModel()->selectedIndexes();
  return  selectedItems;
}
/*************************************************************************//*!
 * Return the data for given index and role.
 */
QVariant    KbvSearchView::getItemData(const QModelIndex &index, int role)
{
  QModelIndex idx = this->sortModel->mapToSource(index);
  return  this->searchModel->data(idx, role);
}
/*************************************************************************//*!
 * SLOT: Slide show when main menu View|Slide show is activated or F8 pressed
 * or return key pressed or double click.
 * If nothing is selected, all items of the current directory are displayed.
 * If a sole item is selected the slide show starts from this item.
 */
void  KbvSearchView::displaySlideShow(QModelIndex idx)
{
  //return key pressed or double click
  Q_UNUSED(idx)
  this->displaySlideShow();
}
void  KbvSearchView::displaySlideShow()
{
  //main menu View|Slide show activated or F8 pressed
  //only if this tab is visible
  if(weAreVisible)
    {
      globalFunc.displayImages(this, searchModel, sortModel);
      //globalFunc.displaySlideShow(this, searchModel, sortModel);
    }
}
/*************************************************************************//*!
 * SLOT: Single or multiple rename via menu in main window.
 */
void    KbvSearchView::renameFromMenu()
{
  QModelIndexList selIndices;
  QString         name1, name2;

  //only if this tab is visible
  if(weAreVisible)
    {
      selIndices = selectionModel()->selectedIndexes();
      for(int i=0; i<selIndices.count(); i++)
        {
          selIndices[i] = sortModel->mapToSource(selIndices[i]);
        }

      emit infoText2(QString("%1").arg(selIndices.count()));
      emit infoText3(tr("Renaming. Please wait."));

      if (selIndices.count() > 1)
        {
          //Multiple rename. Send the names if the first two selected files
          //to dialog for name preview.
          name1 = this->searchModel->data(selIndices[0], Kbv::FileNameRole).toString();
          name2 = this->searchModel->data(selIndices[1], Kbv::FileNameRole).toString();
          if (renameDialog->perform(name1, name2, true) == QDialog::Accepted)
            {
              this->searchModel->multipleRename(selIndices,
                      renameDialog->prefix, renameDialog->suffix, renameDialog->extension,
                      renameDialog->startValue, renameDialog->stepValue,
                      renameDialog->numerals, renameDialog->combination);
            }
        }
      if (selIndices.count() == 1)
        {
          //Single rename. Send the name of the selected file to dialog.
          name1 = this->searchModel->data(selIndices[0], Kbv::FileNameRole).toString();
          name2 = "";
          if (renameDialog->perform(name1, name2, false) == QDialog::Accepted)
            {
              this->searchModel->singleRename(selIndices,
                    renameDialog->prefix, renameDialog->suffix, renameDialog->extension,
                    renameDialog->combination);
            }
        }
      this->clearSelection();

      emit infoText2("");
      emit infoText3("");
    }
}
/****************************************************************************/
