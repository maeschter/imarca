/*****************************************************************************
 * kbvSearchModel
 * This is the standard model for search in albums and collections.
 * This model is designed to collect data from album/collection database or
 * from search result.
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-03-08 19:47:29 +0100 (Do, 08. Mär 2018) $
 * $Rev: 1480 $
 * Created: 2012.08.13
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvSetvalues.h"
#include "kbvReplaceDialog.h"
#include "kbvInformationDialog.h"
#include "kbvSearchModel.h"

extern  KbvSetvalues            *settings;
extern  KbvReplaceDialog        *replaceDialog;
extern  KbvInformationDialog    *informationDialog;

KbvSearchModel::KbvSearchModel(QObject *parent) : QAbstractListModel(parent)
{
  //default
  searchThread = 0;
  dbType = Kbv::TypeNone;
  viewMode = QListView::IconMode;

  searchThread = new KbvSearchThread(nullptr);
  connect(searchThread, SIGNAL(newItems(Kbv::kbvItemList)), this, SLOT(appendItems(Kbv::kbvItemList)));
  connect(searchThread, SIGNAL(updateItem(Kbv::kbvItem*)),  this, SLOT(updateItem(Kbv::kbvItem*)));
  connect(searchThread, SIGNAL(invalidate(qint64)),         this, SLOT(invalidateItem(qint64)));
  connect(searchThread, SIGNAL(endOfSearching()),           this, SLOT(endOfReading()));
  connect(searchThread, SIGNAL(endOfReplaceing()),          this, SLOT(removeEmptyItems()));
  connect(searchThread, SIGNAL(warning(QString, bool)),     this, SIGNAL(warning(QString, bool)));

  connect(searchThread, SIGNAL(statusText1(QString)),   this, SIGNAL(infoText1(QString)));
  connect(searchThread, SIGNAL(statusText2(QString)),   this, SIGNAL(infoText2(QString)));
  connect(searchThread, SIGNAL(statusText3(QString)),   this, SIGNAL(infoText3(QString)));
}
//***************************************************************************
KbvSearchModel::~KbvSearchModel()
{
  //qDebug() << "KbvSearchModel::~KbvSearchModel"; //###########
  delete  searchThread;
  while(itemList.size()>0)
    {
      delete itemList.takeAt(0);
    }
}
/*************************************************************************//*!
 * Set database for search. This is the first step before any search or other
 * operation can be performed by the thread. The database already is open.
 */
void    KbvSearchModel::setDatabase(QSqlDatabase database, KbvDBInfo *databaseInfo)
{
  this->searchDB = database;
  this->dbName = databaseInfo->getName();
  this->dbVer = databaseInfo->getVersion();
  this->dbDescription = databaseInfo->getDescription();
  this->dbType = databaseInfo->getType();
  this->collRoot = databaseInfo->getRootDir();
  this->dbIconSize = databaseInfo->getIconSize();
  this->dbKeywordType = databaseInfo->getKeyWordType();

  searchThread->setDatabase(database, databaseInfo);
}
/*************************************************************************//*!
 * Start thread and perform search. Called from search tab.
 */
void    KbvSearchModel::startThread(QStringList &keywordList, QStringList &pathsList, QList<int> &multilist, int function)
{
  //qDebug() << "KbvSearchModel::startThread"; //###########
  this->clear();
  searchThread->start(keywordList, pathsList, multilist, function);
}
/*************************************************************************//*!
 * SLOT: This function is called at the end of the search thread and enables
 * sorting. Sorting was disabled at the begin of the search thread in function
 * clear().
 */
void    KbvSearchModel::endOfReading()
{
  if(itemList.isEmpty())
    {
      QImage        fileimage;
      QImageReader  reader;
      reader.setFileName(":/kbv/icons/kbv_search_fail.png");
      reader.read(&fileimage);
      
      Kbv::kbvItem *item = new Kbv::kbvItem;
      item->insert(Qt::DecorationRole,  QVariant(fileimage));
      item->insert(Kbv::FileNameRole,   QVariant(QString("search failed")));
      emit layoutAboutToBeChanged();
      itemList.append(item);
      emit layoutChanged();
      emit infoText1(QString());
    }
  emit enableSorting(true);
}
/*************************************************************************//*!
 * Start thread and perform an update of model content.
 * Called when the image editor or batch editor altered images. The files are
 * identified by relative path, name, primary key and crc32
 */
