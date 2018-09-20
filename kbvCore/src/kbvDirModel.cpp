/*****************************************************************************
 * kbvDirModel
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.02.08
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvInformationDialog.h"
#include "kbvDirModel.h"

extern  KbvSetvalues            *settings;
extern  KbvInformationDialog    *informationDialog;


KbvDirModel::KbvDirModel(QObject *parent) : QStandardItemModel(0, 1, parent)
{
  QDir::Filters filter;
  QString   name, path, home;
  int       i, cycle;
  
  //natural sorting of dir names containing numbers
  collator.setLocale(QLocale::system());
  collator.setNumericMode(true);

  settings->beginGroup("App");
  settings->beginGroup("Options-View");
  cycle = 1000 * (settings->value("dirWatchCycle", 2).toInt());
  settings->endGroup();
  settings->endGroup(); //group App

  watchTimer = new QTimer(this);
  watchTimer->setSingleShot(true);
  watchTimer->setInterval(cycle);
  connect(watchTimer, SIGNAL(timeout()), this, SLOT(dirWatcher()));
  
  //create dir object for evaluating directory contents
  actualDir = new QDir(QDir::rootPath());
  actualDir->setSorting(QDir::Name | QDir::LocaleAware);
  filter = QDir::AllDirs | QDir::NoDotAndDotDot;
  if(settings->showHiddenFiles)
    {
      filter = filter | QDir::Hidden;
    }
  actualDir->setFilter(filter);

  //dummy item
  dummyItem = new  QStandardItem();
  dummyItem->setText("kbvDummy");
  //dummyItem->setIcon(generalFunc.iconKbvNoSupport);

  //create invisible root
  rootItem = this->invisibleRootItem();

  //create top entry: /home/user
  home = QDir::homePath();
  i = home.lastIndexOf("/");
  name = home.right(home.length()-i-1);
  path = home.left(i+1);
  userItem = new QStandardItem();
  userItem->setData(QVariant(name), Qt::DisplayRole);
  userItem->setData(QVariant(generalFunc.iconUserHome), Qt::DecorationRole);
  userItem->setData(QVariant(path), Kbv::FilePathRole);
  userItem->setData(QVariant(name), Kbv::FileNameRole);
  rootItem->appendRow(userItem);

  //when subdirs are available add dummy to get a handle in view
  actualDir->setPath(home);
  if(!actualDir->entryList().isEmpty())
    {
      userItem->appendRow(dummyItem->clone());
    }

  //create top entry: file system
  home = QDir::rootPath();
  i = home.lastIndexOf("/");
  path = home.left(i+1);
  name = home.right(home.length()-i-1);
  //qDebug() << "KbvDirModel::KbvDirModel rootPath" << path; //###########
  //qDebug() << "KbvDirModel::KbvDirModel rootName" << name; //###########

  fileSystemItem = new QStandardItem();
  fileSystemItem->setData(QVariant(tr("File System")), Qt::DisplayRole);
  fileSystemItem->setData(QVariant(generalFunc.iconDriveHarddisk), Qt::DecorationRole);
  fileSystemItem->setData(QVariant(path), Kbv::FilePathRole);
  fileSystemItem->setData(QVariant(name), Kbv::FileNameRole);
  rootItem->appendRow(fileSystemItem);

  //when subdirs are available add dummy to get a handle in view
  actualDir->setPath(home);
  if(!actualDir->entryList().isEmpty())
    {
      fileSystemItem->appendRow(dummyItem->clone());
    }

  //default
  dirItem = 0;
  displayedPath = "";
}

KbvDirModel::~KbvDirModel()
{
  //qDebug() << "KbvDirModel::~KbvDirModel";//###########
  delete  actualDir;
  delete  dummyItem;
}
/************************************************************************//*!
 * SLOT: Add mounted devices.
 * This is called at startup or when a hotplugged device has been mounted.
 * The mountInfo contains one ore more triplets: label, mountPoint and icon
 */
