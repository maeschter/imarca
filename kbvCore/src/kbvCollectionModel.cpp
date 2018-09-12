/*****************************************************************************
 * kbvCollectionModel
 * This is the standard model for albums and collections.
 * This model is designed to collect data from album/collection database or
 * from search result.
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-03-08 19:47:29 +0100 (Do, 08. Mär 2018) $
 * $Rev: 1480 $
 * Created: 2011.01.23
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvCollectionModel.h"
#include "kbvGeneral.h"
#include "kbvReplaceDialog.h"
#include "kbvInformationDialog.h"

extern  KbvReplaceDialog        *replaceDialog;
extern  KbvInformationDialog    *informationDialog;

KbvCollectionModel::KbvCollectionModel(QObject *parent, KbvDBInfo *databaseInfo) :
                                        QAbstractListModel(parent)
{
  //Read database properties from databaseInfo
  dbName = databaseInfo->getName();
  dbLocation = databaseInfo->getLocation();
  dbType = databaseInfo->getType();
  dbIconSize = databaseInfo->getIconSize();
  dbKeywordType = databaseInfo->getKeyWordType();
  collRootDir = databaseInfo->getRootDir();

  //The database can be opened since this is verified in constructor if collectionStack.
  dbconnection = "model"+dbName;
  db = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
  db.setHostName("host");
  db.setDatabaseName(dbLocation+dbName+QString(dbNameExt));
  db.open();
  //qDebug() << "KbvCollectionModel::constructor"<<dbName <<dbLocation <<collRootDir <<db.isOpenError(); //###########

  //default
  collThread = nullptr;
  watchThread = nullptr;
  actualBranch = "";
  weAreVisible = false;
  viewMode = QListView::IconMode;
}
//***************************************************************************
KbvCollectionModel::~KbvCollectionModel()
{
  //qDebug() << "KbvCollectionModel::~KbvCollectionModel"; //###########
  //collThread and watchThread get created/deleted in kbvCollectionStack
  while(itemList.size()>0)
    {
      delete itemList.takeAt(0);
    }
  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection
}
/*************************************************************************//*!
 * Set pointers to the collection thread
 */
void    KbvCollectionModel::setThreads(KbvCollectionThread *collthread,
                                       KbvCollectionWatchThread *watchthread)
{
  this->collThread = collthread;
  this->watchThread = watchthread;
  connect(collThread, SIGNAL(statusText1(QString)), this, SLOT(statusText1(QString)));
  connect(collThread, SIGNAL(statusText2(QString)), this, SLOT(statusText2(QString)));
  connect(collThread, SIGNAL(statusText3(QString)), this, SLOT(statusText3(QString)));
  connect(collThread, SIGNAL(endOfReading()),       this, SLOT(endOfReading()));
  connect(collThread, SIGNAL(endOfUpdate(QString)), this, SLOT(endOfUpdate(QString)));
  connect(collThread, SIGNAL(newItems(Kbv::kbvItemList)), this, SLOT(appendItems(Kbv::kbvItemList)));

  connect(watchThread, SIGNAL(finished()),                this, SLOT(removeEmptyItems()));
  connect(watchThread, SIGNAL(invalidate(qint64)),        this, SLOT(invalidateItem(qint64)));
  connect(watchThread, SIGNAL(newItem(Kbv::kbvItem*)),    this, SLOT(appendItem(Kbv::kbvItem*)));
  connect(watchThread, SIGNAL(updateItem(Kbv::kbvItem*)), this, SLOT(updateItem(Kbv::kbvItem*)));
}
/*************************************************************************//*!
 * Toggle view mode between icon mode (default) and list mode.
 */
void    KbvCollectionModel::setViewMode(int mode)
{
  this->viewMode = mode;
}
/*************************************************************************//*!
 * SLOTS: update info fields 1,2,3 when this tab is visible. The slot is
 * triggered by the collection thread.
 */