void  KbvSearchModel::updateModel(const QString &pathname)
{
  Q_UNUSED(&pathname);

  QList<int>    intlist;
  QStringList   datalist, strlist;
  QModelIndex   idx;
  QString       path, name, pk, crc;
  int           n;

  //qDebug() << "KbvSearchModel::updateModel"; //###########
  n = collRoot.length();
  for(int i=0; i<itemList.size(); i++)
    {
      idx = index(i, 0, QModelIndex());
      //get relative path without collection root
      path = this->data(idx, Kbv::FilePathRole).toString();
      path.remove(0, n);
      name = this->data(idx, Kbv::FileNameRole).toString();
      pk   = this->data(idx, Kbv::PrimaryKeyRole).toString();
      crc  = this->data(idx, Kbv::FileCRCRole).toString();
      datalist <<path <<name <<pk <<crc;
      //qDebug() << "KbvSearchModel::updateModel" <<pk <<name <<path; //###########
    }
  searchThread->start(datalist, strlist, intlist, Kbv::replace);
}

/*************************************************************************//*!
 * Toggle view mode between icon mode (default) and list mode.
 */
void    KbvSearchModel::setViewMode(int mode)
{
  this->viewMode = mode;
}
/*************************************************************************//*!
 * SLOT: update info button. The slot is triggered by the search thread.
 */
void    KbvSearchModel::statusText1(QString text)
{
  //qDebug() << "KbvSearchModel::statusText1" << text << dbName; //###########
  emit infoText1(text);
}
void    KbvSearchModel::statusText2(QString text)
{
  //qDebug() << "KbvSearchModel::statusText2" << text << dbName; //###########
  emit infoText2(text);
}
void    KbvSearchModel::statusText3(QString text)
{
  //qDebug() << "KbvSearchModel::statusText3" << text << dbName; //###########
  emit infoText3(text);
}
/*************************************************************************//*!
 * Drag move has accomplished.
 * The drop event moved the files to the target. When the move action failed
 * or was cancelled the file is still present otherwise it has been removed.
 * Cleaning means to remove selected items from searchModel and all related
 * records from search database.
 * The source model and database have been updated by the drop event.
 */
void    KbvSearchModel::dragMoveCleanUp(const QList<QPersistentModelIndex> &idxList)
{
  QString   name, path, stmt, str;
  QFile     file;
  qlonglong pkey;
  QSqlQuery query;
  
  //qDebug() << "KbvSearchModel::dragMoveCleanUp"; //###########

  query = QSqlQuery(searchDB);
  str = QString("DELETE FROM album WHERE pk = %1");
  
  searchDB.transaction();
  for (int i=0; i<idxList.count(); ++i)
    {
      if(idxList[i].isValid())
        {
          path = this->data(idxList[i], Kbv::FilePathRole).toString();
          name = this->data(idxList[i], Kbv::FileNameRole).toString();
          pkey = this->data(idxList[i], Kbv::PrimaryKeyRole).toLongLong();

          //qDebug() << "KbvSearchModel::dragMoveCleanUp file record"<<path <<name <<pkey; //###########
          
          if(!file.exists(path+name))
            {
              this->setData(idxList[i], QVariant(), Kbv::FilePathRole);
              stmt = str.arg(pkey);
              query.exec(stmt);
            }
        }
    }
  searchDB.commit();
  this->removeEmptyItems();
  
  this->statusText1(QString("%1").arg(itemList.size(), 6));
  //qDebug() << "KbvSearchModel::dragMoveCleanUp finished"; //###########
}
/*************************************************************************//*!
 * Cut/paste has been started by ctrl-x key event.
 * Remove all cut items by setting the value of Kbv::FilePathRole as invalid
 * then call the function removeEmptyItems().
 * Removing files and records is done in the ctrl-v key event.
 */
