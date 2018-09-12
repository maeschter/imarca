/*****************************************************************************
 * kbvDirView
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-01-18 17:30:00 +0100 (Do, 18. Jan 2018) $
 * $Rev: 1386 $
 * Created: 2009.02.05
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvMainMenu.h"
#include "kbvConstants.h"
#include "kbvDirView.h"

extern  KbvMainMenu   *mainMenu;

KbvDirView::KbvDirView(QWidget *parent) : QTreeView(parent)
{
  this->setHeaderHidden(true);
  this->setRootIsDecorated(true);
  this->setAnimated(false);
  this->setItemsExpandable(true);
  this->setSelectionBehavior(QAbstractItemView::SelectItems);
  this->setSelectionMode(QAbstractItemView::SingleSelection);
  this->setDropIndicatorShown(true);
  this->setDragEnabled(false);
  this->setAcceptDrops(true);
  this->setDragDropMode(QAbstractItemView::DropOnly);

  createPopup();

  //Drag&Drop as described in doc doesn't work.
  //So dropIndicator and scrolling the view has to be implemented
  dropIndicator = new QRubberBand(QRubberBand::Rectangle, this);
  scrollTimer = new QTimer(this);
  
  fileModel = nullptr;
  dirModel = nullptr;
  displayedDir = "";
  
  connect(mainMenu, SIGNAL(menuRenameDir()), this, SLOT(renameDir()));
  connect(mainMenu, SIGNAL(menuRemoveDir()), this, SLOT(removeDir()));
  connect(mainMenu, SIGNAL(menuInsertDir()), this, SLOT(insertDir()));
}

KbvDirView::~KbvDirView()
{
  //qDebug() << "KbvDirView::~KbvDirView";//###########
}

/************************************************************************//*!
 * Set the model by calling the base class and establish all signals between
 * view and model.
 */
void KbvDirView::setModelAndConnect(QAbstractItemModel *model)
{
  QTreeView::setModel(model);
  dirModel = qobject_cast<KbvDirModel*>(model);

  connect(this,  SIGNAL(activated (const QModelIndex)),     this,  SLOT(itemSelected(const QModelIndex)));
  connect(this,  SIGNAL(collapsed (const QModelIndex)),     this,  SLOT(itemSelected(const QModelIndex)));
  connect(this,  SIGNAL(collapsed (const QModelIndex)),     dirModel, SLOT(collapseDir(const QModelIndex)));
  connect(this,  SIGNAL(expanded (const QModelIndex)),      dirModel, SLOT(expandDir(const QModelIndex)));
  connect(this,  SIGNAL(itemActivated (const QModelIndex)), dirModel, SLOT(activatedDir(const QModelIndex)));

  connect(dirModel, SIGNAL(setCurrent (const QModelIndex)),    this, SLOT(setCurrentIndex(const QModelIndex)));
  connect(dirModel, SIGNAL(collapseView (const QModelIndex)),  this, SLOT(collapse(const QModelIndex)));
  connect(dirModel, SIGNAL(expandView (const QModelIndex)),    this, SLOT(expandDir(const QModelIndex)));
  connect(dirModel, SIGNAL(expandTree (const QModelIndex)),    this, SLOT(expand(const QModelIndex)));
  connect(dirModel, SIGNAL(setWaitCursor (const bool)),        this, SLOT(setWaitCursor(const bool)));

  connect(scrollTimer, SIGNAL(timeout()),  this, SLOT(scrollTreeTimer()));

  //expand user home
  this->expand(dirModel->index(0, 0, QModelIndex()));
  this->resizeColumnToContents(0);

}
/************************************************************************//*!
 * Get a pointer to the fileModel for performing drag'N'drop
 */
void  KbvDirView::setFileModel(KbvFileModel *model)
{
  fileModel = model;
}
/************************************************************************//*!
 * Drag&Drop from file view or from another application to dir view.
 * Only accept copy or move when mime data contains urls or file paths as text.
 * Create a rubber dropIndicator to indicate the drop region. The rubber band
 * shall have the width of the visual part of the view and take care of scroll bar.
 * To find the size of drop indicator we use the collection user root item.
 * This guarantees to get an valid index even if the drop enter event starts
 * in an empty region of the view.
 */
