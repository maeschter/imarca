/*****************************************************************************
 * kvb file model
 * (C): G. Trauth, Erlangen
 * LastChanged: 2018-09-16
 * Created: 2011.01.27
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvInformationDialog.h"
#include "kbvReplaceDialog.h"
#include "kbvFileModel.h"

extern  KbvInformationDialog    *informationDialog;
extern  KbvReplaceDialog        *replaceDialog;

KbvFileModel::KbvFileModel(QObject *parent) : QAbstractListModel(parent)
{
  dirWatchCycle = 2000;
  iconSize = QSize(150, 150);
  viewMode = QListView::IconMode;
  lock = false;
  isRunning = false;
  restart = false;
  rootDir = "";

  fileModelThread = new KbvFileModelThread(nullptr);
  connect(fileModelThread, SIGNAL(newItem(Kbv::kbvItem*)), this, SLOT(appendItem(Kbv::kbvItem*)));
  connect(fileModelThread, SIGNAL(statusText2(QString)),   this, SLOT(statusText2(QString)));
  connect(fileModelThread, SIGNAL(threadFinished()),       this, SLOT(fillThreadFinished()));

  fileWatchThread = new KbvFileWatcherThread(nullptr);
  connect(fileWatchThread, SIGNAL(newItem(Kbv::kbvItem*)), this, SLOT(appendItem(Kbv::kbvItem*)));
  connect(fileWatchThread, SIGNAL(invalidItem(int)),       this, SLOT(invalidateItem(int)));
  connect(fileWatchThread, SIGNAL(removeEmptyItems()),     this, SLOT(removeEmptyItems()));
  connect(fileWatchThread, SIGNAL(replaceItem(int, Kbv::kbvItem*)), this, SLOT(replaceItem(int,Kbv::kbvItem*)));
  connect(fileWatchThread, SIGNAL(threadFinished()),       this, SLOT(watchThreadFinished()));
  
  dirMonitor = new QFileSystemWatcher(nullptr);
  connect(dirMonitor, SIGNAL(directoryChanged(QString)),   this, SLOT(startFileWatchThread(QString)));
}
KbvFileModel::~KbvFileModel()
{
  //qDebug() << "KbvFileModel::~KbvFileModel";//###########
  delete fileModelThread;
  delete fileWatchThread;
  delete dirMonitor;
  while(itemList.size()>0)
    {
      delete itemList.takeAt(0);
    }
}
/*************************************************************************//*!
 * Toggle view mode between icon mode (default) and list mode.
 */
void KbvFileModel::setViewMode(int mode)
{
  this->viewMode = mode;
}
/*************************************************************************//*!
 * SLOT: update info buttons.
 */
void    KbvFileModel::statusText1(QString text)
{
  emit infoText1(text);
}
void    KbvFileModel::statusText2(QString text)
{
  emit infoText2(text);
}
void    KbvFileModel::statusText3(QString text)
{
  emit infoText3(text);
}
/****************************************************************************
 */
Qt::DropActions KbvFileModel::supportedDropActions() const
{
  return (Qt::CopyAction | Qt::MoveAction);
}
/****************************************************************************
 */
Qt::DropActions KbvFileModel::supportedDragActions() const
{
  return (Qt::CopyAction | Qt::MoveAction);
}
/************************************************************************//*!
 * Drop or paste from an application onto file view or onto dirView.
 * The latter calls:
 *   dirView::dropEvent -> fileModel::dropMimeData
 * When the target dir is displayed (drop on dirTree/drop on fileView!) the
 * model must be adjusted due to copied/moved files.
 * We extract urls or file paths from mime data and copy or move the files
 * (no dirs!!) from source to target. The file watcher thread first gets
 * stopped and started again, when all file operations have been done.
 * Before doing something we check if the target is a directory and writable.
 * Hence we can copy files but it may be that an existing file cannot be
 * removed. These errors get collected and displayed finally.
 */
