/*****************************************************************************
 * kvb collection watcher thread
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-02-22 18:20:30 +0100 (Mi, 22. Feb 2017) $
 * $Rev: 1156 $
 * Created: 2016.05.27
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvCollectionWatchThread.h"


KbvCollectionWatchThread::KbvCollectionWatchThread(QObject *parent, KbvDBInfo *databaseInfo, int interval)
                                                    : QThread(parent)
{
  //Read database settings from databaseInfo
  dbName = databaseInfo->getName();
  dbLocation = databaseInfo->getLocation();
  collRootDir = databaseInfo->getRootDir();
  dbIconSize = databaseInfo->getIconSize();
  dbType = databaseInfo->getType();
  dbKeywordType = databaseInfo->getKeyWordType();

  dbconnection = "watch"+dbName;
  db = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
  db.setHostName("host");
  db.setDatabaseName(dbLocation+dbName+QString(dbNameExt));
  db.open();

  abort = false;
  isVisible = false;
  this->interval = interval;

  stmt1 = QString("DELETE FROM album WHERE pk = %1");
  stmt2 = QString("UPDATE OR REPLACE album SET filePath = :filePath, fileName = :fileName, "
                  "icon = :icon, imageW = :imageW, imageH = :imageH, dateChanged = :dateChanged, "
                  "timeChanged = :timeChanged, crc32 = :crc32, fileSize = :fileSize, keyWords = :keyWords "
                  "WHERE pk = %1");
  stmt3 = QString("INSERT INTO album (filePath,fileName,icon,imageW,imageH,dateChanged,timeChanged,crc32, fileSize, keyWords) "
                  "VALUES (:filePath,:fileName,:icon,:imageW,:imageH,:dateChanged,:timeChanged,:crc32,:fileSize,:keyWords)");
  stmt4 = QString("SELECT filePath,fileName,icon,imageW,imageH,fileSize,crc32,dateChanged,timeChanged,userDate,pk FROM album "
                  "WHERE filePath LIKE '%1' AND fileName like '%2'");
  stmt5 = QString("SELECT filePath,fileName,icon,imageW,imageH,fileSize,crc32,dateChanged,timeChanged,userDate,pk FROM album "
                  "WHERE pk = %1");


  //Note: the timer lives inside main thread, not in this one. So we must
  //retrigger the timer in main thread when the watchTread emits the
  //signal finished().
  watchTimer = new QTimer(this);
  watchTimer->setSingleShot(true);
  watchTimer->setInterval(interval);

  connect(watchTimer, SIGNAL(timeout()),  this, SLOT(startByTimer()));
  connect(this,       SIGNAL(finished()), this, SLOT(startTimer()));
}
/*************************************************************************//*!
 * Destructor waits until the thread has finished or wakes up the sleeping
 * thread then finishes.
 */
KbvCollectionWatchThread::~KbvCollectionWatchThread()
{
  //qDebug() << "KbvCollectionWatchThread::~KbvCollectionWatchThread"; //###########
  abort = true;
  wait();
  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection
}
/*************************************************************************//*!
 * Stop thread and wait until the thread has finished.
 */
bool  KbvCollectionWatchThread::stop()
{
  //qDebug() << "KbvCollectionWatchThread::stop" <<path; //###########
  this->watchTimer->stop();
  if(isRunning())
    {
      abort = true;
      wait();
    }
  //qDebug() << "KbvCollectionWatchThread::stop ok"; //###########
  abort = false;
  return true;
}
/*************************************************************************//*!
 * Stores the parameter and triggers the timer to a fast start of the thread.
 * Subsequent starts are performed by the watchTimer with time 'interval'.
 * branch is relative to collection root with terminating "/".
 */
void  KbvCollectionWatchThread::start(const QString &branch)
{
  this->stop();
  //qDebug() << "KbvCollectionWatchThread::startThread name path" <<dbName <<branch; //###########
  path = branch;
  this->watchTimer->start(100);
}
/*************************************************************************//*!
 * SLOT: Retrigger the watchTimer when the run method finished and this tab
 * is visible.
 */
void  KbvCollectionWatchThread::startTimer()
{
  //qDebug() <<"KbvCollectionWatchThread::startTimer" <<dbName <<path; //###########
  if(isVisible)
    {
      this->watchTimer->start(interval);
    }
}
/************************************************************************//*!
 * Slot: Starts the thread when the timer expires. As the timer is set at
 * the end of run() this function never meets a running thread.
 */