void KbvDirView::dragEnterEvent(QDragEnterEvent* event)
{
  QModelIndex   index;
  QRect         rect;

  //adjust and show drop indicator
  index = this->dirModel->indexFromItem(this->dirModel->userRoot());
  rect = this->visualRect(index);
  rect.setWidth(this->viewport()->geometry().width());

  if(event->mimeData()->hasUrls() | event->mimeData()->hasText())
    {
      event->acceptProposedAction();

      this->dropIndicator->setGeometry(rect);
      this->dropIndicator->show();
    }
}

/************************************************************************//*!
 * Drag leave event: hide dropIndicator.
 */
void KbvDirView::dragLeaveEvent(QDragLeaveEvent* event)
{
  Q_UNUSED(event);
  this->dropIndicator->hide();
  this->scrollTree(QPoint(0,0), false);
}

/************************************************************************//*!
 * Evaluates keyboard and determines copy or move due to modifier keys:
 * internal (source == 0): control key -> copy.
 * external (source == 0): shift key -> move.
 * The drop indicator is moved to mouse position. The event only is accepted
 * when an item is the target (valid index).
 */
void KbvDirView::dragMoveEvent(QDragMoveEvent *event)
{
  QRect rect;

  //scroll view up/down or left/right if necessary
  this->scrollTree(event->pos(), true);

  if(indexAt(event->pos()).isValid())
    {
      event->acceptProposedAction();
    }
  else
    {
      event->ignore();
      return;
    }

  rect = this->visualRect(indexAt(event->pos()));
  this->dropIndicator->move(this->visibleRegion().boundingRect().x(), rect.y());

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
/************************************************************************//*!
 * Drop from collectionView, searchView or external application. Only files
 * are copied/moved.
 * We have to distinct 2 cases:
 * 1: target = displayed dir: perform drop in file model and update it
 * 2: target = other dir: perform drop in file model without model update
 * On internal drag x-special/imarca-copied-files contains:
 * from collection/album/search view: db type, db name, list of primary keys
 * from fileView: type=TypeFile, source path
 * Drag from collection/album/search view gets performed as copy action.
 * On drag from fileView we check if the target path is the same as the
 * source path (drag within same dir). In this case nothing gets dropped.
 * When the drop target is different to the source and is the visible tree
 * path (the path displayed in the drag source view, the model must be updated.
 */
void KbvDirView::dropEvent(QDropEvent *event)
{
  QModelIndex   index;
  QString       targetpath, str;
  QByteArray    ba;
  QStringList   strlist;
  bool          ok=false, withindir=false;
  int           type;

  //remove drop indicator and stop scrolling
  this->dropIndicator->hide();
  this->scrollTree(QPoint(0,0), false);
  index = indexAt(event->pos());
  
  if(index.isValid() && (fileModel != 0))
    {
      event->acceptProposedAction();
      
      //Get target path
      targetpath =  this->dirModel->data(index, Kbv::FilePathRole).toString();
      targetpath += this->dirModel->data(index, Kbv::FileNameRole).toString();
      
      //x-special/imarca-copied-files contains: 
      //drag from database: source db type, source db name, list of primary keys
      //drag from file system: TypeFile, source path
      if(event->mimeData()->hasFormat("x-special/imarca-copied-files"))
        {
          ba = event->mimeData()->data("x-special/imarca-copied-files");
          str = QString::fromUtf8(ba.data());
          strlist = str.split("\n", QString::SkipEmptyParts);
          type = strlist.at(0).toInt(&ok);
          
          if(type & Kbv::TypeFile)
            {
              if((targetpath + "/") == strlist.at(1))
                {
                  //qDebug() << "KbvDirView::dropEvent in same dir"; //###########
                  withindir = true;    
                }
            }
        }
      //no drag/drop within a directory
      if(!withindir)
        {
          if(displayedDir == targetpath)
            {
              //qDebug() << "KbvDirView::dropEvent on displayed dir"<<targetpath; //###########
              fileModel->dropData(event->mimeData(), event->dropAction(), targetpath, true, false);
            }
          else
            {
              //qDebug() << "KbvDirView::dropEvent not displayed dir"<<targetpath; //###########
              fileModel->dropData(event->mimeData(), event->dropAction(), targetpath, false, false);
            }
        }
    }
  //qDebug() << "KbvDirView::dropEvent finished"; //###########
}
/************************************************************************//*!
 * Scrolling the viewport. This is working together with a timer event.
 * The timer is started when scrolling is enabled and still not in progress.
 * When scrolling is disabled the timer is stopped immediately.
 * Scrolling will be carried out by timer event when the mouse pointer is near
 * the border of the viewport.
 */
void KbvDirView::scrollTree(QPoint mousePosition, bool enable)
{
  QRect    rect;

  if(!enable)
    {
      //qDebug() << "kbvDirView::scrollTree stop"; //###########
      this->scrollTimer->stop();
    }
  else
    {
      //calculate margins for scroll direction
      rect = this->viewport()->geometry();
      xmargin = mousePosition.x() - rect.x();
      ymargin = mousePosition.y() - rect.y();
      wmargin = rect.width() - mousePosition.x();
      hmargin = rect.height() - mousePosition.y();
      margin = 16;

      //start scroll timer when mouse is at least inside one margin
      //and scrolling still is not running
      if((xmargin<=margin || ymargin<=margin || wmargin<=margin || hmargin<=margin) && !scrollTimer->isActive())
        {
          this->scrollTimer->start(100);
        }
      //stop scroll timer when mouse is outside all margins and
      //scrolling is running
      if((xmargin>margin && ymargin>margin && wmargin>margin && hmargin>margin) && scrollTimer->isActive())
        {
          this->scrollTimer->stop();
        }
   }
}
void KbvDirView::scrollTreeTimer()
{
  //scrolling x direction by pixel
  if(this->horizontalScrollBar()->isVisible())
    {
      value = this->horizontalScrollBar()->value();
      if(xmargin <= margin && xmargin >= 0)
        {
          this->horizontalScrollBar()->setValue(value - 6);
        }
      if(wmargin <= margin && wmargin >= 0)
        {
          this->horizontalScrollBar()->setValue(value + 6);
        }
    }
  //scrolling y direction by item
  if(this->verticalScrollBar()->isVisible())
    {
      value = this->verticalScrollBar()->value();
      if(ymargin <= margin && ymargin >= 0)
        {
          this->verticalScrollBar()->setValue(value - 1);
        }
      if(hmargin <= margin && hmargin >= 0)
        {
          this->verticalScrollBar()->setValue(value + 1);
        }
    }
}

/************************************************************************//*!
 * SLOT: catch mouse click selection
 * Overrides the signal clicked() of base class on left button pressed.
 * Base class is called otherwise the model doesn't work properly.
 * Resize column 0 to content forces horizontal scrollbar if necessary.
 */
void    KbvDirView::mouseReleaseEvent(QMouseEvent * event)
{
  if (event->button() == Qt::LeftButton)
    {
      index = currentIndex();
      QTreeView::mouseReleaseEvent(event);
      this->itemSelected(index);
    }
}

/*************************************************************************//*!
 * Key press event
 * Ctrl-A key or ctrl-V key pressed: inform the visible view
 */
void    KbvDirView::keyPressEvent(QKeyEvent *event)
{
  if(event->modifiers() == Qt::ControlModifier)
    {
      if((event->key() == Qt::Key_A) || (event->key() == Qt::Key_V))
      {
        //qDebug() << "kbvDirView::keyPressEvent ctrl key"; //###########
        emit  keyCtrlAPressed(event);
      }
    }
  else
    {
      //Delegate key event to base class
      QTreeView::keyPressEvent(event);
    }
}
/************************************************************************//*!
 * SLOT:
 * Catch item activation by signal "activated" (enter key or mouse click) or
 * on collapsing an item (signal "collapsed")
 */
void    KbvDirView::itemSelected(const QModelIndex &index)
  {
    //qDebug() << "kbvDirView::itemSelected"; //###########
    if(index.isValid())
      {
        this->setCurrentIndex(index);
        emit itemActivated(index);
        displayedDir = this->dirModel->data(index, Kbv::FilePathRole).toString();
        displayedDir += this->dirModel->data(index, Kbv::FileNameRole).toString();
      }
    this->resizeColumnToContents(0);
  }
/************************************************************************//*!
 * SLOT:
 * Catch item activation by arrow keys or when an activated device has been
 * removed and the selection bar moves to another item.
 */
void    KbvDirView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
  Q_UNUSED(previous)
  
  //qDebug() << "kbvDirView::currentChanged" << current; //###########
  emit itemActivated(current);
}
/************************************************************************//*!
 * SLOT: Set cursor shape to wait cursor and reset to previous shape.
 */
