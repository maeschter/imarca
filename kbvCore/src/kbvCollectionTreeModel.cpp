/*****************************************************************************
 * KbvCollectionTreeModel.cpp
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2010.10.27
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvInformationDialog.h"
#include "kbvCollectionTreeModel.h"

extern  KbvSetvalues            *settings;
extern  KbvInformationDialog    *informationDialog;

KbvCollectionTreeModel::KbvCollectionTreeModel(QObject *parent) : QStandardItemModel(0, 1, parent)
{
  //natural sorting of dir names containing numbers
  collator.setLocale(QLocale::system());
  collator.setNumericMode(true);

  //create anchor for collections, all collection use this as parent
  collectionAnchorItem = new QStandardItem();
  collectionAnchorItem->setData(QVariant(tr("Collections")), Qt::DisplayRole);
  collectionAnchorItem->setData(QVariant(generalFunc.iconPhotoCollection), Qt::DecorationRole);
  collectionAnchorItem->setData(QVariant(Kbv::TypeCollectionAnchor), Kbv::CollectionTypeRole);
  this->invisibleRootItem()->appendRow(collectionAnchorItem);

  //create anchor for photo albums, all albums use this as parent
  albumAnchorItem = new QStandardItem();
  albumAnchorItem->setData(QVariant(tr("Photo albums")), Qt::DisplayRole);
  albumAnchorItem->setData(QVariant(generalFunc.iconPhotoAlbum), Qt::DecorationRole);
  albumAnchorItem->setData(QVariant(Kbv::TypeAlbumAnchor), Kbv::CollectionTypeRole);
  this->invisibleRootItem()->appendRow(albumAnchorItem);
}

KbvCollectionTreeModel::~KbvCollectionTreeModel()
{
  //qDebug() << "KbvCollectionTreeModel::~KbvCollectionTreeModel"; //###########
}

/*************************************************************************//*!
 * This function enables internal move.\n
 * - album and collection roots (direct children of an anchor) are draggable.
 * - all other items are enabled and selectable but not draggable.
 */
Qt::ItemFlags   KbvCollectionTreeModel::flags(const QModelIndex &index) const
{
  int   type;

  if (index.isValid() && index.column() == 0)
    {
      //Only root items are drag enabled
      type = this->data(index, Kbv::CollectionTypeRole).toInt();
      if(type & Kbv::TypeAlbumRoot || type & Kbv::TypeCollectionRoot)
        {
          return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
        }
      //All other like collection root and album root or collection branches
      //aren't drag enabled
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
  return Qt::NoItemFlags;
}

/*************************************************************************//*!
 * Supported drop actions are copy and move.
 */
Qt::DropActions KbvCollectionTreeModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

/*************************************************************************//*!
 * The data() function returns values due to roles the view is asking for:
 * QString - DisplayRole = branch (dir name)
 * QString - FileNameRole = branch (dir name)  <<< not used here
 * QString - FilePathRole = dir path
 * QString - CollectionNameRole = db name = album/collection name
 * QString - CollectionRootDirRole = absolute collection root dir
 * int     - CollectionTypeRole = db type
 * QString - DatabaseLocationRole = absolute dir of database
 * QColor  - ForegroundRole = colour for item forground, e.g. text
 * QIcon   - decoration role
 * An album only exists in model and database, not in file system.
 */
QVariant KbvCollectionTreeModel::data(const QModelIndex &index, int role) const
{
  QStandardItem *item;

  if (!index.isValid() || index.column() < 0)
    {
      return QVariant();
    }
  item = itemFromIndex(index);
  if (role == Qt::DisplayRole)
    {
      return item->data(Qt::DisplayRole);
    }
  if (role == Qt::DecorationRole)
    {
      return item->data(Qt::DecorationRole);
    }
  if (role == Kbv::FilePathRole)
    {
      return item->data(Kbv::FilePathRole);
    }
  if (role == Kbv::CollectionTypeRole)
    {
      return item->data(Kbv::CollectionTypeRole);
    }
  if (role == Kbv::CollectionNameRole)
    {
      return item->data(Kbv::CollectionNameRole);
    }
  if (role == Kbv::CollectionRootDirRole)
    {
      return item->data(Kbv::CollectionRootDirRole);
    }
  if (role == Kbv::DatabaseLocationRole)
    {
      return item->data(Kbv::DatabaseLocationRole);
    }
  if (role == Qt::ForegroundRole)
    {
      return item->data(Qt::ForegroundRole);
    }
  if (role == Qt::ToolTipRole)
    {
      return this->tooltip(index);
    }
  return QVariant();    //All unused roles
}
/*************************************************************************//*!
 * This function inserts data into the model item at index.\n
 * QString - DisplayRole = branch (dir name)
 * QString - FileNameRole = branch (dir name)  <<< not used here
 * QString - FilePathRole = relative path below root dir
 * icon    - decoration role
 * QString - CollectionNameRole = db name = album/collection name
 * QString - CollectionRootDirRole = absolute collection root dir
 * int     - CollectionTypeRole = db type or a db property
 * An album only exists in model and database, not in file system.
 */
bool KbvCollectionTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  QStandardItem *item;

  if (!index.isValid() || index.column() < 0)
    {
      return false;
    }
  //qDebug() << "KbvCollectionTreeModel::setData" << index; //###########
  item = this->itemFromIndex(index);
  if(role == Qt::DisplayRole)
    {
      item->setData(value, Qt::DisplayRole);
    }
  if(role == Kbv::FileNameRole)
    {
      item->setData(value, Kbv::FileNameRole);
    }
  if(role == Kbv::FilePathRole)
    {
      item->setData(value, Kbv::FilePathRole);
    }
  if(role == Qt::DecorationRole)
    {
      item->setData(value, Qt::DecorationRole);
    }
  if(role == Kbv::CollectionNameRole)
    {
      item->setData(value, Kbv::CollectionNameRole);
    }
  if(role == Kbv::CollectionRootDirRole)
    {
      item->setData(value, Kbv::CollectionRootDirRole);
    }
  if(role == Kbv::CollectionTypeRole)
    {
      item->setData(value, Kbv::CollectionTypeRole);
    }
  if(role == Qt::ForegroundRole)
    {
      item->setData(value, Qt::ForegroundRole);
    }
  emit(dataChanged(index, index));
  return true;
}