bool    KbvFileModel::dropData(const QMimeData *mimeData, Qt::DropAction action,
                                  const QString targetDir, bool visible, bool paste)
{
  Kbv::kbvItem  *item;
  QFileInfo     fileinfo;
  QStringList   pathlist;
  QString       msg, targetpath, newpath, name, errorlist;
  int           i, n, retval, next;
  bool          yesall = false, yes = false, no = false;

  
  //Get target path
  if(targetDir.isEmpty())
    {
      targetpath = this->rootDir + "/";
    }
  else
    {
      targetpath = targetDir + "/";
    }
  //qDebug() << "KbvFileModel::dropMimeData target visible action" <<targetpath<<visible<<action; //###########

  //Check if target is dir and writable
  fileinfo = QFileInfo(targetpath);
  if(!fileinfo.isDir() || !fileinfo.isWritable())
    {
      msg = QString(tr("The target either is no directory or write protected."));
      informationDialog->perform(msg,QString(),1);
      return false;
    }

  //Extract a list of absolute file paths from mime data (urls or text)
  pathlist.append(globalFunc.dropExtractMimeData(mimeData));
  //qDebug() << "KbvFileModel::dropMimeData files"<<pathlist.count(); //###########

  n = pathlist[0].lastIndexOf("/", -1, Qt::CaseInsensitive);
  newpath = pathlist[0].left(n+1);

  //no drag-drop but copy-paste into the same dir
  if((targetpath == newpath) && !paste)
    {
      return false;
    }

  //disable sorting and dirMonitor
  emit enableSorting(false);
  this->enableDirMonitor(false);

  //Now we copy or move the files in path list to the destination and collect
  //all errors in errorlist.
  //first append empty items to the model when visible
  next = this->itemList.size();
  if(visible)
    {
      this->insertEmptyItems(next, pathlist.size());
    }
  for(i=0; i<pathlist.size(); i++)
    {
      //create path + name of file to copy
      n = pathlist[i].size() - pathlist[i].lastIndexOf("/", -1, Qt::CaseInsensitive);
      name = pathlist[i].right(n-1);
      newpath = targetpath + name;
      //qDebug() << "KbvFileModel::dropMimeData file no" <<targetpath+name <<i; //###########

      //check for already existing files
      if(QFile::exists(newpath))
        {
          if(paste)
            {
              //cut/copy/paste: copy file to destination and append "copy" to filename
              n = name.indexOf(".", 0, Qt::CaseInsensitive);
              if(n < 0)
                {
                  name.append(" copy");
                }
              else
                {
                  name.insert(n, " copy");
                }
              QFile::copy(pathlist[i], targetpath+name);
              //find existing item and replace it when visible (drop on dirTree!)
              if(visible)
                {
                  item = globalFunc.itemFromFile(targetpath, name, iconSize);
                  this->replaceItem(next+i, item);
                }
              //qDebug() << "KbvFileModel::dropMimeData duplicate" <<targetpath+name <<next+i; //###########
            }
          else
            {
              //drag/drop: ask user for replacing existing file
              if(!yesall)
                {
                  retval = replaceDialog->perform(newpath, pathlist[i]);
                  switch (retval)
                  {
                    case QDialogButtonBox::YesToAll: { yesall = true; }
                    break;
                    case QDialogButtonBox::Yes: { yes = true; }
                    break;
                    case QDialogButtonBox::No: { no = true; }
                    break;
                    default: { return false; }  //cancel
                  }
                }
              //remove existing file on YES or YESALL, do nothing on NO
              if(yesall | yes)
                {
                  //qDebug() << "KbvFileModel::dropMimeData overwrite" << name; //###########
                  if(QFile::remove(newpath))
                    {
                      QFile::copy(pathlist[i], newpath);
                      //find existing item and replace it when visible (drop on dirTree!)
                      if(visible)
                        {
                          item = globalFunc.itemFromFile(targetpath, name, iconSize);
                          n = this->findItem(QVariant(name), Qt::DisplayRole);
                          this->replaceItem(n, item);
                          //qDebug() << "KbvFileModel::dropMimeData replace existing" << name << n; //###########
                        }
                    }
                  else
                    {
                      errorlist.append(pathlist[i]);
                      errorlist.append("\n");
                    }
                }
            }
        }
      else
        {
          //file doesn't exist in target dir:
          //copy file and replace next empty item when visible (drop on dirTree!)
          QFile::copy(pathlist[i], newpath);
          //qDebug() << "KbvFileModel::dropMimeData copy file" << name; //###########
          if(visible)
            {
              item = globalFunc.itemFromFile(targetpath, name, iconSize);
              this->replaceItem(next+i, item);
            }
        }

      //Handle source files:
      //Remove only when files were moved and overwriting was permitted (move action & !no).
      //Do nothing on drag/copy or copy/paste.
      if((action == Qt::MoveAction) & !no)
        {
          //qDebug() << "KbvFileModel::dropMimeData remove source" << pathlist[i]; //###########
          if(!QFile::remove(pathlist[i]))
            {
              errorlist.append(pathlist[i]);
              errorlist.append("\n");
            }
        }
      yes = false;
      no = false;
      //qDebug() << "KbvFileModel::dropMimeData next file"; //###########
    }
  //qDebug() << "KbvFileModel::dropMimeData remove empty items"; //###########
  //remove remaining empty items in target
  this->removeEmptyItems();

  //all files copied or moved: display what was wrong
  if(!errorlist.isEmpty())
    {
      msg = QString(tr("These files couldn't be moved."));
      informationDialog->perform(msg,errorlist,1);
    }
  this->statusText1(QString("%1").arg(itemList.size(), 6));
  this->statusText2(QString(""));

  //enable sorting and dirMonitor
  emit enableSorting(true);
  this->enableDirMonitor(true);

  return true;
}
/*************************************************************************//*!
 * Drag move has accomplished.
 * The drop event moved the files to the target. When the move action failed
 * or was cancelled the file is still present otherwise it has been removed.
 * Cleaning after drop means: remove all model items related to the removed
 * source files (drop move successful)
 * Removing the model item is done by setting the value of Kbv::FilePathRole
 * as invalid then calling the function removeEmptyItems().
 * The file watcher thread already has been stopped.
 */
