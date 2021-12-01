/*****************************************************************************
 * kbvCollectionThread
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2011.11.15
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *
 * Structure of database pattern "imageDB.kbvdb".
 * The database contains two tables: album and description.
 * Table description only holds one record:
 * CREATE TABLE "description" ("id" INTEGER PRIMARY KEY NOT NULL ,
 *  "version" TEXT,"iconSize" INTEGER,"comment" TEXT,"colltype" INTEGER,
 *  "keywordtypes" INTEGER,"rootdir" TEXT)
 * - version = database version: major.minor
 * - comment = the user defined description of this database and its content
 * - iconSize = the user defined dimension for the icons
 * - collType = the user defined type of the database: album or collection
 * - keywordtypes: how to derive keywords from filname
 * CREATE TABLE "album" ("id" INTEGER PRIMARY KEY NOT NULL ,"filePath" TEXT,
 *  "fileName" TEXT,"fileURL" TEXT,"icon" BLOB,"imageW" INTEGER,"imageH" INTEGER,
 *  "dateChanged" INTEGER,"timeChanged" INTEGER,"crc32" INTEGER,"fileSize" INTEGER,
 *  "keyWords" TEXT,"userComment" TEXT,"userDate" INTEGER)
 * CREATE INDEX "main"."albumIdx" ON "album" ("filePath" ASC, "fileName" ASC,
 *  "fileURL" ASC, "imageW" ASC, "imageH" ASC, "dateChanged" ASC,
 *  "keyWords" ASC, "userComment" ASC, "userDate" ASC)
 *****************************************************************************/
#include <QtDebug>
#include "kbvCollectionThread.h"


KbvCollectionThread::KbvCollectionThread(QObject *parent, KbvDBInfo *databaseInfo)
                                        : QThread(parent)
{
  //default;
  yesall = false;
  yes = false;
  no = false;
  cancel = false;
  collVisible = false;
  task = 0;
  abortThread = false;

  //Read database settings from databaseInfo
  dbName = databaseInfo->getName();
  dbLocation = databaseInfo->getLocation();
  collRootDir = databaseInfo->getRootDir(); //empty for albums
  dbIconSize = databaseInfo->getIconSize();
  dbType = databaseInfo->getType();
  dbKeywordType = databaseInfo->getKeyWordType();
  //qDebug() << "KbvCollectionThread::constructor" <<dbIconSize <<dbType <<collRootDir <<dbName <<dbLocation; //###########

  dbconnection = "collthread"+dbName;
  db = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
  db.setHostName("host");
  db.setDatabaseName(dbLocation+dbName+QString(dbNameExt));
  db.open();
}
/*************************************************************************//*!
 * Destructor waits until the thread has finished or wakes up the sleeping
 * thread then finishes.
 */
KbvCollectionThread::~KbvCollectionThread()
{
  //qDebug() << "KbvCollectionThread::~KbvCollectionThread" <<dbconnection; //###########
  abortThread = true;
  wait();
  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection
}
/*************************************************************************//*!
 * The start() method accepts a list of files or paths and an integer
 * parameter "func" which dictates the processing of the list.
 */
void    KbvCollectionThread::start(QStringList &strlist, int func)
{
  if(this->isRunning())
    {
      abortThread = true;
      wait();
    }
  //qDebug() << "KbvCollectionThread::start" << dbName << func; //###########
  this->entryList = strlist;
  this->task = func;
  abortThread = false;
  QThread::start(QThread::HighPriority);
}
/*************************************************************************//*!
 * The stop() method waits until the thread finishes.
 * The method is ignored as long as import/update or renaming is running.
 */
bool    KbvCollectionThread::stop()
{
  if(this->isRunning() && ((task == Kbv::readin) || (task == Kbv::rename)))
    {
      emit warning(QString(tr("The database is currently working!")), true);
      return false;
    }
  if(this->isRunning())
    {
      //qDebug() << "KbvCollectionThread::stop"; //###########
      abortThread = true;
      wait();
    }
  abortThread = false;
  return true;
}
/*************************************************************************//*!
 * Receive visibility from model. When this variable is read the function
 * immediately clears the variable.
*/
void    KbvCollectionThread::visible()
{
  collVisible = true;
}
/*************************************************************************//*!
 * Slot: Receive user input and release wait condition in insert functions
 * addToAlbum() and addToCollection()
*/
void    KbvCollectionThread::getUserInput(int result)
{
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
  mutex.lock();
  waitForUserInput.wakeOne();
  mutex.unlock();
}
/*************************************************************************//*!
 * Manipulate model and database depending on parameter "task":
 * task = Kbv::readOut:
 *        collection: read branch from database into model,
 *        branch is the sole item in entryList
 *        album: read database into collection model,
 *        entryList is invalid
 * task = Kbv::remove:
 *        remove selected items from model and database,
 *        entryList contains paths, names and primary keys
 * task = Kbv::replace:
 *        find the data set, change the file name and update the data set,
 *        entryList contains new path, new name and primary key
 * task = Kbv::readIn:
 *        readin a collection entirely from collection root dir,
 *        update collection (update records, remove orphans and find duplicates),
 *        update branch (update records, remove orphans and find duplicates),
 *        entryList contains the path
 * task = Kbv::rename:
 *        find all data sets, where filePath contains oldPath and replace by newPath,
 *        entryList contains oldPath and newPath
 */