void    KbvSearchModel::ctrlXCleanUp(const QModelIndexList &idxList)
{
  for(int i=0; i<idxList.count(); ++i)
    {
      if(idxList[i].isValid())
        {
          this->setData(idxList[i], QVariant(), Kbv::FilePathRole);
        }
      //qDebug() << "KbvSearchModel::ctrlXCleanUp finished" <<this->data(idxList[i], Kbv::PrimaryKeyRole).toString(); //###########
    }
  this->removeEmptyItems();
}

/** Model functions *******************************************************/
/*************************************************************************//*!
 * Completely clear the model. Called on "Clear search result"
 */
void    KbvSearchModel::clear()
{
  emit enableSorting(false);

  this->beginResetModel();
  while(itemList.size()>0)
    {
      delete itemList.takeAt(0);
    }
  this->endResetModel();
}
/*************************************************************************//*!
 * SLOT: Append items of list to the itemList.
 */
void    KbvSearchModel::appendItems(Kbv::kbvItemList list)
{
  emit layoutAboutToBeChanged();
  itemList.append(list);
  emit layoutChanged();
}
/*************************************************************************//*!
 * Insert "count" empty items at position "row" or append them when "row"
 * is beyond list size. Then set status text 1.
 */
bool    KbvSearchModel::insertItems(int row, int count)
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
  emit infoText1(QString("%1").arg(itemList.size(), 6));
  return true;
}
/*************************************************************************//*!
 * Replaces the "item" at position "row" with new item and deletes the old one.
 */
bool    KbvSearchModel::replaceItem(int row, Kbv::kbvItem *item)
{
  Kbv::kbvItem *oldItem;

  if((row<0) || (row>itemList.size()))
    {
      return false;
    }
  emit layoutAboutToBeChanged();
  oldItem = itemList.at(row);
  itemList.replace(row, item);
  emit layoutChanged();
  delete oldItem;

  return true;
}
/*************************************************************************//*!
 * Removes the item at position"row".
 */
bool    KbvSearchModel::removeItem(int row)
{
  if((row<0) || (row>itemList.size()))
    {
      return false;
    }
  //qDebug() << "KbvSearchModel::removeItem" << row; //###########
  emit beginRemoveRows(QModelIndex(), row, row);
  delete itemList.takeAt(row);
  emit endRemoveRows();
  return true;
}
/*************************************************************************//*!
 * Remove all empty items from model. Used when items are deleted by view.
 * An item is empty when no valid data for Kbv::FilePathRole can be found.
 */
void    KbvSearchModel::removeEmptyItems()
{
  int k = 0;
  while (k < itemList.count())
    {
      if (!itemList.at(k)->value(Kbv::FilePathRole).isValid())
        {
          //qDebug() << "KbvSearchModel::removeEmptyItems at" << k; //###########
          emit beginRemoveRows(QModelIndex(), k, k);
          delete itemList.takeAt(k);
          emit endRemoveRows();
        }
      else
        {
          k++;
        }
    }
}
/*************************************************************************//*!
 * SLOT: Sets the item invalid by clearing filePathRole for primaryKeyRole
 * primaryKey.
 */
