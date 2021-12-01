/*****************************************************************************
 * kbvCollectionDragDrop
 * This class performs drop on collections and albums due to activity diagram
 * "CollectionDropMimeData" and sub activities "CollectionDropOnAlbum" and
 * "CollectionDropOnCollection".
 * This includes copying/moving files and adding to or updating the database.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2012.03.12
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvSetvalues.h"
#include "kbvReplaceDialog.h"
#include "kbvInformationDialog.h"
#include "kbvCollectionDragDrop.h"

extern  KbvSetvalues            *settings;
extern  KbvReplaceDialog        *replaceDialog;
extern  KbvInformationDialog    *informationDialog;

KbvCollectionDragDrop::KbvCollectionDragDrop(QObject *parent) : QObject(parent=nullptr)
{
  messageNoConnect = QString(tr("Cannot connect to database"));
  messageSameAlbum = QString(tr("No drag'n'drop within an album"));
  messageFaultAlbum = QString(tr("Could not add to album"));
  messageFaultCollection = QString(tr("Could not add to collection or could not move file"));
  messageNoTargetDir = QString(tr("Target directory not found in database!"));
  messageFaultReason = QString(tr("Database not up-to-date or corrupt!\nNo files copied or moved."));
  messageNoFileCRC = QString(tr("Cannot open file for CRC check"));

  //default
  targetDBIconSize = 0;
  targetDBKeywordType = 0;
  targetType = 0;
  sourceType = 0;
  move = false;
  error = false;
  yesall = false;
  yes = false;
  no = false;
  cancel = false;
}

KbvCollectionDragDrop::~KbvCollectionDragDrop()
{
  //qDebug() << "KbvCollectionDragDrop::~KbvCollectionDragDrop";//###########
  this->disconnectDB();
}
/************************************************************************//*!
 * Prepare and perform drop/paste due to activity diagram
 * CollectionDragDrop_DropMimeData:
 * - extract urls or file paths as text from mime data
 * - create and open a connection to the target database
 * - create and open a connection to the source database
 * drop/paste between Imarca databases, flag=2
 * drop/paste within same Imarca database, flag=1
 * drop/paste from an external application, flag=0
 * drop/paste from file view, flag=3
 * The model pointer only is valid when the drop target is being displayed.
 * Parameters branch and collection are the targets. The branch is displayed
 * when this function is called by a collectionView. When called by
 * collectionTreeView we must check if the target is the displayed branch.
 * The dropped/pasted files must be added to the model when the branch is
 * displayed or on drop/paste on an album.
 * Mime data "x-special/imarca-copied-files" contains a byte array including:
 * paste action(copy/paste), sourceDBType, sourceDBName and primary keys
 * as characters UTF8. On drop the paste action is empty, on drop/paste from
 * fileview/extern the primary keys are empty.
 */