void    KbvCollectionThread::run()
{
  switch(task)
  {
    case Kbv::readout: //read records from database
      readOut();
      break;
    case Kbv::remove: //remove icons from model and database
      remove();
      break;
    case Kbv::replace: //replace data set with new/renamed file data
      replace();
      break;
    case Kbv::readin: //readin or update records
      readIn();
      break;
    case Kbv::rename: //rename a branch of the collection
      rename();
      break;
    default:
      break;
  }
}
/*************************************************************************//*!
 * Fill the model with content of database. The model must be empty when
 * starting this function.
 * Called when an album was opened or a branch of a collection has to be read.
 * Read branch:
 * The branch is the sole item in entryList and holds the directory path.
 * The branch may be terminated with "/".
 * Read album:
 * The entryList is empty since an album is the database itself without branches.
 */
void    KbvCollectionThread::readOut()
{
  Kbv::kbvItem  *item;
  QSqlQuery     query;
  QString       stmt;
  int           rows, batch;
  
  batch = 50;
  if(dbType & Kbv::TypeAlbum)
    {
      //qDebug() << "KbvCollectionThread::readOut album"; //###########
      stmt = QString("SELECT filePath,fileName,icon,imageW,imageH,fileSize,crc32,dateChanged,timeChanged,userDate,pk FROM album");
      
    }

  if(dbType & Kbv::TypeCollection)
    {
      //qDebug() << "KbvCollectionThread::readOut collection" <<entryList.at(0); //###########
      //Statement: select all records where the filePath equals the branch name.
      stmt = QString("SELECT filePath,fileName,icon,imageW,imageH,fileSize,crc32,dateChanged,timeChanged,userDate,pk FROM album "
                      "WHERE filePath LIKE '%1'").arg(entryList.at(0));
      
    }

  db.transaction();
  query.setForwardOnly(true);
  query = db.exec(stmt);
  db.commit();
  
  //the driver does not report the query size hence we have to count
  if(query.first())
    {
      //get database content for filePath
      rows = 1;
      while(query.next())
        {
          rows++;
        }
      emit statusText1(QString("%1").arg(rows, 6));

      newlist.clear();
      rows = 0;
      if(query.first())
        {
          //qDebug() << "KbvCollectionThread::read first" << query.value(0).toString(); //###########
          do
            {
              if (abortThread)
                {
                  //qDebug() << "KbvCollectionThread::read break"; //###########
                  break;
                }
              else
                {
                  item = globalFunc.itemFromRecord(query, collRootDir);
                  newlist.append(item);
                  rows++;
                  if((rows % batch) == 0)
                    {
                      emit newItems(newlist);
                      emit statusText2(QString("%1").arg(rows, 6));
                      newlist.clear();
                    }
                }
            }
          while(query.next());
          emit newItems(newlist);
        }
    }
  emit statusText2(QString(""));
  emit endOfReading();
  //qDebug() << "KbvCollectionThread::read finished"<<rows; //###########
}
/*************************************************************************//*!
 * Replace
 * data of a record given by primary key with data of file 'newpath, newname'
 * assuming that the file in file system is already renamed or replaced.
 * The record data get created newly from file except userDate and userComment.
 * The function can be used for single/multiple replacing of record data, e.g.
 * when images have been altered by the imageViewer.
 * Each item in entryList is described by three parameters:
 * entryList[i+0] = newpath,
 * entryList[i+1] = newname,
 * entryList[i+2] = primary key.
 * This function gets completed in each case even on closing the database.
 */
void    KbvCollectionThread::replace()
{
  QMap<QString, QVariant>   recordData;
  QSqlQuery     query;
  QString       stmt, primkey;
  QString       path, name;
  qlonglong     pkey;
  quint32       crc;
  int           i;

  //prepare query and statements
  query = QSqlQuery(db);
  stmt = QString("UPDATE OR REPLACE album SET filePath = :filePath, fileName = :fileName, "
                 "icon = :icon, imageW = :imageW, imageH = :imageH, dateChanged = :dateChanged, "
                 "timeChanged = :timeChanged, crc32 = :crc32, fileSize = :fileSize, keyWords = :keyWords "
                 "WHERE pk = %1");

  db.transaction();
  i = 0;
  while(i < entryList.size())
    {
      path = entryList.at(i);
      name = entryList.at(i+1);
      primkey = entryList.at(i+2);
      pkey = primkey.toLongLong();
      //qDebug() << "KbvCollectionThread::replace" << pkey << path << name; //###########

      //calculate new record data from path, name
      //we do not expect problems because files and database are readable and writable
      crc = globalFunc.crc32FromFile(path + name);
      if(crc == 0)
        {
          emit warning(QString(tr("Cannot open file for CRC check:") + " %1").arg(path + name), true);
        }
      else
        {
          globalFunc.createRecordData(path, name, recordData, dbIconSize, dbKeywordType);
          query.prepare(stmt.arg(pkey));
          query.bindValue(":filePath",    QVariant(path));
          query.bindValue(":fileName",    QVariant(name));
          query.bindValue(":icon",        recordData.value("icon"));
          query.bindValue(":imageW",      recordData.value("imageW"));
          query.bindValue(":imageH",      recordData.value("imageH"));
          query.bindValue(":fileSize",    recordData.value("fileSize"));
          query.bindValue(":dateChanged", recordData.value("dateChanged"));
          query.bindValue(":timeChanged", recordData.value("timeChanged"));
          query.bindValue(":crc32",       QVariant(crc));
          query.bindValue(":keyWords",    recordData.value("keywords"));
          query.exec();
        }
      i += 3;
      if((i % 30) == 0) //every 10 items
        {
          emit statusText2(QString("%1").arg(i/3, 6));
        }
    }
  db.commit();
  emit statusText2(QString(""));
  //qDebug() << "KbvCollectionThread::replace end"; //###########
}
/*************************************************************************//*!
 * Remove records from database.
 * The parameters path, name, primary key are contained in entryList in this
 * order. In case of album we only remove records from data base.
 * When this is a collection we remove records and files.
 * When path and name are empty, the file already has been removed.
 */