void  KbvDirModel::mountDevice(QStringList mountInfo)
{
  QString           path;
  QStandardItem     *item;
  QIcon             deviceIcon;
  int               i, n;
  bool              found;
  QDir              dir;
  QDir::Filters     filter;
  
  //qDebug() << "KbvDirModel::mountDevice " <<mountInfo.length() <<mountInfo; //###########
  i = 0;
  while(i<mountInfo.length())
    {
      //check for already added or removed mount points
      //exclude user home (0) and file system (1)
      n = 2;
      found = false;
      while(n < rootItem->rowCount())
        {
          item = rootItem->child(n, 0);
          path = item->data(Kbv::FilePathRole).toString();
          path.append(item->data(Kbv::FileNameRole).toString());
    
          if(path == mountInfo.at(i+1))
            {
              found = true;
              break;
            }
          n++;
        }
      //qDebug() << "KbvDirModel::mountDevice path mounted" <<path <<found; //###########
      
      //mount device
      if(!found)
        {
          //find themed or default icon
          deviceIcon = generalFunc.getThemeIcon(mountInfo.at(i+2));

          dirItem = new QStandardItem();
          dirItem->setData(QVariant(mountInfo.at(i+1)), Kbv::FilePathRole);
          dirItem->setData(QVariant(QString("")), Kbv::FileNameRole);
          dirItem->setData(QVariant(mountInfo.at(i+0)), Qt::DisplayRole);
          dirItem->setData(QVariant(deviceIcon), Qt::DecorationRole);
          rootItem->appendRow(dirItem);

          //add dummy to get a handle in view when subdirs are present
          dir.setPath(mountInfo.at(i+1));
          dir.setSorting(QDir::Name | QDir::LocaleAware);
          filter = QDir::AllDirs | QDir::NoDotAndDotDot;
          if(settings->showHiddenFiles)
            {
              filter = filter | QDir::Hidden;
            }
          dir.setFilter(filter);
          if(!dir.entryList().isEmpty())
            {
              dirItem->appendRow(dummyItem->clone());
            }
        }
      i=i+3;  //next triplet
    }
}
/************************************************************************//*!
 * SLOT: remove dynamically unmounted device.
 * The mountInfo contains two strings: label and mountPoint
 */
void  KbvDirModel::unmountDevice(QStringList mountInfo)
{
  QString           path;
  QStandardItem     *item;
  int               n;

  //qDebug() << "KbvDirModel::unmountDevice label mountPoint" <<mountInfo.at(0) <<mountInfo.at(1); //###########
  //check for available mount points
  //exclude user (0) and file system (1)
  n = 2;
  while(n < rootItem->rowCount())
    {
      item = rootItem->child(n, 0);
      path = item->data(Kbv::FilePathRole).toString();
      path.append(item->data(Kbv::FileNameRole).toString());

      if(path == mountInfo.at(1))
        {
          //qDebug() << "KbvDirModel::mountDevice path unmounted" <<path; //###########
          rootItem->removeRow(n);
          break;
        }
      n++;
    }
}
/************************************************************************//*!
 * The function returns values due to roles the view is asking for:
 * display name -> display role,
 * icon -> decoration role,
 * dir path -> file path role,
 * dir name -> file name role,
 */
QVariant  KbvDirModel::data(const QModelIndex &index, int role) const
{
  QStandardItem *item;

  //qDebug() << "KbvDirModel::data role" << role; //###########
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
  if (role == Kbv::FileNameRole)
    {
      return item->data(Kbv::FileNameRole);
    }
  return QVariant();    //All unused roles
}

/************************************************************************//*!
 * The flags() function sets the first column (col0) enabled, selectable and
 * ready for drop.
 * All other (not used) cols contain a combination provided by base class.
 */
Qt::ItemFlags KbvDirModel::flags(const QModelIndex &index) const
{
  if (index.isValid() && index.column() == 0)
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
    }
  return QStandardItemModel::flags(index);
}

/************************************************************************//*!
 * Getter for album root item and collection root item.
 */
QStandardItem*  KbvDirModel::userRoot()
{
  return userItem;
}
QStandardItem*    KbvDirModel::fileSystemRoot()
{
  return fileSystemItem;
}

/************************************************************************//*!
 * Supported drop actions are copy and move.
 */
Qt::DropActions KbvDirModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

/************************************************************************//*!
 * This function tries to restore the state of dir tree due to previously
 * saved list of expanded branches.
 * The string list contains a string for each branch to expand.
 * An empty list forces to read the settings. The algorithm expects that the
 * view already is expanded at userItem. The file root "/" is excluded.
 * The list is processed in the outer loop. Each string is divided into subsets
 * containing a valid part of a path to a directory. These subsets are processed
 * in the inner loop where they get expanded one after the other. The current
 * get changed to the last expanded branch at the end of the algorithm.
 */