void    KbvCollectionModel::statusText1(const QString &text)
{
  //qDebug() << "KbvCollectionModel::statusText1" << text << dbName; //###########
  emit infoText1(text);
}
void    KbvCollectionModel::statusText2(const QString &text)
{
  //qDebug() << "KbvCollectionModel::statusText2" << text << dbName; //###########
  emit infoText2(text);
}
void    KbvCollectionModel::statusText3(const QString &text)
{
  //qDebug() << "KbvCollectionModel::statusText3" << text << dbName; //###########
  emit infoText3(text);
}
/*************************************************************************//*!
 * Slot: Called by stacked widget when the related tab becomes visible.
 * Update values in status bar of main window and set the visible flag.
 * Inform long running thread tasks to update the status bar.
 */
void    KbvCollectionModel::tabIsVisible(bool visible)
{
  weAreVisible = visible;
  this->watchThread->visible(weAreVisible);

  //qDebug() << "KbvCollectionModel::tabIsVisible"<<weAreVisible <<dbName; //###########
  if(visible)
    {
      this->collThread->visible();
      this->refresh();
    }
}
/*************************************************************************//*!
 * SLOT: Refresh model and view by reading the album or collection branch
 * when this tab is visible. Needed after cut/paste.
 */
void    KbvCollectionModel::refresh()
{
  QStringList   strlist;

  if(weAreVisible)
    {
      strlist.append(actualBranch);

      //qDebug() << "KbvCollectionModel::refresh" << dbName <<actualBranch; //###########
      if(collThread->stop() && watchThread->stop())
        {
          emit enableSorting(false);
          clear();
          collThread->start(strlist, Kbv::readout);
          statusText3(actualBranch);
        }
    }
}

/*** Database functions ***************************************************/
/*************************************************************************//*!
 * Slot: Database function:
 * Clear model and read from database "name" into model: the database itself
 * for albums or the branch for collections. Note: all open databases check
 * if name matches the dbname, so this method will be called multiple times.
 * The sort function in the sortModel is suppressed since this can cause the
 * view to flicker on large models. Sorting is enabled again on thread signal
 * enableSorting.
 * branch is expected to be relative without terminating "/".
 * actualBranch is expected to be relative with terminating "/".
 */
void  KbvCollectionModel::readFromDB(const QString &name, const QString &branch)
{
  QStringList   strlist;

  if(name == dbName)
    {
      //Stop threads and disable sorting
      this->collThread->stop();
      this->watchThread->stop();
      emit enableSorting(false);
      clear();

      //qDebug() << "KbvCollectionModel::readFromDB branch" <<branch; //###########
      if(dbType & Kbv::TypeAlbum)
        {
          actualBranch = dbName;
          strlist.append(actualBranch);
          //qDebug() << "KbvCollectionModel::readFromDB album" <<dbName; //###########
          collThread->start(strlist, Kbv::readout);
        }
      else if((dbType & Kbv::TypeCollection) || (dbType & Kbv::TypeCollectionRoot))
        {
          //an empty branch is identical to collection root dir
          actualBranch = branch;
          if(!branch.isEmpty())
            {
              actualBranch.append("/");
            }
          strlist.append(actualBranch);

          //qDebug() << "KbvCollectionModel::readFromDB actualBranch" <<actualBranch; //###########
          collThread->start(strlist, Kbv::readout);
          statusText3(actualBranch);
        }
    }
}
/*************************************************************************//*!
 * Slot: The model content has been read from database. Enable sorting and
 * start collection watch thread.
 * actualBranch is expected to be relative with terminating "/".
 */
void  KbvCollectionModel::endOfReading()
{
  //qDebug() << "KbvCollectionModel::endOfReading start watchThread" <<actualBranch; //###########
  watchThread->start(actualBranch);
  emit enableSorting(true);
}
/*************************************************************************//*!
 * Slot: Menu "Read in" or "Update" was activated.
 * Reads the collection or updates the database or a branch when this tab
 * is visible. Read in only takes place when the database is empty otherwise
 * the database or a branch get updated.
 * 'path' is expected to be relative with terminating "/".
 */