void    KbvCollectionThread::remove()
{
  QSqlQuery     query;
  QString       stmt, path, primkey;
  QFile         file;
  int           i, n;
  bool          error;

  //qDebug() << "KbvCollectionThread::remove" << entryList.size()/3 << dbType; //###########
  emit statusText3(QString(tr("Removing files. Please wait.")));

  query = QSqlQuery(db);
  query.setForwardOnly(true);
  error = false;
  i = 0;
  n = entryList.size();
  stmt = QString("DELETE FROM album WHERE pk = %1");

  if(dbType & Kbv::TypeAlbum)
    {
      errorList = QString(tr("File not be removed from database:"));
      errorList.append("\n\n");
      db.transaction();
      while(i < n)
        {
          if(abortThread)
            {
              break;
            }
          //entryList contains path, filename and primary key in 3 consecutive lines!
          //records are removed by primary key
          path = entryList.at(i) + entryList.at(i+1);
          primkey = entryList.at(i+2);
          query.exec(stmt.arg(primkey));
          //qDebug() << "KbvCollectionThread::remove stmt" <<stmt; //###########
          if(!query.isActive())
            {
              error = true;
              errorList.append(path);
              errorList.append("\n");
            }
          query.clear();
          if((i % 30) == 0) //every 10 items
            {
              emit statusText2(QString("%1").arg((n-i)/3, 6));
            }
          i += 3;
        }
      //qDebug() << "KbvCollectionThread::remove finished"; //###########
      db.commit();
      emit statusText2(QString(""));
      emit statusText3(QString(""));
    }

  if(dbType & Kbv::TypeCollection)
    {
      errorList = QString(tr("File not removed from database or not deleted:"));
      errorList.append("\n\n");
      db.transaction();
      while(i < n)
        {
          if(abortThread)
            {
              break;
            }
          //entryList contains path, filename and primary key in 3 consecutive lines!
          //records are removed by primary key
          path = entryList.at(i) + entryList.at(i+1);
          primkey = entryList.at(i+2);
          //qDebug() << "KbvCollectionThread::remove" <<path <<primkey; //###########
          //remove file and record from data base
          if(path.isEmpty())
            {
              //remove records only
              query.exec(stmt.arg(primkey));
              //qDebug() << "KbvCollectionThread::remove records" <<stmt <<primkey; //###########
              if(!query.isActive())
                {
                  error = true;
                  errorList.append(path);
                  errorList.append("\n");
                }
              query.clear();
            }
          else
            {
              //move file to dust bin and remove record
              if(generalFunc.moveToTrash(entryList.at(i), entryList.at(i+1)))
                {
                  query.exec(stmt.arg(primkey));
                  //qDebug() << "KbvCollectionThread::remove records&files" <<stmt.arg(primkey) <<path; //###########
                  if(!query.isActive())
                    {
                      error = true;
                      errorList.append(path);
                      errorList.append("\n");
                    }
                  query.clear();
                }
              else
                {
                  error = true;
                  errorList.append(path);
                  errorList.append("\n");
                }
              
            }
          if((i % 30) == 0) //every 30 items
            {
              emit statusText2(QString("%1").arg((n-i)/3, 6));
            }
          i += 3;
        }
      db.commit();
      emit statusText2(QString(""));
      emit statusText3(QString(""));
    }

  //Reindex database and finish
  query.exec("REINDEX albumIdx");
  if(error)
    {
      emit  warning(errorList, true);
    }
}
/**************************************************************************//*!
 * Read a collection entirely from collection root dir or update the
 * database. Read in is performed when the database is empty.
 * Called on menus "Read in" or "Update".
 * Reading in is only interrupted when the related tab gets closed.
 * Path (entryList(0)) is expected to be relative without terminating "/".
 */