void  KbvSearchModel::invalidateItem(qint64 primaryKey)
{
  QModelIndex index;

  //qDebug() << "KbvSearchModel::invalidateItem" <<primaryKey; //###########
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
 * Returns the row of the item containing data1 for role1 AND data2 for role2.
 */
int   KbvSearchModel::findItem(int role1, QVariant data1, int role2, QVariant data2)
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
 * When the database equals the models database the new data get inserted in
 * the model item at primary key/fileName. The watchThread already updated
 * the database record.
 */
void  KbvSearchModel::updateItem(Kbv::kbvItem  *item)
{
  int           pos;
  QVariant      varPKey, varName;

  varPKey = item->value(Kbv::PrimaryKeyRole, QVariant());
  varName = item->value(Kbv::FileNameRole, QVariant());

  pos = this->findItem(Kbv::PrimaryKeyRole, varPKey, Kbv::FileNameRole, varName);
  //qDebug() << "KbvSearchModel::updateItem at"<<pos <<varName.toString(); //###########
  if(pos >= 0)
    {
      this->replaceItem(pos, item);
    }
}
/*************************************************************************//*!
 * All items are enabled and selectable.
 */
Qt::ItemFlags   KbvSearchModel::flags(const QModelIndex &index) const
{
  if (index.isValid() && index.column() == 0)
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   }
  return QAbstractListModel::flags(index);
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
 * Integer   - Kbv::PrimaryKeyRole\n
 * quint32   - Kbv::FileCRCRole\n
 * QString   - Qt::ToolTipRole\n
 */
QVariant KbvSearchModel::data(const QModelIndex &index, int role) const
{
  Kbv::kbvItem *item;

  //qDebug() << "KbvSearchModel::data row valid" << index.row() << index.isValid(); //###########
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
bool KbvSearchModel::setData(const QModelIndex &index, const QVariant &value, int role)
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
  if(role == Kbv::ImageDimRole)
    {
      item->insert(Kbv::ImageDimRole, value);
    }
  emit(dataChanged(index, index));
  return true;
}

/*************************************************************************//*!
 * Pure virtual function of base class. We don't have child indices.
 */
QModelIndex KbvSearchModel::index(int row, int col, const QModelIndex &parent) const
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
QModelIndex KbvSearchModel::parent(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    QModelIndex();
}
/*************************************************************************//*!
 * Pure virtual function of base class.
 */
int KbvSearchModel::rowCount(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    itemList.size();
}
/*************************************************************************//*!
 * Pure virtual function of base class.
 */
int KbvSearchModel::columnCount(const QModelIndex &idx) const
{
  Q_UNUSED(idx)
  return    1;
}

/*** Database functions ***************************************************/
/*************************************************************************//*!
 * SLOT: Remove files in filelist from model and directory.
 * First find the model index of each file then call removeSelection().
 * Note: filelist can contain multiple values (names) for one key (path).
 */
void  KbvSearchModel::removeFiles(QMap<QString, QString> filelist)
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
              //qDebug() << "KbvSearchModel::removeFiles name" <<itemList.at(i)->value(Kbv::FileNameRole).toString(); //###########
              //item of filelist found, construct model index
              index = this->createIndex(i, 0, this);
              idxList.append(index);
            }
        }
    }
  //qDebug() << "KbvSearchModel::removeFiles index" <<idxList; //###########
  this->removeSelection(idxList);
}
/*************************************************************************//*!
 * Slot: Called from view on pressing DEL key.
 * Remove items from data base and model. Files are removed from file system
 * when they are part of a collection. On albums no file gets removed (see
 * searchThread).
 * The parameters path, name, primary key are put in strlist in this order.
 * The indicees already are mapped to the model.
 */
void    KbvSearchModel::removeSelection(const QModelIndexList &indexList)
{
  QList<int>    il;
  QStringList   strlist, none;
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
  //generate path/file list for thread
  for (i=0; i < indexList.size(); i++)
    {
      var = this->data(indexList.at(i), Kbv::PrimaryKeyRole);
      strlist.append(this->data(indexList.at(i), Kbv::FilePathRole).toString());
      strlist.append(this->data(indexList.at(i), Kbv::FileNameRole).toString());
      strlist.append(QString("%1").arg(var.toLongLong()));
    }
  searchThread->start(strlist, none, il, Kbv::remove);

  //remove items from model and update view
  for (i=0; i < indexList.size(); i++)
    {
      //qDebug() << "KbvSearchModel::removeSelection" << i; //###########
      this->setData(indexList.at(i), QVariant(), Kbv::FilePathRole);
    }
  this->removeEmptyItems();

  this->statusText1(QString("%1").arg(itemList.size(), 6));
}
/*************************************************************************//*!
 * Slot: Insert function in thread needs input from user.
 * Called from collection thread. Opens replace dialog and sends user decision
 * to thread.
 */