/*************************************************************************//*!
 * Getter for album anchor and collection anSchor.
 */
QStandardItem*    KbvCollectionTreeModel::albumAnchor()
{
  return albumAnchorItem;
}
QStandardItem*    KbvCollectionTreeModel::collectionAnchor()
{
  return collectionAnchorItem;
}

/*************************************************************************//*!
 * Checks if there is a child "name" below the item of collection tree
 * described by index "parent".
 * First step: verify that there is no album or collection database of same
 * name. (type doesn't matter since shared databases between albums and
 * collections is not allowed.
 * Second step: on collections when the index "parent" is valid check if the
 * item at "parent" does not have children with the same name (siblings).
 */
bool    KbvCollectionTreeModel::childNameExists(const QString name, int type, const QModelIndex &parent)
{
  QStandardItem   *item, *child;

  //Adding or renaming databases.
  //Keep all database names different. This is independent of type since
  //sharing databases between albums and collections is not allowed.
  //Renaming a branch with the name of a db is not possible.
  //qDebug() << "KbvCollectionTreeModel::childNameExists album/collection" << name; //###########
  item = albumAnchorItem;
  for(int i=0; i<item->rowCount(); i++)
    {
      child = item->child(i, 0);
      if((child->data(Qt::DisplayRole)).toString() == name)
        {
          return  true;
        }
    }
  item = collectionAnchorItem;
  for(int i=0; i<item->rowCount(); i++)
    {
      child = item->child(i, 0);
      if((child->data(Qt::DisplayRole)).toString() == name)
        {
          return  true;
        }
    }

  //Renaming a branch
  //The new name is distinct to the database name (checked above) and
  //now must be checked for siblings of the same name. This is allowed
  //when the parent index is valid (invalid index = collection root item).
  //can be used for renaming or adding a collection.
  if((type & Kbv::TypeCollection) && (parent.isValid()))
    {
      //qDebug() << "KbvCollectionTreeModel::childNameExists branch" << name << type << parent; //###########
      item = this->itemFromIndex(parent);
      for(int i=0; i<item->rowCount(); i++)
        {
          child = item->child(i, 0);
          if((child->data(Qt::DisplayRole)).toString() == name)
            {
              return  true;
            }
        }
    }
  return  false;
}
/*************************************************************************//*!
 * Called by view. Move item "source" after item "target" and remove source
 * from model. The view assures that indices are valid and do not cause a
 * model mismatch like album->collection or collection->album.
 * The parents of the moved items always are of type 'anchor' so only albums 
 * and collections are movable but no branches.
 */
void    KbvCollectionTreeModel::moveItem(const QModelIndex &source, const QModelIndex &target)
{
  QList<QStandardItem *> items;
  QStandardItem *tParent, *sParent;
  int           tType, sType;

  tParent = this->itemFromIndex(target.parent());
  sParent = this->itemFromIndex(source.parent());
  tType = tParent->data(Kbv::CollectionTypeRole).toInt();
  sType = sParent->data(Kbv::CollectionTypeRole).toInt();

  if((sType & tType) && ((tType & Kbv::TypeAlbumAnchor) || (tType & Kbv::TypeCollectionAnchor)))
    {
      //qDebug() << "KbvCollectionTreeModel::moveItem target source" <<target <<source; //###########
      items = sParent->takeRow(source.row());
      tParent->insertRow(target.row(), items);
    }
}

