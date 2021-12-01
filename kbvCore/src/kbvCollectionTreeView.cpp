/*****************************************************************************
 * KbvCollectionTreeView.cpp
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2010.10.27
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "kbvConstants.h"
#include "kbvMainMenu.h"
#include "kbvSetvalues.h"
#include "kbvInformationDialog.h"
#include "kbvCollectionTreeView.h"
#include "kbvCollectionTabs.h"

extern  KbvMainMenu             *mainMenu;
extern  KbvSetvalues            *settings;
extern  KbvInformationDialog    *informationDialog;

KbvCollectionTreeView::KbvCollectionTreeView(QWidget *parent) : QTreeView(parent)
{
  this->setHeaderHidden(true);
  this->setAnimated(false);
  this->setItemsExpandable(true);
  this->setSelectionBehavior(QAbstractItemView::SelectItems);
  this->setSelectionMode(QAbstractItemView::SingleSelection);
  this->setDropIndicatorShown(true);
  this->setDragEnabled(true);
  this->setAcceptDrops(true);
  this->setDragDropMode(QAbstractItemView::InternalMove);
  this->setMouseTracking(true);
  this->createPopUp();

  dbOptionsDialog = new KbvDBOptions(this);
  dbExportDialog = new KbvDBExport(this);
  dbImportDialog = new KbvDBImport(this);
  dbImExProgress = new KbvDBImExportProgress(this);
  dbImExThread   = new KbvDBImExportThread(nullptr);

  connect(mainMenu, SIGNAL(menuOpenColl(QString)), this, SLOT(databaseOpen(QString)));
  connect(mainMenu, SIGNAL(menuImportColl()),      this, SLOT(databaseImport()));
  connect(mainMenu, SIGNAL(menuExportColl()),      this, SLOT(databaseExport()));
  connect(mainMenu, SIGNAL(menuDBCreate()),        this, SLOT(databaseCreate()));
  connect(mainMenu, SIGNAL(menuDBRemove()),        this, SLOT(databaseRemove()));
  connect(mainMenu, SIGNAL(menuDBRead()),          this, SLOT(databaseRead()));
  connect(mainMenu, SIGNAL(menuDBUpdate()),        this, SLOT(databaseUpdate()));
  connect(mainMenu, SIGNAL(menuDBRename()),        this, SLOT(collRename()));
  connect(mainMenu, SIGNAL(menuDBInsert()),        this, SLOT(collInsert()));
  connect(mainMenu, SIGNAL(menuDBDelete()),        this, SLOT(collDelete()));
  
  connect(dbImExProgress, SIGNAL(cancel()),       dbImExThread, SLOT(stop()));
  connect(dbImExThread,   SIGNAL(files(int)),     dbImExProgress, SLOT(setFilecount(int)));
  connect(dbImExThread,   SIGNAL(size(quint64)),  dbImExProgress, SLOT(setDiskSize(quint64)));
  connect(dbImExThread,   SIGNAL(progress(int)),  dbImExProgress, SLOT(setProgress(int)));
  connect(dbImExThread,   SIGNAL(finished(int)),  this, SLOT(databaseImExportFinished(int)));
  connect(dbImExThread,   SIGNAL(setDB(QString)), this, SLOT(databaseImExportsetDB(QString)));

  dropIndicator = new QRubberBand(QRubberBand::Rectangle, this);
  scrollTimer = new QTimer(this);
}

KbvCollectionTreeView::~KbvCollectionTreeView()
{
  //qDebug() << "KbvCollectionTreeView::~KbvCollectionTreeView"; //###########
  delete  dbImExThread;
}

/*************************************************************************//*!
 * Set the model by calling the base class and establish all signals between
 * view and model. The model pointer is stored in member collTreeModel to prevent
 * a lot of qobject_cast<KbvCollectionTreeModel*>().
 */
void KbvCollectionTreeView::setModelAndConnect(QAbstractItemModel *model)
  {
    QTreeView::setModel(model);
    collTreeModel = qobject_cast<KbvCollectionTreeModel*>(model);

    connect(this,  SIGNAL(activated(const QModelIndex)),  this, SLOT(itemSelected(const QModelIndex)));
    connect(this,  SIGNAL(expanded (const QModelIndex)),  this, SLOT(itemExpanded(const QModelIndex)));
    connect(this,  SIGNAL(expanded (const QModelIndex)),  collTreeModel, SLOT(expand(const QModelIndex)));
    connect(this,  SIGNAL(collapsed (const QModelIndex)), collTreeModel, SLOT(collapse(const QModelIndex)));
    connect(scrollTimer, SIGNAL(timeout()),               this, SLOT(scrollTreeTimer()));
    connect(collTreeModel, SIGNAL(setWaitCursor (const bool)),  this, SLOT(setWaitCursor(const bool)));

    //get all collections and photo albums from config file
    collTreeModel->setCollTreeState();

    //show all collections and albums by expanding the roots and
    //install horizontal/vertical scrollbars if needed
    this->expandToDepth(0);
    this->resizeColumnToContents(0);
  }

/*************************************************************************//*!
 * Accept drag from external application, file view, album/collection view or
 * search view or move of items inside tree view.\n
 * To find the size of drop indicator we use the collection root item. This
 * guarantees to get an valid index even if the drop enter event starts in an
 * empty region of the view.
 */
void KbvCollectionTreeView::dragEnterEvent(QDragEnterEvent* event)
{
  QModelIndex   index;
  QRect         rect;

  //adjust and show drop indicator
  index = this->collTreeModel->indexFromItem(this->collTreeModel->collectionAnchor());
  rect = this->visualRect(index);
  rect.setWidth(this->viewport()->geometry().width());
  dropIndicator->setGeometry(rect);
  dropIndicator->show();
  event->accept();
  //qDebug() << "KbvCollectionTreeView::dragEnterEvent"<<event->dropAction(); //###########
}
/*************************************************************************//*!
 * Drag leave event: hide dropIndicator.
 */
void KbvCollectionTreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
  Q_UNUSED(event);
  this->dropIndicator->hide();
  this->scrollTree(QPoint(0,0), false);
}
/*************************************************************************//*!
 * Drag from external application, file view, album/collection view or search
 * view or move of items inside view.\n
 * Internal move: only album->album or collection->collection is permitted.
 * No drop on root items.
 */