void    KbvFileModel::dragMoveCleanUp(const QList<QPersistentModelIndex> &idxList)
{
  QString   path, name;
  QFile     file;
  
  //qDebug() << "KbvFileModel::dragMoveCleanUp index count" <<idxList.count(); //###########
  emit enableSorting(false);
  
  for (int i=0; i<idxList.count(); ++i)
    {
      if(idxList.at(i).isValid())
        {
          path = this->data(idxList.at(i), Kbv::FilePathRole).toString();
          name = this->data(idxList.at(i), Kbv::FileNameRole).toString();

          if(!file.exists(path+name))
            {
              this->setData(idxList.at(i), QVariant(), Kbv::FilePathRole);
            }
        }
    }
  this->removeEmptyItems();
  this->statusText1(QString("%1").arg(itemList.size(), 6));
  
  //qDebug() << "KbvFileModel::dragMoveCleanUp finished"; //###########
  emit enableSorting(true);
}
/*************************************************************************//*!
 * Cut/paste has been started by ctrl-x key event.
 * Remove all cut items by setting the value of Kbv::FilePathRole as invalid
 * then call the function removeEmptyItems().
 * Removing files and records is done in the ctrl-v key event.
 */
void    KbvFileModel::ctrlXCleanUp(const QModelIndexList &idxList)
{
  for (int i=0; i<idxList.count(); ++i)
    {
      if(idxList[i].isValid())
        {
          this->setData(idxList[i], QVariant(), Kbv::FilePathRole);
        }
    }
  this->removeEmptyItems();
  //qDebug() << "KbvFileModel::ctrlXCleanUp finished" <<idxList.count(); //###########
}

/*************************************************************************//*!
 * This function returns values due to roles the view is asking for.
 * Tool tips are created on the fly for tooltip role.
 * Item structure (data is returned as QVariant):\n
 * QString   - Qt::DisplayRole\n
 * icon      - Qt::decorationRole\n
 * QString   - Kbv::FileNameRole\n
 * QString   - Kbv::FilePathRole\n
 * qint64    - Kbv::FileSizeRole\n
 * QDateTime - Kbv::FileDateRole\n
 * QSize     - Kbv::ImageDimRole\n
 * quint64   - Kbv::ImageSizeRole\n
 * QString   - Qt::ToolTipRole\n
 */
QVariant KbvFileModel::data(const QModelIndex &index, int role) const
{
  Kbv::kbvItem *item;

  //qDebug() << "KbvFileModel::data row length valid" << index.row() << itemList.length() << index.isValid(); //###########
  if (!index.isValid() || index.column() < 0)
    {
      return QVariant();
    }
  item = itemList.at(index.row());

  if (role == Qt::DisplayRole)
    {
      return globalFunc.displayRoleData(item, viewMode);
    }
  if (role == Qt::DecorationRole)
    {
      if(viewMode == QListView::IconMode)
        {
          return item->value(Qt::DecorationRole);
        }
      else
        {
          return QVariant(generalFunc.pixMimeImage);
        }
    }
  if (role == Kbv::FilePathRole)
    {
      return item->value(Kbv::FilePathRole);
    }
  if (role == Kbv::FileNameRole)
    {
      return item->value(Kbv::FileNameRole);
    }
  if (role == Kbv::FileSizeRole)
    {
      return item->value(Kbv::FileSizeRole);
    }
  if (role == Kbv::FileDateRole)
    {
      return item->value(Kbv::FileDateRole);
    }
  if (role == Kbv::ImageSizeRole)
    {
      return item->value(Kbv::ImageSizeRole);
    }
  if (role == Kbv::ImageDimRole)
    {
      return item->value(Kbv::ImageDimRole);
    }
  if (role == Qt::ToolTipRole)
    {
      return globalFunc.tooltip(item);
    }
  return QVariant();    //All unused roles
}
/*************************************************************************//*!
 * This function inserts data into the model.\n
 * Item structure required:\n
 * QString   - Qt::DisplayRole\n
 * icon      - Qt::decorationRole\n
 * QString   - Kbv::FileNameRole\n
 * QString   - Kbv::FilePathRole\n
 * qint64    - Kbv::FileSizeRole\n
 * QDateTime - Kbv::FileDateRole\n
 * QSize     - Kbv::ImageDimRole\n
 * quint64   - Kbv::ImageSizeRole\n
 */
bool KbvFileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  Kbv::kbvItem *item;

  //qDebug() << "KbvFileModel::setData" << index.row(); //###########
  if (!index.isValid() || index.column() < 0)
    {
      return false;
    }
  item = itemList.at(index.row());
  if(role == Qt::DisplayRole)
    {
      item->insert(Qt::DisplayRole, value);
    }
  if(role == Qt::DecorationRole)
    {
      item->insert(Qt::DecorationRole, value);
    }
  if(role == Kbv::FilePathRole)
    {
      item->insert(Kbv::FilePathRole, value);
    }
  if(role == Kbv::FileNameRole)
    {
      item->insert(Kbv::FileNameRole, value);
    }
  if(role == Kbv::FileSizeRole)
    {
      item->insert(Kbv::FileSizeRole, value);
    }
  if(role == Kbv::FileDateRole)
    {
      item->insert(Kbv::FileDateRole, value);
    }
  if(role == Kbv::ImageSizeRole)
    {
      item->insert(Kbv::ImageSizeRole, value);
    }
  if(role == Kbv::ImageDimRole)
    {
      item->insert(Kbv::ImageDimRole, value);
    }
  emit(dataChanged(index, index));
  return true;
}

