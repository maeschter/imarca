/*****************************************************************************
 * kbvSearchThread
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 15:27:08 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1470 $
 * Created: 2012.08.24
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 * Structure of database pattern "imageDB.kbvdb".
 * The database contains two tables: album and description.
 * Table description only holds one record:
 * CREATE TABLE "description" ("id" INTEGER PRIMARY KEY NOT NULL ,
 *  "version" TEXT,"iconSize" INTEGER,"comment" TEXT,"colltype" INTEGER,
 *  "keywordtypes" INTEGER,"rootdir" TEXT)
 *
 * - version = database version: major.minor
 * - comment = the user defined description of this database and its content
 * - iconSize = the user defined dimension for the icons
 * - collType = the user defined type of the database: album or collection
 * - keywordtypes: how to derive keywords from filname
 *
 * CREATE TABLE "album" ("id" INTEGER PRIMARY KEY NOT NULL ,"filePath" TEXT,
 *  "fileName" TEXT,"fileURL" TEXT,"icon" BLOB,"imageW" INTEGER,"imageH" INTEGER,
 *  "dateChanged" INTEGER,"timeChanged" INTEGER,"crc32" INTEGER,"fileSize" INTEGER,
 *  "keyWords" TEXT,"userComment" TEXT,"userDate" INTEGER)
 *
 * CREATE INDEX "main"."albumIdx" ON "album" ("filePath" ASC, "fileName" ASC,
 *  "fileURL" ASC, "imageW" ASC, "imageH" ASC, "dateChanged" ASC,
 *  "keyWords" ASC, "userKeyWords" ASC, "userDate" ASC)
 *****************************************************************************/
#include <QtDebug>
#include "kbvSearchThread.h"


KbvSearchThread::KbvSearchThread(QObject *parent) : QThread(parent)
{
  abortThread = false;

  //default
  dbType = Kbv::TypeNone;
  dbKeywordType = 0;
  cat = 0;
  dbIconSize = 0;
  task = 0;
  error = false;
}
/*************************************************************************//*!
 * Destructor waits until the thread has finished or wakes up the sleeping
 * thread then finishes.
 */
KbvSearchThread::~KbvSearchThread()
{
  //qDebug() << "KbvSearchThread::~KbvSearchThread"; //###########
  abortThread = true;
  wait();
}
/*************************************************************************//*!
 * Set database for search. This is the first step before any search or other
 * operation can be performed by the thread. The database already is open.
 */
void    KbvSearchThread::setDatabase(QSqlDatabase database, KbvDBInfo *databaseInfo)
{
  error = false;
  if(this->isRunning())
    {
      abortThread = true;
      wait();
    }
  abortThread = false;

  this->db = database;

  this->dbName = databaseInfo->getName();
  this->dbVer = databaseInfo->getVersion();
  this->dbIconSize = databaseInfo->getIconSize();
  this->dbDescription = databaseInfo->getDescription();
  this->dbType = databaseInfo->getType();
  this->dbKeywordType = databaseInfo->getKeyWordType();
  this->collRoot = databaseInfo->getRootDir();

  this->dbconnection = "search"+dbName;
  //qDebug() << "KbvSearchThread::setDatabase"<<dbName <<database; //###########
}
/*************************************************************************//*!
 * The start() method accepts a list of keywords, a list of paths, a list of
 * integers (e.g. dates, the logical treatment of keywords) and
 * an integer parameter "function" which dictates the processing of the lists.
 * kewordList and pathsList mutually exclude them (either keywords or paths).
 * When the thread is running a restart gets suppressed.
 * When a search on a new database shall be performed the current database
 * gets closed and the connection removed.
 * Search keywords in keywordList: item 0-n: keywords
 * Search paths in pathsList: item 0-n: paths
 * Multilist:
 *  item 0: datetimeFrom as integer "JulianDay"
 *  item 1: datetimeTo as integer "JulianDay"
 *  item 2: keyword logic AND or OR
 *  item 3: search logic ACCURATE or INACCURATE
 */
