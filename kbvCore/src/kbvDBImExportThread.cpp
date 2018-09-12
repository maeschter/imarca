/*****************************************************************************
 * kvb database import export thread
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-03-08 19:47:29 +0100 (Do, 08. Mär 2018) $
 * $Rev: 1480 $
 * Created: 2017.01.30
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <sys/statfs.h>
#include <QtDebug>
#include <QFlags>
#include "kbvSetvalues.h"
#include "kbvConstants.h"
#include "kbvDBImExportThread.h"

extern  KbvSetvalues            *settings;

KbvDBImExportThread::KbvDBImExportThread(QObject *parent) : QThread(parent)
{
  //qDebug() << "KbvDBImExportThread::constructor" <<dbName <<targetDir; //###########
  //Do nothing when this thread already is running
  if(!this->isRunning())
    {
      abort = false;
    }
}
/*************************************************************************//*!
 * Destructor waits until the thread has finished or wakes up the sleeping
 * thread then finishes and destroys the database connection.
 */
KbvDBImExportThread::~KbvDBImExportThread()
{
  //qDebug() << "KbvDBImExportThread::~KbvDBImExportThread"; //###########
  abort = true;
  wait();
  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection
}
/*************************************************************************//*!
 * SLOT: Stop thread and wait until the thread has finished.
 */
bool  KbvDBImExportThread::stop(void)
{
  if(this->isRunning())
    {
      //qDebug() << "KbvDBImExportThread::stop"; //###########
      abort = true;
      wait();
    }
  abort = false;
  return true;
}
/************************************************************************//*!
 * SLOT: Starts the thread when it's not running. Otherwise do nothing.
 * When the database cannot be opened infotext(flag1) get displayed by signal
 * finished(flag1).
 */
void  KbvDBImExportThread::start(const QString &name, const QString &location,
                                const QString &rootDir, const QString &targetDir, int task)
{
  if(this->isRunning())
    {
      return;
    }
  this->task = task;
  this->dbName = name;
  this->dbLocation = location;
  this->collRoot = rootDir;
  this->targetDir = targetDir + "/";

  QThread::start(QThread::HighPriority);
}
/************************************************************************//*!
 */
void  KbvDBImExportThread::run()
{
  switch(task)
  {
    case Kbv::dbexport: {dbExport();}  //export database
      break;
    case Kbv::dbimport: {dbImport();}  //import database
      break;
    default:
      break;
  }
}
/************************************************************************//*!
 * Export a database and the related collection to an arbitrary drive.
 * The database get copied to the new location and the root dir in table
 * "description" is set to the new location also. All collection files
 * contained in table "album" get copied in to the new location.
 * All required directories have been created before copying the files.
 * A report file contains all files for who an error occured.
 */