void  KbvCollectionModel::readinOrUpdateDB(const QString &path)
{
  QStringList   strlist;

  //qDebug() << "KbvCollectionModel::readinOrUpdateDB visible"<<dbName <<path <<weAreVisible; //###########
  if(weAreVisible)
    {
      if(collThread->stop() && watchThread->stop())
        {
          this->clear();
          this->statusText1("");
          this->statusText2("");
          this->statusText3("");
          actualBranch = path;
          strlist.append(path);
          collThread->start(strlist, Kbv::readin);
        }
    }
}
/*************************************************************************//*!
 * Slot: end of import or update thread. Show statistics.
 * Called from collection thread. Database name identifies the tab.
 */
void  KbvCollectionModel::endOfUpdate(const QString &text)
{
  emit animationInfoText(dbName, text);
}
/*************************************************************************//*!
 * Slot: Database function: Rename a collection branch in the data base.
 * Called from tree view on pop up menu "rename". The database already is
 * open and there is no need to check visibility.
 * "names" contains: database name, relative oldPath and relative newPath
 * without trailing "/", separated by "\n".
 */
void  KbvCollectionModel::renameDBBranch(const QString &names)
{
  QStringList   strlist;

  //check if this is the right model and database (names must match)
  strlist = names.split("\n", QString::SkipEmptyParts);
  if(strlist[0] == dbName)
    {
      if(collThread->stop() && watchThread->stop())
        {
          this->clear();
          strlist.removeFirst();
          //qDebug() << "KbvCollectionModel::renameDBBranch" <<strlist; //###########
          collThread->start(strlist, Kbv::rename);
        }
    }
}
/*************************************************************************//*!
 * SLOT: Remove files in filelist from model and directory.
 * First find the model index of each file then call removeSelection().
 * Note: filelist can contain multiple values (names) for one key (path).
 */
void  KbvCollectionModel::removeFiles(QMap<QString, QString> filelist)
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
 * Slot: Database function: Remove items from data base.
 * Called from view or slide show on pressing DEL key. Files are removed from
 * file system when they are part of a collection. On albums no file gets
 * removed (see collectionThread).
 * The parameters path, name, primary key are put in strlist in this order.
 * The indicees already are mapped to the model.
 */
void  KbvCollectionModel::removeSelection(const QModelIndexList &indexList)
{
  QStringList   strlist;
  QString       msg1, msg2;
  QVariant      var;
  int           i;

  if(!generalFunc.validTrashDir())
    {
      msg1 = QString(tr("Warning"));
      msg2 = QString(tr("No dust bin available to move files!\n"
                        "Files are deleted from file system!"));
      informationDialog->perform(msg1, msg2, 1);
    }
  if(collThread->stop() && watchThread->stop())
    {
      //generate path/file list for thread
      for(i=0; i < indexList.size(); i++)
        {
          var = this->data(indexList.at(i), Kbv::PrimaryKeyRole);
          strlist.append(this->data(indexList.at(i), Kbv::FilePathRole).toString());
          strlist.append(this->data(indexList.at(i), Kbv::FileNameRole).toString());
          strlist.append(QString("%1").arg(var.toLongLong()));
        }
      collThread->start(strlist, Kbv::remove);
      //remove items from model and update view
      for(i=0; i < indexList.size(); i++)
        {
          this->setData(indexList.at(i), QVariant(), Kbv::FilePathRole);
        }
      this->removeEmptyItems();
    
      this->statusText1(QString("%1").arg(itemList.size(), 6));
    }
}
/*************************************************************************//*!
 * Slot: Insert function in thread needs input from user.
 * Called from collection thread. Opens replace dialog and sends user decision
 * to thread.
 */