/*************************************************************************//*!
 * This creates a new album root item or collection root item.
 * The created item depends on type, receives the appropriate icon and is
 * added below the related root. When the tree gets populated, the type
 * either is TypeAlbumRoot or TypeCollectionRoot. When a new database gets
 * created by dialog, the type is TypeAlbum or TypeCollection. In these cases
 * we have to add a new album root or collection root below the anchors. 
 * Concerning collections we have to distinct several cases:
 * - Create new database (from menu via collectionTreeView), joined=false,
 *   attention=false, rootdir=empty when db already exists, rootdir= db root
 *   when the db is new: The item has to be added.
 * - Setup database at startup, joined=false, attention=false: The item has
 *   to be added, the  existing database must be opened to read the root dir.
 * - Setup database at startup, joined=false, attention=true: The database
 *   either doesn't exist or isn't readable or isn't writable. The item has
 *   to be added with type=TypeNone and attention mark
 * - Open database (from menu via collectionTreeView), joined=true, attention=false.
 *   The db exists and is readable. The item has to be added, parameter
 *   "rootdir" holds the root directory = db location.
 */
void    KbvCollectionTreeModel::createItem(const QString name, QString rootdir, int type, bool attention, bool joined)
{
  QStandardItem   *item, *child;
  QString         collRootDir, path, str;
  QDir::Filters   filter;
  QDir            actualDir;

  //qDebug() << "KbvCollectionTreeModel::createItem" <<name <<rootdir <<type <<attention; //###########
  //append photo album
  //Qt::DisplayRole contains the name of the collection.
  //Kbv::CollectionNameRole contains the name of the database
  //Kbv::CollectionTypeRole contains the type of the database
  //Kbv::DatabaseLocationRole contains the path to the database
  if(type & Kbv::TypeAlbum || type & Kbv::TypeAlbumRoot)
    {
      item = new QStandardItem();
      item->setData(QVariant(name), Qt::DisplayRole);
      item->setData(QVariant(Kbv::TypeAlbumRoot), Kbv::CollectionTypeRole);
      item->setData(QVariant(name), Kbv::CollectionNameRole);
      path = settings->dataBaseDir + "/";
      item->setData(QVariant(path), Kbv::DatabaseLocationRole);
      if(attention)
        {
          item->setData(QVariant(generalFunc.iconAttention), Qt::DecorationRole);
          item->setData(QVariant(Kbv::TypeNone), Kbv::CollectionTypeRole);
        }
      albumAnchorItem->appendRow(item);
      return;
    }

  //Create new database or setup existing db at startup
  //Kbv::CollectionTypeRole contains the type of the database and the bit joined
  //Kbv::CollectionRootDirRole contains the absolute collection root directory
  //Kbv::FilePathRole contains the relative path of a branch (empty in root).
  //Qt::DisplayRole contains the name of the collection.
  //Kbv::DatabaseLocationRole contains the path to the database
  if(type & Kbv::TypeCollection || type & Kbv::TypeCollectionRoot)
    {
      item = new QStandardItem();
      item->setData(QVariant(name), Qt::DisplayRole);
      item->setData(QVariant(QString("")), Kbv::FilePathRole);
      item->setData(QVariant(name), Kbv::CollectionNameRole);
      //joined=false, attention=false
      //rootdir=empty when db already exists -> read root dir from db
      //rootdir = db root when the db is new
      if(!joined && !attention)
        {
          if(rootdir.isEmpty())
            {
              //startup, append existing database
              //qDebug() << "KbvCollectionTreeModel::createItem startup" <<name; //###########
              QSqlDatabase  db;
              QSqlQuery     query;
              //existing database, read root dir from table "description"
              db = QSqlDatabase::addDatabase("QSQLITE", "tree"+name);
              db.setHostName("host");
              path = settings->dataBaseDir + "/" + name + QString(dbNameExt);
              db.setDatabaseName(path);
              if(db.open())
                {
                  str = QString("SELECT rootdir FROM description");
                  query = QSqlQuery(str, db);
                  if(query.isActive())
                    {
                      while(query.next())
                        {
                          collRootDir = query.value(0).toString();
                        }
                    }
                  query.clear();
                }
              db = QSqlDatabase();                        //destroy db-object
              QSqlDatabase::removeDatabase("tree"+name);  //close connection
            }
          else
            {
              //new database
              collRootDir = rootdir;
              //qDebug() << "KbvCollectionTreeModel::createItem new db or attention" <<name; //###########
            }
          item->setData(QVariant(collRootDir), Kbv::CollectionRootDirRole);
          item->setData(QVariant(Kbv::TypeCollectionRoot), Kbv::CollectionTypeRole);
          item->setData(QVariant(settings->dataBaseDir+"/"), Kbv::DatabaseLocationRole);
        }

      //joined=false, attention=true: The db doesn't exist or isn't readable or writable.
      if(!joined && attention)
        {
          item->setData(QVariant(generalFunc.iconAttention), Qt::DecorationRole);
          item->setData(QVariant(Kbv::TypeNone), Kbv::CollectionTypeRole);
          item->setData(QVariant(settings->dataBaseDir+"/"), Kbv::DatabaseLocationRole);
          collRootDir = "";
        }
      //joined=true: open external db, dbLocation = rootDir
      if(joined)
        {
          collRootDir = rootdir;
          item->setData(QVariant(Kbv::TypeCollectionRoot | Kbv::TypeJoined), Kbv::CollectionTypeRole);
          item->setData(QVariant(collRootDir), Kbv::CollectionRootDirRole);
          item->setData(QVariant(collRootDir), Kbv::DatabaseLocationRole);
          item->setData(QVariant(QColor(Qt::darkCyan)), Qt::ForegroundRole);
          //qDebug() << "KbvCollectionTreeModel::createItem joined" <<name <<collRootDir <<type; //###########
        }

      //append in tree
      collectionAnchorItem->appendRow(item);

      //Add a dummy to get expansion grips when rootdir already exists and has sub dirs
      //or create rootdir
      //qDebug() << "KbvCollectionTreeModel::createItem rootdir"<<collRootDir; //###########
      if(!collRootDir.isEmpty())
        {
          actualDir.setPath(collRootDir);
          actualDir.setSorting(QDir::Name | QDir::LocaleAware);
          filter = QDir::AllDirs | QDir::NoDotAndDotDot;
          actualDir.setFilter(filter);
          if(actualDir.exists())
            {
              //qDebug() << "KbvCollectionTreeModel::createItem existing dir"<<collRootDir; //###########
              //insert a dummy item if there are sub dirs
              if(!actualDir.entryList().isEmpty())
                {
                  child = new  QStandardItem();
                  child->setData(QVariant(QString("kbvDummy")), Qt::DisplayRole);
                  item->appendRow(child);
                }
            }
          else
            {
              //qDebug() << "KbvCollectionTreeModel::createItem new dir"<<collRootDir; //###########
              //create root dir for new collection
              actualDir.mkdir(collRootDir);
            }
        }
    }
}
/*************************************************************************//*!
 * Called by view. Create a new collection branch "name" below the item
 * indicated by "index". This creates the related subdirectory also.
 * The view assures that index is a collection root or branch.
 */
