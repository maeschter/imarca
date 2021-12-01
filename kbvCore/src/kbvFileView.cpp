/*****************************************************************************
 * kvb file view displays content of the selected directory as icons.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.03.02
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvFileView.h"
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvPluginInterfaces.h"
//#include "kbvSlideView.h"
//#include "kbvSlideScene.h"
#include "kbvRenameDialog.h"

extern  KbvSetvalues            *settings;
extern  KbvRenameDialog         *renameDialog;
extern  KbvImageEditorInterface *imageEditPlugin;

KbvFileView::KbvFileView(QWidget *parent) : QListView(parent)
{
  this->setWrapping(true);
  this->setResizeMode(QListView::Adjust);
  this->setViewMode(QListView::IconMode); //layout from left to right
  this->setFrameShadow (QFrame::Raised);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setSelectionBehavior(QAbstractItemView::SelectItems);
  this->setTabKeyNavigation(true);

  this->setDragDropMode(QAbstractItemView::DragDrop);
  this->setDragEnabled(true);
  this->setAcceptDrops(true);
  this->setDropIndicatorShown(true);

  //default
  clipboard = QApplication::clipboard();
  weAreVisible = false;
  fileModel = 0;
  sortModel = 0;
}

KbvFileView::~KbvFileView()
{
  //qDebug() << "KbvFileView::~KbvFileView";//###########
}
/*************************************************************************//*!
 * Set the model by calling the base class and establish all signals between
 * view and model.
 */
void KbvFileView::setModels(KbvSortFilterModel *sortmodel, KbvFileModel *model)
{
  this->fileModel = model;
  this->sortModel = sortmodel;

  this->sortModel->setSourceModel(fileModel);
  this->setModel(sortModel);

  //Double click, Enter or Return on a selection of icons
  connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(displaySlideShow(QModelIndex)));
  //manage view on directory changes
  connect(fileModel, SIGNAL(scrollViewToTop ()), this, SLOT(scrollToTop()));
  connect(fileModel, SIGNAL(scrollViewToTop ()), this, SLOT(clearSelection()));

  updateOptions();
}

/*************************************************************************//*!
 * Drag&Drop from another application to file view.
 */
void KbvFileView::dragEnterEvent(QDragEnterEvent* event)
{
  if(event->mimeData()->hasUrls() | event->mimeData()->hasText())
    {
      event->acceptProposedAction();
    }
}
/*************************************************************************//*!
 * Evaluates keyboard and determines copy or move due to modifier keys:
 * drag from fileView (internal): control key -> copy.
 * drag from external application: shift key -> move.
 */
void KbvFileView::dragMoveEvent(QDragMoveEvent *event)
{
  //qDebug() << "KbvFileView::dragMoveEvent external=0"<<event->source(); //###########
  if(event->source() != NULL)
    {
      //internal drag
      if(event->keyboardModifiers() & Qt::ControlModifier)
        {
          event->setDropAction(Qt::CopyAction);
        }
      else
        {
          event->setDropAction(Qt::MoveAction);
        }
    }
  else
    {
      if(event->keyboardModifiers() & Qt::ShiftModifier)
        {
          event->setDropAction(Qt::MoveAction);
        }
      else
        {
          event->setDropAction(Qt::CopyAction);
        }
    }
}

/*************************************************************************//*!
 * Drop from any view or other application onto view.\n
 * Dragged data are handled by the model. No drag&drop inside fileView.
 */
void KbvFileView::dropEvent(QDropEvent *event)
{
  //No drag&drop inside fileView
  if(event->source() != this)
    {
      event->acceptProposedAction();
      this->fileModel->dropData(event->mimeData(), event->dropAction(), QString(), true, false);
    }
}
/*************************************************************************//*!
 * Drag from file view to dir view, collection tree view or to external app.
 * Dropped data are handled by the target: copy or move files to destination. 
 * The drag is performed and when the drag object returns from drag move
 * action, the model gets adjusted by dragMoveCleanUp() if required.
 * For internal drags the mime type x-special/imarca-copied-files is added to
 * the drag object containing type=KbvFile and path of sender.
 * The file watcher thread will be stopped before performing the drag, to
 * prevent crashes. When the drag has finished, the filewatcher thread gets
 * started.
 */