/*************************************************************************//*!
 * This function sets items of first column (col0) enabled, selectable
 * and ready for drag.
 * All other contain the flags provided by base class.
 */
Qt::ItemFlags KbvFileModel::flags(const QModelIndex &index) const
{
  if (index.isValid() && index.column() == 0)
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    }
  return  QAbstractListModel::flags(index);
}
/*************************************************************************//*!
 * SLOT: Remove files in filelist from model and directory.
 * First find the model index of each file then call removeSelection().
 * Note: filelist can contain multiple values (names) for one key (path).
 */
void  KbvFileModel::removeFiles(QMap<QString, QString> filelist)
{
  QModelIndexList   idxList;
  QModelIndex       index;
  QStringList       values;
  QString           str;
  
  for(int i=0; i<itemList.length(); i++)
    {
      //find key (file path) of item in filelist
      if(filelist.contains(itemList.at(i)->value(Kbv::FilePathRole).toString()))
        {
          //find value (file name) of item in filelist
          str = itemList.at(i)->value(Kbv::FilePathRole).toString();
          values = filelist.values(str);
          if(values.contains(itemList.at(i)->value(Kbv::FileNameRole).toString()))
            {
              //qDebug() << "KbvFileModel::removeFiles name" <<itemList.at(i)->value(Kbv::FileNameRole).toString(); //###########
              //item of filelist found, construct model index
              index = this->createIndex(i, 0, this);
              idxList.append(index);
            }
        }
    }
  //qDebug() << "KbvFileModel::removeFiles index" <<idxList; //###########
  this->removeSelection(idxList);
}

/*************************************************************************//*!
 * SLOT: Remove selected files from model and directory.
 * First stop dirMonitor and start it again when finished. To keep
 * the model index valid we delete in two steps:\
 * - 1a When dust bin provided:
 *      move files to dust bin and remove data from appropriate items
 * - 1b When no dust bin available:
 *      remove files from file system and remove data from appropriate items
 * - 2  remove all empty model items and display list of all files that
 *      couldn't be removed
 * All model indexes must have been mapped from sort model to file model!
 * A item is empty when the value of Kbv::FilePathRole is set invalid.
 */
void  KbvFileModel::removeSelection(QModelIndexList indexList)
{
  QModelIndexList   idxList;
  QString           name, path, nameList, msg1, msg2;
  QFile             file;
  int               i;

  //qDebug() << "KbvFileModel::removeSelection start"; //###########
  this->statusText3(QString(tr("Deleting files. Please wait.")));
  this->statusText2(QString("%1").arg(idxList.size(), 6));
  QCoreApplication::processEvents(QEventLoop::AllEvents, 10);

  //Stop dirMonitor
  this->enableDirMonitor(false);
  idxList = indexList;
  nameList.clear();

  //no dustbin
  if (!generalFunc.validTrashDir())
    {
      msg1 = QString(tr("Warning"));
      msg2 = QString(tr("No dust bin available to move files!\n"
                        "Files are deleted from file system!"));
      informationDialog->perform(msg1, msg2, 1);

      for (i = 0; i < idxList.count(); ++i)
        {
          name = this->data(idxList[i], Kbv::FileNameRole).toString();
          path = this->data(idxList[i], Kbv::FilePathRole).toString();
          file.setFileName(path+name);
          if(file.remove())
            {
              //qDebug() << "KbvFileModel::removeSelection not to trash" << name; //###########
              this->setData(idxList[i], QVariant(), Kbv::FilePathRole);
            }
          else
            {
              nameList.append(name + "\n");
            }
        }
      this->removeEmptyItems();
    }
  else
    {
      //Move to dust bin
      //qDebug() << "KbvFileModel::removeSelection to trash"; //###########
      for (i = 0; i < idxList.count(); ++i)
        {
          name = this->data(idxList[i], Kbv::FileNameRole).toString();
          path = this->data(idxList[i], Kbv::FilePathRole).toString();
          if(generalFunc.moveToTrash(path, name))
            {
              //qDebug() << "KbvFileModel::removeSelection to trash" << name; //###########
              this->setData(idxList[i], QVariant(), Kbv::FilePathRole);
            }
          else
            {
              //Couldn't be moved to dust bin (maybe already inside - multiple deletion)
              //-> remove file
              file.setFileName(path+name);
              if(file.remove())
                {
                  this->setData(idxList[i], QVariant(), Kbv::FilePathRole);
                }
              else
                {
                  nameList.append(name + "\n");
                }
            }
        }
      this->removeEmptyItems();
    }
  //display all files which couldn't be removed
  if (!nameList.isEmpty())
    {
      msg1 = QString(tr("Remove files"));
      msg2 = QString(tr("File not removed:\n %1").arg(nameList));
      informationDialog->perform(msg1, msg2, 1);
    }
  this->statusText1(QString("%1").arg(itemList.size(), 6));
  this->statusText2(QString(""));
  this->statusText3(QString(""));

  //qDebug() << "KbvFileModel::removeSelection end"; //###########
  //Activate dirMonitor again
  enableDirMonitor(true);
}
/*************************************************************************//*!
 * This function searches the position of an item characterised by role and
 * value. Returns the index in the list or -1 if search fails.
 */