void    KbvCollectionTreeModel::insertItem(const QModelIndex &index, const QString name)
{
  QStandardItem *item, *parent;
  QDir          dir;
  QString       parentPath, parentName, rootDir, msg;
  QVariant      var_collection, var_rootDir, var_location, var_foreGround;
  int           type;

  var_collection = this->data(index, Kbv::CollectionNameRole);
  var_rootDir    = this->data(index, Kbv::CollectionRootDirRole);
  var_location   = this->data(index, Kbv::DatabaseLocationRole);
  var_foreGround = this->data(index, Qt::ForegroundRole);

  parentPath = this->data(index, Kbv::FilePathRole).toString();
  parentName = this->data(index, Qt::DisplayRole).toString();
  type       = this->data(index, Kbv::CollectionTypeRole).toInt();
  rootDir    = var_rootDir.toString();

  //rootDir contains the absolute collection root dir
  //parentPath/parentName is the relative path of the parent dir.
  //parentPath/parentName/name is the relative path of the new dir.
  //parentPath/parentName must be written into filePathRole of the new item
  //location contains the path to the database file
  //When the index points to collection root, don't append parentName since
  //this could be the collection name and therefore produce an invalid path
  if(!(type & Kbv::TypeCollectionRoot))
    {
      if(!parentPath.isEmpty())
        {
          parentPath.append("/");
        }
      parentPath.append(parentName);
    }
  type = Kbv::TypeCollection;         //clear type joined

  //qDebug() << "KbvCollectionTreeModel::insertItem" <<rootDir <<parentPath <<name; //###########
  dir.setPath(rootDir + parentPath);
  if(dir.mkdir(name))
    {
      parent = this->itemFromIndex(index);
      item = new QStandardItem();
      item->setData(QVariant(name), Qt::DisplayRole);
      item->setData(QVariant(type), Kbv::CollectionTypeRole);
      item->setData(QVariant(parentPath), Kbv::FilePathRole);
      
      item->setData(var_collection, Kbv::CollectionNameRole);
      item->setData(var_rootDir, Kbv::CollectionRootDirRole);
      item->setData(var_location, Kbv::DatabaseLocationRole);
      item->setData(var_foreGround, Qt::ForegroundRole);
      parent->appendRow(item);
    }
  else
    {
      msg = QString(tr("Cannot create the desired directory!"));
      informationDialog->perform(msg, QString(), 1);
    }
}
/*************************************************************************//*!
 * Called by view. Delete the item under "index" (delete collection branch).
 * The branch is empty, all content got previously removed from file system.
 * At this point no action on the database has been performed.
 * The view doesn't check the item type so we have to do it here.
 */