void  KbvCollectionModel::askForUserInput(QString text1, QString text2)
{
  int   retval;

  retval = replaceDialog->perform(text1, text2);
  emit userInput(retval);
}
/*************************************************************************//*!
 * Drag move or has accomplished.
 * The drop event moved the files to the target. When the move action failed
 * or was cancelled the file is still present otherwise it has been removed.
 * Cleaning means to remove all model items and all related records for all
 * files which have been removed from source.
 * Removing the model item is done by setting the value of Kbv::FilePathRole
 * as invalid then calling the function removeEmptyItems().
 * Removing the record is done by collectionThread. The thread function
 * 'remove' expects a stringlist of triplets path-name-primarykey for each
 * record to remove.
 * Here we leave path and name empty when the file already got removed.
 * When the file still exists, we remove nothing.
 */
void  KbvCollectionModel::dragMoveCleanUp(const QList<QPersistentModelIndex> &idxList)
{
  QStringList strlist;
  QString     path, name;
  QFile       file;
  qlonglong   pkey;
  
  for(int i=0; i<idxList.count(); ++i)
    {
      if(idxList[i].isValid())
        {
          path = this->data(idxList[i], Kbv::FilePathRole).toString();
          name = this->data(idxList[i], Kbv::FileNameRole).toString();
          pkey = this->data(idxList[i], Kbv::PrimaryKeyRole).toLongLong();
          file.setFileName(path+name);   //absolute path for file operation

          //qDebug() << "KbvCollectionModel::dragMoveCleanUp file path pk" <<path+name <<pkey; //###########
          if(!file.exists())
            {
              //qDebug() << "KbvCollectionModel::dragMoveCleanUp file moved"; //###########
              this->setData(idxList[i], QVariant(), Kbv::FilePathRole);
              strlist.append("");
              strlist.append("");
              strlist.append(QString("%1").arg(pkey));
            }
        }
    }
  collThread->start(strlist, Kbv::remove);
  this->removeEmptyItems();
  this->statusText1(QString("%1").arg(itemList.size(), 6));
  //qDebug() << "KbvCollectionModel::dragMoveCleanUp end"; //###########
}
/*************************************************************************//*!
 * Return the recent (visible) branch
 */
QString  KbvCollectionModel::getActualBranch()
{
  return actualBranch;
}

/** Model functions *******************************************************/
/*************************************************************************//*!
 * Remove all model data
 */
void  KbvCollectionModel::clear()
{
  int   count;

  count = itemList.size();
  //qDebug() << "KbvCollectionModel::clear"<<count; //###########
  emit layoutAboutToBeChanged();
  for(int i=0; i<count; i++)
    {
      delete itemList.takeLast();
    }
  emit layoutChanged();
  //qDebug() << "KbvCollectionModel::clear end"; //###########
}
/*************************************************************************//*!
 * SLOT: Append one item at the end of the itemList.
 */
void  KbvCollectionModel::appendItem(Kbv::kbvItem *item)
{
  //qDebug() << "KbvCollectionModel::appendItems"; //###########
  emit layoutAboutToBeChanged();
  itemList.append(item);
  emit layoutChanged();
  //qDebug() << "KbvCollectionModel::appendItems end"; //###########
}
/*************************************************************************//*!
 * SLOT: Append items of list to the itemList.
 */
void  KbvCollectionModel::appendItems(Kbv::kbvItemList list)
{
  //qDebug() << "KbvCollectionModel::appendItems"; //###########
  emit layoutAboutToBeChanged();
  itemList.append(list);
  emit layoutChanged();
  //qDebug() << "KbvCollectionModel::appendItems end"; //###########
}
/*************************************************************************//*!
 * Insert "count" empty items at position "row" or append them when "row"
 * is beyond list size. It's done this way to prevent the view of flickering.
 */