void    KbvSearchThread::start(QStringList &keywordList, QStringList &pathsList, QList<int> &multilist, int function)
{
  //qDebug() << "KbvSearchThread::start function" <<keywordList <<pathsList <<multilist <<function; //###########
  if(this->isRunning())
    {
      return;
    }
  abortThread = false;
  this->searchPath = false;
  this->task = function;
  entryList.clear();
  if(!pathsList.isEmpty())
    {
      this->searchPath = true;
      this->entryList = pathsList;
    }
  if(!keywordList.isEmpty())
    {
      this->entryList = keywordList;
    }

  if(!multilist.empty())
    {
      this->dateFirst = multilist.at(0);
      this->dateLast = multilist.at(1);
      this->cat = multilist.at(2);
      this->prec = multilist.at(3);
    }
  //The database connection must have been set before any thread function can
  //be successfully triggered. Here we don't care about the database, it's
  //present and open.
  QThread::start(QThread::HighPriority);
}

/*************************************************************************//*!
 * Manipulate model and database depending on parameter "task":
 * task = Kbv::search: search database for keywords, paths or dates,
 *        consider entryList, dateList is invalid
 * task = Kbv::replace: find and update the data set
 *        entryList contains path, name, primary key of each item
 * task = Kbv::remove: remove the data set by primary key
 *        entryList contains path, name, primary key of each item
 */
void    KbvSearchThread::run()
{
  switch(task)
  {
    case Kbv::search: //search database
        search();
        break;
    case Kbv::replace: //replace data set with new data
        replace();
        break;
    case Kbv::remove: //remove data set
        remove();
        break;
    default:
        break;
  }
}
/*************************************************************************//*!
 * Search the database for keywords, paths or dates.
 * Search keywords in entryList: item 0-n
 * Search dates in dateList:
 *  item 1: datetimeFrom, item 2: datetimeTo, both as integer "JulianDay"
 * The method creates a SQL query depending on keywords and dates:
 *  SELECT <columns> FROM album WHERE (keywords LIKE %kw1%) OR (keywords LIKE %kw2%) OR (...)
 *  INTERSECT
 *  SELECT <columns> FROM album WHERE (dateChanged BETWEEN from AND to) OR (userDate BETWEEN from AND to)
 * This matches the search cases: keywords only, dates only or keywords and dates.
 * Search terms are surrounded by %. Hence the string between the % can be found at
 * arbitrary position in column <keywords>. This performs an inaccurate search.
 * For accurate search the search terms are enclosed in word boundaries then surrounded
 * by %. So the complete term including the boundaries must match a keyword.
 */