void  KbvCollectionWatchThread::startByTimer()
{
  QThread::start(QThread::HighPriority);
}
/************************************************************************//*!
 * Enable retrigger of timer when tab is visible or trigger timer when tab
 * becomes visible or stop timer when tab becomes invisible.
 */
void  KbvCollectionWatchThread::visible(bool visible)
{
  if((visible && !isVisible) && visible)
    {
      this->watchTimer->start(interval);
    }
  if((!visible && isVisible) && !visible)
    {
      this->watchTimer->stop();
    }
  isVisible = visible;
  //qDebug() <<"KbvCollectionWatchThread::visible" <<isVisible <<dbName; //###########
}

/*************************************************************************//*!
 * Watch collection branch for changes due to activity diagram
 * KbvCollectionWatcherThread_run.uxf\n
 * The run method processes all files in the directory "branch" (excluding
 * sub directories) and all datasets in the database containing "branch" in
 * the file path, then finishes the thread (returns from run).\n
 * List L1 contains all files of the recent branch. Map L2 comprises the
 * primary keys and file names and map L3 covers the file names and crc32
 * values for this branch in the database.
 * The method compares L2 against L1. The database gets adjusted by removing
 * orphanded and adding new or updating existing datasets.
 * On adding a new dataset a new item will be created from these new dataset
 * and be appended to the model to keep view and database synchronised.
 */
void  KbvCollectionWatchThread::run()
{
  QSqlQuery         query;
  QFileInfo         fileInfo;
  int               filecount;
  quint32           crc;
  qlonglong         pkey;
  QString           absPath, filename, stmt;

  this->readFileNamesForBranch();
  this->readDatasetsForBranch();

  //prepare query and statements
  query = QSqlQuery(db);

  filecount = L1.size();
  absPath = collRootDir+path;   //absolute path
  //qDebug() <<"KbvCollectionWatchThread::run" <<absPath; //###########

  db.transaction();
  for(int i=0; i<filecount; i++)
    {
      filename = L1.at(i);
      //qDebug() <<"KbvCollectionWatchThread::run" <<filename; //###########
      //directory removed while thread is running -> stop immediately
      fileInfo = QFileInfo(absPath);
      if(!fileInfo.exists() || abort)
        {
          //qDebug() <<"KbvCollectionWatchThread::run abort"; //###########
          watchTimer->stop();
          abort = false;
          db.commit();
          return;
        }

      //L2 contains the filename of L1[i] ?
      if (L2.contains(filename))
        {
          //calculate crc32 for filename and check in dir map L1 and db map L2
          crc = globalFunc.crc32FromFile(absPath+filename);
          //qDebug() <<"KbvCollectionWatchThread::run L1 crc  L2 crc" <<crc <<L2.value(filename) <<filename; //###########
          if(crc != L2.value(filename))
            {
              //different crc32: update dataset and model item related to L3.pk
              pkey = L3.value(filename);
              //calculate record data from new path, new name, crc already is calculated
              //we do not expect problems because files and database are readable and writable
              if(crc > 0)
                {
                  globalFunc.createRecordData(absPath, filename, recordData, dbIconSize, dbKeywordType);
                  //qDebug() <<"KbvCollectionWatchThread::run update" <<path <<filename <<pkey <<recordData.value("keywords").toString(); //###########
                  stmt = stmt2.arg(pkey);
                  query.prepare(stmt);
                  query.bindValue(":filePath",    QVariant(path));    //in db relatve paths only
                  query.bindValue(":fileName",    QVariant(filename));
                  query.bindValue(":icon",        recordData.value("icon"));
                  query.bindValue(":imageW",      recordData.value("imageW"));
                  query.bindValue(":imageH",      recordData.value("imageH"));
                  query.bindValue(":dateChanged", recordData.value("dateChanged"));
                  query.bindValue(":timeChanged", recordData.value("timeChanged"));
                  query.bindValue(":crc32",       QVariant(crc));
                  query.bindValue(":fileSize",    recordData.value("fileSize"));
                  query.bindValue(":keyWords",    recordData.value("keywords"));
                  query.exec();

                  //qDebug() <<"KbvCollectionWatchThread::run update error" <<query.lastError(); //###########
                  //update model item from new dataset
                  stmt = stmt5.arg(pkey);
                  query = db.exec(stmt);
                  if(query.first())
                    {
                      item = globalFunc.itemFromRecord(query, collRootDir);
                      emit updateItem(item);
                    }
                }
            }
        }
      else
        {
          //file not in database, add new dataset for filename to db with stmt3
          //qDebug() <<"KbvCollectionWatchThread::run add new" <<filename; //###########
          crc = globalFunc.crc32FromFile(absPath + filename);
          if(crc > 0)
            {
              globalFunc.createRecordData(absPath, filename, recordData, dbIconSize, dbKeywordType);
              query.prepare(stmt3);
              query.bindValue(":filePath",    QVariant(path));    //in db relatve paths only
              query.bindValue(":fileName",    QVariant(filename));
              query.bindValue(":icon",        recordData.value("icon"));
              query.bindValue(":imageW",      recordData.value("imageW"));
              query.bindValue(":imageH",      recordData.value("imageH"));
              query.bindValue(":dateChanged", recordData.value("dateChanged"));
              query.bindValue(":timeChanged", recordData.value("timeChanged"));
              query.bindValue(":crc32",       QVariant(crc));
              query.bindValue(":fileSize",    recordData.value("fileSize"));
              query.bindValue(":keyWords",    recordData.value("keywords"));
              query.exec();

              //qDebug() <<"KbvCollectionWatchThread::run add error" <<query.lastError(); //###########
              //add model item from new dataset
              stmt = stmt4.arg(path).arg(filename);
              query = db.exec(stmt);
              if(query.first())
                {
                  item = globalFunc.itemFromRecord(query, collRootDir);
                  emit newItem(item);
                }
            }
        } //end of processing filename
    } //end loop

  db.commit();

  //when loop finished regularly: remove orphaned datasets and model items
  //read DB again, remove dir content from L3 - remaining datasets are orphaned.
  //Don't care when we can not remove an orphan
  this->readDatasetsForBranch();
  for(int i=0; i<filecount; i++)
    {
      L3.remove(L1.at(i));
    }
  //L3 contains orphaned data sets, remove by primary key
  db.transaction();
  itpk = L3.constBegin();
  while(itpk != L3.constEnd())
    {
      stmt = stmt1.arg(itpk.value());
      query.exec(stmt);
      query.clear();
      emit invalidate(itpk.value());
      itpk++;
    }
  db.commit();

  //qDebug() <<"KbvCollectionWatchThread::run loop finished"; //###########
  emit  finished();
}
/*************************************************************************//*!
 * Read all file names and crc32 values of watched directory 'path' into
 * map L1. 'path' is expected to be relative with/without terminating "/".
 */