int    KbvFileModel::findItem(const QVariant &value, int role)
{
  for(int i=0; i<itemList.length(); i++)
    {
      if(itemList.at(i)->value(role) == value)
        {
          return i;
        }
    }
  return -1;
}
/*************************************************************************//*!
 * SLOT: This removes all items from model were Kbv::FilePathRole is empty.
 * An item is empty when no valid data for Kbv::FilePathRole can be found.
 * Run a while loop over the itemList. When an empty item was found at actual
 * position, remove it, otherwise increase counter.
 */
void    KbvFileModel::removeEmptyItems()
{
  int k = 0;
  //qDebug() << "KbvFileModel::removeEmptyItems start"; //###########
  
  while (k < itemList.count())
    {
      if (!itemList.at(k)->value(Kbv::FilePathRole).isValid())
        {
          emit beginRemoveRows(QModelIndex(), k, k);
          delete itemList.takeAt(k);
          emit endRemoveRows();
        }
      else
        {
          k++;
        }
    }
  //qDebug() << "KbvFileModel::removeEmptyItems end"; //###########
}
/*************************************************************************//*!
 * SLOT: This removes the item at position. Used by file watcher thread.
 */
void    KbvFileModel::removeItem(int position)
{
  //qDebug() << "KbvFileModel::removeItem at" << position; //###########
  emit beginRemoveRows(QModelIndex(), position, position);
  delete itemList.takeAt(position);
  emit endRemoveRows();
}
/*************************************************************************//*!
 * SLOT: Sets the item at row invalid by clearing filePathRole.
 * Used by file watcher thread.
 */
void    KbvFileModel::invalidateItem(int row)
{
  QModelIndex index;

  //qDebug() << "KbvFileModel::removeItem at" << position; //###########
  if((row<0) || (row>itemList.length()))
    {
      return;
    }
  index = this->index(row, 0, QModelIndex());
  this->setData(index, QVariant(), Kbv::FilePathRole);
}
/*************************************************************************//*!
 * SLOT: This replaces the item at position. Used by watch thread.
 * Signal layout changed is necessary to adjust the view properly.
 */
void    KbvFileModel::replaceItem(int position, Kbv::kbvItem *item)
{
  Kbv::kbvItem *oldItem;

  if((position<0) || (position>itemList.length()))
    {
      return;
    }
  //qDebug() << "KbvFileModel::replaceItem listsize position" << itemList.size() << position; //###########
  oldItem = itemList.at(position);
  emit layoutAboutToBeChanged();
  itemList.replace(position, item);
  emit layoutChanged();
  delete oldItem;
}
/*************************************************************************//*!
 * This inserts "count" new rows starting at row "position" into "itemList".
 * This is used when a new dir is selected and the model is filled with items.
 * Inserting new rows one after the other let the view flicker so we have to
 * insert empty items and replace them by the fill thread (insertItem()).
 */
void    KbvFileModel::insertEmptyItems(int position, int count)
{
  Kbv::kbvItem *emptyItem;

  //qDebug() << "KbvFileModel::insertEmptyItems" << position << count; //###########
  emit beginInsertRows(QModelIndex(), position, position+count);
  for(int i=0; i<count; i++)
    {
      emptyItem = new Kbv::kbvItem;
      itemList.insert(position+i, emptyItem);
    }
  emit endInsertRows();
}
/*************************************************************************//*!
 * This inserts a new item "*item" at position "position" into "itemList".
 * This is used when a new dir is selected and the model is filled with items.
 */
void    KbvFileModel::insertItem(int position, Kbv::kbvItem *item)
{
  //qDebug() << "KbvFileModel::insertItem" << position; //###########
  emit layoutAboutToBeChanged();
  itemList.insert(position, item);
  emit layoutChanged();
}
/*************************************************************************//*!
 * SLOT: This appends a new item at the end of "itemList".
 * Note: Inserting a lot of new rows one after the other causes the view to
 * flicker when insertion is surrounded by beginInsertRows/endInsertRows.
 */
void    KbvFileModel::appendItem(Kbv::kbvItem *item)
{
  //qDebug() << "KbvFileModel::appendItem" << itemList.size(); //###########
  emit layoutAboutToBeChanged();
  itemList.insert(itemList.size(), item);
  emit layoutChanged();
}
/*************************************************************************//*!
 * SLOT: enables or disables the dirMonitor.
 */