void    KbvCollectionThread::readIn()
{
  QSqlQuery     query;
  QString       stmt, msgImport, msgDuration, resultList, path;
  QTime         duration;
  QFile         logFile;
  QTextStream   stream;
  int           r1=0, r2=0, r3=0;

  struct  Results res = {0, 0, 0, 0};

  duration.start();
  do
    {
      //wait for animation start
    }
  while(duration.elapsed()<100);
  //Message texts:
  msgDuration = QString(tr("Duration:") + "   %L1 s\n");

  //check if database contains records, the limit 3 only saves time
  stmt = QString("SELECT ALL (pk) FROM album LIMIT 3");
  query = db.exec(stmt);
  query.first();
  if(query.value(0).toInt() > 0)
    {
      //database not empty: update database
      //prepare animation info and resultlist for log file entry
      msgImport = QString(tr("Update finished.") + "\n");
      resultList.append(QString(tr("Update performed: ")));
      resultList.append(QString("%1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") ));

      path = entryList.at(0);
      //qDebug() << "KbvCollectionThread::readIn update" <<path; //###########

      //1. step: Remove orphaned records from database branch and refresh
      //remaining records when the related file was altered (when crc32 doesn't match).
      r1 = this->removeOrphaned(path, resultList);

      //Step 2: add new records for newly found files.
      //The first item of entryList describes the path (collection branch) to be parsed.
      r2 = this->addNew(path, resultList, res);
      
      // Step 3: search, identify and remove multiple records.
      // Multiple records are detected when filePath, fileName, fileSize and crc32
      // are identical in different records (different primary key).
      r3 = this->removeMultiple(path, resultList);

      //work done: show result in animation
      msgImport.append(msgDuration.arg(double(duration.elapsed()/1000.0), 0, 'f', 3));
      msgImport.append(QString(tr("Removed records:") + "  %1\n").arg(r1, 6));
      msgImport.append(QString(tr("Added records:") + "   %1\n").arg(r2, 6));
      msgImport.append(QString(tr("Multiple records removed:") + "   %1\n").arg(r3, 6));
    }
  else
    {
      //database is empty: read in files
      //qDebug() << "KbvCollectionThread::readIn read in" <<collRootDir; //###########
      msgImport = QString(tr("Read in finished.") + "\n");
      errorList.clear();
      
      resultList.append(QString(tr("Read in performed: ")));
      resultList.append(QString("%1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") ));
      
      this->readInCollection(collRootDir, res);  //recursive directory parser

      stmt = msgDuration.arg(double(duration.elapsed()/1000.0), 0, 'f', 3);
      msgImport.append(stmt);
      resultList.append(stmt);

      stmt = QString(tr("Sub-directories:") + "  %1\n").arg(res.r1, 6);
      msgImport.append(stmt);
      resultList.append(stmt);
      
      stmt = QString(tr("Read files:") + "   %1\n").arg(res.r2, 6);
      msgImport.append(stmt);
      resultList.append(stmt);
      if(res.r3 > 1048576)
        {
          stmt = QString(tr("Disk space:") + "  %L1 MiB").arg((double(res.r3)/1048576.0), 0, 'f', 1);
        }
      else
        {
          stmt = QString(tr("Disk space:") + "  %L1 KiB").arg((double(res.r3)/1024.0), 0, 'f', 1);
        }
      msgImport.append(stmt);
      resultList.append(stmt + "\n");
      if(errorList.size() > 0)
        {
          resultList.append(QString("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
          resultList.append(QString(tr("These files couldn'd be added:") + "\n"));
          resultList.append(errorList);
        }
    }
  //write result to log file
  logFile.setFileName(dbLocation + dbName + ".log");
  logFile.open(QIODevice::ReadWrite | QIODevice::Text);
  logFile.resize(0);
  stream.setDevice(&logFile);
  stream.seek(0);
  stream << resultList;
  stream.flush();
  logFile.close();
  
  //show statistics in animation
  emit endOfUpdate(msgImport);
  emit statusText2("");
  emit statusText3("");
}
/*************************************************************************//*!
 * Read all image files by parsing the absolute path in a recursive way to
 * fetch all files in all sub directories.
 * On abort the recursion must return properly step by step. Hence do not
 * alter variable "abort" (only can happen when the tab gets closed).
 */