void  KbvCollectionWatchThread::readFileNamesForBranch()
{
  QDir    dir;

  L1.clear();
  dir = QDir(collRootDir+path);
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
  dir.setSorting(QDir::Name | QDir::LocaleAware);
  L1 = dir.entryList();
  //qDebug() << "KbvCollWatchThread::readFileNamesForBranch" <<path; //###########
  //qDebug() << "KbvCollWatchThread::readFileNamesForBranch" <<L1; //###########

}
/*************************************************************************//*!
 * Read all fileName, pk and crc32 values from database where filePath
 * contains 'path'.
 * 'path' is expected to be relative with/without terminating "/".
 */
void  KbvCollectionWatchThread::readDatasetsForBranch()
{
  QSqlQuery     query;
  QString       stmt;
  bool          ok;

  //qDebug() << "KbvCollWatchThread::readDatasetsForBranch" <<path; //###########
  stmt = QString("SELECT fileName,crc32,pk FROM album WHERE filePath LIKE '%1'").arg(path);
  L2.clear();
  L3.clear();

  if(dbType & Kbv::TypeCollection)
    {
      db.transaction();
      query = db.exec(stmt);

      if(query.first())
        {
          //qDebug() << "KbvCollWatchThread::readDatasetsForBranch" << query.value(0).toString(); //###########
          do
            {
              L2.insert(query.value(0).toString(), (quint32) query.value(1).toInt(&ok));
              L3.insert(query.value(0).toString(), query.value(2).toLongLong(&ok));
            }
          while(query.next());
        }
      db.commit();
      //qDebug() << "KbvCollWatchThread::readDatasetsForBranch" <<L2; //###########
    }
}

/*****************************************************************************/