void    KbvFileView::mouseMoveEvent(QMouseEvent *event)
{
  QList<QPersistentModelIndex>  perIndices;
  QModelIndexList   selIndices;
  QString           text, name, path;
  QList<QUrl>       urllist;
  QUrl              url;
  QByteArray        ba;
  int               dropact;

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
  emit infoText2(QString("%1").arg(selIndices.count(), 6));
  emit infoText3(tr("Moving / copying. Please wait."));
  this->fileModel->enableDirMonitor(false);
  
  //qDebug() << "KbvFileView::mouseMoveEvent start drag"; //###########
  //map selection to model using persistent model indices
  for (int i = 0; i < selIndices.count(); ++i)
    {
      perIndices.append(sortModel->mapToSource(selIndices[i]));
    }
  //construct URLs from selected files
  for (int i = 0; i < perIndices.count(); ++i)
    {
      path = this->fileModel->data(perIndices[i], Kbv::FilePathRole).toString();
      name = this->fileModel->data(perIndices[i], Kbv::FileNameRole).toString();
      urllist.append(url.fromLocalFile(path+name));
    }

  //qDebug() << "KbvFileView::mouseMoveEvent urllist"<<urllist.count(); //###########

  //Mime data (type x-special/imarca-copied-files) for internal drag/drop
  //Data as strings: emptystring, type = TypeFile, source path,
  text = "";
  text.append(QString("\n%1").arg(Kbv::TypeFile, 0, 10));
  text.append(QString("\n") + path);
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
  dropact = drag->exec(Qt::MoveAction|Qt::CopyAction);
  //qDebug() << "KbvFileView::mouseMoveEvent drag finished"; //###########

  //Cleanup model
  //Drag move action finishes when the target has handled all files. This
  //works on internal drag as well as on external drag (XDND protocol).
  if(dropact == Qt::MoveAction)
    {
      this->fileModel->dragMoveCleanUp(perIndices);
    }

  this->fileModel->enableDirMonitor(true);
  emit infoText2("");
  emit infoText3("");
}
/*************************************************************************//*!
 * Drag from file view to other view or external app:
 * Save mouse position as starting point for dragging. Delegate key event to
 * base class to perform further mouse events.
 */
void    KbvFileView::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
    {
      dragStartPos = event->pos();
    }
  //qDebug() << "KbvFileView::mousePressEvent row"<< this->indexAt(event->pos()).row(); //###########
  
  QListView::mousePressEvent(event);
}
/*************************************************************************//*!
 * SLOT: Key press event
 * Delete key pressed: delete items in file model, move them to waste bin
 * Ctrl A key pressed: select all items in view
 * Ctrl C key pressed: copy selected items to clipboard
 * Ctrl X key pressed: move selected items to clipboard
 * Ctrl V key pressed: paste clipboard to dir
 */