void    KbvDirModel::setTreeState(QStringList strlist)
{
  QStandardItem *parent, *child;
  QStringList   expandedPaths;
  QString       path, str, selectedPath;
  int           i, n, len, rc;
  bool          found;

  //default initialisation
  found = false;
  parent = userItem;
  child = userItem;
  //get tree state from stored configuration file
  //or use the list in method arguments
  expandedPaths = strlist;
  if(strlist.isEmpty())
    {
      settings->beginGroup("App");
      settings->beginGroup("MainWindow");
      expandedPaths = settings->value("dirTreeState", QStringList()).toStringList();
      selectedPath = settings->value("dirTreeActive", QString()).toString();
      settings->endGroup();
      settings->endGroup(); //group App
      if(expandedPaths.isEmpty())
        {
          //nothing in settings
          emit expandView(indexFromItem(parent));       //set dir as current
          this->activatedDir(indexFromItem(parent));    //set file model
          return;
        }
    }
  //Outer loop: process entries in list of expanded paths, remember length
  //of home dir and set userItem as parent
  for(i=0; i<expandedPaths.length(); i++)
    {
      //qDebug() <<"KbvDirModel::setTreeState expand" << expandedPaths[i] <<i; //###########
      //The rootItem contains /home/user/.., the file root /.. and mounted file
      //systems /media/... as children
      //So we have to find the appropriate child, which is matching the path but
      //we exclude the file root since this contains /home/user or /media also
      rc = rootItem->rowCount();
      for(int k=0; k<rc; k++)
        {
          path = rootItem->child(k, 0)->data(Kbv::FilePathRole).toString();
          path += rootItem->child(k, 0)->data(Kbv::FileNameRole).toString();
          //qDebug() <<"KbvDirModel::setTreeState path" <<k <<path; //###########

          //Exclude file system root "/"
          if(path != "/")
            {
              if(expandedPaths[i].startsWith(path))
                {
                  parent = rootItem->child(k, 0);
                  found = true;
                  break;
                }
            }
        }
      if(found)
        {
          found = false;
          //The parent item already contains the mountpoint in dirTree (/home/user/
          //or /media/label/ or /media/user/label/), so we have to start right of this
          path = parent->data(Kbv::FilePathRole).toString();
          path += parent->data(Kbv::FileNameRole).toString();
          len = path.length();
          //qDebug() << "KbvDirModel::setTreeState path begins" <<path; //###########

          //Inner loop: We begin at the end of the mountpoint since this is already
          //expanded. Structure of a path: "/..../.../....."
          //the while loop stops when no more "/" can be found
            n = len;
            while (n>=0)
              {
              //find all sub paths and at last the longest one containing
              //the complete string (not terminated by "/")
                n = expandedPaths[i].indexOf("/", n+1, Qt::CaseInsensitive);
                if(n<0)
                  {
                    path = expandedPaths[i];
                  }
                else
                  {
                    path = expandedPaths[i].left(n);
                  }
                //qDebug() << "KbvDirModel::setTreeState set path" << i << path; //###########
                //now we know the name of a dir and if it's not more existing we
                //have to finish the inner loop and process next entry in list
                actualDir->setPath(path);
                if(!actualDir->exists())
                  {
                    break;
                  }

                //Find the dir in the child items of parent, get an model index,
                //and expand it. When expanded set it as new parent.
                //Finish loop
                rc = parent->rowCount();
                for (int j=0; j<rc; j++)
                  {
                    child = parent->child(j, 0);
                    str = child->data(Kbv::FilePathRole).toString();
                    str += child->data(Kbv::FileNameRole).toString();
                    //qDebug() << "KbvDirModel::setTreeState found" << str; //###########
                    if(str == path)
                      {
                        if (expandDir(indexFromItem(child)))
                          {
                            parent = child;
                          }
                        //expand tree but do not change current dir
                        emit expandTree(indexFromItem(child));
                        break;
                      }
                  }
              }
        }
    }
  
  //The desired tree state has been restored, now restore the active dir
  if(!selectedPath.isEmpty())
    {
      //find item of selectedPath
      bool valid;
      parent = findItemForPath(selectedPath, valid);
      if(valid)
        {
          actualDir->setPath(selectedPath);
          child = parent;
        }
    }
  //expand view and send a signal to file model fill thread
  if (actualDir->exists())
    {
      //qDebug() << "KbvDirModel::setTreeState last activated" << actualDir->path(); //###########
      emit expandView(indexFromItem(child));       //set dir as current
      this->activatedDir(indexFromItem(child));    //set file model
    }
}

/************************************************************************//*!
 * This function retrieves all expanded branches of user directory as list of
 * strings. This is done with a recursive helper function.
 * Then the list is stored in configuration file if required.
 */
void    KbvDirModel::storeTreeState()
{
  QStringList   expandedPaths, paths;
  QStandardItem *item;
  int           rc;

  expandedPaths = QStringList();
  rc = userItem->rowCount();
  if(rc > 0)
    {
      for (int i=0; i < rc; i++)
        {
          item = userItem->child(i, 0);
          paths = parseBranch(item);
          if(!paths.isEmpty())
            {
              expandedPaths.append(paths);
            }
        }
    }
  //Write state of directory tree. Keep previous settings if not enabled.
  if (settings->saveDirTreeState)
    {
      //qDebug() << "KbvDirModel::getTreeState" << expandedPaths; //###########
      settings->beginGroup("App");
      settings->beginGroup("MainWindow");
      settings->setValue("dirTreeState", expandedPaths);
      settings->setValue("dirTreeActive", displayedPath);
      settings->endGroup();
      settings->endGroup(); //group App
      settings->sync();
    }
}

/************************************************************************//*!
 * Recursive test for expanded children. If row count of parent is zero an
 * empty list is returned. If parent contains only one row this may be dummy
 * item or a sub dir which may contain further dirs we have to parse.
 * If parent contains more rows we have to parse them in a loop and step down
 * for each one to find the last which is expanded.
 */