void    KbvFileModel::enableDirMonitor(bool enable)
{
  //qDebug() << "KbvFileModel::enableDirMonitor" <<enable; //###########
  if(enable)
    {
      // monitoring of dir modifications
      this->dirMonitor->addPath(rootDir);
    }
  else
    {
      //stop monitoring of dir modifications
      if(!this->dirMonitor->directories().isEmpty())
        {
          this->dirMonitor->removePaths(this->dirMonitor->directories());
        }
    }
}
/*************************************************************************//*!
 * SLOT: start fill thread on directory change.
 * startFillThread() works together with fillThreadFinished() to achive a
 * synchronised behaviour of model and thread.
 * This part decides whether a new thread has to be started and fillThreadFinished()
 * starts either the new fill thread or enables the dirMonitor on the signal
 * threadFinished().
 * First set lock to inhibit start of watch thread when the fill thread stops.
 * Then stop fill model thread and file watcher thread.
 */
void    KbvFileModel::startFillThread(const QString &newDir)
{
  QDir      dir;

  lock = true;                          //don't start watch thread
  dir.setPath(newDir);
  if (newDir!=rootDir && dir.exists())
    {
      //qDebug() << "KbvFileModel::startFillThread" <<newDir <<rootDir; //###########
      rootDir = newDir;                 //do it exactly here!!!
      this->enableDirMonitor(false);
      this->fileModelThread->stopThread();
      this->fileWatchThread->stopThread();
      this->sortDisable();

      isRunning = false;
      restart = false;
    }
}
/*************************************************************************//*!
 * SLOT: Called when the fill thread finished or got stopped.
 * When the lock is set
 * - the model gets cleared
 * - the new fill thread get started
 * - the lock get cleared
 * When the lock is not set, only the dirMonitor gets enabled by setting the
 * dir to be watched.
 */
void    KbvFileModel::fillThreadFinished()
{
  QDir      dir;

  if(lock)
    {
      //qDebug() << "KbvFileModel::fillThreadFinished start fill thread" <<rootDir; //###########
      //scroll view to top, clear the model and start pending fillThread
      lock = false;
      emit  scrollViewToTop();
      emit layoutAboutToBeChanged();
      while(itemList.size()>0)
        {
          delete itemList.takeLast();
        }
      emit layoutChanged();

      dir.setPath(rootDir);
      dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
      dir.setSorting(QDir::Name | QDir::LocaleAware);
      fileEntryList = dir.entryList();

      this->statusText1(QString("%1").arg(fileEntryList.count(), 6));
      this->fileModelThread->startThread(rootDir, &fileEntryList, iconSize, 0);
    }
  else
    {
      //qDebug() << "KbvFileModel::fillThreadFinished start dir monitor"; //###########
      //fillThread finished, enable dirMonitor and enable sorting
      this->statusText1(QString("%1").arg(this->itemList.size(), 6));
      isRunning = false;
      restart = false;
      this->sortEnable();
      this->enableDirMonitor(true);
    }
}
/*************************************************************************//*!
 * SLOT: Called when the dirMonitor detects modifications in the observed dir
 * or when the image editor performed ab batch image processing.
 * This works together with watchThreadFinished().
 * When the thread already is running the flag 'restart' will be set since
 * there are further requests. Otherwise the model content get read into
 * watchList and the thread started.
 */
void    KbvFileModel::startFileWatchThread(const QString path)
{
  Q_UNUSED(path)
  QModelIndex             index;
  QPair<QString, qint64>  item;
  int   n;

  qDebug() << "KbvFileModel::startFileWatchThread" <<rootDir <<restart <<isRunning; //###########
  if(isRunning)
    {
      restart = true;
    }
  else
    {
      isRunning = true;
      restart = false;
      this->sortEnable();
      watchList.clear();
      n = this->itemList.size();
    
      for (int i=0; i<n; ++i)
        {
          index = this->index(i, 0, QModelIndex());
          item.first = this->data(index, Kbv::FileNameRole).toString();
          item.second = this->data(index, Kbv::FileSizeRole).toLongLong();
          watchList.append(item);
        }
      fileWatchThread->startThread(rootDir, &watchList, iconSize);
    }
}
/*************************************************************************//*!
 * SLOT: Called when the file watch thread finished running.
 * When the flag 'restart' is set, the thread get started again.
 */
void    KbvFileModel::watchThreadFinished()
{
  qDebug() << "KbvFileModel::watchThreadFinished restart" <<restart; //###########
  isRunning = false;
  this->removeEmptyItems();
  this->statusText1(QString("%1").arg(this->itemList.size(), 6));
  if(restart)
    {
      restart = false;
      this->startFileWatchThread(QString(""));
    }
}
/*************************************************************************//*!
 * SLOT: This function enables sorting and is called at the end of each run
 * of the file watcher thread.
 */
void    KbvFileModel::sortEnable()
{
  emit enableSorting(true);
  //qDebug() << "KbvFileModel::sortEnable"; //###########
}
/*************************************************************************//*!
 * SLOT: This function  disables sorting and is called at the start of fill
 * thread at each run of the file watcher thread.
 */