void    KbvFileView::keyPressEvent(QKeyEvent *event)
{
  QModelIndexList   selIndices;
  QString           str;
  QByteArray        ba;
  QStringList       mimeStrings;
  Qt::DropAction    dropAction = Qt::CopyAction;

  //qDebug() << "kbvFileView::keyPressEvent" <<event->key() <<weAreVisible; //###########
  if(event->key() == Qt::Key_Delete)
    {
      selIndices = this->selectedIndexes();
      for(int i=0; i<selIndices.count(); i++)
        {
          selIndices[i] = sortModel->mapToSource(selIndices[i]);
        }
      this->fileModel->removeSelection(selIndices);
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
      this->filesToClipboard(selIndices, false);
      //This needs no cleanUp since since nothing has been changed on model or view
    }

  //Ctrl X key pressed
  else if((event->key() == Qt::Key_X) && (event->modifiers() == Qt::ControlModifier) && weAreVisible)
    {
      selIndices = this->selectedIndexes();
      for(int i=0; i<selIndices.count(); i++)
        {
          selIndices[i] = sortModel->mapToSource(selIndices[i]);
        }
      this->filesToClipboard(selIndices, true);
      //Don't remove the selection from model. The files are still present
      //hence the fileWatcher would add them to the model again.
      //Cleanup must be done after pasting the data.
    }

  //Ctrl V key pressed. copy/paste from file system, fileView or database
  else if((event->key() == Qt::Key_V) && (event->modifiers() == Qt::ControlModifier) && weAreVisible)
    {
      //x-special/imarca-copied-files contains:
      //copy/paste from database: data as strings: action(copy/cut), type(album/collection), dbName and primary keys
      //copy/paste from file view: data as strings: action(copy/cut), type(file), source path
      emit  infoText3(QString(tr("Pasting files. Please wait.")));
      QCoreApplication::processEvents(QEventLoop::AllEvents);
      QMimeData const *mimeData = clipboard->mimeData();

      //get source path to prevent action within same directory
      if(mimeData->hasFormat("x-special/imarca-copied-files"))
        {
          ba = mimeData->data("x-special/imarca-copied-files");
          str = QString::fromUtf8(ba.data());
          mimeStrings = str.split("\n", QString::SkipEmptyParts);

          dropAction = ("cut" == mimeStrings.at(0)) ? Qt::MoveAction : Qt::CopyAction;
          //qDebug() << "kbvFileView::keyPressEvent ctrl-v imarca"<<mimeStrings.at(0) <<mimeStrings.at(2); //###########
          this->fileModel->dropData(clipboard->mimeData(), dropAction, QString(), true, true);
          if(dropAction == Qt::MoveAction)
            {
              //No cleanup, since dropMimeData performed a file move operation.
              //Hence source files have been removed from source and the source
              //view is responsible for itself.
              //cut files can't be pasted twice
              clipboard->clear();
            }
        }
      else if(mimeData->hasFormat("x-special/gnome-copied-files"))
        {
          //copy/paste from gnome: data as strings: action(cut)
          ba = mimeData->data("x-special/gnome-copied-files");
          str = QString::fromUtf8(ba.data());
          mimeStrings = str.split("\n", QString::SkipEmptyParts);
          //qDebug() << "kbvFileView::keyPressEvent ctrl-v gnome" <<mimeStrings; //###########

          dropAction = ("cut" == mimeStrings.at(0)) ? Qt::MoveAction : Qt::CopyAction;
          this->fileModel->dropData(clipboard->mimeData(), dropAction, QString(), true, true);
          if(dropAction == Qt::MoveAction)
            {
              //No cleanup, since dropMimeData performed a file move operation.
              //Hence source files have been removed from source and the source
              //view is responsible for itself.
              //cut files can't be pasted twice
              clipboard->clear();
            }
        }
      else
        {
          //external, copy/paste action
          if(mimeData->hasText() || mimeData->hasUrls())
            {
              this->fileModel->dropData(clipboard->mimeData(), Qt::CopyAction, QString(), true, true);
            }
        }
      emit  infoText3(QString(""));
    }

  //Delegate key event to base class only when not used and view is visible
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
void KbvFileView::filesToClipboard(const QModelIndexList &indexList, bool move)
{
  QString           text, name, path, gnomepaths;
  QList<QUrl>       urllist;
  QUrl              url;
  QByteArray        ba;

  if(!indexList.isEmpty())
    {
      this->fileModel->enableDirMonitor(false);

      //construct URLs from files
      for (int i = 0; i < indexList.count(); ++i)
        {
          path = this->fileModel->data(indexList[i], Kbv::FilePathRole).toString();
          name = this->fileModel->data(indexList[i], Kbv::FileNameRole).toString();

          gnomepaths.append("\nfile://" + path+name);
          urllist.append(url.fromLocalFile(path+name));
        }

      QMimeData *mimeData = new QMimeData;
      mimeData->setUrls(urllist);

      //Mime data (type x-special/imarca-copied-files) for internal copy/paste
      //Data as strings: action(copy/cut), type = TypeFile, source path,
      text = move ? "cut" : "copy";
      text.append(QString("\n%1").arg(Kbv::TypeFile, 0, 10));
      text.append(QString("\n") + path);
      ba = text.toUtf8();
      mimeData->setData("x-special/imarca-copied-files", ba);
      //qDebug() << "kbvFileView::filesToClipboard x-special/imarca-copied-files"<<text; //###########

      //Mime data (type x-special/gnome-copied-files) for copy/paste to Gnome
      //Data as strings: action(copy/cut), file paths
      text = move ? "cut" : "copy";
      text.append(gnomepaths);
      ba = text.toUtf8();
      mimeData->setData("x-special/gnome-copied-files", ba);
      //qDebug() << "kbvFileView::filesToClipboard x-special/gnome-copied-files"<<text; //###########

      clipboard->clear();
      clipboard->setMimeData(mimeData);

      this->fileModel->enableDirMonitor(true);
    }
}

/*************************************************************************//*!
 * Toggle view mode between icon mode (default) and list mode.
 */
void KbvFileView::changeViewMode(int mode)
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
  this->fileModel->setViewMode(mode);
  this->updateOptions();
  //qDebug() << "kbvFileView::changeViewMode" << mode; //###########
}
/*************************************************************************//*!
 * SLOT: Read icon sizes from settings and adjust the view. When the model
 * is already set (suppress during construction) then the update function is
 * called and the model refreshes the content.
 */