void    KbvCollectionThread::readInCollection(QString path, struct Results &res)
{
  QList<QPair<QString, QString> >   filePathNames;
  QMap<QString, QVariant>   recordData;
  QString       stmt1, str;
  QSqlQuery     query;
  int           i, infostep, lengthRootDir;
  quint32       crc;

  query = QSqlQuery(db);
  lengthRootDir = path.length();

  //qDebug() << "KbvCollectionThread::readInCollection path" <<path; //###########

  //read all files in collection
  this->parseDirectories(path, filePathNames, res);  //recursive directory parser

  infostep = filePathNames.length()/1000;      //number of files per infostep
  infostep = infostep < 20 ? 20 : infostep;    //refresh every n records
  emit statusText1(QString("%1").arg(filePathNames.length(), 6));

  //insert files of this branch
  db.transaction();
  stmt1 = QString("INSERT INTO album (filePath,fileName,icon,imageW,imageH,dateChanged,timeChanged,crc32,fileSize,keyWords) "
                  "VALUES (:filePath,:fileName,:icon,:imageW,:imageH,:dateChanged,:timeChanged,:crc32,:fileSize,:keyWords)");

  //add image files only (checked by crc calculation)
  for(i=0; i<filePathNames.length(); i++)
    {
      //on abort the recursion must return properly step by step
      if (abortThread)
        {
          return;
        }
      //make relative path for record once per directory
      str = filePathNames[i].first;
      str.remove(0, lengthRootDir);
      //qDebug() << "KbvCollectionThread::readInCollection dir" <<str; //###########
      crc = globalFunc.crc32FromFile(filePathNames[i].first + filePathNames[i].second);
      if(crc > 0)
        {
          //record data from image files only
          if(globalFunc.createRecordData(filePathNames[i].first, filePathNames[i].second, recordData, dbIconSize, dbKeywordType))
            {
              res.r2++;
              res.r3 += recordData.value("fileSize").toLongLong();
              query.prepare(stmt1);
              query.bindValue(":filePath",    QVariant(str));
              query.bindValue(":fileName",    QVariant(filePathNames[i].second));
              query.bindValue(":icon",        recordData.value("icon"));
              query.bindValue(":imageW",      recordData.value("imageW"));
              query.bindValue(":imageH",      recordData.value("imageH"));
              query.bindValue(":fileSize",    recordData.value("fileSize"));
              query.bindValue(":dateChanged", recordData.value("dateChanged"));
              query.bindValue(":timeChanged", recordData.value("timeChanged"));
              query.bindValue(":crc32",       QVariant(crc));
              query.bindValue(":keyWords",    recordData.value("keywords"));
              query.exec();
              //qDebug() << "KbvCollectionThread::readInCollection record" << recordData.value("keywords") << recordData.value("fileSize"); //###########
            }
          else
            {
              errorList.append(str + filePathNames[i].second + "\n");
            }
        }
      else
        {
          errorList.append(str + filePathNames[i].second + "\n");
        }
      if((i % infostep) == 0)
        {
          emit statusText2(QString("%1").arg(i, 6));
          emit statusText3(str);
        }
      //update static info fields when the tab becomes visible
      if(collVisible)
        {
          collVisible = false;
          emit statusText1(QString("%1").arg(filePathNames.length(), 6));
          emit statusText3(str);
        }
    }
  db.commit();
}
/*************************************************************************//*!
 * Parse the absolute path in recursive way through all sub directories.
 * All found files are collected in the list 'files' (path & name).
 * On abort the recursion must return properly step by step. Hence do not
 * alter variable "abort" (this only can happen when the tab gets closed).
 */
void    KbvCollectionThread::parseDirectories(QString path, QList<QPair<QString, QString> > &files, struct Results &res)
{
  QDir          actualDir;
  QStringList   dirlist, filelist;
  QString       filepath;
  QPair<QString, QString> pathname;
  int           i, k;


  actualDir.setPath(path);
  actualDir.setSorting(QDir::Name | QDir::LocaleAware);
  QDir::Filters filter = QDir::AllDirs | QDir::NoDotAndDotDot;
  actualDir.setFilter(filter);

  dirlist = actualDir.entryList();
  res.r1 += dirlist.length();

  actualDir.setFilter(QDir::Files | QDir::NoSymLinks);
  filelist = actualDir.entryList();
  filepath = path;
  if(!path.endsWith("/"))
    {
      filepath.append("/");
    }
  //qDebug() << "KbvCollectionThread::parseDirectories" << filepath; //###########
  for(i=0; i<filelist.size(); i++)
    {
      //qDebug() << "KbvCollectionThread::parseDirectories" <<  filelist.at(i); //###########
      pathname.first = filepath;
      pathname.second = filelist.at(i);
      files.append(pathname);
    }

  //recursion: parse sub dirs of this branch
  for(k=0; k<dirlist.length(); k++)
    {
      //on abort the recursion must return properly step by step
      if (abortThread)
        {
          return;
        }
      //qDebug() << "KbvCollectionThread::parseDirectories" << filepath << dirlist[k]; //###########
      parseDirectories(filepath + dirlist[k], files, res);
    }
}
/*************************************************************************//*!
 * Add new records when files are found in the selected branch which aren't
 * already contained in the database.
 * New files are found by collecting the files of all sub directories of the
 * branch in a list and then removing all known files, taken from records.
 * The remaining entries in the list have to be added.
 * dir is expected to be relative without terminating "/".
 * dir will be empty when the collection root is selected. This query shall
 * return all records.
 */