void    KbvFileModel::sortDisable()
{
  emit enableSorting(false);
  //qDebug() << "KbvFileModel::sortDisable"; //###########
}

/*************************************************************************//*!
 * SLOT: Update all data items with new path of renamed dir.
 */
void    KbvFileModel::renamedDirUpdate(const QString &path)
{
  QString   filepath;

  filepath = path + "/";
  for (int i=0; i<itemList.count(); i++)
    {
      itemList.at(i)->insert(Kbv::FilePathRole, QVariant(filepath));
    }
}
/*************************************************************************//*!
 * Pure virtual function of base class. We don't have child indices.
 */
QModelIndex KbvFileModel::index(int row, int col, const QModelIndex &parent) const
{
  Q_UNUSED(parent)
  if((row<0) | (col!=0))
    {
      return  QModelIndex();
    }
  return    QAbstractListModel::index(row, 0, QModelIndex());
}
/*************************************************************************//*!
 * Pure virtual function of base class. We don't have parent indices.
 */
QModelIndex KbvFileModel::parent(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    QModelIndex();
}
/*************************************************************************//*!
 * Pure virtual function of base class.
 */
int KbvFileModel::rowCount(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    itemList.count();
}
/*************************************************************************//*!
 * Pure virtual function of base class.
 */
int KbvFileModel::columnCount(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    1;
}
/*************************************************************************//*!
 * SLOT: refresh the model by reading the currently displayed dir.\n
 * refreshModel() works together with fillThreadFinished() to achive a
 * synchronised behaviour of model and thread.
 * First set lock to inhibit start of watch thread then stop fill model thread
 * and file watcher thread.
 * On signal threadFinished() the function fillThreadFinished() starts either
 * the new fill thread (lock set) or the dirMonitor (lock unset).
 */
void    KbvFileModel::refreshModel()
{
  //qDebug() << "KbvFileModel::refreshModel" <<rootDir; //###########
  lock = true;                            //don't start watch thread
  this->enableDirMonitor(false);
  this->fileModelThread->stopThread();
  this->fileWatchThread->stopThread();
  this->sortDisable();
}
/*************************************************************************//*!
 * Called from imageViewer when an image has been saved. We suppose the image
 * has been altered before saving though we have to update the related model
 * item.
 */
void    KbvFileModel::updateItem(const QString &pathName, const QString &fileName)
{
  Kbv::kbvItem  *item;
  int pos;
  
  item = globalFunc.itemFromFile(pathName, fileName, iconSize);
  pos = this->findItem(QVariant(fileName), Kbv::FileNameRole);
  this->replaceItem(pos, item);
  
  //qDebug() << "KbvFileModel::updateItem at pos" <<pos; //###########
}
/*************************************************************************//*!
 * SLOT: called from view when setting have been changed.
 * When icon size changes in icon view mode the model will be refreshed.
 */
void    KbvFileModel::updateOptions(QSize size, int watchCycle)
{
  QSize iconsOld;

  dirWatchCycle = watchCycle;

  //Refresh only when the model is not empty, icon size changed and
  //the view is in icon mode
  if (viewMode == QListView::IconMode)
    {
      iconsOld = iconSize;
      iconSize = size;
      if (!iconsOld.isEmpty() && (iconsOld != iconSize))
        {
          //qDebug() << "KbvFileModel::updateOptions refresh"; //###########
          this->refreshModel();
        }
    }
}
/*************************************************************************//*!
 * Rename multiple files.\n Returns TRUE when all files were renamed
 * or displays a list of files which couldn't be renamed. When renaming has
 * success the appropriate model colunm is actualised with the new name.\n
 * The method always arranges new names due to parameter combination:\n
 *  0: nothing
 *  1: [prefix] count [suffix] [extension]
 *  2: [prefix] count name [suffix] [extension]
 *  3: [prefix] name count [suffix] [extension]
 *  4: [prefix] name [suffix] [extension]
 * 'name' is the origin name. Count is calculated from startValue + stepValue
 * and filled with leading zeros up to "numerals" numerals.\n
 * Prefix and suffix are added always but may be empty.\n
 * When no new file extension is specified the old ones are used instead.
 */