void    KbvSearchModel::askForUserInput(QString text1, QString text2)
{
  int   retval;

  retval = replaceDialog->perform(text1, text2);
  emit userInput(retval);
}
/*************************************************************************//*!
 * Rename single file.\n Returns TRUE when the file was renamed or displays
 * a warn message. The method is divided in two parts:
 * 1. When renaming of file has success the model item receive the new name.
 * 2. When renaming of file has success the item in the database gets updated.
 * The database already is open.
 */
bool    KbvSearchModel::singleRename(QModelIndexList idxList,
                                     QString prefix, QString suffix, QString fileExt, int combination)
{
  QString       faultList, stmt, str, path, oldName, newName, extension, keywords;
  QVariant      pk;
  bool          faults;
  int           i;
  QFile         file;
  QSqlQuery     query;


  str = QString("UPDATE OR REPLACE album SET fileName = :fileName, keyWords = :keyWords WHERE pk = %1");
  query = QSqlQuery(searchDB);

  faults = false;
  emit enableSorting(false);

  pk =      this->data(idxList.at(0), Kbv::PrimaryKeyRole);
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
  
  file.setFileName(path + oldName);
  if (file.permissions() & (QFile::WriteUser | QFile::WriteOwner))
    {
      if(file.rename(path + newName))
        {
          searchDB.transaction();
          this->setData(idxList.at(0), QVariant(newName), Qt::DisplayRole);
          this->setData(idxList.at(0), QVariant(newName), Kbv::FileNameRole);
          keywords = globalFunc.extractKeywords(newName, dbKeywordType);
          stmt = str.arg(pk.toLongLong());
          query.prepare(stmt);
          query.bindValue(":fileName", QVariant(newName));
          query.bindValue(":keyWords", QVariant(keywords));
          query.exec();
          searchDB.commit();
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

  //All done, show fault list
  if (faults)
    {
      informationDialog->perform(QString(tr("File not renamed:")), faultList, 1);
      return false;
    }
  return true;
}
/*************************************************************************//*!
 * Rename multiple files.\n Returns TRUE when all files were renamed
 * or displays a list of files which couldn't be renamed.\n
 * The method is divided in two parts:\n
 * 1. When renaming of files has success the model items receive their new names.
 * 2. When renaming of files has success the items in the database get updated.\n
 * The method always arranges
 * <prefix name_count suffix> or <prefix count_name suffix> due to parameter
 * "combination": 0=no rename, 1=count (no name), 2=count - name, 3=name - count
 * Count is calculated from startValue + stepValue and filled with leading zeros
 * up to "numerals" numerals.\n
 * Prefix and suffix are added always but may be empty.\n
 * When no new file extension is specified the old ones are used instead.
 */
bool    KbvSearchModel::multipleRename(QModelIndexList idxList,
                                  QString prefix, QString suffix, QString fileExt,
                                  int startValue, int  stepValue, int numerals, int combination)
{
  int           i, k, countValue;
  QString       faultList, stmt, str, str1, str2;
  QString       path, name, extension, oldName, newName, countString, keywords;
  QVariant      pk;
  bool          faults;
  QFile         file;
  QSqlQuery     query;


  str1 = QString(tr("Multiple rename"));
  str2 = QString(tr("No valid name available!\n No files renamed!"));
  //Abort on undefined names (combination=0)
  if (combination == 0)
    {
      informationDialog->perform(str1, str2, 1);
      return false;
    }

  emit enableSorting(false);
  query = QSqlQuery(searchDB);
  str = QString("UPDATE OR REPLACE album SET fileName = :fileName, keyWords = :keyWords WHERE pk = %1");
  searchDB.transaction();

  countValue = startValue;
  faultList.clear();
  faults = false;
  //start rename loop
  for (k = 0; k < idxList.length(); ++k)
    {
      pk =      this->data(idxList.at(k), Kbv::PrimaryKeyRole);
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
              stmt = str.arg(pk.toLongLong());
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
  searchDB.commit();

  emit enableSorting(true);

  //All done, show fault list
  if (faults)
    {
      informationDialog->perform(QString(tr("Files not renamed:")) ,faultList, 1);
    }
  return true;
}
/****************************************************************************/