bool  KbvCollectionModel::insertItems(int row, int count)
{
  Kbv::kbvItem *item;
  int   i;

  if((row<0) || (count<0))
    {
      return false;
    }
  if(row>itemList.size())
    {
      row = itemList.size();
    }
  emit beginInsertRows(QModelIndex(), row, row+count);
  for(i=0; i<count; i++)
    {
      item = new Kbv::kbvItem;
      itemList.insert(row+i, item);
    }
  emit endInsertRows();
  return true;
}
/*************************************************************************//*!
 * Replaces the "item" at position "row" with new item and deletes the old one.
 */
bool  KbvCollectionModel::replaceItem(int row, Kbv::kbvItem *item)
{
  Kbv::kbvItem *oldItem;

  if((row<0) || (row>itemList.size()))
    {
      return false;
    }
  //qDebug() << "KbvCollectionModel::replaceItem"<<row; //###########
  emit layoutAboutToBeChanged();
  oldItem = itemList.at(row);
  itemList.replace(row, item);
  emit layoutChanged();
  delete oldItem;
  //qDebug() << "KbvCollectionModel::replaceItem"; //###########

  return true;
}
/*************************************************************************//*!
 * SLOT: Sets the item invalid by clearing filePathRole for primaryKeyRole
 * primaryKey.
 */
void  KbvCollectionModel::invalidateItem(qint64 primaryKey)
{
  QModelIndex index;

  //qDebug() << "KbvCollectionModel::invalidateItem" <<primaryKey; //###########
  for(int i=0; i<itemList.length(); i++)
    {
      if(itemList.at(i)->value(Kbv::PrimaryKeyRole).toLongLong() == primaryKey)
        {
          index = this->index(i, 0, QModelIndex());
          this->setData(index, QVariant(), Kbv::FilePathRole);
        }
    }
}

/*************************************************************************//*!
 * Removes the item at position"row".
 */
bool  KbvCollectionModel::removeItem(int row)
{
  if((row<0) || (row>itemList.size()))
    {
      return false;
    }
  emit beginRemoveRows(QModelIndex(), row, row);
  delete itemList.takeAt(row);
  emit endRemoveRows();
  return true;
}
/*************************************************************************//*!
 * Remove all empty items from model. Used when items are deleted by view.
 * An item is empty when no valid data for Kbv::FilePathRole can be found.
 */
void  KbvCollectionModel::removeEmptyItems()
{
  int k = 0;
  //qDebug() << "KbvCollectionModel::removeEmptyItems start"; //###########
  
  while (k < itemList.count())
    {
      if (!itemList.at(k)->value(Kbv::FilePathRole).isValid())
        {
          //qDebug() << "KbvCollectionModel::removeEmptyItems" <<itemList.at(k)->value(Kbv::FileNameRole); //###########
          emit beginRemoveRows(QModelIndex(), k, k);
          delete itemList.takeAt(k);
          emit endRemoveRows();
        }
      else
        {
          k++;
        }
    }
  //qDebug() << "KbvCollectionModel::removeEmptyItems end"; //###########
}
/*************************************************************************//*!
 * Returns the row of the item containing data1 for role1 AND data2 for role2.
 */
int KbvCollectionModel::findItem(int role1, QVariant data1, int role2, QVariant data2)
{
  QVariant  d1, d2;
  int       i, k;

  k = this->itemList.size();
  for(i=0; i<k; i++)
    {
      d1 = itemList.at(i)->value(role1);
      d2 = itemList.at(i)->value(role2);
      if((d1 == data1) && (d2 == data2))
        {
          return i;
        }
    }
  return -1;
}
/*************************************************************************//*!
 * SLOT: Called from collectionWatchThread when an image has been altered.
 * The new data get inserted in the model item at primary key/fileName.
 * The watchThread already updated the database record.
 */