void    KbvFileView::updateOptions()
{
  int   id;

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
      iconSize = QSize(settings->iconSize, settings->iconSize);
      setGridSize(iconSize + QSize(10,30+id*10));
      setIconSize(iconSize);
    }
  else
    {
      iconSize = QSize(Kbv::IconList, Kbv::IconList);
      setGridSize(iconSize + QSize(476, 6)); //see: KbvGlobal::displayRoleData()
      setIconSize(iconSize);
    }
  //qDebug() << "kbvFileView::updateOptions grid" << gridSize(); //###########
  if(this->fileModel != NULL)
    {
      this->fileModel->updateOptions(iconSize, settings->dirWatchCycle);
    }
}
/*************************************************************************//*!
 * SLOT: Slide show when main menu Image|Slide show is activated or F8 pressed
 * or return key pressed or double click.
 * If nothing is selected, all items of the current directory are displayed.
 * If a sole item is selected the slide show starts from this item.
 */
void  KbvFileView::displaySlideShow(QModelIndex idx)
{
  //return key pressed or double click
  Q_UNUSED(idx)
  this->displaySlideShow();
}
void  KbvFileView::displaySlideShow()
{
  //main menu Image|Slide show activated or F8 pressed
  //only if this tab is visible
  if(weAreVisible)
    {
      //globalFunc.displaySlideShow(this, fileModel, sortModel);
      globalFunc.displayImages(this, fileModel, sortModel);
    }
}
/*************************************************************************//*!
 * SLOT: popUp menu: get database path + name.ext
 * Send signal to collectionTreeView to append the selected item in tree.
 */
void    KbvFileView::openDatabase()
{
  QModelIndex index;
  QString     path, name;

  index = this->sortModel->mapToSource(this->selectionModel()->currentIndex());

  path = this->fileModel->data(index, Kbv::FilePathRole).toString();
  name = this->fileModel->data(index, Kbv::FileNameRole).toString();
  path.append(name);
  qDebug() << "kbvFileView::openDatabase" <<path; //###########
  emit  fileOpenColl(path);
}
/*************************************************************************//*!
 * Return a list of model indicees of selected items. The indicees are not
 * mapped to the data model.
 */
QModelIndexList&  KbvFileView::getSelectedItems()
{
  selectedItems = this->selectionModel()->selectedIndexes();
  return  selectedItems;
}
/*************************************************************************//*!
 * Return the data for given index and role.
 */
QVariant    KbvFileView::getItemData(const QModelIndex &index, int role)
{
  QModelIndex idx = this->sortModel->mapToSource(index);
  return  this->fileModel->data(idx, role);
}
/*************************************************************************//*!
 * Slot: receive information about the visible tab by tab index.
 * This slot is for file view which doesn't have a title.
 */
void    KbvFileView::currentTabIndex(int index)
{
  if(index == KbvConf::fileViewTabIndex)
    {
      weAreVisible = true;
    }
  else
    {
      weAreVisible = false;
    }
  //qDebug() << "KbvFileView::currentTabIndex visible" << weAreVisible; //###########
}
/*************************************************************************//*!
 * SLOT: Single or multiple rename via menu in main window.
 */
void    KbvFileView::renameFromMenu()
  {
    QModelIndexList selIndices;
    QString         name1, name2;

    //only if this tab is visible
    if(weAreVisible)
      {
        selIndices = this->selectionModel()->selectedIndexes();
        for(int i=0; i<selIndices.count();i++)
          {
            selIndices[i] = this->sortModel->mapToSource(selIndices[i]);
          }
        
        if (selIndices.count() > 1)
          {
            //Multiple rename. Send the names of the first two selected files
            //to the dialog for name preview.
            name1 = this->fileModel->data(selIndices[0], Kbv::FileNameRole).toString();
            name2 = this->fileModel->data(selIndices[1], Kbv::FileNameRole).toString();

            if (renameDialog->perform(name1, name2, true) == QDialog::Accepted)
              {
                this->fileModel->multipleRename(selIndices,
                      renameDialog->prefix, renameDialog->suffix, renameDialog->extension,
                      renameDialog->startValue, renameDialog->stepValue,
                      renameDialog->numerals, renameDialog->combination);
              }
          }
        if (selIndices.count() == 1)
          {
            //Single rename. Send the name of the selected file to the dialog.
            name1 = this->fileModel->data(selIndices[0], Kbv::FileNameRole).toString();
            name2 = "";
            if (renameDialog->perform(name1, name2, false) == QDialog::Accepted)
              {
                this->fileModel->singleRename(selIndices,
                      renameDialog->prefix, renameDialog->suffix, renameDialog->extension,
                      renameDialog->combination);
              }
          }
        this->selectionModel()->clearSelection();
      }
  }

/****************************************************************************/