void    KbvCollectionDragDrop::dropMimeData(const QMimeData *mimeData, Qt::DropAction action,
                QString branch, KbvDBInfo *info, KbvCollectionModel *cm)
{
  QByteArray          ba;
  QString             str;
  bool                ok=false, internal=false;

  this->targetType = info->getType();
  this->targetDBName = info->getName();
  this->targetDBLocation = info->getLocation();
  this->targetCollRoot = info->getRootDir();
  this->targetDBIconSize = info->getIconSize();
  this->targetDBKeywordType = info->getKeyWordType();

  sourceType = Kbv::TypeNone;
  entryList.clear();

  //qDebug() << "KbvCollectionDragDrop::dropMimeData target" <<targetDBName <<branch; //###########

  this->move = false;
  if(action == Qt::MoveAction)
    {
      this->move = true;
    }  
  //Create a list of file paths from mime data (urls or text)
  //first entry is the actual branch (the drop target = relative path below root),
  //the following are the file paths of files to drop
  entryList.append(globalFunc.dropExtractMimeData(mimeData));
  if((targetType & Kbv::TypeCollection) || (targetType & Kbv::TypeCollectionRoot))
    {
      entryList.prepend(branch);
    }
  //qDebug() << "KbvCollectionDragDrop::dropMimeData source" <<entryList; //###########

  //Drop or paste within Imarca, format "x-special/imarca-copied-files", mime data as strings
  //Drop from atabase: empty string, dbType, dbName, dbLocation and primary keys
  //Paste from database: action(copy/cut), dbType, dbName, dbLocation and primary keys
  //Drop from fileView: empty string, TypeFile, source path
  //Paste from fileView: action(copy/cut), TypeFile, source path
  if(mimeData->hasFormat("x-special/imarca-copied-files"))
    {
      internal = true;
      ba = mimeData->data("x-special/imarca-copied-files");
      str = QString::fromUtf8(ba.data());
      mimeStrings = str.split("\n", QString::KeepEmptyParts); //on drop first item is empty
      sourceType = mimeStrings.at(1).toInt(&ok);
      if((sourceType & Kbv::TypeAlbumRoot) || (sourceType & Kbv::TypeCollection) || (sourceType & Kbv::TypeCollectionRoot))
        {
          sourceDBName = mimeStrings.at(2);
          sourceDBLocation = mimeStrings.at(3);
          //reduce list of mime strings to primary keys only
          mimeStrings.removeFirst();
          mimeStrings.removeFirst();
          mimeStrings.removeFirst();
          mimeStrings.removeFirst();
        }
    }
  //qDebug() << "KbvCollectionDragDrop::dropMimeData targetDB sourceDB" <<targetDBName <<sourceDBName; //###########

  //Open connection to target database
  if(!connectDB(targetDBName, false))
    {
      informationDialog->perform(messageNoConnect,targetDBName,1);
    }
  else
    {
      //qDebug() << "KbvCollectionDragDrop::dropMimeData target" << targetDBName << branch; //###########
      //check for drag within Imarca
      if((sourceType & Kbv::TypeAlbumRoot) || (sourceType & Kbv::TypeCollection) || (sourceType & Kbv::TypeCollectionRoot))
        {
          //qDebug() << "KbvCollectionDragDrop::dropMimeData internal source" <<sourceDBName << sourceType; //###########
          //source DB is Imarca
          if(sourceDBName == targetDBName)
            {
              //within same Imarca DB, flag=1
              //update the model since on copy/cut/paste it displays the target, while on drag/drop
              //it displays the source!
              if((targetType & Kbv::TypeCollection) || (targetType & Kbv::TypeCollectionRoot))
                {
                  this->addToCollection(Kbv::flag1, cm);
                }
              else
                {
                  informationDialog->perform(messageSameAlbum.arg(sourceDBName),QString(),1);
                }
            }
          else
            {
              //between different Imarca DBs, flag=2
              if(!connectDB(sourceDBName, true))
                {
                  informationDialog->perform(messageNoConnect,sourceDBName,1);
                }
              else
                {
                  //perform drag'n'drop between Imarca databases, flag=2
                  if((targetType & Kbv::TypeCollection) || (targetType & Kbv::TypeCollectionRoot))
                    {
                     this->addToCollection(Kbv::flag2, cm);
                    }
                  if(targetType & Kbv::TypeAlbumRoot)
                    {
                      this->addToAlbum(Kbv::flag2, cm);
                    }
                  this->disconnectDB();
                }
            }
        }
      else
        {
          //drag from external application (flag=0) or from file view (flag=3)
          if((targetType & Kbv::TypeCollection) || (targetType & Kbv::TypeCollectionRoot))
            {
              if(internal)
                {
                  //qDebug() << "KbvCollectionDragDrop::dropMimeData from file view" << cm; //###########
                  this->addToCollection(Kbv::flag3, cm);
                }
              else
                {
                  //qDebug() << "KbvCollectionDragDrop::dropMimeData from external app" << cm; //###########
                  this->addToCollection(Kbv::noflag, cm);
                }
            }
          if(targetType & Kbv::TypeAlbumRoot)
            {
              //qDebug() << "KbvCollectionDragDrop::dropMimeData on album ext/file view" << cm; //###########
              this->addToAlbum(Kbv::noflag, cm);
            }
        }
    }
  //all finished, close connection to target database
  this->disconnectDB();
}
/*************************************************************************//*!
 * Open connection to database, either target or source.
 */