void    KbvCollectionTreeModel::deleteItem(const QModelIndex &index)
{
  QStandardItem *parent, *item;
  QDir          dir;
  QString       rootDir, name, path, msg1, msg2;
  int           type;
  bool          nobranch;

  nobranch = true;
  if(index.isValid())
    {
      //rootDir' contains the absolute collection root dir
      //path is the relative path of the parent dir.
      //path/name is the relative path of the dir to remove.
      item = this->itemFromIndex(index);
      type = item->data(Kbv::CollectionTypeRole).toInt();
      path     = item->data(Kbv::FilePathRole).toString();
      name     = item->data(Qt::DisplayRole).toString();
      rootDir  = this->data(index, Kbv::CollectionRootDirRole).toString();
      //qDebug() << "KbvCollectionTreeModel::deleteItem" <<path <<name <<type; //###########
      
      //Branches always are of Kbv::TypeCollection
      if(type & Kbv::TypeCollection)
        {
          nobranch = false;
          //The parent path is empty, when it is identical to the root dir
          if(!path.isEmpty())
            {
              path.append("/");
            }
          path.append(name);
          dir.setPath(rootDir + path);
          dir.setSorting(QDir::NoSort);
          dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
          if(dir.entryList().isEmpty())
            {
              if(dir.rmdir(rootDir + path))
                {
                  parent = item->parent();
                  for(int i=0; i<parent->rowCount(); i++)
                    {
                      item = parent->child(i, 0);
                      if((item->data(Qt::DisplayRole)).toString() == name)
                        {
                          parent->removeRow(i);
                        }
                    }
                 }
                else
                  {
                    msg1 = QString(tr("Could not remove branch!"));
                    informationDialog->perform(msg1, QString(), 1);
                  }
            }
          else
            {
              msg1 = QString(tr("The branch is not empty!"));
              msg2 = QString(tr("Please save all the content then delete the branch."));
              informationDialog->perform(msg1, msg2, 1);
            }
        }
    }

  if(nobranch)
    {
      msg1 = QString(tr("Please select a collection branch!"));
      informationDialog->perform(msg1, QString(), 1);
    }
}
/*************************************************************************//*!
 * Called on populating the collection tree.
 * Check if name is a file, readable and writable and is a SQLite3 data base.
 * This can be verified reading the first 20 characters of the file and
 * looking for the string "SQLite format 3".
 */
bool    KbvCollectionTreeModel::existsDatabase(const QString &name)
{
  QString       path;
  QFileInfo     info;
  QFile         dbFile;
  QTextStream   in;
  QString       content;

  path = settings->dataBaseDir + "/" + name + QString(dbNameExt);

  info.setFile(path);
  if(!info.isFile() || !info.isReadable() || !info.isWritable())
    {
      return false;
    }
  //qDebug() << "KbvCollectionTreeModel::existsDatabase" << path; //###########
  dbFile.setFileName(path);
  dbFile.open(QIODevice::ReadOnly);
  in.setDevice(&dbFile);
  content = in.read(20);  //only 15 chars needed.
  if (!content.startsWith(QString(dbIdentifier)))
    {
      dbFile.close();
      return false;
    }
  dbFile.close();
  return true;
}
/*************************************************************************//*!
 * Called by view. Remove the database "title" of "type" from tree but do
 * not delete the database file. The collection anchor or album anchor must
 * be selected to perform this.
 */
void    KbvCollectionTreeModel::removeDatabase(const QModelIndex &index)
{
  QStandardItem *item, *parent;
  int type;

  item = itemFromIndex(index);
  parent = item->parent();
  if(parent != 0)
    {
      type = parent->data(Kbv::CollectionTypeRole).toInt();
      if((type & Kbv::TypeAlbumAnchor) || (type & Kbv::TypeCollectionAnchor))
        {
          parent->removeRow(item->row());
          this->storeCollTreeState();
        }
    }
}
/*************************************************************************//*!
 * Called by view. Rename photo album or collection or collection branch.\n
 * The preconditions already have been checked by the view.
 * When renaming is performed a signal is sent to collectionTabs updating
 * the tab name and the tree gets collapsed. Latter is necessary since the
 * expanded items would contain a wrong collection name.
 * For renaming collection branches the complete relative paths are neccessary.
 * This prevents renaming similiar parts of other branches.
 * The path to the database file must be read fom item (opened databases!).
 */