QStringList    KbvDirModel::parseBranch(const QStandardItem *parent)
{
  QStandardItem *child;
  QStringList   paths, subs;
  QString       str;
  int           rc;

  paths = QStringList();
  str = "";
  rc = parent->rowCount();
  //one subdir or dummy item only
  if(rc == 1)
    {
      str = parent->data(Kbv::FilePathRole).toString();
      str += parent->data(Kbv::FileNameRole).toString();
      child = parent->child(0, 0);
      if(child->text() != "kbvDummy")
        {
          subs = parseBranch(child);
          if(!subs.isEmpty())
            {
              paths.append(subs);
            }
          else
            {
              paths.append(str);
            }
        }
    }
  //more than one subdir
  if(rc > 1)
    {
      for (int i=0; i < rc; i++)
        {
          child = parent->child(i, 0);
          if(child->text() != "kbvDummy")
            {
              subs = parseBranch(child);
              if(!subs.isEmpty())
                {
                  paths.append(subs);
                }
            }
        }
      //if not expanded -> return parent
      if(paths.isEmpty())
        {
          str = parent->data(Kbv::FilePathRole).toString();
          str += parent->data(Kbv::FileNameRole).toString();
          paths.append(str);
        }
    }
  return paths;
}

/************************************************************************//*!
 * Returns the pointer to the item at the end of the path 'selectedPaths'.
 * itemValid=true when the item exists. 
 */
QStandardItem*	KbvDirModel::findItemForPath(const QString &selectedPath, bool &itemValid)
{
  QStandardItem *parent, *child;
  QString       path, str;
  int           len, n, rc;
  bool          next;

  //default initialisation
  parent = userItem;
  child = userItem;
  next = false;

  //qDebug() << "KbvDirModel::findItemForPath selectedPath" <<selectedPath; //###########
  //The userIten already contains /home/user, so we have to start right of this
  path = userItem->data(Kbv::FilePathRole).toString();
  path += userItem->data(Kbv::FileNameRole).toString();
  len = path.length();

  //Parse all substrings surrounded by "/.../" in 'selectedPath', beginning after "/user/home/"
  //and try to find them in one of the children of parent.
  //When found take this child as new parent and process the next substring in 'selectedPath'
  //The while loop finishes when the complete string 'selectedPath' has been examined.
    n = len;
    while(n>=0)
      {
      //find all sub paths and at last the longest one containing
      //the complete string (not terminated by "/")
        n = selectedPath.indexOf("/", n+1, Qt::CaseInsensitive);
        if(n<0)
          {
            path = selectedPath;
          }
        else
          {
            path = selectedPath.left(n);
          }
        //check if one of the children is part of selectedPath and use this as new parent
        //when not found set next=false
        rc = parent->rowCount();
        for (int j=0; j<rc; j++)
          {
            child = parent->child(j, 0);
            str = child->data(Kbv::FilePathRole).toString();
            str += child->data(Kbv::FileNameRole).toString();
            //qDebug() << "KbvDirModel::findItemForPath path child" <<path <<str; //###########
            if(str == path)
              {
                parent = child;
                next = true;
                break;      //for loop
              }
            else
              {
                next = false;
              }
          }
      }
    //when selectedPath was found in a sequence of items the last item is the
    //interesting one and next=true. Otherwise next=false.
    itemValid = next;
    return child;
}

/************************************************************************//*!
 * SLOT: collects all events from view or model which activated a dir.
 * The path of the activated dir must be read from index. When the path is
 * different to the stored one, the watchTimer has to be stopped.
 * When the activated dir exists, the path will be stored and sent to the
 * fileView. The index is stored too and the watchTimer get started again.
 * The existance of the activated dir must be checked since it could have
 * been altered by an external app and the model is out of synchronisation.
 */
void    KbvDirModel::activatedDir(const QModelIndex &index)
{
  QDir        dir;
  QString     path;

  //do nothing if the path is being displayed
  path = this->data(index, Kbv::FilePathRole).toString();
  path += this->data(index, Kbv::FileNameRole).toString();
  if(displayedPath != path)
    {
      watchTimer->stop();

      dir.setPath(path);
      if(dir.exists())
        {
          //qDebug() << "KbvDirModel::activatedDir" <<path; //###########
          displayedPath = path;
          emit dirActivated(displayedPath);
          watchedIndex = QPersistentModelIndex(index);
          watchTimer->start();
        }
    }
}
/************************************************************************//*!
 * SLOT: watch activatedDir for changes in children and siblings and update
 * the model regularily. The related model index of the activated dir was
 * stored in watchedIndex.
 * When a dir get activated the function starts to watch the children and
 * siblings of the dir. Example:
 * A--A1
 * B--B1--B11
 *      --B12
 *      --B13
 * C--C1
 * D
 * Dir B1 get activated. This will watch the children B11, B12, B13 and B1 itself.
 * The function adds a handle to the watched dirs when children get detected and
 * removes the handle when there are no child dirs.
 * Added/removed sub dirs or siblings in the activated dir B1 get presented/removed
 * as children, resp. siblings
 * When the watched dir get removed the parent will be activated. When a parent dir
 * get removed, the tree will be collapsed to the next present parent.
 */