bool    KbvFileModel::multipleRename(QModelIndexList idxList,
                              QString prefix, QString suffix, QString fileExt,
                              int startValue, int  stepValue, int numerals, int combination)
{
  int           i, countValue;
  QString       faultList, warnNoWriteAccess, warnAlreadyExists, warnBoxTitle, warnNoValidName;
  QString       path, name, extension, oldName, newName, countString;
  bool          faults;
  QFile         file;

  //qDebug() << "KbvFileModel::multipleRename row"<<idxList; //###########
  
  warnBoxTitle = QString(tr("Multiple rename"));
  warnNoValidName = QString(tr("No valid name available!\n No files renamed!"));
  warnNoWriteAccess = QString(tr(" - no write access\n"));
  warnAlreadyExists = QString(tr(" - already exists\n"));

  countValue = startValue;
  faultList = QString(tr("Files not renamed:\n"));
  faults = false;

  //Abort on undefined names (combination=0)
  if (combination == 0)
    {
      informationDialog->perform(warnBoxTitle,warnNoValidName,1);
      return false;
    }
  //disable dirMonitor, disable sorting
  this->enableDirMonitor(false);
  emit enableSorting(false);

  //start rename loop
  for (int var = 0; var < idxList.length(); ++var)
    {
      path = this->data(idxList[var], Kbv::FilePathRole).toString();
      oldName = this->data(idxList[var], Kbv::FileNameRole).toString();
      i = oldName.lastIndexOf(".");
      name = oldName.left(i);

      if (fileExt.isEmpty())
        {
          extension.clear();
          if (i >= 0)
            {
              extension = oldName.right(oldName.length() - i);
            }
        }
      else
        {
          extension = "." + fileExt;
        }

      countString = QString("%1").arg(countValue, numerals, 10, QLatin1Char('0'));

      switch (combination)
        {
          case 1: // 1=[prefix] count [suffix] [extension] (count only)
              newName = prefix + countString + suffix + extension;
              break;
          case 2: // 2=[prefix] count name [suffix] [extension] (count name)
              newName = prefix + countString + "_" + name + suffix + extension;
              break;
          case 3: // 3=[prefix] name count [suffix] [extension] (name count)
              newName = prefix + name + "_" + countString + suffix + extension;
              break;
          case 4: // 4=[prefix] name [suffix] [extension]
              newName = prefix + name + suffix + extension;
              break;
          default:
              newName = oldName;
              break;
        }

      file.setFileName(path + oldName);
      if (file.permissions() | QFile::WriteUser | QFile::WriteOwner)
        {
          if(file.rename(path + newName))
            {
              //emit layoutAboutToBeChanged();
              this->setData(idxList[var], QVariant(newName), Qt::DisplayRole);
              this->setData(idxList[var], QVariant(newName), Kbv::FileNameRole);
              //emit layoutChanged();
            }
          else
            {
              faultList.append(newName + warnAlreadyExists);
              faults = true;
            }
        }
      else
        {
          faultList.append(oldName + warnNoWriteAccess);
          faults = true;
        }
      
      //next count value
      countValue += stepValue;
    }
  
  //enable dirMonitor, enable sorting
  emit enableSorting(true);
  this->enableDirMonitor(true);

  //All done, show fault list
  if (faults)
    {
      informationDialog->perform(warnBoxTitle,faultList,1);
    }
  return true;
}
/*************************************************************************//*!
 * Rename single file.\n Returns TRUE when the file was renamed or displays
 * a warn message. When renaming has success the appropriate model all items
 * receive their new name. During the operation the file watchr sleeps.
 */
bool    KbvFileModel::singleRename(QModelIndexList idxList,
                            QString prefix, QString suffix, QString fileExt, int combination)
{
  QString       faultList, warnNoWriteAccess, warnAlreadyExists, warnBoxTitle;
  QString       path, oldName, newName, extension;
  bool          faults;
  int           i;
  QFile         file;

  warnBoxTitle = QString(tr("Single rename"));
  warnNoWriteAccess = QString(tr(" - no write access\n"));
  warnAlreadyExists = QString(tr(" - already exists\n"));
  faultList = QString(tr("File not renamed:\n"));
  faults = false;

  //disable dirMonitor, disable sorting
  this->enableDirMonitor(false);
  emit enableSorting(false);
  

  path = this->data(idxList[0], Kbv::FilePathRole).toString();
  oldName = this->data(idxList[0], Kbv::FileNameRole).toString();
  i = oldName.lastIndexOf(".");
  if (fileExt.isEmpty())
    {
      extension.clear();
      if (i >= 0)
        {
          extension = oldName.right(oldName.length() - i);
        }
    }
  else
    {
      extension = "." + fileExt;
    }
  if(combination==4) // 4=[prefix] name [suffix] [extension]
    {
      newName = prefix + oldName.left(i) + suffix + extension;
    }
  else
    {
      newName = prefix + suffix + extension;
    }

  file.setFileName(path + oldName);
  if (file.permissions() & (QFile::WriteUser | QFile::WriteOwner))
    {
      if(file.rename(path + newName))
        {
          //emit layoutAboutToBeChanged();
          this->setData(idxList[0], QVariant(newName), Qt::DisplayRole);
          this->setData(idxList[0], QVariant(newName), Kbv::FileNameRole);
          //emit layoutChanged();
        }
      else
        {
          faultList.append(newName + warnAlreadyExists);
          faults = true;
        }  //qDebug() << "KbvFileModel::multipleRename row"<<idxList; //###########

    }
  else
    {
      faultList.append(oldName + warnNoWriteAccess);
      faults = true;
    }
  //enable dirMonitor, enable sorting
  emit enableSorting(true);
  this->enableDirMonitor(true);

  //All done, show fault list
  if (faults)
    {
      informationDialog->perform(warnBoxTitle,faultList,1);
      return false;
    }
  return true;
}
/****************************************************************************/