void    KbvSearchThread::search()
{
  Kbv::kbvItem  *item;
  QSqlQuery query;
  QString   str1, str2, str3, str4, stmt, stmt1, stmt2, andor, kwb;
  int       i, rows, batch;

  batch = 60;
  kwb = QString(keywordBoundary);

  //A percent symbol ("%") in the LIKE pattern matches any sequence of zero or more characters:
  //each search term is surrounded by % symbols to find the string between them
  str1 = QString("SELECT filePath,fileName,icon,imageW,imageH,fileSize,crc32,"
                        "dateChanged,timeChanged,userDate,pk FROM album WHERE");
  str2 = QString(" keyWords LIKE '%%1%' ");   // keyword search
  str3 = QString(" ((dateChanged BETWEEN %1 AND %2) OR (userDate BETWEEN %1 AND %2))"); //date search
  str4 = QString(" filePath LIKE '%%1%' ");   // path search

  //qDebug() << "KbvSearchThread::search entryList" <<entryList <<searchPath <<cat <<prec; //###########

  //searching paths excludes keywords and dates
  if(searchPath)
    {
      //search paths: this always uses OR logic since we hope
      //to find all complete and incomplete path strings
      stmt = str1;
      stmt.append(str4.arg(entryList.at(0)));
      for(i=1; i<entryList.length(); i++)
        {
          stmt.append("OR");    //"OR" catenation for paths
          stmt.append(str4.arg(entryList.at(i)));
        }
    }
  else
    {
      //Search keywords and/or dates
      if(!entryList.isEmpty())
        {
          //1. when accurate search is required we have to enclose the search terms in word boundaries.
          if(prec == Kbv::accurate)
            {
              for(i=0; i<entryList.size(); i++)
                {
                  entryList.replace(i, (kwb + entryList.at(i) + kwb));
                }
            }

          if(cat == Kbv::logicAND)
            {
              andor = "AND";
            }
          if(cat == Kbv::logicOR)
            {
              andor = "OR";
            }

          //2. prepare the SELECT statement stmt1 for keywords evaluating entryList
          stmt1 = str1;
          stmt1.append(str2.arg(entryList.at(0)));
          for(i=1; i<entryList.length(); i++)
            {
              stmt1.append(andor);    //"AND" or "OR" catenation
              stmt1.append(str2.arg(entryList.at(i)));
            }
          //qDebug() << "KbvSearchThread::search keywords" << stmt1; //###########
        }
      
      //3. prepare the SELECT statement stmt2 for dates if required
      if((dateFirst != 0) && (dateLast != 0))
        {
          stmt2 = str1;
          stmt2.append(str3.arg(dateFirst).arg(dateLast));
        }
      //qDebug() << "KbvSearchThread::search dates" << stmt2; //###########

      //4. finish the statement, intersection is required when both searches
      //(keywords and date) are desired, otherwise one of the statements is empty
      stmt.clear();
      if(!stmt1.isEmpty() && !stmt2.isEmpty())
        {
          stmt.append(stmt1);
          stmt.append(QString(" INTERSECT "));
          stmt.append(stmt2);
        }
      if(!stmt1.isEmpty() && stmt2.isEmpty())
        {
          stmt.append(stmt1);
        }
      if(stmt1.isEmpty() && !stmt2.isEmpty())
        {
          stmt.append(stmt2);
        }
    }
  
  //qDebug() << "KbvSearchThread::search stmt" <<stmt; //###########
  db.transaction();
  query.setForwardOnly(true);
  query = db.exec(stmt);
  //qDebug() << "KbvSearchThread::search query" << query.lastError(); //###########
  error = false;

  //do nothing when query is empty or not active
  if((query.first() && query.isActive()))
    {
      //qDebug() << "KbvSearchThread::search performed"; //###########
      //the driver does not report the query size hence we have to count
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
          //qDebug() << "KbvSearchThread::search first" << query.value(0).toString(); //###########
          do
            {
              if (abortThread)
                {
                  break;
                }
              else
                {
                  item = globalFunc.itemFromRecord(query, collRoot);
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

  db.commit();
  emit statusText2(QString(""));
  emit endOfSearching();
}
/*************************************************************************//*!
 * Replace data of a record given by primary key
 * with data of file 'newpath, newname' assuming that the file in file system
 * has been altered. Records get removed when they couldn't be found in the
 * database. The record data get created newly from file where userDate
 * and userComment are not touched.
 * Each item in entryList is described by four values:
 * entryList[i+0] = newpath,
 * entryList[i+1] = newname,
 * entryList[i+2] = primary key.
 * entryList[i+3] = crc32.
 * This function is completed in every case even on closing the database. */
void    KbvSearchThread::replace()
{
  QMap<QString, QVariant>   recordData;
  QList<qlonglong>  removelist;
  QSqlQuery     query;
  QString       stmt1, stmt2, stmt3, path, name, absPath;
  Kbv::kbvItem  *item;
  qlonglong     pkey;
  quint32       crc, crc1;
  int           i;

  //prepare query and statements
  query = QSqlQuery(db);
  stmt1 = QString("UPDATE OR REPLACE album SET filePath = :filePath, fileName = :fileName, "
      "icon = :icon, imageW = :imageW, imageH = :imageH, dateChanged = :dateChanged, "
      "timeChanged = :timeChanged, crc32 = :crc32, fileSize = :fileSize, keyWords = :keyWords, "
      "userDate = :userDate, userComment = :userComment WHERE pk = %1");
  stmt2 = QString("SELECT filePath,fileName,icon,imageW,imageH,fileSize,crc32,dateChanged,timeChanged,userDate,pk FROM album "
                  "WHERE pk = %1");
  stmt3 = QString("DELETE FROM album WHERE pk = %1");

  //qDebug() << "KbvSearchThread::replace" << entryList; //###########

  db.transaction();
  i = 0;
  while(i < entryList.size())
    {
      path = entryList.at(i);     //relative path below collection root
      name = entryList.at(i+1);
      pkey = entryList.at(i+2).toLongLong();
      crc1 = entryList.at(i+3).toUInt();

      absPath = collRoot + path;
      //calculate record data from new path, new name
      //we do not expect problems because files and database are readable and writable
      crc = globalFunc.crc32FromFile(absPath+name);    //absolute path
      if(crc > 0)
        {
          if(crc != crc1)
            {
              //qDebug() << "KbvSearchThread::replace crc crc1" <<crc <<crc1; //###########
              globalFunc.createRecordData(absPath, name, recordData, dbIconSize, dbKeywordType);
              query.prepare(stmt1.arg(pkey));
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

              //update model item from new dataset or remove record
              query = db.exec(stmt2.arg(pkey));
              if(query.first())
                {
                  //qDebug() << "KbvSearchThread::replace item" <<pkey <<name <<path; //###########
                  item = globalFunc.itemFromRecord(query, collRoot);
                  emit updateItem(item);
                }
              else
                {
                  //qDebug() << "KbvSearchThread::replace removelist" <<pkey; //###########
                  removelist.append(pkey);
                }
            }
        }
      else
        {
          //qDebug() << "KbvSearchThread::replace removelist crc" <<pkey; //###########
          removelist.append(pkey);
        }
      i += 4;
      if((i % 40) == 0) //every 10 items
        {
          emit  statusText2(QString("%1").arg(i/4, 6));
        }
    }   //end while
  db.commit();

  emit  statusText2(QString(""));
  emit  statusText3(QString(""));
  db.transaction();
  for(int i=0; i<removelist.size(); i++)
    {
      query = db.exec(stmt3.arg(removelist.at(i)));
      query.clear();
      emit invalidate(removelist.at(i));
    }
  db.commit();
  emit  endOfReplaceing();
}
/*************************************************************************//*!
 * Remove records from database.
 * The parameters path, name, primary key are contained in entryList in this
 * order. In case of album we only remove records from data base.
 * When this is a collection we remove records and files.
 * When path and name are empty, the file already has been removed.
 */
void    KbvSearchThread::remove()
{
  QSqlQuery     query;
  QString       stmt, path, primkey;
  int           i, n;

  //qDebug() << "KbvSearchThread::remove" << entryList.size()/2 << dbType; //###########
  emit  statusText3(QString(tr("Removing files. Please wait.")));

  query = QSqlQuery(db);
  query.setForwardOnly(true);
  error = false;
  i = 0;
  n = entryList.size();
  stmt = QString("DELETE FROM album WHERE pk = %1");

  if(dbType & Kbv::TypeAlbum)
    {
      errorList = QString(tr("File not removed from database:"));
      errorList.append("\n\n");
      db.transaction();
      while(i < n)
        {
          if (abortThread)
            {
              break;
            }
          //entryList contains path, filename and primary key in 3 consecutive lines!
          //records are removed by primary key
          //entryList contains path, filename and primary key in 3 consecutive lines!
          //records are removed by primary key
          path = entryList.at(i) + entryList.at(i+1);
          primkey = entryList.at(i+2);
          query.exec(stmt.arg(primkey));
          //qDebug() << "KbvSearchThread::remove" << stmt; //###########
          //qDebug() << "KbvSearchThread::remove" << query.lastError(); //###########
          if(!query.isActive())
            {
              error = true;
              errorList.append(path);
              errorList.append("\n");
            }
          query.clear();
          if((i % 30) == 0) //every 10 items
            {
              emit  statusText2(QString("%1").arg((n-i)/3, 6));
            }
          i += 3;
        }
      //qDebug() << "KbvSearchThread::remove finished"; //###########
      db.commit();
      emit  statusText2(QString(""));
      emit  statusText3(QString(""));
    }

  if(dbType & Kbv::TypeCollection)
    {
      errorList = QString(tr("File not removed from database or not deleted:"));
      errorList.append("\n\n");
      db.transaction();
      while(i < n)
        {
          if (abortThread)
            {
              break;
            }
          //entryList contains path, filename and primary key in 3 consecutive lines!
          //records are removed by primary key
          path = entryList.at(i) + entryList.at(i+1);
          primkey = entryList.at(i+2);
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
              emit  statusText2(QString("%1").arg((n-i)/3, 6));
            }
          i += 3;
        }
      db.commit();
      emit  statusText2(QString(""));
      emit  statusText3(QString(""));
    }

  //finish
  if(error)
    {
      emit  warning(errorList, true);
    }
}

/*****************************************************************************/