void    KbvDirModel::dirWatcher()
{
  QModelIndex     index;
  QStandardItem   *dirItem, *item;
  QString         dirPath, dirName, path, name;
  QStringList     L2;
  QDir            watchedDir;
  bool            hasDirs, hasChildren, last;
  int             row, n;

  last = false;
  index = watchedIndex;

  if(settings->showHiddenFiles)
    {
      watchedDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    }
  else
    {
      watchedDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    }
  //qDebug() << "KbvDirModel::dirWatcher watch" << index.data(Kbv::FileNameRole).toString(); //###########

  //Outer loop runs twice to perform two steps
  //last=false: step 1: process children of watchedIndex
  //last=true:  step 2: process siblings (the children of watchedIndex.parent)
  for (int j=0; j<2; j++)
    {
      L2.clear();
      //get item of index and watched dir path
      dirItem = this->itemFromIndex(index);
      dirPath = dirItem->data(Kbv::FilePathRole).toString();
      dirPath += dirItem->data(Kbv::FileNameRole).toString();
      watchedDir.setPath(dirPath);

      //branch 1. the watched dir doesn't exist any more
      if(!watchedDir.exists())
        {
          //qDebug() << "KbvDirModel::dirWatcher dir removed" <<dirPath; //###########
          //check if any parent dir exists (the model items still are valid) then
          //collapse view at this dir, stop at tree root
          while(dirItem != rootItem)
            {
              dirItem = dirItem->parent();
              dirPath = dirItem->data(Kbv::FilePathRole).toString();
              dirPath += dirItem->data(Kbv::FileNameRole).toString();
              watchedDir.setPath(dirPath);

              //qDebug() << "KbvDirModel::dirWatcher check dir" <<dirPath; //###########
              //valid parent found or ttree root reached
              //remove all children and collapse tree
              if(watchedDir.exists() || (dirItem->parent() == rootItem))
                {
                  //qDebug() << "KbvDirModel::dirWatcher dir found" <<dirPath; //###########
                  n = dirItem->rowCount();
                  index = dirItem->index();
                  emit layoutAboutToBeChanged();
                  for(int i=0; i<n; i++)
                    {
                      dirItem->removeRow(0);
                    }
                  emit layoutChanged();
                  emit collapseView(index);
                  break;
                }
            }
          break;  //stop outer loop, trigger timer
        } //outer loop - end of branch 1
      else
        {
          //branch 2. the watched dir still exists
          //get item from index and collect sub dirs
          dirItem = this->itemFromIndex(index);
          dirPath = dirItem->data(Kbv::FilePathRole).toString();
          dirPath += dirItem->data(Kbv::FileNameRole).toString();
          //qDebug() << "KbvDirModel::dirWatcher watched dir exists" <<dirPath; //###########
          watchedDir.setPath(dirPath);
          L2.append(watchedDir.entryList());

          if(L2.isEmpty())
            {
              //2a. when L2 is empty we simply remove all children of dirItem.
              //This collapses the tree at watchedIndex or watchedIndex.parent
              if(dirItem->hasChildren())
                {
                  //qDebug() << "KbvDirModel::dirWatcher watched dir without sub dirs"; //###########
                  n = dirItem->rowCount();
                  for (int i=0; i<n; i++)
                    {
                      emit layoutAboutToBeChanged();
                      dirItem->removeRow(0);
                      emit layoutChanged();
                    }
                }
              last = true;
              index = watchedIndex.parent();
            } //outer loop - end of branch 2a
          else
            {
              //2b. last=false: step 1: watched dir exists and has sub dirs: now check children
              //    last=true:  step 2: process siblings except the watched dir
              if(last)
                {
                  L2.removeAll(dirName);
                }
              //Debug() << "KbvDirModel::dirWatcher sub dirs" <<L2; //###########

              //inner loop:
              //all children of watched index which are contained in L2 must be
              //removed from L2
              //all children of watched index which are not in L2 must be
              //removed from model
              //remaining items in L2 must be added to the model
              row = 0;
              while (row < dirItem->rowCount())
                {
                  name = dirItem->child(row)->data(Kbv::FileNameRole).toString();
                  if(name == dirName)
                    {
                      //exclude activated dir in loop 2
                      row++;
                    }
                  else if((L2.indexOf(name) < 0) && ("kbvDummy" != dirItem->child(row)->text()))
                    {
                      //name not found in L2, remove non existant child
                      //don't remove dummy item and don't increase row counter
                      emit layoutAboutToBeChanged();
                      dirItem->removeRow(row);
                      emit layoutChanged();
                      //qDebug() << "KbvDirModel::dirWatcher remove child at" <<row <<name; //###########
                    }
                  else
                    {
                      //child found in L2
                      path = dirItem->child(row)->data(Kbv::FilePathRole).toString();
                      path += dirItem->child(row)->data(Kbv::FileNameRole).toString();
                      hasDirs = hasSubDirs(path);
                      hasChildren = dirItem->child(row)->hasChildren();
                      //qDebug() << "KbvDirModel::dirWatcher child" <<path; //###########

                      //consider 3 conditions
                      //1. no sub dirs present but child item has children -> remove them
                      if(!hasDirs && hasChildren)
                        {
                          //qDebug() << "KbvDirModel::dirWatcher remove children"; //###########
                          n = dirItem->child(row)->rowCount();
                          for (int i=0; i<n; i++)
                            {
                              emit layoutAboutToBeChanged();
                              dirItem->child(row)->removeRow(0);
                              emit layoutChanged();
                            }
                        }
                      //2. sub dirs present and child item has children -> remove dummy if any
                      else if (hasDirs && hasChildren)
                        {
                          if((dirItem->child(row)->rowCount() > 1) && ("kbvDummy" == dirItem->child(row)->child(0)->text()))
                            {
                              //qDebug() << "KbvDirModel::dirWatcher remove dummy"; //###########
                              emit layoutAboutToBeChanged();
                              dirItem->child(row)->removeRow(0);
                              emit layoutChanged();
                            }
                        }
                      //3. sub dirs present but child item has no children
                      // add sub dirs as children since the view could be expanded
                      else if (hasDirs && !hasChildren)
                        {
                          //qDebug() << "KbvDirModel::dirWatcher add dummy item" <<path; //###########
                          QDir  dir(path);
                          QStringList sl = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                          for(int k=0; k<sl.length(); k++)
                            {
                              item = new  QStandardItem();
                              item->setData(QVariant(sl.at(k)), Qt::DisplayRole);
                              item->setData(QVariant(generalFunc.iconFolder), Qt::DecorationRole);
                              item->setData(QVariant(path + "/"), Kbv::FilePathRole);
                              item->setData(QVariant(sl.at(k)), Kbv::FileNameRole);
                              emit layoutAboutToBeChanged();
                              dirItem->child(row)->appendRow(item);
                              emit layoutChanged();
                            }
                        }
                      //finally remove entry L2.name
                      L2.removeAt(L2.indexOf(name));
                      row++;
                    }
                } //end of inner loop

              //Remaining entries in L2: append new children to dirItem
              //dirItem with dummy: do nothing (view is collapsed)
              //dirItem with children: append new child (view is expanded)
              //dirItem without children: view state unknown -> collapse view
              if(L2.length() > 0)
                {
                  if (dirItem->hasChildren())
                    {
                      if("kbvDummy" != dirItem->child(0)->text())
                        {
                          //dirItem with children: append new child
                          for (int i=0; i < L2.length(); i++)
                            {
                              item = new  QStandardItem();
                              item->setData(QVariant(L2.at(i)), Qt::DisplayRole);
                              item->setData(QVariant(generalFunc.iconFolder), Qt::DecorationRole);
                              item->setData(QVariant(dirPath + "/"), Kbv::FilePathRole);
                              item->setData(QVariant(L2.at(i)), Kbv::FileNameRole);

                              emit layoutAboutToBeChanged();
                              dirItem->appendRow(item);
                              emit layoutChanged();

                              path = item->data(Kbv::FilePathRole).toString();
                              path += item->data(Kbv::FileNameRole).toString();
                              //qDebug() << "KbvDirModel::dirWatcher append new child" <<path; //###########
                            }
                        }
                    }
                  else
                    {
                      //dirItem without children: collapse view
                      //the view calls slot collapseDir which adds a dummy
                      emit collapseView(dirItem->index());
                      //qDebug() << "KbvDirModel::dirWatcher append new dummy"<<j; //###########
                    }
                }
              last = true;
              index = watchedIndex.parent();
            } //outer loop - end of branch 2b
        } //outer loop - end of branch 2

      //prepare step 2 of outer loop
      if(!index.isValid())
        {
          break;  //prevent invald indices (e.g. root index)
        }

    } //end of outer loop

  watchTimer->start();  
}