void KbvCollectionTreeView::dragMoveEvent(QDragMoveEvent* event)
{
  QModelIndex   targetIdx;
  QString       text, collection;
  int           targetType, sourceType;
  bool          ok;
  QRect         rect;

  //scroll view up/down or left/right if necessary
  this->scrollTree(event->pos(), true);

  //show selection rectangle at drop position.
  targetIdx = indexAt(event->pos());
  rect = this->visualRect(targetIdx);
  dropIndicator->move(this->visibleRegion().boundingRect().x(), rect.y());

  //get type of target
  targetType = this->collTreeModel->data(targetIdx, Kbv::CollectionTypeRole).toInt();
  collection = this->collTreeModel->data(targetIdx, Kbv::CollectionNameRole).toString();

  //qDebug() << "KbvCollectionTreeView::dragMoveEvent target source"<<this <<event->source(); //###########

  //Internal move (rearrange items) when source == collectionTreeView
  //Allow drag: album->album and collection->collection and ignore the roots
  //SourceType always is of type album or type collection but never of type
  //albumroot or type collectionroot.
  if(event->source() == this)
    {
      event->setDropAction(Qt::MoveAction);
      //get type of source
      text = event->mimeData()->text();
      sourceType = text.toInt(&ok, 10);
      //qDebug() << "KbvCollectionTreeView::dragMoveEvent intern"; //###########

      //Assure drag from album->album and collection->collection
      if(sourceType == targetType)
        {
          event->accept();
        }
      else
        {
          event->ignore();
        }
    }

  //Copy or move from external application or internal fileView, collectionView or
  //searchView on collectionTree. Ignore the roots.
  //On external drags setting of dropActions is not working hence we may see a wrong
  //icon. The drop event controls the right action (album: copy only).
  else
    {
      if((targetType & Kbv::TypeAlbumAnchor) || (targetType & Kbv::TypeCollectionAnchor))
        {
         event->ignore();
        }
      else
        {
          if(targetType & Kbv::TypeAlbumRoot)
            {
              event->setDropAction(Qt::CopyAction);
            }
          if((targetType & Kbv::TypeCollection) || (targetType & Kbv::TypeCollectionRoot))
            {
              if(event->keyboardModifiers() & Qt::ControlModifier)
                {
                  event->setDropAction(Qt::CopyAction);
                }
              else
                {
                  event->setDropAction(Qt::MoveAction);
                }
            }
       }
      event->accept();
      //qDebug() << "KbvCollectionTreeView::dragMoveEvent" <<event->dropAction(); //###########
    }
}
/*************************************************************************//*!
 * Move of items inside tree view (rearrangement) or drop from fileView,
 * collectionView, searchView or external application.\n
 * It's important to figure out if this is an internal drag from any view
 * to the visible tree path (the path displayed in the source view).
 * These cases are drops on theirself which must be supressed to prevent data
 * loss. This can be done by reading the drag source from mime data 
 * (x-special/imarca-copied-files). On albums we don't drop when the target is the source.
 * On collections we don't drop when target is the source and the path is
 * visible (shown in view).
 */
void KbvCollectionTreeView::dropEvent(QDropEvent* event)
{
  QModelIndex       dropIdx;
  QString           path, branch, name, location, rootDir, str;
  KbvCollectionTabs *tabsRight=0;
  QByteArray        ba;
  QStringList       strlist;  
  int               type;
  bool              visible=false, internal=false;

  //remove drop indicator and stop scrolling
  this->dropIndicator->hide();
  this->scrollTree(QPoint(0,0), false);
  //get model index at drop position
  dropIdx = this->indexAt(event->pos());

  //Internal move (rearrange items) when source == collectionTreeView
  //Move album to album or collection to collection inside treeView.
  //The item is moved in the model.
  if(event->source() == this)
    {
      this->collTreeModel->moveItem(dragItemIndex, dropIdx);
    }

  //Copy or move from external application or internal fileView, collectionView or
  //searchView on collectionTree. Internal drop has mime format "x-special/imarca-copied-files"
  //including source database and type. This is necessary for drop handler.
  else
    {
      //qDebug() << "KbvCollectionTreeView::dropEvent" <<event->dropAction(); //###########
      
      //The drop position delivers the target index, db name, collection name, root and type
      //Path is the relative path of branch below the collection root dir
      branch = this->collTreeModel->data(dropIdx, Qt::DisplayRole).toString();
      path = this->collTreeModel->data(dropIdx, Kbv::FilePathRole).toString();
      name = this->collTreeModel->data(dropIdx, Kbv::CollectionNameRole).toString();
      location = this->collTreeModel->data(dropIdx, Kbv::DatabaseLocationRole).toString();
      rootDir = this->collTreeModel->data(dropIdx, Kbv::CollectionRootDirRole).toString();
      type = this->collTreeModel->data(dropIdx, Kbv::CollectionTypeRole).toInt();

      //x-special/imarca-copied-files contains:
      //Mime data (type x-special/imarca-copied-files) for internal drag/drop
      //drag from database: empty string, source db type, source db name, source db location, list of primary keys
      //drag from file system: empty string, TypeFile, source path
      if(event->mimeData()->hasFormat("x-special/imarca-copied-files"))
        {
          ba = event->mimeData()->data("x-special/imarca-copied-files");
          str = QString::fromUtf8(ba.data());
          strlist = str.split("\n", QString::SkipEmptyParts); //on drop first part is empty
          if(name == strlist.at(1)) //source db name expected
            {
              internal = true;
            }
          //qDebug() << "KbvCollectionTreeView::dropEvent"<<collection <<strlist.at(0) <<strlist.at(1) <<strlist.at(2); //###########
        }
      
      //Find pointer to kbvTabsRight to get the drop handler. Search from widget
      //list of application since this doesn't depend on widget structure
      foreach(QWidget *widget, qApp->allWidgets())
        {
          if(widget->objectName() == "kbvSplitter")
            {
              tabsRight = widget->findChild<KbvCollectionTabs*>("kbvTabsRight");
            }
        }
      //qDebug() << "KbvCollectionTreeView::dropEvent ptr tabsRight" <<tabsRight; //###########

      //pointer valid? -> perform drop
      if(tabsRight != NULL)
        {
          //Construct a databaseInfo object for dropDataScheduler and dndHandler
          dbInfo = new KbvDBInfo(this);
          if(this->createDBInfo(dbInfo, name, location, rootDir, type))
            {
              //On albums only drop copy is supported
              //Check if the target is presented in collectionView
              //activatedPath and activatedTitle get set in itemSelected()
              if(type & Kbv::TypeAlbumRoot)
                {
                  if(name == activatedTitle)
                    {
                      visible = true;
                    }
                  //don't drop within the same album
                  if(!internal)
                    {
                      event->setDropAction(Qt::CopyAction);
                      //qDebug() << "KbvCollectionTreeView::dropEvent album" <<activatedPath <<path <<name <<visible; //###########
                      tabsRight->dropDataScheduler(event->mimeData(), event->dropAction(), path, dbInfo, visible);
                    }
                }
              else
                {
                  //On collections copy/move can be performed (move: shift modifier)
                  //Check if the target is presented in collectionView
                  if((type & Kbv::TypeCollection) || (type & Kbv::TypeCollectionRoot))
                    {
                      //branch is the name of the drop target:
                      //Append branch to path when the type is not the root (this would deliver a wrong path)
                      //and append "/" when path isn't empty (in his case path is identical to collection root
                      if(!(type & Kbv::TypeCollectionRoot))
                        {
                          if(!path.isEmpty())
                            {
                              path.append("/");
                            }
                          path.append(branch);
                        }

                      //activatedPath and activatedTitle get set in itemSelected()
                      //both are empty at drop on non open collection
                      if((name == activatedTitle) && (path == activatedPath))
                        {
                          visible = true;
                        }
                      //don't drop within the same branch of the collection
                      if(!(internal && visible))
                        {
                          event->acceptProposedAction();
                          //qDebug() << "KbvCollectionTreeView::dropEvent collection" <<activatedPath <<activatedTitle <<path <<name <<visible; //###########
                          tabsRight->dropDataScheduler(event->mimeData(), event->dropAction(), path, dbInfo, visible);
                        }
                    }
                }
            }
          delete dbInfo;
        }
    }
}
/*************************************************************************//*!
 * Drag&Drop: start internal move of items or enable/disable of collection
 * update. Pass event to base class.
 */