void    KbvCollectionTreeModel::renameDatabase(const QModelIndex &index, const QString oldName, const QString newName)
{
  QString       rootDir, path, oldPath, newPath, collName;
  QString       msg1;
  QDir          dir;
  QFile         file;
  int           type;


  msg1 = QString(tr("Cannot rename the database!"));

  type = this->data(index, Kbv::CollectionTypeRole).toInt();
  //qDebug() << "KbvCollectionTreeModel::renameDatabase type" <<type <<oldName <<newName; //###########

  //rename database and when successful rename album/collection and the related tab
  if((type & Kbv::TypeCollectionRoot) || (type & Kbv::TypeAlbumRoot))
    {
      path = this->data(index, Kbv::DatabaseLocationRole).toString();
      file.setFileName(path + oldName + QString(dbNameExt));
      if(file.permissions() | QFile::WriteUser | QFile::WriteOwner)
        {
          if(file.rename(path + newName + QString(dbNameExt)))
            {
              this->setData(index, QVariant(newName), Qt::DisplayRole);
              this->setData(index, QVariant(newName), Kbv::CollectionNameRole);
              emit collRenamedInTree(oldName + "\n" + newName);
            }
          else
            {
              informationDialog->perform(msg1,QString(""),1);
            }
        }
    }
  if(type & Kbv::TypeCollection)
    {
      //rename branch and when successful rename all concerned dir paths in the database
      //the view gets collapsed one level above since after renaming all paths may be wrong
      rootDir  = this->data(index, Kbv::CollectionRootDirRole).toString();
      collName = this->data(index, Kbv::CollectionNameRole).toString();
      path     = this->data(index, Kbv::FilePathRole).toString();
      if(!path.isEmpty())
        {
          path.append("/");
        }
      //absolute paths for renaming dir
      oldPath = rootDir + path + oldName;
      newPath = rootDir + path + newName;
      //qDebug() << "KbvCollectionTreeModel::rename branch old new" <<collName <<path+oldName <<path+newName; //###########
      if(dir.rename(oldPath, newPath))
        {
          //relative paths for database
          this->setData(index, QVariant(newName), Qt::DisplayRole);
          emit branchRenamedInTree(collName + "\n" + path+oldName + "\n" + path+newName);
        }
      else
        {
          informationDialog->perform(msg1,QString(""),1);
        }
    }
}
/*************************************************************************//*!
 * SLOT: Called by view. Expand index.
 * The index contains at least a dummy item when there are children. Now we
 * have to remove the dummy and insert the children. This keeps all in an
 * actual state even when the directory is altered from outside.
 */
bool    KbvCollectionTreeModel::expand(const QModelIndex &index)
{
  QDir          actualDir;
  QStandardItem *item, *child, *dummy;
  QStringList   entryList;
  QString       rootDir, path, name;
  QVariant      var_location, var_foreGround;
  int           type, i, k;

  if (!index.isValid())
    {
      //qDebug() << "KbvCollectionTreeModel::expand index not valid"; //###########
      return false;
    }
  emit setWaitCursor(true);
  //rootDir contains the absolute collection root dir
  //path is the relative path of the parent dir.
  //path/name is the relative path of the dir to expand.
  //location is the path to the database file
  item    = itemFromIndex(index);
  path    = item->data(Kbv::FilePathRole).toString();
  name    = item->data(Qt::DisplayRole).toString();
  rootDir = item->data(Kbv::CollectionRootDirRole).toString();
  type    = item->data(Kbv::CollectionTypeRole).toInt();
  var_location   = item->data(Kbv::DatabaseLocationRole);
  var_foreGround = this->data(index, Qt::ForegroundRole);

  //for root items the path is empty
  if(!(type & Kbv::TypeCollectionRoot))
    {
      if(!path.isEmpty())
        {
          path.append("/");
        }
      path.append(name);
    }
  //qDebug() << "KbvCollectionTreeModel::expand" << path; //###########

  actualDir.setPath(rootDir + path);
  actualDir.setSorting(QDir::NoSort);
  actualDir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

  entryList = actualDir.entryList();
  std::sort(entryList.begin(), entryList.end(), collator);
  //qDebug() << "KbvCollectionTreeModel::expand subdirs" << entryList; //###########
  
  //do not expand if no sub dirs and return false
  if(entryList.isEmpty())
    {
      emit setWaitCursor(false);
      return false;
    }
  //do not expand if already expanded (no dummy item) and return true
  name = item->child(0,0)->data(Qt::DisplayRole).toString();
  if(name != "kbvDummy")
    {
      emit setWaitCursor(false);
      return true;
    }
  //1. step:
  //remove dummy of expanded item and add collection sub dirs
  //the new children contain dirpath, dirname and collectionname
  item->removeRow(0);
  name = item->data(Kbv::CollectionNameRole).toString();
  type = item->data(Kbv::CollectionTypeRole).toInt();
  //when the collection root gets expanded we have to insert children
  //of type Kbv::TypeCollection
  if(type & Kbv::TypeCollectionRoot)
    {
      type = Kbv::TypeCollection;
    }
  for(i=0; i < entryList.length(); i++)
    {
      //qDebug() << "KbvCollectionTreeModel::expand child" << path << entryList[i] << name; //###########
      child = new  QStandardItem();
      child->setData(QVariant(entryList[i]), Qt::DisplayRole);
      child->setData(QVariant(path), Kbv::FilePathRole);
      child->setData(QVariant(name), Kbv::CollectionNameRole);
      child->setData(QVariant(rootDir), Kbv::CollectionRootDirRole);
      child->setData(QVariant(type), Kbv::CollectionTypeRole);
      child->setData(var_location, Kbv::DatabaseLocationRole);
      child->setData(var_foreGround, Qt::ForegroundRole);
      item->appendRow(child);
    }
  //2. step:
  //check children for sub dirs and add dummies if required
  //the children contain both path and name
  actualDir.setSorting(QDir::NoSort);
  k = item->rowCount();
  for(i=0; i < k; i++)
    {
      child = item->child(i, 0);
      path = child->data(Kbv::FilePathRole).toString();
      name = child->data(Qt::DisplayRole).toString();
      if(!path.isEmpty())
        {
          path.append("/");
        }
      path.append(name);
      //qDebug() << "KbvCollectionTreeModel::expand check for dummy" << path; //###########
      actualDir.setPath(rootDir + path);
      if(!actualDir.entryList().isEmpty())
        {
          dummy = new  QStandardItem();
          dummy->setData(QVariant(QString("kbvDummy")), Qt::DisplayRole);
          child->appendRow(dummy);
        }
    }
  emit setWaitCursor(false);
  //qDebug() << "KbvCollectionTreeModel::expand actualDir"; //###########
  
  return true;
}
/*************************************************************************//*!
 * SLOT: Called by view. Collapse index.
 * We have to remove the children of index and insert a dummy item as long as
 * the index doesn't point to the anchors!
 */