/************************************************************************//*!
 * Check for sub dirs in given path.
 */
bool    KbvDirModel::hasSubDirs(const QString &path)
{
  QDir  dir(path);
  return !dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty();
}

/************************************************************************//*!
 * SLOT: Called by dir watcher thread
 * Check all row items of model index parent if an expansion grip has to be
 * added or removed since someone added or removed sub dirs.
 * The grip appears in the view when a child item is added and disappears
 * when the row item doesn't contain children. Consider three cases:\n
 * - 1 The row contains items and no dummy and sub dirs: branch is expanded.
 *     this is checked by recursion down the branch
 * - 2 The row contains items and no sub dir can be found: remove all items
 *     and collapse view. This is necessary since the view remembers the
 *     state and may get confused.
 * - 3 The row doesn't contain items but sub dirs are found: add dummy item.
 */
void    KbvDirModel::checkExpansionGrip(const QModelIndex &parent)
{
  QStandardItem *parentItem, *child;
  QString       path;
  QStringList   dirlist;
  QDir::Filters filter;
  QDir          dir;

  if (!parent.isValid())
    {
      return;
    }
  dir.setSorting(QDir::Name | QDir::LocaleAware);
  filter = QDir::AllDirs | QDir::NoDotAndDotDot;
  if(settings->showHiddenFiles)
    {
      filter = filter | QDir::Hidden;
    }
  dir.setFilter(filter);

  if (this->hasChildren(parent))
    {
      //check all row items of model index parent
      parentItem = this->itemFromIndex(parent);
      for (int i=0; i<parentItem->rowCount(); i++)
        {
          child = parentItem->child(i, 0);
          path = child->data(Kbv::FilePathRole).toString();
          path += child->data(Kbv::FileNameRole).toString();
          //qDebug() << "KbvDirModel::checkExpansionGrip" << path; //###########
          dir.setPath(path);
          dirlist = dir.entryList();
          if (child->hasChildren())
            {
              //1 The row contains items and sub dirs and no dummy item: expanded
              //recursive processing of grandchildren
              if(!dirlist.isEmpty() && (child->child(0, 0)->data(Qt::DisplayRole) != "kbvDummy"))
                {
                  checkExpansionGrip(this->indexFromItem(child));
                }
              //2 The row contains items and no sub dirs: remove all items and collapse view
              //This is necessary since the view remembers the state and gets confused
              if(dirlist.isEmpty())
                {
                  child->removeRows(0, child->rowCount());
                  emit collapseView(this->indexFromItem(child));
                }
            }
          else
            {
              //3 The row doesn't contain items but sub dirs: add dummy item.
              if(!dirlist.isEmpty())
                {
                  child->appendRow(dummyItem->clone());
                }
            }
        }
    }
}
/************************************************************************//*!
 * SLOT: Expand directory at given index by view (signal expandDir).
 * When the dir doesn't contain subdirs or is aleady expanded nothing will
 * happen. If there are sub dirs first the dummy item at row 0 is removed
 * then an item for each sub directory is added.
 * If a new subdir contains further sub directories then append a dummy.
 * Return value:
 * FALSE if not expanded, TRUE if just expanded or already been expanded
 */