void    KbvDirView::setWaitCursor(bool wait)
{
  if(wait)
    {
      cursor = this->viewport()->cursor();
      this->viewport()->setCursor(Qt::WaitCursor);
    }
  else
    {
      this->viewport()->setCursor(cursor);
    }
}

/************************************************************************//*!
 * Popup menu for directory operations: remove and insert.
 */
void    KbvDirView::contextMenuEvent(QContextMenuEvent *event)
{
    popupDirView->exec(event->globalPos());
    emit (clearStatusMessage());
}
/************************************************************************//*!
 * SLOT: Expand to the given index at startup or when invoked by command
 * line and set this dir as current (displays the content in file view).
 */
void    KbvDirView::expandDir(const QModelIndex &index)
  {
    if(index.isValid())
      {
        this->expand(index);
        this->setCurrentIndex(index);
      }
  }
/************************************************************************//*!
 * SLOT: Create new directory at model index. Called from menu or popup.
 * All further work is done in kbvDirModel.
 */
void    KbvDirView::insertDir(void)
{
  QModelIndex   index;
  QString       name, newname;
  bool          dialogOK;
  
  index = currentIndex();
  name = dirModel->data(index, Kbv::FileNameRole).toString();
  
  newname = QInputDialog::getText(this, tr("Insert directory"),
                                   tr("Please enter a name"),
                                  QLineEdit::Normal, name,  &dialogOK);
  if (dialogOK && !newname.isEmpty())
    {
      dirModel->insertDir(index, newname);
    }
}