void    KbvCollectionTreeModel::collapse(const QModelIndex &index)
{
  QDir::Filters filter;
  QDir          actualDir;
  QStandardItem *item, *dummy;
  QString       rootDir, path, name;
  int           k, type;


  //do not remove the collection/album root dirs below the anchors
  type = this->data(index, Kbv::CollectionTypeRole).toInt();
  if((type & Kbv::TypeAlbumAnchor) || (type & Kbv::TypeCollectionAnchor))
    {
      return;
    }

  //rootDir contains the absolute collection root dir
  //path is the relative path of the parent dir.
  item    = itemFromIndex(index);
  path    = item->data(Kbv::FilePathRole).toString();
  name    = item->data(Qt::DisplayRole).toString();
  rootDir = item->data(Kbv::CollectionRootDirRole).toString();

  //in root items the relative path is empty
  if(!(type & Kbv::TypeCollectionRoot))
    {
      if(!path.isEmpty())
        {
          path.append("/");
        }
      path.append(name);
    }
  //qDebug() << "KbvCollectionTreeModel::collapse" <<rootDir <<path; //###########

  //remove all children in reverse order
  k = item->rowCount();
  if(k>0)
    {
      item->removeRows(0, k);
    }

  //add dummy item if there still are sub directories
  actualDir.setPath(rootDir + path);
  filter = QDir::AllDirs | QDir::NoDotAndDotDot;
  actualDir.setFilter(filter);
  if(!actualDir.entryList().isEmpty())
    {
      dummy = new  QStandardItem();
      dummy->setData(QVariant(QString("kbvDummy")), Qt::DisplayRole);
      item->appendRow(dummy);
    }
}
/*************************************************************************//*!
 * SLOT: called from view to get the index for the collection "title".
 */
QModelIndex KbvCollectionTreeModel::indexFromCollection(const QString &title)
{
  QModelIndex   index, parent;
  QString       name;
  int           n, i;

  parent = this->indexFromItem(collectionAnchorItem);
  n = collectionAnchorItem->rowCount();
  for(i=0; i<n; i++)
    {
      index = this->index(i, 0, parent);
      name = this->data(index, Qt::DisplayRole).toString();
      if(name == title)
        {
          return index;
        }
    }
  return QModelIndex();
}
/*************************************************************************//*!
 * SLOT: This function restores the state of the collection tree out of a
 * previously stored list of strings where each string represents an album or
 * a collection. The string list is empty when invalid. Only the top level
 * tree branches are restored (the collection or album root dirs).
 * Nothing happens to databases.
 * While populating the tree the related databases are searched. If the
 * data base is not present a small red label is shown left to the album name
 * and the database type is set to TypeNone. Based on this label an adequate
 * tool tip informs the user.
 */
void    KbvCollectionTreeModel::setCollTreeState()
{
  QStringList     dbNames;
  QStandardItem   *anchor;
  QString         key;
  int             rows, type;

  //the invisible root item is parent for collection anchor and album anchor
  rows = this->invisibleRootItem()->rowCount();
  for(int i=0; i < rows; i++)
    {
      //the children of the anchors are collection roots and album roots
      //key contains the string identifier for collection or album type
      anchor = this->invisibleRootItem()->child(i, 0);
      type = anchor->data(Kbv::CollectionTypeRole).toInt();
      if(type & Kbv::TypeCollectionAnchor)
        {
          key = QString("%1").arg(Kbv::TypeCollectionRoot);
          type = Kbv::TypeCollectionRoot;
        }
      else if(type & Kbv::TypeAlbumAnchor)
        {
          key = QString("%1").arg(Kbv::TypeAlbumRoot);
          type = Kbv::TypeAlbumRoot;
        }
      //qDebug() << "KbvCollectionTreeModel::setCollTreeState key"<<key <<type; //###########

      //read all database names matching the key
      settings->beginGroup("App");
      settings->beginGroup("AlbumCollectionTree");
      dbNames = settings->value(key, QStringList()).toStringList();
      settings->endGroup();
      settings->endGroup(); //group App
      //qDebug() << "KbvCollectionTreeModel::setCollTreeState key names" <<key <<dbNames; //###########

      //check the databases and append them below the anchor
      for(int k=0; k<dbNames.length(); k++)
        {
          if(!dbNames.at(k).isEmpty())
            {
              //qDebug() << "KbvCollectionTreeModel::setCollTreeState set" << dbNames.at(k); //###########
              if(this->existsDatabase(dbNames.at(k)))
                {
                  this->createItem(dbNames.at(k), QString(), type, false, false);
                }
              else
                {
                  this->createItem(dbNames.at(k), QString(), type, true, false);
                }
            }
        }
    }
}