void  KbvDBImExportThread::dbExport()
{
  QFile         file;
  QDir          dir;
  QSqlQuery     query;
  QString       stmt;
  QStringList   fileList, ncFiles, ncPaths;
  int           batch, num=0, i;
  quint64       fileSpace, diskSpace=0;
  float         stepsize, advance;
  bool          failed;

  struct        statfs diskData;

  failed = false;
  advance = 5.0;
  emit  progress(int (advance));   //step 1 may take some time

  //Step 1
  //open source database, get number of files in collection,
  //calculate reqired diskspace in target and size of source database (in dbLocation)
  dbconnection = "export"+dbName;
  db = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
  db.setHostName("host");
  db.setDatabaseName(dbLocation+dbName+QString(dbNameExt));
  if(!db.open())
    {
      emit  finished(Kbv::flag1);                 //source db not open
      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase(dbconnection); //close connection
      return;
    }

  //calculate size of database file
  file.setFileName(dbLocation+dbName+QString(dbNameExt));
  fileSpace = file.size();

  //get all file paths and names, calculate required disk space
  db.transaction();
  stmt = QString("SELECT ALL filePath, fileName, fileSize FROM album");
  query = db.exec(stmt);
  db.commit();
  if(query.isActive())
    {
      query.first();
      do
        {
          fileList.append(query.value(0).toString() + query.value(1).toString());  //file path+name
          fileSpace += query.value(2).toLongLong();
        }
      while(query.next());
    }
  num = fileList.length();
  advance = 10.0;
  emit  files(num);
  emit  size(fileSpace);
  emit  progress(int (advance));

  //calculate available disk space and abort operation when not sufficient
  if((statfs(targetDir.toLocal8Bit().constData(), &diskData)) < 0 )
    {
      failed = true;
      //qDebug() << "KbvDBImExportThread::dbExport statfs failed"<<diskData.f_bfree <<diskData.f_bsize; //###########
    }
  else
    {
      diskSpace = diskData.f_bfree * diskData.f_bsize;
    }
  if((diskSpace < 1.1*fileSpace) || failed)
    {
      emit  finished(Kbv::flag6);                 //disk space not sufficient
      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase(dbconnection); //close connection
      return;
    }
  //qDebug() << "KbvDBImExportThread::dbExport filespace diskSpace" <<num <<fileSpace <<diskSpace; //###########

  //Step 2
  //copy database to targetDir (= new rootDir) and close database connection
  file.setFileName(dbLocation+dbName+QString(dbNameExt));
  file.copy(targetDir+dbName+QString(dbNameExt));

  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection

  //Step 3
  //establish the target database connection
  dbconnection = "target"+dbName;
  db = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
  db.setHostName("host");
  db.setDatabaseName(targetDir+dbName+QString(dbNameExt));
  if(!db.open())
    {
      emit  finished(Kbv::flag2);                 //copied db not open
      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase(dbconnection); //close connection
      return;
    }
  //qDebug() << "KbvDBImExportThread::dbExport target db" <<targetDir <<db; //###########

  //set new collection root dir (=targetDir)
  query = QSqlQuery(db);
  db.transaction();
  query.prepare(QString("UPDATE description SET rootDir = :rootDir"));
  query.bindValue(":rootDir",  QVariant(targetDir));
  query.exec();
  db.commit();

  //Step 4
  //get all distinct filepaths of collection and create these in target
  //then copy all files
  db.transaction();
  stmt = QString("SELECT DISTINCT filePath FROM album");
  query = db.exec(stmt);
  db.commit();
  if(query.isActive())
    {
      query.first();
      do
        {
          //qDebug() << "KbvDBImExportThread::dbExport create path" <<argetDir+query.value(0).toString(); //###########
          if(!dir.mkpath(targetDir+query.value(0).toString()))
            {
              ncPaths.append(query.value(0).toString());
            }
        }
      while(query.next());
    }

  //copy files to target
  batch = 30;                 //every 30 files we send a progress
  stepsize = 90.0*batch/num;  //step size for remaining 90 percent
  i = 0;
  while(i < num)
    {
      if(abort)
        {
          break;
        }
      //copy files and collect faults
      if(!file.copy(collRoot+fileList.at(i), targetDir+fileList.at(i)))
        {
          ncFiles.append(fileList.at(i));
        }
      i++;                  //no progress for i=0
      if((i % batch) == 0)
        {
          advance += stepsize;
          emit  progress(int (advance));
          //qDebug() << "KbvDBImExportThread::dbExport progress" <<i <<advance; //###########
        }
    }
  emit  progress(100);

  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection

  //Step 5
  //process faults and finish
  if(abort)
    {
      emit  finished(Kbv::flag3); //thread aborted by user
    }
  else
    {
      ncPaths.append(ncFiles);
      if(!ncPaths.isEmpty())
        {
          this->createReport(ncPaths, task);
          emit  finished(Kbv::flag5); //finished with faults
        }
      else
        {
          emit  finished(Kbv::flag4); //finished successfully
        }
    }
}
/************************************************************************//*!
 * Import a database and the related collection from a arbitrary drive.
 * The database gets copied to the databaseDir (options) and the root dir
 * in table "description" will be set to the new location also. All collection
 * files contained in table "album" get copied in to the new location.
 * All required directories have been created before copying the files.
 * A report file contains all files for who an error occured.
 */