void    KbvCollectionTreeView::mousePressEvent(QMouseEvent *event)
{
  QModelIndex   idx;
  int           type;
  
  if(event->button() == Qt::LeftButton)
    {
      dragStartPos = event->pos();
    }
  if(event->button() == Qt::RightButton)
    {
      idx = this->indexAt(event->pos());
      type = this->collTreeModel->data(idx, Kbv::CollectionTypeRole).toInt();
      if(type & Kbv::TypeCollectionRoot || type & Kbv::TypeCollection)
        {
          this->actDBUpdate->setEnabled(true);
          this->actDBRead->setEnabled(true);
        }
      else
        {
          this->actDBUpdate->setEnabled(false);
          this->actDBRead->setEnabled(false);
        }
    }
  
  QTreeView::mousePressEvent(event);
}
/*************************************************************************//*!
 * Drag&Drop: internal move of items or enable/disable of collection update.
 */
void KbvCollectionTreeView::mouseMoveEvent(QMouseEvent *event)
{
  QString       text;
  int           type;

  if((event->buttons() & Qt::LeftButton))
    {
      if((event->pos() - dragStartPos).manhattanLength() > QApplication::startDragDistance())
       {
          //qDebug() << "KbvCollectionTreeView::mouseMoveEvent drag"; //###########
          dragItemIndex = this->currentIndex();
          if(!dragItemIndex.isValid())
            {
              return;
            }
          //remove TypeJoined
          type = this->collTreeModel->data(dragItemIndex, Kbv::CollectionTypeRole).toInt();
          type = type & ~Kbv::TypeJoined;   //keep all except TypeJoined
          text = QString("%1").arg(type);
          //qDebug() << "KbvCollectionTreeView::mouseMoveEvent type" <<type <<text; //###########

          QMimeData *mimeData = new QMimeData;
          mimeData->setText(text);
        
          QDrag *drag = new QDrag(this);
          drag->setMimeData(mimeData);
          drag->setPixmap(QPixmap(":/kbv/icons/image-x-generic.png"));
          drag->exec(Qt::MoveAction, Qt::MoveAction);
       }
    }
}
/*************************************************************************//*!
 * SLOT: catch mouse click selection\n
 * Overrides the signal clicked() of base class on left button pressed.
 * Base class is called otherwise the model doesn't work properly.
 */
void KbvCollectionTreeView::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      QTreeView::mouseReleaseEvent(event);
      this->itemSelected(currentIndex());
    }
}
/*************************************************************************//*!
 * Key press event
 * Ctrl-A key or ctrl-V key pressed: inform the visible view
 */
void KbvCollectionTreeView::keyPressEvent(QKeyEvent *event)
{
  if(event->modifiers() == Qt::ControlModifier)
    {
      if((event->key() == Qt::Key_A) || (event->key() == Qt::Key_V))
        {
          //qDebug() << "kbvCollectionTreeView::keyPressEvent ctrl key"; //###########
          emit  keyCtrlAPressed(event);
        }
    }
  else
    {
      //Delegate key event to base class
      QTreeView::keyPressEvent(event);
    }
}
/*************************************************************************//*!
 * Scrolling the viewport. This is working together with a timer event.
 * The timer is started when scrolling is enabled and still not in progress.
 * When scrolling is disabled the timer is stopped immediately.
 * Scrolling will be carried out by timer event when the mouse pointer is near
 * the border of the viewport.
 */