/*************************************************************************//*!
 * SLOT: The function retrieves all collections and all photo albums both as
 * a list of strings. The invisible root of collection tree contains the roots
 * for collections and albums.\n
 * So we use two nested loops to process those:
 * - Outer loop:
 *   Get child of invisible root item and use its type as key for the list. The
 *   child either is the collection root or the album root.
 * - Inner loop:
 *   Find all siblings of child and add their names to the string list. When done
 *   store the list and process next child.
 */
void    KbvCollectionTreeModel::storeCollTreeState()
{
  QStringList     branches = QStringList();
  QStandardItem   *anchor;
  QString         key, name;
  int             rows, childRows, type;

  //the invisible root item is parent for anchors of collections and albums
  rows = this->invisibleRootItem()->rowCount();
  //qDebug() << "KbvCollectionTreeModel::storeCollTreeState rows" <<rows; //###########
  for(int i=0; i < rows; i++)
    {
      //the children are the anchors of collections and albums
      //key contains the string identifier for type collection root or album root"
      anchor = this->invisibleRootItem()->child(i, 0);
      type = anchor->data(Kbv::CollectionTypeRole).toInt();
      if(type & Kbv::TypeCollectionAnchor)
        {
          key = QString("%1").arg(Kbv::TypeCollectionRoot);
        }
      else if(type & Kbv::TypeAlbumAnchor)
        {
          key = QString("%1").arg(Kbv::TypeAlbumRoot);
        }

      //the children of achor are the collection roots or album roots
      childRows = anchor->rowCount();
      for(int k=0; k < childRows; k++)
        {
          //do not store joined (external) collections
          type = anchor->child(k, 0)->data(Kbv::CollectionTypeRole).toInt();
          //qDebug() << "KbvCollectionTreeModel::storeCollTreeState type" << type; //###########
          if(!(type & Kbv::TypeJoined))
            {
              name = anchor->child(k, 0)->data(Qt::DisplayRole).toString();
              branches.append(name);
            }
        }
      //qDebug() << "KbvCollectionTreeModel::storeCollTreeState" <<key <<branches; //###########
      settings->beginGroup("App");
      settings->beginGroup("AlbumCollectionTree");
      settings->setValue(key, QVariant(branches));
      settings->endGroup();
      settings->endGroup(); //group App
      settings->sync();
      branches.clear();
    }
}
/*************************************************************************//*!
 * This function composes a tool tip with information about the database.
 * When the data base of a collection or album could not be found in the file
 * system or is corrupt a small label is presented left to the database name
 * and the type is TypeNone. In this case an error text gets displayed.
 * For album or collection anchors the tool tip is suppressed.
 * Tool tips are requested from view by tooTipRole in the data() function.
 */
QString KbvCollectionTreeModel::tooltip(const QModelIndex &index) const
{
  QString       tip;
  int           type;

  type = this->data(index, Kbv::CollectionTypeRole).toInt();

  //Show content of collection item - this only is for debugging
  if(type & Kbv::TypeCollection || type & Kbv::TypeCollectionRoot)
    {
      tip.append(QString("Name:   %1\n").arg(this->data(index, Kbv::CollectionNameRole).toString()));
      tip.append(QString("Root:   %1\n").arg(this->data(index, Kbv::CollectionRootDirRole).toString()));
      //tip.append(QString("Type:   %1\n").arg(type));
      tip.append(QString("DB-dir: %1").arg(this->data(index, Kbv::DatabaseLocationRole).toString()));
    }

  //Show content of album item - this only is for debugging
  if(type & Kbv::TypeAlbumRoot)
    {
      tip.append(QString("Name:   %1\n").arg(this->data(index, Kbv::CollectionNameRole).toString()));
      //tip.append(QString("Type:   %1\n").arg(type));
      tip.append(QString("DB-dir: %1").arg(this->data(index, Kbv::DatabaseLocationRole).toString()));
    }

  //for TypeNone no Bit is set (=0)
  if(type == Kbv::TypeNone)
    {
      tip.append(tr("Error! \nData base not found or not readable\n"
                "or not writable or file is corrupt!"));
    }
  return tip;
}

/****************************************************************************/