bool    KbvDirModel::expandDir(const QModelIndex &index)
{
  QDir          dir;
  QStandardItem *expandedItem, *item;
  QStringList   entries;
  QString       path, name;
  int           i, k;


  dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
  if(settings->showHiddenFiles)
    {
      dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);
    }
  dir.setSorting(QDir::NoSort);

  if (!index.isValid())
    {
      //qDebug() << "KbvDirModel::expandDir index not valid"; //###########
      return false;
    }
  
  emit setWaitCursor(true);
  expandedItem = itemFromIndex(index);
  path = expandedItem->data(Kbv::FilePathRole).toString();
  path += expandedItem->data(Kbv::FileNameRole).toString();
  if(path.length() > 1)
    {
      path += "/";    //root path only contains "/"
    }
  //qDebug() << "KbvDirModel::expandDir path" << path; //###########
  
  dir.setPath(path);
  entries = dir.entryList();
  std::sort(entries.begin(), entries.end(), collator);
  //qDebug() << "KbvDirModel::expandDir dirs" << entries; //###########
  
  //do not expand when no sub dirs, remove dummy item, return false
  if(entries.isEmpty())
    {
      expandedItem->removeRow(0);
      emit setWaitCursor(false);
      return false;
    }
  //do not try to expand if already expanded (no dummy item), return true
  if(expandedItem->child(0,0)->text() != "kbvDummy")
    {
      emit setWaitCursor(false);
      return true;
    }
  //1. step:
  //remove dummy of expanded item and add existent subdirs
  expandedItem->removeRow(0);
  for(i=0; i < entries.length(); i++)
    {
      dirItem = new  QStandardItem();
      dirItem->setData(QVariant(entries[i]), Qt::DisplayRole);
      dirItem->setData(QVariant(generalFunc.iconFolder), Qt::DecorationRole);
      dirItem->setData(QVariant(path), Kbv::FilePathRole);
      dirItem->setData(QVariant(entries[i]), Kbv::FileNameRole);
      expandedItem->appendRow(dirItem);
    }
  //2. step:
  //check subdirs for further subdirs and add dummies if required
  k = expandedItem->rowCount();
  for(i=0; i < k; i++)
    {
      item = expandedItem->child(i, 0);
      name = item->data(Kbv::FileNameRole).toString();
      dir.setPath(path+name);
      if(!dir.entryList().isEmpty())
        {
          item->appendRow(dummyItem->clone());
        }
    }
  emit setWaitCursor(false);
  return true;
}

/************************************************************************//*!
 * SLOT: Collapse directory at given index by view (signal collapseDir).
 * Remove all children of collapsed item. This destroys all children even when
 * the branch is expanded deeply. When the collapsed dir contains
 * sub directories then add a dummy item to get the handle in the view..
 */