int    KbvCollectionThread::addNew(QString dir, QString &report, Results &res)
{
  QList<QPair<QString, QString> >   filePathNames;
  QPair<QString, QString>           pathname;
  QList<QVariant>                   queryData;
  QMap<QString, QVariant>           recordData;
  QSqlQuery                         query;
  QString       stmt1, stmt2, faultList, msgTask, str;
  int           i, infostep, idx, added, n;
  quint32       crc;


  msgTask = QString(tr("Search and insert new collection files"));
  stmt1 = QString("SELECT filePath, fileName, crc32, pk FROM album WHERE filePath LIKE '%1%'");
  stmt2 = QString("INSERT INTO album (filePath,fileName,icon,imageW,imageH,dateChanged,timeChanged,crc32,"
                  "fileSize, keyWords) VALUES "
                  "(:filePath,:fileName,:icon,:imageW,:imageH,:dateChanged,:timeChanged,:crc32,:fileSize,:keyWords)");

  query = QSqlQuery(db);
  added = 0;

  emit statusText3(msgTask);
  emit statusText2("");
  //qDebug() << "KbvCollectionThread::addNew dir" <<dir; //###########

  db.transaction();
  //query.exec(stmt1);
  query.exec(stmt1.arg(dir));
  if(query.first())
    {
      do
        {
          queryData.append(query.value(0));  //path
          queryData.append(query.value(1));  //name
        }
      while(query.next());
      //qDebug() << "KbvCollectionThread::addNew records" << queryData.size()/2; //###########

      //read all files of selected branch, make branch absolute
      this->parseDirectories(collRootDir+dir, filePathNames, res);  //recursive directory parser
      //qDebug() << "KbvCollectionThread::addNew found" <<collRootDir+dir <<filePathNames.size(); //###########

      emit statusText1(QString("%1").arg(filePathNames.size(), 6));
      //remove known files: query has relative paths, filepaths are absolute
      i=0;
      while(i<queryData.size())
        {
          pathname.first = collRootDir + queryData.at(i).toString();  //absolute path
          pathname.second = queryData.at(i+1).toString();
          idx = filePathNames.indexOf(pathname, 0);
          if(idx >= 0)
            {
              filePathNames.removeAt(idx);
            }
          i += 2;
        }
      //qDebug() << "KbvCollectionThread::addNew new files" <<filePathNames; //###########
    }
  emit statusText2(QString("%1").arg(filePathNames.size(), 6));
  infostep = 20;
  //add new records
  if(filePathNames.size() > 0)
    {
      report.append(QString("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
      report.append(QString(tr("New records added:") + "\n"));
    }
  n = collRootDir.length();
  for(i=0; i<filePathNames.size(); i++)
    {
      //create relative path
      str = filePathNames[i].first;
      str.remove(0, n);
      crc = globalFunc.crc32FromFile(filePathNames[i].first + filePathNames[i].second);
      if(crc > 0)
        {
          //record data from image files only
          if(globalFunc.createRecordData(filePathNames[i].first, filePathNames[i].second, recordData, dbIconSize, dbKeywordType))
            {
              query.prepare(stmt2);
              query.bindValue(":filePath",    QVariant(str));
              query.bindValue(":fileName",    QVariant(filePathNames[i].second));
              query.bindValue(":icon",        recordData.value("icon"));
              query.bindValue(":imageW",      recordData.value("imageW"));
              query.bindValue(":imageH",      recordData.value("imageH"));
              query.bindValue(":fileSize",    recordData.value("fileSize"));
              query.bindValue(":dateChanged", recordData.value("dateChanged"));
              query.bindValue(":timeChanged", recordData.value("timeChanged"));
              query.bindValue(":crc32",       QVariant(crc));
              query.bindValue(":keyWords",    recordData.value("keywords"));
              query.exec();
              added++;;
              report.append(str + filePathNames[i].second + "\n");
              //qDebug() << "KbvCollectionThread::addNew" <<filePathNames[i].second; //###########
            }
          else
            {
              faultList.append(str + filePathNames[i].second + "\n");
            }
        }
      else
        {
          faultList.append(str + filePathNames[i].second + "\n");
        }
      //show processed file amount every 'infostep' checked records
      if((i % infostep) == 0)
        {
          emit statusText2(QString("%1").arg(i, 6));
        }
      //update static info fields when the tab becomes visible
      if(collVisible)
        {
          collVisible = false;
          emit statusText1(QString("%1").arg(filePathNames.size(), 6));
          emit statusText3(msgTask);
        }
    }
  db.commit();
  //add faults to report
  if(faultList.size() > 0)
    {
      report.append(QString("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
      report.append(QString(tr("These files couldn'd be added:") + "\n"));
      report.append(faultList);
    }
  return added;
}
/*************************************************************************//*!
 * Remove orphaned records of selected branch 'dir' from DB. Records are
 * orphaned when no related file can be found. The remaining records get
 * updated when CRC32 of the record and the file are different.
 * Note: paths taken from records are relative to collection root dir. For
 * file operations the collection root dir must be prepended to these paths.
 * dir is expected to be relative without terminating "/".
 * dir will be empty when the collection root is selected. This query shall
 * return all records.
 */
int    KbvCollectionThread::removeOrphaned(QString dir, QString &report)
{
  QMap<QString, QVariant>   recordData;
  QList<QVariant>           queryData;
  QSqlQuery     query;
  QString       stmt1, stmt2, stmt4, path, name, msgTask;
  QString       removedList;
  int           i, infostep, removed;
  qint64        pk;
  quint32       crc;

  msgTask = QString(tr("Remove orphaned records and update existing records"));
  stmt1 = QString("SELECT filePath, fileName, crc32, pk FROM album WHERE filePath LIKE '%1%'");
  stmt2 = QString("DELETE FROM album WHERE pk = %1");
  stmt4 = QString("UPDATE album SET icon = :icon, imageW = :imageW, imageH = :imageH, "
                  "dateChanged = :dateChanged, timeChanged = :timeChanged, crc32 = :crc32, "
                  "fileSize = :fileSize, keyWords = :keyWords WHERE pk = %1");

  query = QSqlQuery(db);
  removed = 0;

  //qDebug() << "KbvCollectionThread::removeOrphaned path" <<dir; //###########

  //Get a list of all records containing path, name, crc32 and primary key.
  //Show the number of records in the info bar of the related tab. 
  //stmt1 also works when dir is empty
  query.exec(stmt1.arg(dir));
  if(query.first())
    {
      do
        {
          queryData.append(query.value(0));  //path
          queryData.append(query.value(1));  //name
          queryData.append(query.value(2));  //crc32
          queryData.append(query.value(3));  //pk
        }
      while(query.next());
      emit statusText1(QString("%1").arg(queryData.size()/4, 6));
      emit statusText3(msgTask);

      //qDebug() << "KbvCollectionThread::removeOrphaned queryData.size" << queryData.size()/4; //###########
      //qDebug() << "KbvCollectionThread::removeOrphaned queryData 1.name" << queryData.at(1).toString(); //###########
      infostep = queryData.length()/4000;      //number of files per infostep
      infostep = infostep < 40 ? 40 : infostep; //refresh every 10th record
      i=0;
      while(i<queryData.size())
        {
          db.transaction();
          do
            {
              path = queryData.at(i).toString();
              name = queryData.at(i+1).toString();
              pk = queryData.at(i+3).toLongLong();
              //check if files are existing and crc32 is valid
              crc = globalFunc.crc32FromFile(collRootDir+path+name);    //crc=0: not able to open file
              //qDebug() << "KbvCollectionThread::removeOrphaned crc" <<crc <<collRootDir+path+name; //###########
              if(crc == 0)
                {
                  //remove orphaned record from data base
                  removed++;
                  removedList.append(path+name+"\n");
                  query.exec(stmt2.arg(QString("%1").arg(pk, 0, 10)));
                  //qDebug() << "KbvCollectionThread::removeOrphaned stmt2:" << query.lastError(); //###########
                  //qDebug() << "KbvCollectionThread::removeOrphaned stmt2:" << stmt2.arg(prime); //###########
                }
              else
                {
                  //update record with newly generated values
                  globalFunc.createRecordData(collRootDir+path, name, recordData, dbIconSize, dbKeywordType);

                  query.prepare(stmt4.arg(QString("%1").arg(pk, 0, 10)));
                  //qDebug() << "KbvCollectionThread::removeOrphaned stmt4:" << query.lastError(); //###########
                  query.bindValue(":icon",        recordData.value("icon"));
                  query.bindValue(":imageW",      recordData.value("imageW"));
                  query.bindValue(":imageH",      recordData.value("imageH"));
                  query.bindValue(":fileSize",    recordData.value("fileSize"));
                  query.bindValue(":dateChanged", recordData.value("dateChanged"));
                  query.bindValue(":timeChanged", recordData.value("timeChanged"));
                  query.bindValue(":crc32",       QVariant(crc));
                  query.bindValue(":keyWords",    recordData.value("keywords"));
                  query.exec();
                }
              i += 4;
              //inner loop reached end of list?
              if(i >= queryData.size())
                {
                  break;
                }
            }
          while((i % infostep) != 0);
          //end of inner loop - commit in db
          emit statusText2(QString("%1").arg(i/4, 6));
          db.commit();

          if (abortThread)
            {
              break;
            }
          //update static info fields when the tab becomes visible
          if(collVisible)
            {
              collVisible = false;
              emit statusText1(QString("%1").arg(queryData.size()/4, 6));
              emit statusText3(msgTask);
            }
        }
      //end of outer loop - list completely processed
    }
  if(removed > 0)
    {
      report.append(QString("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
      report.append(QString(tr("Orphaned records removed:") + "\n"));
      report.append(removedList);
    }
  return removed;
}
/*************************************************************************//*!
 * Search, identify and remove multiple records within an arbitrary
 * collection branch 'dir'. Multiple records are identified by identical
 * filePath and fileName.
 * Statement
 * ("SELECT filePath, fileName, fileSize, crc32, pk FROM album "
 *  "WHERE filePath LIKE '%1%' AND pk NOT IN "
 *  "(SELECT X.pk FROM album AS X "
 *  "GROUP BY X.filePath, X.fileName, X.fileSize, X.crc32 HAVING COUNT(*)=1)");
 * This selects all records and removes these which are unique from the selection.
 * The pk (primary key) is used to identify the duplicates.
 * The algorithm keeps the first one and removes all multiple records of the
 * same file.
 * Note: paths taken from records are relative to collection root dir. For
 * file operations the collection root dir must be prepended to these paths.
 * dir is expected to be relative without terminating "/".
 * dir will be empty when the collection root is selected. This query shall
 * return all records.
 */
int    KbvCollectionThread::removeMultiple(QString dir, QString &report)
{
  QSqlQuery       query;
  QList<QString>  nameList;
  QList<qint64>   pkList;
  QString         stmt1, stmt2, path, name, msgTask;
  qint64          pk;

  query = QSqlQuery(db);
  query.setForwardOnly(true);

  //find identical records checking fileName,fileSize and crc32
  msgTask = QString(tr("Search and remove multiple records"));
  stmt1 = QString("SELECT filePath, fileName, fileSize, crc32, pk FROM album "
                 "WHERE filePath LIKE '%1%' AND pk NOT IN "
                 "(SELECT X.pk FROM album AS X "
                 "GROUP BY X.filePath, X.fileName, X.fileSize, X.crc32 HAVING COUNT(*)=1)");
  stmt2 = QString("DELETE FROM album WHERE pk = %1");
  
  emit statusText1("");
  emit statusText3(msgTask);
  //qDebug() << "KbvCollectionThread::removeMultiple path" <<dir; //###########

  db.transaction();
  query.exec(stmt1.arg(dir));
  //qDebug() << "KbvCollectionThread::removeMultiple stmt" << stmt1; //###########
  //qDebug() << "KbvCollectionThread::removeMultiple query" << query.lastError(); //###########
  if(query.first())
    {
      report.append(QString("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
      report.append(QString(tr("Multiple records removed:") + "\n"));
      do
        {
          path = query.value(0).toString();
          name = query.value(1).toString();
          pk = query.value(4).toLongLong();
          //collect all pk of multiple records
          if(nameList.contains(path+name))
            {
              pkList.append(pk);
              report.append(path + ", " + name + "\n");
              //qDebug() << "KbvCollectionThread::removeMultiple"<<pk <<path <<name; //###########
            }
          else
            {
              nameList.append(path+name);
            }
          if(collVisible)
            {
              collVisible = false;
              emit statusText3(msgTask);
            }
        }
      while(query.next());
      for(int i=0; i<pkList.size(); i++)
        {
          pk = pkList.at(i);
          query.exec(stmt2.arg(QString("%1").arg(pk, 0, 10)));
        }
    }
  db.commit();
  return pkList.size();
}
/*************************************************************************//*!
 * Rename a collection branch.
 * EntryList contains the two strings "oldPath" and "newPath".
 * Find in table "album" all records where "filePath" contains oldPath and
 * collect them in a list.
 * Replace oldPath against newPath in this list then korrect all records.
 * Statements:
 * Find all filePaths starting with given "oldPath"
 * "SELECT pk, filePath FROM album WHERE filePath LIKE '%1%'";
 * Replace all filePath with given primary key pk
 * "UPDATE album SET filePath = :filePath WHERE pk = %1";
 * Note: paths in records are relative to collection root dir.
 * file operations the collection root dir must be prepended to these paths.
 * paths in entryList are expected to be relative without terminating "/".
 */
void    KbvCollectionThread::rename()
{
  QList<QString>    keys;
  QList<QString>    paths;
  QSqlQuery         query;
  QString           stmt1, stmt2, msgTask, str;
  int               infostep;

  //qDebug() << "KbvCollectionThread::renameBranch" <<entryList[0] <<entryList[1]; //###########

  msgTask = QString(tr("Renaming collection branch"));
  emit statusText3(msgTask);

  infostep = 20;
  stmt1 = QString("SELECT pk, filePath FROM album WHERE filePath LIKE '%1%'");
  stmt2 = QString("UPDATE album SET filePath = :filePath WHERE pk = %1");
  query = QSqlQuery(db);
  query.setForwardOnly(true);

  //get all items of old pathname, quotes in pathname must be doubled for search!
  str = entryList[0];
  str.replace("'", "''");

  db.transaction();
  query.exec(stmt1.arg(str));
  //qDebug() << "KbvCollectionThread::renameBranch query" << stmt1.arg(str); //###########
  //qDebug() << "KbvCollectionThread::renameBranch query" << query.lastError(); //###########
  if(query.first())
    {
      do
        {
          keys.append(query.value(0).toString());
          paths.append(query.value(1).toString());
        }
      while(query.next());
    }
  db.commit();
  emit statusText1(QString("%1").arg(keys.size(), 6));

  //replace file path in database
  db.transaction();
  for(int i=0; i < keys.size();i++)
    {
      paths[i].replace(entryList[0], entryList[1]);
      //qDebug() << "KbvCollectionThread::renameBranch replaced" << keys.at(i) << paths.at(i); //###########
      query.prepare(stmt2.arg(keys.at(i)));
      query.bindValue(":filePath",  QVariant(paths[i]));
      query.exec();
      //qDebug() << "KbvCollectionThread::renameBranch stmt2" << query.lastError(); //###########
      if((i % infostep) == 0)
        {
          emit statusText2(QString("%1").arg(i, 6));
        }
    }
  db.commit();
  //qDebug() << "KbvCollectionThread::renameBranch" << keys.size() << QString("time: %1s").arg(double(duration.elapsed()/1000.0), 0, 'f', 3); //###########
  emit statusText3("");
  emit statusText2("");
  emit statusText1("");
}
/*****************************************************************************/