bool    KbvCollectionDragDrop::connectDB(QString dbname, bool source)
{
  bool  open;

  if(source)
    {
      //connection to source database
      sourceDB = QSqlDatabase::addDatabase("QSQLITE", "dnd"+dbname);
      sourceDB.setHostName("source");
      sourceDB.setDatabaseName(sourceDBLocation+dbname+QString(dbNameExt));
      open = sourceDB.open();
      //qDebug() << "KbvCollectionDragDrop::connectDB source" << sourceDB; //###########
    }
  else
    {
      //connection to target database
      targetDB = QSqlDatabase::addDatabase("QSQLITE", "dnd"+dbname);
      targetDB.setHostName("target");
      targetDB.setDatabaseName(targetDBLocation+dbname+QString(dbNameExt));
      open = targetDB.open();
      //qDebug() << "KbvCollectionDragDrop::connectDB target" << targetDB; //###########
    }
  if(!open)
    {
      return false;
    }
  return true;
}
/*************************************************************************//*!
 * Close connection to target and/or source database.
 */
void    KbvCollectionDragDrop::disconnectDB()
{
  QString connection;

  if(sourceDB.isOpen())
    {
      connection = sourceDB.connectionName();
      //qDebug() << "KbvCollectionDragDrop::disconnectDB source" <<connection; //###########
      sourceDB = QSqlDatabase();                //destroy db-object
      QSqlDatabase::removeDatabase(connection); //close connection
    }
  if(targetDB.isOpen())
    {
      connection = targetDB.connectionName();
      //qDebug() << "KbvCollectionDragDrop::disconnectDB target" <<connection; //###########
      targetDB = QSqlDatabase();                //destroy db-object
      QSqlDatabase::removeDatabase(connection); //close connection
    }
}
/*************************************************************************//*!
 * Receive user input for replacing files.
 */
void    KbvCollectionDragDrop::getUserInput(QString text1, QString text2)
{
  int   result;

  result = replaceDialog->perform(text1, text2);
  switch (result)
  {
    case QDialogButtonBox::YesToAll: { yesall = true; }
    break;
    case QDialogButtonBox::Yes: { yes = true; }
    break;
    case QDialogButtonBox::No: { no = true; }
    break;
    default: { cancel = true; }
    break;
  }
}
/*************************************************************************//*!
 * Add new records to or update existing records in the album database due to
 * activity diagram "CollectionDropOnAlbum".
 * The album database doesn't contain a root dir but each record contains the
 * absolute file path as well as each model.
 * The entryList contains the paths of the files to drop. Since this is an
 * album we only copy the new members to the database and do nothing with the
 * sources. Consider: the model pointer only is valid when the album is being
 * displayed and model update is required.
 */