void  KbvCollectionModel::updateItem(Kbv::kbvItem  *item)
{
  int           pos;
  QVariant      varPKey, varName;

  varPKey = item->value(Kbv::PrimaryKeyRole, QVariant());
  varName = item->value(Kbv::FileNameRole, QVariant());

  pos = this->findItem(Kbv::PrimaryKeyRole, varPKey, Kbv::FileNameRole, varName);
  //qDebug() << "KbvCollectionModel::updateItem at"<<pos <<varName.toString(); //###########
  if(pos >= 0)
    {
      this->replaceItem(pos, item);
    }
}
/*************************************************************************//*!
 * All items are enabled and selectable.
 */
Qt::ItemFlags   KbvCollectionModel::flags(const QModelIndex &index) const
{
  if (index.isValid() && index.column() == 0)
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   }
  return QAbstractListModel::flags(index);
}
/*************************************************************************//*!
 * This function returns values due to roles the view is asking for:\n
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
 * Integer   - Kbv::PrimaryKeyRole\n
 * quint32   - Kbv::FileCRCRole\n
 * QString   - Qt::ToolTipRole\n
 */
QVariant KbvCollectionModel::data(const QModelIndex &index, int role) const
{
  Kbv::kbvItem  *item;

  //qDebug() << "KbvCollectionModel::data role" << role; //###########
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
  if (role == Kbv::PrimaryKeyRole)
    {
      return item->value(Kbv::PrimaryKeyRole);
    }
  if (role == Kbv::FileCRCRole)
    {
      return item->value(Kbv::FileCRCRole);
    }
  if (role == Qt::ToolTipRole)
    {
      return globalFunc.tooltip(item);
    }
  return QVariant();    //All unused roles
}
/*************************************************************************//*!
 * This function inserts data into the model, i.e. on editing.\n
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
bool  KbvCollectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  Kbv::kbvItem *item;

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
  if (role == Kbv::ImageDimRole)
    {
      item->insert(Kbv::ImageDimRole, value);
    }
  emit(dataChanged(index, index));
  return true;
}

/*************************************************************************//*!
 * Pure virtual function of base class. We don't have child indices.
 */
QModelIndex KbvCollectionModel::index(int row, int col, const QModelIndex &parent) const
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
QModelIndex KbvCollectionModel::parent(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    QModelIndex();
}
/*************************************************************************//*!
 * Pure virtual function of base class.
 */
int KbvCollectionModel::rowCount(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    itemList.size();
}
/*************************************************************************//*!
 * Pure virtual function of base class.
 */
int KbvCollectionModel::columnCount(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    1;
}
/*************************************************************************//*!
 * Rename multiple files.\n Returns TRUE when all files were renamed
 * or displays a list of files which couldn't be renamed.\n
 * When renaming of file has success
 * - the model item receives the new name
 * - the item in the database gets updated.\n
 * The method always arranges
 * <prefix name_count suffix> or <prefix count_name suffix> due to parameter
 * "combination": 0=no rename, 1=count (no name), 2=count - name, 3=name - count
 * Count is calculated from startValue + stepValue and filled with leading zeros
 * up to "numerals" numerals.\n
 * Prefix and suffix are added always but may be empty.\n
 * When no new file extension is specified the old ones are used instead.
 * actualBranch is expected to be relative with terminating "/".
 */