/************************************************************************//*!
 * SLOT: Delete directory at model index. Called from menu or popup.
 * All further work is done in kbvDirModel.
 */
void    KbvDirView::removeDir(void)
{
  int           retval;

  //show warning
  retval = QMessageBox::warning(this, tr("Remove directory"),
              tr("This deletes the directory and all contents finally!\n"
                  "Do you really want to remove?"),
           QMessageBox::Cancel | QMessageBox::Ok,
           QMessageBox::Cancel);
  if(retval == QMessageBox::Ok)
    {
      dirModel->removeDir(currentIndex());
    }
}

/************************************************************************//*!
 * SLOT: Rename directory at model index. Called from menu or popup.
 * All further work is done in kbvDirModel.
 */
void    KbvDirView::renameDir(void)
{
  QModelIndex   index;
  QString       name, newname;
  bool          dialogOK;
  
  index = currentIndex();
  name = dirModel->data(index, Kbv::FileNameRole).toString();
  
  newname = QInputDialog::getText(this, tr("Rename directory"),
                                  tr("Please enter a name"),
                                  QLineEdit::Normal, name,  &dialogOK);
  if (dialogOK && !newname.isEmpty())
    {
      dirModel->renameDir(index, newname);
    }
}

/************************************************************************//*!
 * Popup menu for directory operations: remove and insert.
 */
void  KbvDirView::createPopup()
  {
    actRenameDir = new QAction(tr("Rename Directory"), this);
    actRenameDir->setStatusTip(tr("rename this directory"));
    actInsertDir = new QAction(tr("Insert Directory"), this);
    actInsertDir->setStatusTip(tr("Create a new directory as subdirectory at selected position"));
    actRemoveDir = new QAction(tr("Remove Directory"), this);
    actRemoveDir->setStatusTip(tr("Warning: the directory and all contents are deleted from file system"));

    popupDirView = new QMenu(this);
    popupDirView->addAction(actRenameDir);
    popupDirView->addSeparator();
    popupDirView->addAction(actInsertDir);
    popupDirView->addAction(actRemoveDir);

    connect(actRenameDir, SIGNAL(triggered()), this, SLOT(renameDir()));
    connect(actInsertDir, SIGNAL(triggered()), this, SLOT(insertDir()));
    connect(actRemoveDir, SIGNAL(triggered()), this, SLOT(removeDir()));

  }

/****************************************************************************/