void KbvCollectionTreeView::scrollTree(QPoint mousePosition, bool enable)
{
  QRect    rect;

  if(!enable)
    {
      //qDebug() << "kbvCollectionTreeView::scrollTree stop"; //###########
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
/*************************************************************************//*!
 * Scroll timer.
 */
void KbvCollectionTreeView::scrollTreeTimer()
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
/*************************************************************************//*!
 * SLOT: called when a collectionTab gets closed.\n
 * This must collapse the related collection completely.
 */
void KbvCollectionTreeView::collapseItem(const QString &title)
{
  QModelIndex   index;

  index = this->collTreeModel->indexFromCollection(title);
  if(index.isValid())
    {
      collapse(index);
    }
}
/*************************************************************************//*!
 * SLOT: called when a collection gets expanded.\n
 * When the expanded item is the root dir of a collection (the first level
 * below collection anchor) then set this item as current (select it). Hence
 * signal "collItemActivated" is emitted to display tab and related database
 * for this collection. Do nothing in all other cases.
 */
void KbvCollectionTreeView::itemExpanded(const QModelIndex &index)
{
  int           type;

  if(index.isValid())
    {
      type = this->collTreeModel->data(index, Kbv::CollectionTypeRole).toInt();
      if(type & Kbv::TypeCollectionRoot)
        {
          this->setCurrentIndex(index);
        }
    }
}
/*************************************************************************//*!
 * SLOT: catch row activation by signal "activated" (containing enter key
 * or mouse click).\n
 * Emit signal "collItemActivated". This signal displays tab and related
 * database for albums or collections when not yet present.
 * For collections the selected branch (directory path) is transmitted.
 */
void KbvCollectionTreeView::itemSelected(const QModelIndex &index)
{
  QString   title, branch, path, location, root;
  int       type;
  
  if(index.isValid())
    {
      //Path is the relative path (below collection root dir) to branch
      path = this->collTreeModel->data(index, Kbv::FilePathRole).toString();
      branch = this->collTreeModel->data(index, Qt::DisplayRole).toString();
      type = this->collTreeModel->data(index, Kbv::CollectionTypeRole).toInt();
      title = this->collTreeModel->data(index, Kbv::CollectionNameRole).toString();
      location = this->collTreeModel->data(index, Kbv::DatabaseLocationRole).toString();
      root = this->collTreeModel->data(index, Kbv::CollectionRootDirRole).toString();

      if((type & Kbv::TypeAlbumAnchor) || (type & Kbv::TypeCollectionAnchor))
        {
          return;
        }
      else
        {
          //album tabs only are interested in type album
          if(type & Kbv::TypeAlbumRoot)
            {
              type = Kbv::TypeAlbum;
            }
          //append branch to path when the type is not the root (this would deliver a wrong path)
          //and append "/" when path isn't empty (in his case path is identical to collection root)
          if(type & Kbv::TypeCollection)
            {
              if(!path.isEmpty())
                {
                  path.append("/");
                }
              path.append(branch);
            }
          //collection tabs only are interested in type collection
          if(type & Kbv::TypeCollectionRoot)
            {
              type = Kbv::TypeCollection;
            }
          //qDebug() << "KbvCollectionTreeView::itemSelected" << title << path << type; //###########
          //Path is the relative path below the collection root dir
          emit collItemActivated(title, path, type, location, root);  //-> kbvTabsRight->addCollectionTab()
        }
      activatedTitle = title;
      activatedPath = path;
      this->resizeColumnToContents(0);
    }
}
/************************************************************************//*!
 * SLOT: Set cursor shape to wait cursor and reset to previous shape.
 */
void    KbvCollectionTreeView::setWaitCursor(bool wait)
{
  QCursor   cursor;

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
/*************************************************************************//*!
 * SLOT: Menu action: Create new database for collection or album.\n
 * Create only when the database directory is properly set and the desired
 * name is not an already existing collection or album. When created, ask for
 * icon size, type and description. The type returned from dialog either is
 * Kbv::TypeAlbum or Kbv::TypeCollection.
 */
void KbvCollectionTreeView::databaseCreate()
{
  QString   path, name, res, description, version, collroot;
  QFile     newDB;
  int       type, iconsize = KbvConf::stdIconSize, keywordtype, result;
  QSqlDatabase  db;
  QSqlQuery     query;
  QString       str1, str2;

  version = QString(dbMinVer);

  //Check if database dir exists and is writable and readable
  path = settings->dataBaseDir;
  if(!databaseDirExists(path))
    {
      str1 = QString(tr("No directory for databases declared or no access possible!"));
      str2 = QString(tr("Please open menu options and select one."));
      informationDialog->perform(str1,str2,1);
      return;
    }

  //Ask for basic settings of new database
  dbOptionsDialog->setWindowTitle(tr("Create new database"));
  result = dbOptionsDialog->exec();
  name = dbOptionsDialog->databaseName();
  iconsize = dbOptionsDialog->iconsize();
  type = dbOptionsDialog->databaseType();
  description = dbOptionsDialog->description();
  keywordtype = dbOptionsDialog->keywordType();
  collroot = dbOptionsDialog->rootDir();

  if(result != QDialog::Accepted)
    {
      //qDebug() << "KbvCollectionTreeView::databaseCreate result" << result; //###########
      return;
    }
  if(name.isEmpty() || (collroot.isEmpty() && (type & Kbv::TypeCollection)))
    {
      informationDialog->perform(str1,QString(),1);
      str1 = QString(tr("No valid collection name or root directory."));
      return;
    }

  //check for items with same name, the type always is collection or album
  //qDebug() << "KbvCollectionTreeView::databaseCreate type" << name << type; //###########
  if(this->collTreeModel->childNameExists(name, type, QModelIndex()))
    {
      str1 = QString(tr("Database already exists!"));
      str2 = QString(tr("Please choose another name."));
      informationDialog->perform(str1,str2,1);
      return;
    }

  //all ok: try to copy database pattern "imageDB" to database dir
  //If this fails the database is already available
  path = path + "/" + name + QString(dbNameExt);
  res = QString(":/kbv/other/imageDB") + QString(dbNameExt);
  newDB.setFileName(res);

  //path to collection root must end with "/", album root is empty
  if(!collroot.isEmpty())
    {
      collroot.append("/");
    }
  //qDebug() << "KbvCollectionTreeView::databaseCreate" <<collroot << path << res; //###########

  if(!newDB.copy(path))
    {
      //qDebug() << "KbvCollectionTreeView::databaseCreate db exists" << path; //###########
      //Database is already available:
      //Don't set root dir (in case of collection the model reads from database)
      //and albums don't care about root dir.
      this->collTreeModel->createItem(name, "", type, false, false);
      this->resizeColumnToContents(0);

      str1 = QString(tr("Database already exists!"));
      informationDialog->perform(str1,QString(),1);
    }
  else
    {
      //qDebug() << "KbvCollectionTreeView::databaseCreate db is new" << path; //###########
      //Database is new:
      //Set access rights, create model item and write settings to database
      //We set root dir to get a proper collection (Albums ignore root dir).
      newDB.setPermissions(path, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther);
      this->collTreeModel->createItem(name, collroot, type, false, false);
      this->resizeColumnToContents(0);

      //Open database and set values in table 'description': icon size, description, type
      db = QSqlDatabase::addDatabase("QSQLITE", "con"+name);
      db.setHostName("host");
      db.setDatabaseName(path);
      if(db.open())
        {
          query = QSqlQuery(db);
          query.prepare("INSERT INTO description "
                    "(iconSize, comment, colltype, version, keywordtypes, rootdir) VALUES "
                    "(:iconSize, :comment, :colltype, :version, :keywordtypes, :rootdir)");

          query.bindValue(":iconSize",     QVariant(iconsize));
          query.bindValue(":comment",      QVariant(description));
          query.bindValue(":colltype",     QVariant(type));
          query.bindValue(":version",      QVariant(version));
          query.bindValue(":keywordtypes", QVariant(keywordtype));
          query.bindValue(":rootdir",      QVariant(collroot));

          if(query.exec())
            {
              this->collTreeModel->storeCollTreeState();
            }
          //qDebug() << "KbvCollectionTreeView::databaseCreate error" << query.lastError(); //###########
        }
      else
        {
          str1 = QString(tr("Cannot connect to database"));
          informationDialog->perform(str1,name,1);
        }
      db = QSqlDatabase();                       //destroy db-object
      QSqlDatabase::removeDatabase("con"+name);  //close connection
    }
}
/*************************************************************************//*!
 * SLOT: Popup menu action: Show database info.
 */
void KbvCollectionTreeView::databaseInfo()
{
  QSqlDatabase  db;
  QSqlQuery     query;
  QModelIndex index;
  QString     str, rootDir, connection;
  int         result=0;
 
  index = this->currentIndex();
  dbName = this->collTreeModel->data(index, Kbv::CollectionNameRole).toString();
  dbType = this->collTreeModel->data(index, Kbv::CollectionTypeRole).toInt();
  dbLocation = this->collTreeModel->data(index, Kbv::DatabaseLocationRole).toString();
  rootDir = this->collTreeModel->data(index, Kbv::CollectionRootDirRole).toString();
  //qDebug() << "KbvCollectionTreeView::databaseInfo" <<dbName <<dbType <<dbLocation; //###########

  if((!index.isValid()) || (dbType & Kbv::TypeCollectionAnchor) || (dbType & Kbv::TypeAlbumAnchor) || (dbType == Kbv::TypeNone))
    {
      //Avoid album anchor, collection anchor and damaged databases
      str = QString(tr("Please select a collection or album!"));
      informationDialog->perform(str, QString(), 1);
    }
  else
    {
      //Create database info object and open dialog
      dbInfo = new KbvDBInfo(this);

      if(this->createDBInfo(dbInfo, dbName, dbLocation, rootDir, dbType))
        {
          result = this->dbInfo->exec();
          if(result == QDialog::Accepted)
            {
              //Create database connection and update description
              connection = "info" + dbName;
              db = QSqlDatabase::addDatabase("QSQLITE", connection);
              db.setHostName("host");
              db.setDatabaseName(dbLocation+dbName+QString(dbNameExt));
              if(db.open())
                {
                  str = dbInfo->getDescription(); //text or empty
                  query = QSqlQuery(db);
                  db.transaction();
                  //write table 'description'
                  query.prepare(QString("UPDATE OR REPLACE description SET comment = :comment"));
                  query.bindValue(":comment", QVariant(str));
                  query.exec();
                  db.commit();
                }
              db = QSqlDatabase();                      //destroy db-object
              QSqlDatabase::removeDatabase(connection); //close connection
            }
        }
      delete  dbInfo;
    }
}
/*************************************************************************//*!
 * Fill the database info object *info with data.
 */
bool  KbvCollectionTreeView::createDBInfo(KbvDBInfo *info, const QString name, const QString location,
                                          const QString root, int type)
{
  QSqlDatabase  db;
  QSqlQuery     query;
  QString       connection, ver="", description="";
  int           iconSize=0, keywordType=0;

  //Create database connection and read basic collection settings
  connection = "dbinfo" + name;
  db = QSqlDatabase::addDatabase("QSQLITE", connection);
  db.setHostName("host");
  db.setDatabaseName(location+name+QString(dbNameExt));
  if(!db.open())
    {
      //No connection established
      informationDialog->perform(QString(tr("Cannot connect to database %1")).arg(dbName), QString(), 2);
      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase(connection);   //close connection
      return false;
    }
  //Read database settings from table description
  query = QSqlQuery(db);
  db.transaction();
  query = db.exec("SELECT version, iconSize, comment, keywordtypes FROM description");
  db.commit();
  if(query.isActive())
    {
      while(query.next())
        {
          ver = query.value(0).toString();
          iconSize = (query.value(1).toInt());
          description = query.value(2).toString();
          keywordType = query.value(3).toInt();
        }
    }
  query.finish();

  //Update database info
  info->setDBInfo(type, iconSize, keywordType, ver, name, description, location, root);

  db = QSqlDatabase();                      //destroy db-object
  QSqlDatabase::removeDatabase(connection); //close connection
  return true;
}

/*************************************************************************//*!
 * SLOT: Menu action: Read a collection from file system into database.\n
 * When the current index is not a collection or not valid nothing is done.
 * Reading only is possible when the database is visible. This is checked
 * in kbvCollectionTabs. Title = collection name, type = collection type
 */
void  KbvCollectionTreeView::databaseRead()
  {
    QModelIndex     index;
    QString         title, path, location, root, str1;
    int             type;

    index = this->currentIndex();
    title = this->collTreeModel->data(index, Kbv::CollectionNameRole).toString();
    path = this->collTreeModel->data(index, Kbv::FilePathRole).toString();
    type = this->collTreeModel->data(index, Kbv::CollectionTypeRole).toInt();
    location = this->collTreeModel->data(index, Kbv::DatabaseLocationRole).toString();
    root = this->collTreeModel->data(index, Kbv::CollectionRootDirRole).toString();

    //Avoid album root and collection root, albums and damaged databases
    if((index.isValid()) && (type & Kbv::TypeCollectionRoot))
      {
        //qDebug() << "KbvCollectionTreeView::databaseRead" <<title <<path <<type; //###########
        emit updateDB(title, path, type, location, root);   //->kbvTabsRight->readinOrUpdate()
      }
    else
      {
        str1 = QString(tr("Please select a collection for importing files!"));
        informationDialog->perform(str1,QString(),1);
      }
  }
/*************************************************************************//*!
 * SLOT: Menu action: Refresh a collection or collection branch.
 * Updating only is possible when the database is visible. This is checked
 * in kbvCollectionTabs. Title = collection name, type = collection type
 */
void KbvCollectionTreeView::databaseUpdate()
  {
    QModelIndex index;
    QString     title, path, location, root, dir, str1;
    int         type;

    index = this->currentIndex();
    title = this->collTreeModel->data(index, Kbv::CollectionNameRole).toString();
    path = this->collTreeModel->data(index, Kbv::FilePathRole).toString();
    dir  = this->collTreeModel->data(index, Qt::DisplayRole).toString();
    type = this->collTreeModel->data(index, Kbv::CollectionTypeRole).toInt();
    location = this->collTreeModel->data(index, Kbv::DatabaseLocationRole).toString();
    root = this->collTreeModel->data(index, Kbv::CollectionRootDirRole).toString();

    //Menus 'Read in' and 'Update' are disabled for album and root items
    if(index.isValid())
      {
        if(type & Kbv::TypeCollectionRoot)
          {
            //For collection root the path always is empty
            //qDebug() << "KbvCollectionTreeView::databaseUpdate" <<title <<path <<type; //###########
            emit updateDB(title, path, type, location, root);   //->kbvTabsRight->readinOrUpdate()
          }
        else if(type & Kbv::TypeCollection)
          {
            //Consider empty path on appending "/"
            if(!path.isEmpty())
              {
                path.append("/");
              }
            path.append(dir);
            //qDebug() << "KbvCollectionTreeView::branchUpdate" <<title <<path <<type; //###########
            emit updateDB(title, path, type, location, root);   //->kbvTabsRight->readinOrUpdate()
          }
      }
    else
      {
        str1 = QString(tr("Please select a collection for update!"));
        informationDialog->perform(str1,QString(),1);
      }
  }
/*************************************************************************//*!
 * SLOT: Menu action: Removes a photo album or collection from tree.
 * A related view tab get closed in addition. Database file and collection
 * get not deleted but the config-file will be cleaned.
 */
void KbvCollectionTreeView::databaseRemove()
{
  QModelIndex index;
  QString     title, msg1, msg2;
  int         retval;

  msg1 = QString(tr("Do you really want to remove the database from tree?"));
  msg2 = QString(tr("The database and image files are not deleted!"));
  retval = informationDialog->perform(msg1, msg2, 3);
  if(retval == QMessageBox::Yes)
    {
      index = this->currentIndex();
      title = this->collTreeModel->data(index, Kbv::CollectionNameRole).toString();
      this->collTreeModel->removeDatabase(index);
      emit collRemovedFromTree(title);

      //clear existing entries of removed collection
      settings->beginGroup("CsvChecker");
      settings->remove(title);
      settings->endGroup();
      settings->sync();
    }
}
/*************************************************************************//*!
 * SLOT: Menu action: Open and mount a collection on an arbitrary drive.
 * Called from main menu or popup in file view.
 * The parameter database contains the absolute path + name.ext or is empty.
 * The invisible root item of the collection tree contains two children:
 * the anchor for collections and the anchor for albums. The children of these
 * anchors get saved on shut down and restored on start up.
 * This shall not take place for collections which are opened here. Imarca
 * shall 'forget' them at shut down. This will be performed in the model due
 * to parameter "joined". Note, that in joined databases the collection root
 * is identical to the database location. The type of the database is of
 * TypeCollection.
 */
void KbvCollectionTreeView::databaseOpen(const QString database)
{
  QSqlDatabase  db;
  QSqlQuery     query;
  QString       location, connection, path, name;
  QStandardItem *anchor, *item;
  int           type, n;

  type = Kbv::TypeNone;

  //ask for database file
  if(database.isEmpty())
    {
      location = QFileDialog::getOpenFileName(this, tr("Select collection database file"),
                         QDir::homePath(), QString("*")+QString(dbNameExt));
    }
  else
    {
      location = database;
    }

  //the path of a valid database name must end with dbNameExt
  if(!location.endsWith(QString(dbNameExt)))
    {
      informationDialog->perform(QString(tr("Database name is invalid.")), QString(), 2);
      return;
    }
  else
    {
      //split path into database path and name
      n = location.lastIndexOf("/");
      name = location.right(location.length()-n-1);
      name.remove(QString(dbNameExt));
      path = location.left(n+1);
      //qDebug() << "KbvCollectionTreeView::databaseOpen" <<location <<path <<name; //###########

      //check tree for already existing database of same name
      anchor = this->collTreeModel->collectionAnchor();
      n = anchor->rowCount();
      for(int i=0; i<n; i++)
        {
          item = anchor->child(i, 0);
          if(name == item->data(Kbv::CollectionNameRole).toString())
            {
              informationDialog->perform(QString(tr("Database of same name already exists.\nAbort.")), QString(), 2);
              return;
            }
        }

      //open database and read the type
      connection = "open"+name;
      db = QSqlDatabase::addDatabase("QSQLITE", connection);
      db.setHostName("host");
      db.setDatabaseName(location);     //absolute path + name.ext
      if(db.open())
        {
          query = QSqlQuery(db);
          db.transaction();
          query = db.exec("SELECT colltype FROM description");
          db.commit();
          if(query.first())
            {
              type = query.value(0).toInt();
            }
          //qDebug() << "KbvCollectionTreeView::databaseOpen query" << query.lastError(); //###########
        }
      else
        {
          informationDialog->perform(QString(tr("Cannot connect to database.\nAbort.")), QString(), 2);
          return;
        }

      //mount database in model and activate
      this->collTreeModel->createItem(name, path, type, false, true);
      emit activated(this->collTreeModel->indexFromCollection(name));

      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase(connection);   //close connection
    }
}
/*************************************************************************//*!
 * SLOT: Menu action: Exports a collection to a removable drive. Albums can
 * not be exported. The available databases are read from model where we can
 * find all other parameters to start the thread properly.
 */
void KbvCollectionTreeView::databaseExport()
{
  QStandardItem *anchor, *item;
  QStringList   dblist;
  QString       name, destination, location, root;
  QDir          dir;
  int           result, n;

  //get available database names
  anchor = this->collTreeModel->collectionAnchor();
  n = anchor->rowCount();
  for(int i=0; i<n; i++)
    {
      item = anchor->child(i, 0);         //collection root item = db name
      dblist.append(item->data(Kbv::CollectionNameRole).toString());
    }
  //qDebug() << "KbvCollectionTreeView::databaseExport names" <<dblist; //###########

  //Open dialog
  result = dbExportDialog->perform(dblist);

  if(result == QDialog::Accepted)
    {
      name = dbExportDialog->databaseName();        //name without extension
      destination = dbExportDialog->destination();
      //Create destination directory where appropriate
      if(!dir.exists(destination))
        {
          if(!dir.mkpath(destination))
            {
              informationDialog->perform(QString(tr("Cannot create destination directory:")), destination, 2);
              return;
            }
        }
      n = dblist.indexOf(name);
      item = anchor->child(n, 0);
      location = item->data(Kbv::DatabaseLocationRole).toString();
      root = item->data(Kbv::CollectionRootDirRole).toString();
      //qDebug() << "KbvCollectionTreeView::databaseExport"<<name <<location <<root <<destination; //###########

      dbImExProgress->setTitle(QString(tr("Export collection %1")).arg(name),  Kbv::dbexport);
      dbImExProgress->setInfotext("");
      dbImExProgress->setProgress(0);
      dbImExProgress->show();
      dbImExThread->start(name, location, root, destination, Kbv::dbexport);
    }
}
/*************************************************************************//*!
 * SLOT: Menu action: Imports a collection from removable drive or CD/DVD.
 * The parameter database contains the absolute path + name.ext or is empty.
 */
void KbvCollectionTreeView::databaseImport()
{
  QStandardItem *anchor, *item;
  QString   sourcedb, destination, name, path;
  QDir      dir;
  int       result, n;

  result = dbImportDialog->exec();

  if(result == QDialog::Accepted)
    {
      sourcedb = dbImportDialog->databaseName();  //absolute path, name.ext
      if(!sourcedb.endsWith(QString(dbNameExt)))
        {
          informationDialog->perform(QString(tr("Database name is invalid.\nAbort.")), "", 1);
          return;
        }
      else
        {
          //split path into database path and name
          n = sourcedb.lastIndexOf("/");
          name = sourcedb.right(sourcedb.length()-n-1);
          name.remove(QString(dbNameExt));
          path = sourcedb.left(n+1);        //ends with "/"
        }

      //get available database names
      anchor = this->collTreeModel->collectionAnchor();
      n = anchor->rowCount();
      for(int i=0; i<n; i++)
        {
          item = anchor->child(i, 0);         //collection root item = db name
          if(name == item->data(Kbv::CollectionNameRole).toString())
            {
              informationDialog->setCaptionAndIcon(QString(tr("Database of same name already exists.\nAbort.")), "", 2);
              informationDialog->exec();
              return;
            }
        }

      //Create destination directory where appropriate
      destination = dbImportDialog->destination();  //absolute path without "/"
      if(!dir.exists(destination))
        {
          if(!dir.mkpath(destination))
            {
              informationDialog->perform(QString(tr("Cannot create destination directory:")), destination, 2);
              return;
            }
        }

      dbImExProgress->setTitle(QString(tr("Import collection %1")).arg(name),  Kbv::dbimport);
      dbImExProgress->setInfotext("");
      dbImExProgress->setProgress(0);
      dbImExProgress->show();

      //qDebug() << "KbvCollectionTreeView::databaseImport" <<name <<path <<destination; //###########
      dbImExThread->start(name, path, path, destination, Kbv::dbimport);
    }
}
/*************************************************************************//*!
 * SLOT: Signal fron import/export thread: finished.
 * flag indicates the reason: noFlag=faultless. All other flags display a text
 * in the progress dialog:
 */
void KbvCollectionTreeView::databaseImExportFinished(int flag)
{
  QString str;
  QString textflag1 = QString(tr("The source database cannot be opened.\nAbort operation."));
  QString textflag2 = QString(tr("The copied database cannot be opened.\nAbort operation."));
  QString textflag3 = QString(tr("Operation aborted."));
  QString textflag4 = QString(tr("Operation finished successfully."));
  QString textflag5 = QString(tr("Operation finished with faults.\nSee report."));
  QString textflag6 = QString(tr("Space on destination not sufficient.\nAbort operation."));
  QString textflag7 = QString(tr("This is an album.\nOnly collections can be exported.\nAbort operation."));

  //qDebug() << "KbvCollectionTreeView::databaseImExportFinished" <<flag; //###########
  switch(flag)
    {
    case Kbv::flag1:  str = textflag1;
      break;
    case Kbv::flag2:  str = textflag2;
      break;
    case Kbv::flag3:  str = textflag3;
      break;
    case Kbv::flag4:  str = textflag4;
      break;
    case Kbv::flag5:  str = textflag5;
      break;
    case Kbv::flag6:  str = textflag6;
      break;
    case Kbv::flag7:  str = textflag7;
      break;
    default: str.clear();
    }
  dbImExProgress->setInfotext(QString(str));
}
/*************************************************************************//*!
 * SLOT: Signal from import/export thread: setDB(name). Create a database
 * item in collection tree when the import thread finished.
 */
void KbvCollectionTreeView::databaseImExportsetDB(QString name)
{
  //append new collection to collection tree, empty root -> read from db
  //qDebug() << "KbvCollectionTreeView::databaseImExportSetDB" <<name; //###########
  this->collTreeModel->createItem(name, QString(), Kbv::TypeCollectionRoot, false, false);
}
/************************************************************************//*!
 * SLOT: a dynamically mounted device has been unmounted/removed.
 * The mountInfo contains two strings: label and mountPoint
 * Remove the database from tree and close the collection tab if a joined
 * connection was located under the mount point.
 */
void KbvCollectionTreeView::databaseUnmount(QStringList mountInfo)
{
  QString           path, title;
  QStandardItem     *anchor, *item;
  int               type;

  //check all collection root items for this mount point
  //start at last row since this will not crash when a row was removed
  anchor = this->collTreeModel->collectionAnchor();
  for(int i=anchor->rowCount()-1; i>=0; i--)
    {
      item = anchor->child(i, 0);
      path = item->data(Kbv::DatabaseLocationRole).toString();
      type = item->data(Kbv::CollectionTypeRole).toInt();
      //qDebug() << "KbvCollectionTreeView::unmountDevice label mountPoint dbPath" <<mountInfo.at(0) <<mountInfo.at(1) <<path; //###########
      if(path.startsWith(mountInfo.at(1)) && (type & Kbv::TypeJoined))
        {
          title = item->data(Kbv::CollectionNameRole).toString();
          this->collTreeModel->removeDatabase(item->index());
          emit collRemovedFromTree(title);
        }
    }
}
/*************************************************************************//*!
 * SLOT: Menu action: Deletes a collection branch. The branch must be empty.
 */
void KbvCollectionTreeView::collDelete()
  {
  this->collTreeModel->deleteItem(this->currentIndex());
  }
/*************************************************************************//*!
 * SLOT: Menu action: Inserts a new empty collection branch below the
 * selected item.  When created, ask for icon size, type and description.
 * The type returned either is Kbv::TypeAlbum or Kbv::TypeCollection.
 */
void KbvCollectionTreeView::collInsert()
{
  QModelIndex parent;
  QString     childName, msg;
  int         type;
  bool        dialogOK;

  parent = this->currentIndex();
  type = this->collTreeModel->data(parent, Kbv::CollectionTypeRole).toInt();
  //qDebug() << "KbvCollectionTreeView::collInsert" << parent << type; //###########

  //Avoid collection anchor, album anchor, albums and damaged databases
  if((parent.isValid()) && ((type & Kbv::TypeCollection) || (type & Kbv::TypeCollectionRoot)))
    {
      //get name of new directory
      childName = QInputDialog::getText(this, tr("Insert collection branch"),
                                         tr("Please enter a name"),
                                         QLineEdit::Normal, "",  &dialogOK);
      if (dialogOK && !childName.isEmpty())
        {
          //insert item below parentName (index "parent")
          this->collTreeModel->insertItem(parent, childName);
        }
    }
  else
    {
      msg = QString(tr("Please select a collection!"));
      informationDialog->perform(msg, QString(), 1);
    }
}
/*************************************************************************//*!
 * SLOT: Menu action: Rename photo album or collection or collection branch.\n
 * Renaming only is accomplished when the new name is unique. Two photo albums
 * or collections with same name are not possible. When renaming is performed
 * a signal is sent to collectionTabs updating the tab name and the tree is
 * collapsed. Latter is necessary since the expanded items would contain a
 * wrong collection name.
 */
void KbvCollectionTreeView::collRename()
{
  QModelIndex   index, parent;
  QString       newName, oldName;
  QString       msg1, msg2, msg3;
  bool          ok;
  int           type, parentType;

  msg1 = QString(tr("The name already exists!"));
  msg2 = QString(tr("Please choose another one."));
  msg3 = QString(tr("Renaming the collection or branches of it may take some time!\n"
                    "Don't close the collection meanwhile!\n\n"
                    "New name:"));

  index = this->currentIndex();

  //do not try to rename the anchor items or a damaged database
  type = this->collTreeModel->data(index, Kbv::CollectionTypeRole).toInt();
  //qDebug() << "KbvCollectionTreeView::collRename type" <<type; //###########

  if((type & Kbv::TypeAlbumAnchor) || (type & Kbv::TypeCollectionAnchor) || (type == Kbv::TypeNone))
    {
      return;
    }
  //get actual name and new name
  oldName = (this->collTreeModel->data(index, Qt::DisplayRole)).toString();
  newName = QInputDialog::getText(this, QString(tr("Rename")), msg3,
                                  QLineEdit::Normal, QString(oldName), &ok);

  //do nothing when new = old
  if(newName == oldName || !ok)
    {
      return;
    }

  //check item at index "parent" for children with same new name
  parent = index.parent();
  parentType = this->collTreeModel->data(parent, Kbv::CollectionTypeRole).toInt();
  if(this->collTreeModel->childNameExists(newName, parentType, parent))
    {
      informationDialog->setCaptionAndIcon(msg1,msg2,1);
      informationDialog->exec();
      return;
    }

  //All ok, try to rename collection or branch
  //Collection: close related tab since all objects using the collection name cease to work
  //Branch: collapse the parent of the concerned branch since all paths are wrong
  //at last adjust view (destroys index)
  //qDebug() << "KbvCollectionTreeView::collRename" <<index <<type <<oldName <<newName; //###########
  this->collTreeModel->renameDatabase(index, oldName, newName);
  if((type & Kbv::TypeCollectionRoot) || (type & Kbv::TypeAlbumRoot))
    {
      this->collapse(index);                //collapse to root
      emit collRemovedFromTree(newName);    //close renamed collection tab
    }
  if(type & Kbv::TypeCollection)
    {
      this->collapse(parent);
    }
  this->resizeColumnToContents(0);
}
/*************************************************************************//*!
 * Check if directory for storing databases is already set and valid.
 */
bool    KbvCollectionTreeView::databaseDirExists(QString path)
  {
    QFileInfo   info;

    info = QFileInfo(path);
    return  (info.isDir() && info.isReadable() && info.isWritable());
  }
/*************************************************************************//*!
 * Enable menu collection when collection tab is active.
 */
void    KbvCollectionTreeView::enableMenu(int index)
  {
  if(index == KbvConf::CollTreeTab)
    {
      mainMenu->menuCollection->setEnabled(true);
      mainMenu->dbToolBar->setEnabled(true);
    }
  else
    {
      mainMenu->menuCollection->setEnabled(false);
      mainMenu->dbToolBar->setEnabled(false);
    }
  }
/*************************************************************************//*!
 * Popup menu for photo album/collection operations.
 */
void    KbvCollectionTreeView::contextMenuEvent(QContextMenuEvent *event)
  {
    popupCollection->exec(event->globalPos());
  }
/*************************************************************************//*!
 * Creates the collection menu and pop up menu and establishes all signals.
 */
void KbvCollectionTreeView::createPopUp()
  {
    //Menu: Collection
  actDBInfo = new QAction(tr("Database info"), this);
  actDBInfo->setStatusTip(tr("Show settings of the database."));
  actDBCreate = new QAction(tr("New database"), this);
  actDBCreate->setStatusTip(tr("Create a new empty database."));
  actDBRemove = new QAction(tr("Remove database"), this);
  actDBRemove->setStatusTip(tr("Removes the selected database from tree. No file will be deleted."));
  actDBRead = new QAction(tr("Read"), this);
  actDBRead->setStatusTip(tr("Read a collection from file system into database."));
  actDBUpdate = new QAction(tr("Update"), this);
  actDBUpdate->setStatusTip(tr("Scan the collection or branch and update the database."));
  actRename = new QAction(tr("Rename"), this);
  actRename->setStatusTip(tr("Rename the selected item."));
  actInsert = new QAction(tr("Insert branch"), this);
  actInsert->setStatusTip(tr("Insert a new empty collection branch below the selected item."));
  actDelete = new QAction(tr("Delete branch"), this);
  actDelete->setStatusTip(tr("Delete the selected collection branch. The branch must be empty."));

  popupCollection = new QMenu(this);
  popupCollection->addAction(actDBInfo);
  popupCollection->addAction(actDBCreate);
  popupCollection->addAction(actDBRemove);
  popupCollection->addSeparator();
  popupCollection->addAction(actDBRead);
  popupCollection->addAction(actDBUpdate);
  popupCollection->addSeparator();
  popupCollection->addAction(actRename);
  popupCollection->addAction(actInsert);
  popupCollection->addSeparator();
  popupCollection->addAction(actDelete);

  connect(actDBInfo,    SIGNAL(triggered()), this, SLOT(databaseInfo()));
  connect(actDBCreate,  SIGNAL(triggered()), this, SLOT(databaseCreate()));
  connect(actDBRemove,  SIGNAL(triggered()), this, SLOT(databaseRemove()));
  connect(actDBUpdate,  SIGNAL(triggered()), this, SLOT(databaseUpdate()));
  connect(actDBRead,    SIGNAL(triggered()), this, SLOT(databaseRead()));
  connect(actRename,    SIGNAL(triggered()), this, SLOT(collRename()));
  connect(actInsert,    SIGNAL(triggered()), this, SLOT(collInsert()));
  connect(actDelete,    SIGNAL(triggered()), this, SLOT(collDelete()));
}

/****************************************************************************/