bool  KbvCollectionModel::multipleRename(QModelIndexList idxList,
                                  QString prefix, QString suffix, QString fileExt,
                                  int startValue, int  stepValue, int numerals, int combination)
{
  int           i, k, countValue;
  QString       faultList, stmt, str1, str2;
  QString       path, name, extension, oldName, newName, countString, keywords;
  QVariant      var;
  bool          faults;
  QFile         file;
  QSqlQuery     query;

  //Abort on undefined names (combination=0)
  if (combination == 0)
    {
      str1 = QString(tr("Multiple rename"));
      str2 = QString(tr("No valid name available!\n No files renamed!"));
      informationDialog->perform(str1,str2,1);
      return false;
    }

  this->watchThread->stop();
  emit enableSorting(false);

  str1 = QString("UPDATE OR REPLACE album SET fileName = :fileName, keyWords = :keyWords WHERE pk = %1");
  query = QSqlQuery(db);
  db.transaction();

  countValue = startValue;
  faultList.clear();
  faults = false;
  //start rename loop
  for (k = 0; k < idxList.length(); ++k)
    {
      var =     this->data(idxList.at(k), Kbv::PrimaryKeyRole);
      path =    this->data(idxList.at(k), Kbv::FilePathRole).toString();
      oldName = this->data(idxList.at(k), Kbv::FileNameRole).toString();
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
              this->setData(idxList.at(k), QVariant(newName), Qt::DisplayRole);
              this->setData(idxList.at(k), QVariant(newName), Kbv::FileNameRole);
              keywords = globalFunc.extractKeywords(newName, dbKeywordType);
              stmt = str1.arg(var.toLongLong());
              query.prepare(stmt);
              query.bindValue(":fileName", QVariant(newName));
              query.bindValue(":keyWords", QVariant(keywords));
              query.exec();
            }
          else
            {
              faultList.append(newName + QString(tr(" - already exists\n")));
              faults = true;
            }
        }
      else
        {
          faultList.append(oldName + QString(tr(" - no write access\n")));
          faults = true;
        }
      //next count value
      countValue += stepValue;
    }
  //finish transaction
  db.commit();

  emit enableSorting(true);
  this->watchThread->start(actualBranch);

  //All done, show fault list
  if (faults)
    {
      informationDialog->perform(QString(tr("Files not renamed:")),faultList, 1);
    }
  return true;
}
/*************************************************************************//*!
 * Rename single file.\n Returns TRUE when the file was renamed or displays
 * a warn message. When renaming of file has success
 * - the model item receives the new name
 * - the item in the database gets updated.
 * actualBranch is expected to be relative with terminating "/".
 */
bool  KbvCollectionModel::singleRename(QModelIndexList idxList,
                                         QString prefix, QString suffix, QString fileExt, int combination)
{
  QString       faultList, stmt, str1, path, oldName, newName, extension, keywords;
  QVariant      var;
  bool          faults;
  int           i;
  QFile         file;
  QSqlQuery     query;


  str1 = QString("UPDATE OR REPLACE album SET fileName = :fileName, keyWords = :keyWords WHERE pk = %1");
  query = QSqlQuery(db);

  faults = false;
  emit enableSorting(false);
  this->watchThread->stop();

  var =     this->data(idxList.at(0), Kbv::PrimaryKeyRole);
  path =    this->data(idxList.at(0), Kbv::FilePathRole).toString();
  oldName = this->data(idxList.at(0), Kbv::FileNameRole).toString();
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
  
  //qDebug() << "KbvCollectionModel::singleRename old new path"<<oldName <<newName <<path; //###########
  file.setFileName(path + oldName);
  if (file.permissions() & (QFile::WriteUser | QFile::WriteOwner))
    {
      if(file.rename(path + newName))
        {
          db.transaction();
          this->setData(idxList.at(0), QVariant(newName), Qt::DisplayRole);
          this->setData(idxList.at(0), QVariant(newName), Kbv::FileNameRole);
          keywords = globalFunc.extractKeywords(newName, dbKeywordType);
          stmt = str1.arg(var.toLongLong());
          query.prepare(stmt);
          query.bindValue(":fileName", QVariant(newName));
          query.bindValue(":keyWords", QVariant(keywords));
          query.exec();
          db.commit();
        }
      else
        {
          faultList.append(newName + QString(tr(" - already exists\n")));
          faults = true;
        }
    }
  else
    {
      faultList.append(oldName + QString(tr(" - no write access\n")));
      faults = true;
    }
  emit enableSorting(true);
  this->watchThread->start(actualBranch);

  //All done, show fault list
  if (faults)
    {
      informationDialog->perform(QString(tr("File not renamed:")), faultList, 1);
      return false;
    }
  return true;
}

/****************************************************************************/