void    KbvCollectionDragDrop::addToAlbum(int flag, KbvCollectionModel *model)
{
  QMap<QString, QVariant>   recordData;
  QSqlQuery     query, sourceQuery;
  QString       name, path, stmt1, stmt2, stmt3, stmt4;
  int           i, n, last;
  quint32       crc;

  yesall = false;
  yes = false;
  no = false;
  cancel = false;
  error = false;
  query = QSqlQuery(targetDB);
  query.setForwardOnly(true);

  emit infoText2(QString(""));

  //prepare query strings and error list
  stmt1 = QString("INSERT INTO album (filePath,fileName,icon,imageW,imageH,dateChanged,timeChanged,crc32,"
                  "fileSize,keyWords,userDate,userComment) VALUES (:filePath,:fileName,:icon,:imageW,"
                  ":imageH,:dateChanged,:timeChanged,:crc32,:fileSize,:keyWords,:userDate,:userComment)");
  stmt2 = QString("SELECT filePath,fileName FROM album WHERE fileName LIKE '%1' AND filePath LIKE '%2'");
  stmt3 = QString("DELETE FROM album WHERE fileName LIKE '%1' AND filePath LIKE '%2'");
  stmt4 = QString("SELECT filePath,fileName,icon,imageW,imageH,dateChanged,timeChanged,crc32,fileSize,keyWords,"
                  "userDate,userComment FROM album WHERE fileName LIKE '%1' AND filePath LIKE '%2'");

  errorList.clear();
  //qDebug() << "KbvCollectionDragDrop::addToAlbum from ext app" <<entryList.size(); //###########

  targetDB.transaction();
  for(i=0; i<entryList.size(); i++)
    {
      n = entryList.at(i).lastIndexOf("/", -1, Qt::CaseInsensitive);
      name = entryList.at(i).right(entryList.at(i).size()-n-1);
      path = entryList.at(i).left(n+1);     //absolute path, includes trailing"/"

      //Check if this file already is in database and call replace dialog
      //stop thread and wait for user input (result: yesall, yes, no, cancel)
      query.exec(stmt2.arg(name).arg(path));
      if(query.next())
        {
          //record already exists in db
          if(!yesall)
            {
              getUserInput(path+name, path+name);
            }
          //finish inserting data on cancel
          if(cancel)
            {
              break;
            }
          //remove existing file on YES or YESALL from DB and model
          if(yesall || yes)
            {
              query.exec(stmt3.arg(name).arg(path));
              if(model != nullptr)
                {
                  last = model->findItem(Kbv::FilePathRole, QVariant(path), Kbv::FileNameRole, QVariant(name));
                  //qDebug() << "KbvCollectionDragDrop::addToAlbum remove" <<name<<last; //###########
                  if(last >= 0)
                    {
                      model->removeItem(last);
                    }
                }
            }
        }
      if(!no)
        {
          //insert the record from Imarca database or file system
          if(flag == Kbv::flag2)
            {
              //source is an Imarca database -> read record
              //qDebug() << "KbvCollectionDragDrop::addToAlbum from Imarca" << sourceDBName; //###########
              sourceQuery = sourceDB.exec(stmt4.arg(name).arg(path));
              if(sourceQuery.first())
                {
                  recordData.insert("filePath",     sourceQuery.value(0));
                  recordData.insert("fileName",     sourceQuery.value(1));
                  recordData.insert("icon",         sourceQuery.value(2));
                  recordData.insert("imageW",       sourceQuery.value(3));
                  recordData.insert("imageH",       sourceQuery.value(4));
                  recordData.insert("dateChanged",  sourceQuery.value(5));
                  recordData.insert("timeChanged",  sourceQuery.value(6));
                  recordData.insert("crc32",        sourceQuery.value(7));
                  recordData.insert("fileSize",     sourceQuery.value(8));
                  recordData.insert("keywords",     sourceQuery.value(9));
                  recordData.insert("userDate",     sourceQuery.value(10));
                  recordData.insert("userComment",  sourceQuery.value(11));
                }
            }
          else
            {
              //source is file system -> create new record data
              //qDebug() << "KbvCollectionDragDrop::addToAlbum from ext app" << path; //###########
              recordData.insert("filePath", QVariant(path));
              recordData.insert("fileName", QVariant(name));
              crc = globalFunc.crc32FromFile(entryList.at(i));
              recordData.insert("crc32",  QVariant(crc));
              if(crc == 0)
                {
                  informationDialog->perform(messageNoFileCRC,entryList.at(i),1);
                }
              if(!globalFunc.createRecordData(path, name, recordData, targetDBIconSize, targetDBKeywordType))
                {
                  error = true;
                  errorList.append(entryList.at(i) + "\n");
                }
            }
          //insert new record data into database
          query.prepare(stmt1);
          query.bindValue(":filePath",    recordData.value("filePath"));
          query.bindValue(":fileName",    recordData.value("fileName"));
          query.bindValue(":icon",        recordData.value("icon"));
          query.bindValue(":imageW",      recordData.value("imageW"));
          query.bindValue(":imageH",      recordData.value("imageH"));
          query.bindValue(":dateChanged", recordData.value("dateChanged"));
          query.bindValue(":timeChanged", recordData.value("timeChanged"));
          query.bindValue(":crc32",       recordData.value("crc32"));
          query.bindValue(":fileSize",    recordData.value("fileSize"));
          query.bindValue(":keyWords",    recordData.value("keywords"));
          query.bindValue(":userDate",    recordData.value("userDate"));
          query.bindValue(":userComment", recordData.value("userComment"));
          query.exec();
          //qDebug() << "KbvCollectionDragDrop::addToAlbum" << query.lastError(); //###########
        }
      yes = false;
      no = false;
      if((i % 5) == 0)
        {
          emit infoText2(QString("%1").arg(i, 6));
          QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }   //end loop
  targetDB.commit();

  if(model != 0)
    {
      last = model->rowCount(QModelIndex());
      emit infoText1(QString(QString("%1").arg(last, 6)));
    }
  emit infoText2(QString(""));
  if(error)
    {
      informationDialog->perform(messageFaultAlbum,errorList,1);
    }
}
/*************************************************************************//*!
 * Add records to the collection database or update existing records due to
 * activity diagram "CollectionDragDrop_addToCollection".
 * The collection database contains the absolute collection root dir and each
 * record contains the relative file path below this root. Each model item
 * contains the absolute file path.
 * Cases: external source dir to the target collection in collection tree\n
 *        external source dir to the collection view\n
 *        internal file or album/collection view to the target collection\n
 * Copy/move items and add them to target database. On internal action the
 * user date and user comment are copied from source to target.\n
 * Flags: drag between Imarca databases, flag=2
 *        drag within same Imarca database, flag=1
 *        drag from an external application, flag = 0
 *        drag from file view, flag=3
 * The target branch is the first item in entryList as relative path (below root dir).
 * The following entries are the absolute paths of the files to be dropped.
 * Consider: the model pointer only is valid when the album is being displayed
 * and model update is required.
*/
void    KbvCollectionDragDrop::addToCollection(int flag, KbvCollectionModel *model)
{
  QMap<QString, QVariant>   recordData;
  QFile         file;
  QSqlQuery     query, sourceQuery;
  Kbv::kbvItem  *item;
  QImage        img;
  QSize         imgDim;
  QVariant      userDate, userComment;
  QString       name, path, targetpath, targetbranch;
  QString       stmt0, stmt1, stmt2, stmt3, stmt4, str;
  int           i, n, last;
  qlonglong     pkey;
  quint64       imgSize;
  quint32       crc;
  bool          exists;


  yesall = false;
  yes = false;
  no = false;
  cancel = false;
  exists = false;
  error = false;
  query = QSqlQuery(targetDB);
  query.setForwardOnly(true);
  query.finish();

  errorList.clear();
  emit infoText2(QString(""));
  //prepare query strings and error list
  stmt0 = QString("SELECT pk FROM album WHERE fileName LIKE '%1' AND filePath LIKE '%2'");
  stmt1 = QString("SELECT userDate, userComment FROM album WHERE pk='%1'");
  stmt4 = QString("SELECT filePath, fileName, icon, imageW, imageH, dateChanged, timeChanged, crc32,"
                  "fileSize, keyWords, userDate, userComment FROM album WHERE  pk='%1'");
  stmt2 = QString("INSERT INTO album (filePath,fileName,icon,imageW,imageH,dateChanged,timeChanged,crc32,"
                  "fileSize,keyWords,userDate,userComment) VALUES (:filePath,:fileName,:icon,:imageW,:imageH,"
                  ":dateChanged,:timeChanged,:crc32,:fileSize,:keyWords,:userDate,:userComment)");
  stmt3 = QString("UPDATE album SET icon=:icon,imageW=:imageW,imageH=:imageH,dateChanged=:dateChanged,"
                  "timeChanged=:timeChanged,crc32=:crc32,fileSize=:fileSize,keyWords=:keyWords "
                  "WHERE pk=:pk");

  //The branch is the first entry in entryList. This always is a relative
  //directory path below root without trailing '/' and can be empty.
  //targetbranch must be initialised otherwise this produces a NULL filepath in
  //the database when the branch (entryList[0]) is empty
  targetbranch = QString("");
  targetbranch.append(entryList.at(0));
  if(!targetbranch.isEmpty())
    {
      targetbranch.append("/");
    }
  targetpath = targetCollRoot + targetbranch;
  //qDebug() << "KbvCollectionDragDrop::addToCollection targetpath targetbranch flag" <<targetpath <<targetbranch <<flag; //###########

  targetDB.transaction();
  for(i=1; i<entryList.size(); i++)
    {
      //separate path and name of source file
      n = entryList.at(i).lastIndexOf("/", -1, Qt::CaseInsensitive);
      name = entryList.at(i).right(entryList.at(i).size()-n-1);
      path = entryList.at(i).left(n+1);     //absolute path, includes trailing"/"

      //qDebug() << "KbvCollectionDragDrop::addToCollection source" <<path <<name <<i; //###########
      //Check if this file exists in target dir and call replace dialog
      //stop thread and wait for user input (result: yesall, yes, no, cancel)
      file.setFileName(targetpath+name);
      if(file.exists())
        {
          //record already exists in db
          exists = true;
          if(!yesall)
            {
              //stop thread and wait for input of replaceDialog
              this->getUserInput(targetpath+name, path+name);
            }
        }
      else
        {
          //new record
          exists = false;
        }
      //finish on cancel
      if(cancel)
        {
          break;
        }
      //update existing record on YES or YESALL or insert new record
      if((exists && (yesall || yes)) || !exists)
        {
          //flag2: within different databases, target and source are open
          //read userDate and userComment for target data set
          if(flag == Kbv::flag2)
            {
              sourceQuery = sourceDB.exec(stmt1.arg(mimeStrings.at(i-1)));
              if(sourceQuery.first())
                {
                  userDate = sourceQuery.value(0);
                  userComment = sourceQuery.value(1);
                }
              //qDebug() << "KbvCollectionDragDrop::addToCollection flag2" <<userDate <<userComment; //###########
              sourceQuery.finish();
            }
          else
            {
              userDate = QVariant();
              userComment = QVariant();
            }

          //remove existing file and copy entrylist[i] (path name) to target (targetpath name)
          //copy fails when existing file wasn't removed or on other reasons
          if(exists)
            {
              file.remove(targetpath+name);
            }
          //qDebug() << "KbvCollectionDragDrop::addToCollection target file" << targetpath+name << exists; //###########
          //qDebug() << "KbvCollectionDragDrop::addToCollection source file" << path+name; //###########
          file.setFileName(path+name);
          if(file.copy(targetpath+name))
            {
              //flag1: within same database, only target is open
              //read existing record
              if(flag == Kbv::flag1)
                {
                  recordData.clear();
                  sourceQuery = targetDB.exec(stmt4.arg(mimeStrings.at(i-1)));
                  if(sourceQuery.first())
                    {
                      recordData.insert("filePath",    QVariant(targetbranch));  //relative path
                      recordData.insert("fileName",    sourceQuery.value(1));
                      recordData.insert("icon",        sourceQuery.value(2));
                      recordData.insert("imageW",      sourceQuery.value(3));
                      recordData.insert("imageH",      sourceQuery.value(4));
                      recordData.insert("dateChanged", sourceQuery.value(5));
                      recordData.insert("timeChanged", sourceQuery.value(6));
                      recordData.insert("crc32",       sourceQuery.value(7));
                      recordData.insert("fileSize",    sourceQuery.value(8));
                      recordData.insert("keywords",    sourceQuery.value(9));
                      recordData.insert("userDate",    sourceQuery.value(10));
                      recordData.insert("userComment", sourceQuery.value(11));
                    }
                  sourceQuery.finish();
                  //qDebug() << "KbvCollectionDragDrop::addToCollection stmt4 pk" <<mimeStrings.at(i-1); //###########
                }
              else
                {
                  //flag0: external source, flag2: different databases, flag3: fileView
                  //create new record data: icon, imageW, imageH, fileSize, dateChanged, timeChanged, keywords
                  recordData.clear();
                  crc = globalFunc.crc32FromFile(entryList.at(i));   //CRC of source file
                  globalFunc.createRecordData(targetpath, name, recordData, targetDBIconSize, targetDBKeywordType);
                  recordData.insert("filePath",    QVariant(targetbranch));  //relative path
                  recordData.insert("fileName",    QVariant(name));
                  recordData.insert("crc32",       QVariant(crc));
                  recordData.insert("userDate",    userDate);
                  recordData.insert("userComment", userComment);
                  //qDebug() << "KbvCollectionDragDrop::addToCollection flag0, flag2, flag3"; //###########
                }
              //get primary key if there is a record for existing file
              str = stmt0.arg(name).arg(targetbranch);
              query.exec(str);
              pkey = 0;
              if(query.first())
                {
                  pkey = query.value(0).toLongLong();
                }
              //qDebug() << "KbvCollectionDragDrop::addToCollection exist pkey" <<pkey; //###########
              if(pkey > 0)
                {
                  //update existing record with collected record data
                  query.prepare(stmt3);
                  query.bindValue(":filePath",    recordData.value("filePath"));
                  query.bindValue(":fileName",    recordData.value("fileName"));
                  query.bindValue(":icon",        recordData.value("icon"));
                  query.bindValue(":imageW",      recordData.value("imageW"));
                  query.bindValue(":imageH",      recordData.value("imageH"));
                  query.bindValue(":fileSize",    recordData.value("fileSize"));
                  query.bindValue(":dateChanged", recordData.value("dateChanged"));
                  query.bindValue(":timeChanged", recordData.value("timeChanged"));
                  query.bindValue(":crc32",       recordData.value("crc32"));
                  query.bindValue(":keyWords",    recordData.value("keywords"));
                  query.bindValue(":userDate",    recordData.value("userDate"));
                  query.bindValue(":userComment", recordData.value("userComment"));
                  query.bindValue(":pk",          QVariant(pkey));
                  query.exec();
                  //qDebug() << "KbvCollectionDragDrop::addToCollection stmt3" << name; //###########
                  //qDebug() << "KbvCollectionDragDrop::addToCollection update" << query.lastError(); //###########
                  query.finish();

                  //qDebug() << "KbvCollectionDragDrop::addToCollection new item in model" << model; //###########
                  if(model != 0)
                    {
                      //qDebug() << "KbvCollectionDragDrop::addToCollection replace item"; //###########
                      last = model->findItem(Kbv::FilePathRole, QVariant(targetpath), Kbv::FileNameRole, QVariant(name));
                      img = QImage::fromData(recordData.value("icon").toByteArray());
                      imgDim = QSize(recordData.value("imageW").toInt(), recordData.value("imageH").toInt());
                      imgSize = recordData.value("imageW").toInt() * recordData.value("imageH").toInt();

                      item = new  Kbv::kbvItem;
                      item->insert(Kbv::FilePathRole,   QVariant(targetpath));  //absolute path
                      item->insert(Kbv::FileNameRole,   QVariant(name));
                      item->insert(Kbv::FileSizeRole,   recordData.value("fileSize"));
                      item->insert(Kbv::FileDateRole,   QVariant(QFileInfo(targetpath+name).lastModified()));
                      item->insert(Kbv::ImageSizeRole,  QVariant(imgSize));
                      item->insert(Kbv::ImageDimRole,   QVariant(imgDim));
                      item->insert(Kbv::UserDateRole,   userDate);
                      item->insert(Qt::DecorationRole,  QVariant(img));
                      item->insert(Kbv::PrimaryKeyRole, QVariant(pkey));
                      model->replaceItem(last, item);
                    }
                }
              else
                {
                  //insert new record with collected record data
                  query.prepare(stmt2);
                  query.bindValue(":filePath",    recordData.value("filePath"));
                  query.bindValue(":fileName",    recordData.value("fileName"));
                  query.bindValue(":icon",        recordData.value("icon"));
                  query.bindValue(":imageW",      recordData.value("imageW"));
                  query.bindValue(":imageH",      recordData.value("imageH"));
                  query.bindValue(":fileSize",    recordData.value("fileSize"));
                  query.bindValue(":dateChanged", recordData.value("dateChanged"));
                  query.bindValue(":timeChanged", recordData.value("timeChanged"));
                  query.bindValue(":crc32",       recordData.value("crc32"));
                  query.bindValue(":keyWords",    recordData.value("keywords"));
                  query.bindValue(":userDate",    recordData.value("userDate"));
                  query.bindValue(":userComment", recordData.value("userComment"));
                  query.exec();
                  //qDebug() << "KbvCollectionDragDrop::addToCollection stmt2" <<name <<targetbranch <<query.lastError(); //###########
                  //query.finish();

                  if(model != 0)
                    {
                      //get primary key of new record
                      str = stmt0.arg(name).arg(targetbranch);
                      query.exec(str);
                      query.first();
                      pkey = query.value(0).toLongLong();
                      //qDebug() << "KbvCollectionDragDrop::addToCollection new pk" <<pkey; //###########
                      last = model->rowCount(QModelIndex());
                      model->insertItems(last, 1);

                      img = QImage::fromData(recordData.value("icon").toByteArray());
                      imgDim = QSize(recordData.value("imageW").toInt(), recordData.value("imageH").toInt());
                      imgSize = recordData.value("imageW").toInt() * recordData.value("imageH").toInt();
                      //qDebug() << "KbvCollectionDragDrop::addToCollection update model" << name; //###########

                      item = new  Kbv::kbvItem;
                      item->insert(Kbv::FilePathRole,   QVariant(targetpath)); //absolute path
                      item->insert(Kbv::FileNameRole,   QVariant(name));
                      item->insert(Kbv::FileSizeRole,   recordData.value("fileSize"));
                      item->insert(Kbv::FileDateRole,   QVariant(QFileInfo(targetpath+name).lastModified()));
                      item->insert(Kbv::ImageSizeRole,  QVariant(imgSize));
                      item->insert(Kbv::ImageDimRole,   QVariant(imgDim));
                      item->insert(Kbv::UserDateRole,   userDate);
                      item->insert(Qt::DecorationRole,  QVariant(img));
                      item->insert(Kbv::PrimaryKeyRole, QVariant(pkey));
                      model->replaceItem(last, item);
                    }
                }

              //qDebug() << "KbvCollectionDragDrop::addToCollection remove"<<path+name <<flag; //###########
              //Successful drop move: remove source file
              if(move)
                {
                  if(!file.remove(path + name))
                    {
                      error = true;
                      errorList.append(QString(tr("File copied not moved: %1\n").arg(name)));
                    }
                } //end move
            }
          else
            {
              //not copied to target dir
              error = true;
              errorList.append(QString(tr("File neither copied nor moved: %1\n").arg(name)));
            }
        } //end yes/yesall
      emit infoText2(QString("%1").arg(i, 6));
      QCoreApplication::processEvents(QEventLoop::AllEvents);
      yes = false;
      no = false;
    }   //end loop
  targetDB.commit();
  if(model != 0)
    {
      last = model->rowCount(QModelIndex());
      emit infoText1(QString(QString("%1").arg(last, 6)));
    }  
  emit infoText2(QString(""));
  //qDebug() << "KbvCollectionDragDrop::addToCollection finished"; //###########
  
  //all files copied or moved, now display what was wrong
  if(error)
    {
      informationDialog->perform(messageFaultCollection,errorList, 1);
    }
}
/*************************************************************************//*!
 * Remove the record data from database and when successful remove the file.
 * To keep database and file system consistent we may not remove the file when
 * it's still in database.
*/
bool    KbvCollectionDragDrop::removeFromDB(QString path, QString name, bool source)
{
  QFile     file;
  QSqlQuery query;
  QString   stmt;
  bool      removed;

  //qDebug() << "KbvCollectionDragDrop::removeFromDB" <<source; //###########

  stmt = QString("DELETE FROM album WHERE fileName LIKE '%1' AND filePath LIKE '%2'");
  if(source)
    {
      sourceDB.transaction();
      query = QSqlQuery(sourceDB);
      query.setForwardOnly(true);
      removed = query.exec(stmt.arg(name).arg(path));
      query.clear();
      sourceDB.commit();
    }
  else
    {
      targetDB.transaction();
      query = QSqlQuery(targetDB);
      query.setForwardOnly(true);
      removed = query.exec(stmt.arg(name).arg(path));
      query.clear();
      targetDB.commit();
    }
  if(removed)
    {
      removed = file.remove(path + name);
    }
  return removed;
}
/*****************************************************************************/