void  KbvDBImExportThread::dbImport()
{
  QFile         file;
  QDir          dir;
  QSqlQuery     query;
  QString       stmt;
  QStringList   fileList, ncFiles, ncPaths;
  int           batch, num, i;
  quint64       fileSpace, diskSpace=0;
  float         stepsize, advance;
  bool          failed;

  //qDebug() << "KbvDBImExportThread::dbImport" <<dbName <<dbLocation <<collRoot <<targetDir; //###########

  struct    statfs diskData;
  QFileDevice::Permissions  permissions = QFlag(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther);
  failed = false;
  advance = 5.0;
  emit  progress(int (advance));   //step 1 may take some time

  //Step 1
  //open source database, get number of files in collection
  //calculate reqired diskspace in target and size of source database
  dbconnection = "import"+dbName;
  db = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
  db.setHostName("host");
  db.setDatabaseName(dbLocation+dbName+QString(dbNameExt));
  if(!db.open())
    {
      emit  finished(Kbv::flag1);                 //source db not open
      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase(dbconnection); //close connection
      return;
    }

  //size of database file
  file.setFileName(dbLocation+dbName+QString(dbNameExt));
  fileSpace = file.size();

  //get all file paths and names, calculate required disk space
  db.transaction();
  stmt = QString("SELECT ALL filePath, fileName, fileSize FROM album");
  query = db.exec(stmt);
  db.commit();
  if(query.isActive())
    {
      query.first();
      do
        {
          fileList.append(query.value(0).toString() + query.value(1).toString());  //file path+name
          fileSpace += query.value(2).toLongLong();
        }
      while(query.next());
    }
  num = fileList.length();
  advance = 10.0;
  emit  files(num);
  emit  size(fileSpace);
  emit  progress(int (advance));

  //calculate available disk space and abort operation when not sufficient
  if((statfs(targetDir.toLocal8Bit().constData(), &diskData)) < 0 )
    {
      failed = true;
      //qDebug() << "KbvDBImExportThread::dbImport statfs failed"<<diskData.f_bfree <<diskData.f_bsize; //###########
    }
  else
    {
      diskSpace = diskData.f_bfree * diskData.f_bsize;
    }
  if((diskSpace < 1.1*fileSpace) || failed)
    {
      emit  finished(Kbv::flag6);                 //disk space not sufficient
      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase(dbconnection); //close connection
      return;
    }

  //Step 2
  //copy database to databaseDir (from Options) and close connection
  //set permissions owner r/w, user r/w, group r/w, other r/-
  file.setFileName(dbLocation+dbName+QString(dbNameExt));
  stmt = settings->dataBaseDir + "/";
  file.copy(stmt+dbName+QString(dbNameExt));
  file.setPermissions(stmt+dbName+QString(dbNameExt), permissions);

  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection

  //Step 3
  //establish the target database connection and set root
  dbconnection = "target"+dbName;
  db = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
  db.setHostName("host");
  db.setDatabaseName(stmt+dbName+QString(dbNameExt));
  if(!db.open())
    {
      emit  finished(Kbv::flag2);                 //copied db not open
      db = QSqlDatabase();                        //destroy db-object
      QSqlDatabase::removeDatabase(dbconnection); //close connection
      return;
    }
  //qDebug() << "KbvDBImExportThread::dbImport target db" <<targetDir <<db; //###########

  //set new collection root dir (=targetDir)
  query = QSqlQuery(db);
  db.transaction();
  query.prepare(QString("UPDATE description SET rootDir = :rootDir"));
  query.bindValue(":rootDir",  QVariant(targetDir));
  query.exec();
  db.commit();

  //Step 4
  //get all distinct filepaths of collection and create these in target
  //then copy all files
  db.transaction();
  stmt = QString("SELECT DISTINCT filePath FROM album");
  query = db.exec(stmt);
  db.commit();
  if(query.isActive())
    {
      query.first();
      do
        {
          stmt = query.value(0).toString();
          //qDebug() << "KbvDBImExportThread::dbExport create path" <<targetDir+stmt; //###########
          if(!stmt.isEmpty())
            {
              if(!dir.mkpath(targetDir+stmt))
                {
                  ncPaths.append(stmt);
                }
            }
        }
      while(query.next());
    }

  //copy files to target
  batch = 50;                 //every batch files we send a progress
  stepsize = 90.0*batch/num;  //step size for remaining 90 percent
  i = 0;
  while(i < num)
    {
      if(abort)
        {
          break;
        }
      //copy files and collect faults
      if(!file.copy(collRoot+fileList.at(i), targetDir+fileList.at(i)))
        {
          ncFiles.append(fileList.at(i));
        }
      file.setPermissions(targetDir+fileList.at(i), permissions);
      i++;                  //no progress for i=0
      if((i % batch) == 0)
        {
          advance += stepsize;
          emit  progress(int (advance));
          //qDebug() << "KbvDBImExportThread::dbExport progress" <<i <<advance; //###########
        }
    }
  //remove copied source database from collection
  QFile::remove(targetDir+dbName+QString(dbNameExt));
  emit  progress(100);

  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection); //close connection

  //Step 5
  //process faults and finish
  if(abort)
    {
      emit  finished(Kbv::flag3); //thread aborted by user
    }
  else
    {
      ncPaths.append(ncFiles);
      if(!ncPaths.isEmpty())
        {
          this->createReport(ncPaths, task);
          emit  finished(Kbv::flag5); //finished with faults
        }
      else
        {
          emit  finished(Kbv::flag4); //finished successfully
          emit  setDB(dbName);
        }
    }
}
/************************************************************************//*!
 * Collect issues into a report file.
 */
void  KbvDBImExportThread::createReport(const QStringList filelist, int task)
{
  QFile           reportFile;
  QTextStream     outReport;
  QString         reportPath, action, str, linend;

  switch(task)
  {
    case Kbv::dbexport: action = QString("export");  //export database
      break;
    case Kbv::dbimport: action = QString("import");  //import database
      break;
    default:
      break;
  }

  reportPath = targetDir + action + "_log.txt";
  reportFile.setFileName(reportPath);

  outReport.setCodec("UTF-8");
  linend = QString("\n");

  if(reportFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      reportFile.resize(0);
      outReport.setDevice(&reportFile);

      //Header
      QDateTime dt = QDateTime::currentDateTime();
      str = dt.toString("yyyy.MM.dd-hh:mm");
      outReport <<QString("Imarca collection %1").arg(action) <<linend;
      outReport <<QString("Collection: %1").arg(dbName) <<linend;
      outReport <<QString("Date: %1").arg(str) <<linend <<linend;

      outReport <<QString("Paths not created and files not copied") <<linend;
      outReport <<QString("--------------------------------------------------------------------------------") <<linend;
      for(int i=0; i<filelist.length(); i++)
        {
          outReport <<filelist.at(i) <<linend;
        }

      reportFile.flush();
      reportFile.close();
    }
}

/*****************************************************************************/