void    KbvDirModel::collapseDir(const QModelIndex &index)
{
  QStandardItem *collapsedItem;
  QDir          dir;
  QString       path;
  int           k;


  collapsedItem = itemFromIndex(index);
  path = collapsedItem->data(Kbv::FilePathRole).toString();
  path += collapsedItem->data(Kbv::FileNameRole).toString();
  //qDebug() << "KbvDirModel::collapseDir" << path; //###########

  //remove all children in reverse order
  k = collapsedItem->rowCount();
  if(k>0)
    {
      collapsedItem->removeRows(0, k);
    }
  //add dummy item if there still are sub directories
  dir.setPath(path);
  dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
  if(!dir.entryList().isEmpty())
    {
      collapsedItem->appendRow(dummyItem->clone());
    }
}
/************************************************************************//*!
 * SLOT: Insert directory as subdirectory at the given index.
 * Collapsing and expanding the view refreshes the model.
 */
void    KbvDirModel::insertDir(const QModelIndex &parent, const QString &name)
{
  QStandardItem *item;
  QFileInfo     info;
  QDir          newDir;
  QString       path, str1, str2;
  bool          fault;

  if (!parent.isValid())
    {
      return;
    }

  fault = false;
  item = itemFromIndex(parent);
  path = this->data(parent, Kbv::FilePathRole).toString();
  path += this->data(parent, Kbv::FileNameRole).toString();
  //qDebug() << "KbvDirModel::insertDir at" << path; //###########

  info = QFileInfo(path);
  if(info.isDir() && info.isWritable())
    {
      //create directory.
      //on success we add a kbvDummy when the parent doesn't have children,
      //since the new dir is not set as child and the view is not able to collapse and expand.
      newDir.setPath(path);
      if (newDir.mkdir(name))
        {
          if(!item->hasChildren())
            {
              item->appendRow(dummyItem->clone());
            }
          //refresh the model by collapsing and expanding
          emit collapseView(parent);
          emit expandView(parent);
        }
      else
        {
          fault = true;
        }
    }
  else
    {
      fault = true;
    }
  
  if(fault)
    {
      str1 = QString(tr("Directory can not be created!"));
      str2 = QString(tr("No access rights or directory already exists!"));
      informationDialog->perform(str1,str2,1);
    }
}

/************************************************************************//*!
 * SLOT: Remove directory at given index.
 * The directory at given path and all content is removed permanently. If the
 * dir contains subdirs which are not empty it can not be removed.
 * The view is collapsed and expanded to refresh the model.
 */
void    KbvDirModel::removeDir(const QModelIndex &index)
{
  QStandardItem *item, *parent;
  QDir          dir;
  QStringList   entryList;
  QString       path, rmpath, str1, str2;

  if (!index.isValid())
    {
      return;
    }

  //user wants to remove
  item = itemFromIndex(index);
  path = item->data(Kbv::FilePathRole).toString();
  path += item->data(Kbv::FileNameRole).toString();
  //qDebug() << "KbvDirModel::removeDir" << path; //###########

  //1. step: collapse the view
  //this removes all child items from model when expanded
  emit collapseView(item->index());

  //2. step: try to remove all files and subdirs
  //only empty subdirs are removed
  dir = QDir(path);
  dir.setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
  entryList = dir.entryList();
  //remove all files and empty subdirs
  for (int i=0; i < entryList.length(); ++i)
    {
      rmpath = path + "/" + entryList.at(i);
      //try to remove dir
      if (!dir.rmdir(rmpath))
        {
          //try to remove file
          dir.remove(rmpath);
        }
    }
  //3. step:
  //try to remove dir: success if empty or display a warning
  if (dir.rmdir(path))
    {
      //collapse and expand parent item to refresh the model
      //the view expands only when the parent has still childs
      parent = item->parent();
      emit collapseView(parent->index());
      emit expandView(parent->index());
      this->activatedDir(parent->index());
    }
  else
    {
      str1 = QString(tr("Directory can not be removed!"));
      str2 = QString(tr("Directory not empty, hidden files or missing access rights!"));
      informationDialog->perform(str1,str2,1);
      return;
    }
}

/************************************************************************//*!
 * SLOT: Rename directory at given index.
 * The directory at given path and all content is renamed.
 */
void    KbvDirModel::renameDir(const QModelIndex &index, const QString &newname)
{
  QDir          dir;
  QString       path, name, str1;

  if (!index.isValid())
    {
      return;
    }
  path = this->data(index, Kbv::FilePathRole).toString();
  name = this->data(index, Kbv::FileNameRole).toString();

  dir.setPath(path+name);

  if(dir.rename(path+name, path+newname))
    {
      this->setData(index, newname, Kbv::FileNameRole);
      this->setData(index, newname, Qt::DisplayRole);
      //when renamed dir = expanded dir -> correct path and update file model
      if (displayedPath == path+name)
        {
          displayedPath = path+newname;
          emit  expandedDirRenamed(path+newname);
          //qDebug() << "KbvDirModel::renameDir displayedPath" << displayedPath; //###########
        }
    }
  else
    {
      str1 = QString(tr("Directory can not be renamed!"));
      informationDialog->perform(str1,QString(),1);
    }
}

/****************************************************************************/
