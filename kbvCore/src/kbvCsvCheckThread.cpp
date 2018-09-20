/*****************************************************************************
 * kbvCsvCheckThread
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2014.11.21
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvInformationDialog.h"
#include "kbvCsvCheckThread.h"

extern  KbvInformationDialog  *informationDialog;

KbvCsvCheckThread::KbvCsvCheckThread(QObject *parent, KbvDBInfo *databaseInfo) : QThread(parent)
{
  abortThread = false;

  //Read database settings from databaseInfo
  dbName = databaseInfo->getName();
  dbLocation = databaseInfo->getLocation();
  collRootDir = databaseInfo->getRootDir();
  dbIconSize = databaseInfo->getIconSize();
  dbType = databaseInfo->getType();
  dbKeywordType = databaseInfo->getKeyWordType();

  informationDialog = new KbvInformationDialog(nullptr);

  //the thread destroys itself when finished
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
  connect(this, SIGNAL(finished()), this, SLOT(showFilesWithIssues()));
  id = -1;
  flags = Kbv::none;
}
KbvCsvCheckThread::~KbvCsvCheckThread()
{
  //qDebug() << "KbvCsvCheckThread::~KbvCsvCheckThread";//###########
  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection
}
/*************************************************************************//*!
 * The start() method accepts the thread id, the path of the csv-file, the
 * path to the collection, the path to the trading dir, the pointer to the
 * calling object for writing results and the flags.
 * Note: The thread has been instiantiated newly when the start() method is
 * called. So check for running will always return false but the database
 * connection may already exist (when the thread is started twice). So we
 * must inhibit calling the method when the thread has been started already.
 */
void    KbvCsvCheckThread::start(int id, QString csvFilePath, QString collectionPath, QString exchangeDir, KbvCsvChecker *checker, int flags)
{
    QString     str;
  //qDebug() << "KbvCsvCheckThread::start" <<id <<csvFilePath <<collectionPath <<exchangeDir; //###########

  if(!isRunning())
    {
      this->id = id;
      this->csvPath = csvFilePath;
      this->collPath = collectionPath;
      this->tradeDir = exchangeDir;
      this->csvChecker = checker;
      this->flags = flags;

      //establish unique db connection
      str = QString("%1").arg(id);
      dbconnection = dbName+str;
      db = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
      db.setHostName("host");
      db.setDatabaseName(dbLocation+dbName+QString(dbNameExt));
      db.open();
      //qDebug() << "KbvCsvCheckThread::start" <<dbconnection <<db; //###########

      QThread::start(QThread::HighPriority);
    }
}
/*************************************************************************//*!
 * The stop() method waits until the thread finishes.
 */
bool    KbvCsvCheckThread::stop()
{
  if(this->isRunning())
    {
      //qDebug() << "KbvCsvCheckThread::start abort"; //###########
      abortThread = true;
      wait();
    }
  abortThread = false;
  return true;
}
/*************************************************************************//*!
 * The run() method performs multiple functions.
 */
void    KbvCsvCheckThread::run()
{
  if(flags & KbvCheck::checkCollection)
    {
      this->checkDirectories();
    }
  if(flags & KbvCheck::copyToUploadDir)
    {
      this->copyToUploadDir();
    }
  if(flags & KbvCheck::moveToCollection)
    {
      this->moveToCollection();
    }
  if(flags & KbvCheck::createCsvFile)
    {
      this->createCsvFile();
    }
}

/*************************************************************************//*!
 * SLOT: show files on which a function didn't had success, e.g. file not
 * moved). The first text part contains the reason, the remaining text
 * comprises the file names. All text parts are separated by "\n".
 */
void    KbvCsvCheckThread::showFilesWithIssues()
{
  if(!filesWithIssues.isEmpty())
    {
      informationDialog->perform("", filesWithIssues, 0);
    }
}

/*************************************************************************//*!
 * Check directories. The csv-file and all involved directories must be valid.
 * For details see activity diagram KbvCsvCheckerThread_run.
 * The csv-file is a pure text file and requires the following fields without
 * any header line:
 * File name, file size, file crc32, file path (optionally), comment (optionally).
 * All fields must be separated by comma. Fields four and five may be empty. A
 * trailing comma will be ignored. File size is in decimal format, file crc
 * in hexadecimal format.
 * The check starts in collPath (which may be a collection subdirectory) and
 * includes all files found in collPath and it's subdrirectories.
 * Produced results:
 *  collFileList contains: name, path
 *  collCRCList contains: crc32, path/name
 *  foundlist contains: name, crc32, size, path
 *  missList contains: name, crc32, size
 *  wrongSizeList contains: name, crc32, size, path
 *  wrongCrcList contains: name, crc32, size, path
 */
void    KbvCsvCheckThread::checkDirectories()
{
  QMultiMap<QString, QString> collFileList;
  QMultiMap<int, QString>     collCRCList;
  
  QStringList     entryList, crcList, foundList, wrongCrcList, wrongSizeList, wrongPathList;
  QStringList     missList, dupList, renameList, faultyLines;
  QTime           duration;
  QString         checktime, csvLineItems, name, path, str;
  QFile           csvFile;
  QFileInfo       fileInfo;
  QTextStream     inCSV;
  int             unknown, crc, size, crc1, size1, size2, csvLines, total, len;

  total = 0;
  csvLines = 0;
  duration.start();
  
  //parse collection path and list all files in collFileList
  //collFileList: name, path and collCRCList: crc32, path, name 
  parseDirectories(collPath, collFileList, collCRCList);
  len = collPath.length();
  
  //qDebug() << "KbvCsvCheckThread::run collection path"<<collPath <<len; //###########
 
  //open csv-file
  csvFile.setFileName(csvPath);
  if(!csvFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      //file problem, clear check results
      csvChecker->saveCheckResults(id, 0, 0, 0, 0, 0, 0, 0);
      return; 
    }
  inCSV.setDevice(&csvFile);
  
  //Outer loop: process all lines in csv-file
  while(!inCSV.atEnd() && !abortThread)
    {
      //read line from csv-file and split components
      csvLines++;
      csvLineItems = inCSV.readLine();
      //qDebug() << "KbvCsvCheckThread::run csv line"<<csvLineItems; //###########
      //ignores empty or corrupt lines, check all components for correctness
      if(splitCsvLine(csvLineItems, name, crc, size, path))
        {
          //process all correct lines of collFileList
          total++;
          if(collFileList.contains(name))
            {
              //qDebug() << "KbvCsvCheckThread::run name found:"<<name; //###########
              entryList = collFileList.values(name);  //all paths for name
              
              //Inner loop: check all occurances of name:
              //add all to foundList (includes duplicates)
              for(int i=0; i < entryList.size(); i++)
                {
                  crc1 = globalFunc.crc32FromFile(entryList.at(i) + name);
                  fileInfo.setFile(entryList.at(i) + name);
                  size1 = fileInfo.size();
                  
                  //check crc
                  if(crc1 == crc)
                    {
                      if(size1 == size)
                        {
                          //add to foundlist: name, crc32, size, path
                          //qDebug() << "KbvCsvCheckThread::run found list"<<name; //###########
                          foundList.append(name);
                          foundList.append(QString("%1").arg((quint32) crc, 8, 16, QChar('0')));
                          foundList.append(QString("%1").arg((uint) size, 0, 10));
                          foundList.append(entryList.at(i));
                        }
                      else
                        {
                          //wrongSizeList: name, crc32, size, path
                          //qDebug() << "KbvCsvCheckThread::run wrongSizeList"<<name; //###########
                          wrongSizeList.append(name);
                          wrongSizeList.append(QString("%1").arg((quint32) crc, 8, 16, QChar('0')));
                          wrongSizeList.append(QString("%1").arg((uint) size, 0, 10));
                          wrongSizeList.append(entryList.at(i));
                        }
                      //check paths of ecsv
                      //remove collection path from file path to get the relative ecsv path
                      if((flags & KbvCheck::includePath) && !path.isEmpty())
                        {
                           str = entryList.at(i);
                           if(!(path == str.remove(0, len)))
                             {
                               wrongPathList.append(name);
                               wrongPathList.append(entryList.at(i));
                               //qDebug() << "KbvCsvCheckThread::run wrongPathList"<<name <<entryList.at(i); //###########
                             }
                         }
                    }
                  else
                    {
                      //add to wrong crc list: : name, crc32, size, path
                      //qDebug() << "KbvCsvCheckThread::run wrongCrcList"<<name; //###########
                      wrongCrcList.append(name);
                      wrongCrcList.append(QString("%1").arg((quint32) crc, 8, 16, QChar('0')));
                      wrongCrcList.append(QString("%1").arg((uint) size, 0, 10));
                      wrongCrcList.append(entryList.at(i));
                    }
                } //end of inner loop
            }
          else
            {
              //name not in collFileList, search crc in collCRCList
              //qDebug() << "KbvCsvCheckThread::run name not found:"<<name; //###########
              if(collCRCList.contains(crc))
                {
                  //Inner loop 2: check all occurances of crc:
                  crcList = collCRCList.values(crc); //all crc, path/name for this crc
                  for(int k=0; k<crcList.size();k++)
                    {
                      fileInfo.setFile(crcList.at(k));
                      size2 = fileInfo.size();
                      if(size2 == size)
                        {
                          //wrong name but crc and size ok, file can be renamed
                          //renameList contains correct name, correct crc,
                          //correct size and path of wrong name
                          renameList.append(name);
                          renameList.append(QString("%1").arg((quint32) crc, 8, 16, QChar('0')));
                          renameList.append(QString("%1").arg((uint) size, 0, 10));
                          renameList.append(crcList.at(k));   //path/name
                          //qDebug() << "KbvCsvCheckThread::run renameList"<<name <<crcList.at(k); //###########
                        }
                      else
                        {
                          //crc found but wrong size
                          //missList: : name, crc32, size
                          //qDebug() << "KbvCsvCheckThread::run missList"<<name; //###########
                          missList.append(name);
                          missList.append(QString("%1").arg((quint32) crc, 8, 16, QChar('0')));
                          missList.append(QString("%1").arg((uint) size, 0, 10));
                        }
                    } //end of inner loop 2
                }
              else
                {
                  //neither name nor crc found, missList: name, crc32, size
                  //qDebug() << "KbvCsvCheckThread::run missList"<<name; //###########  delete infoDialog;

                  missList.append(name);
                  missList.append(QString("%1").arg((quint32) crc, 8, 16, QChar('0')));
                  missList.append(QString("%1").arg((uint) size, 0, 10));
                }
            }
        }
      else
        {
          //faulty line, corrupt name, crc, size or path, add to list
          if(!name.isEmpty())
            {
              faultyLines.append(QString("%1").arg(csvLines, 0, 10));
            }
        }
    } //end of outer loop

  //process results only when thread finished regularily
  if(!abortThread)
    {
      extractDuplicates(foundList, dupList);
      createWrongMiss(wrongCrcList, wrongSizeList, missList);
      if(flags & KbvCheck::rename)
        {
          renameFiles(foundList, renameList);
        }
      
      checktime = QString("check time:  %L1s").arg(double(duration.elapsed()/1000.0), 0, 'f', 3);
      createReport(foundList, wrongCrcList, wrongSizeList, wrongPathList, missList, dupList, renameList, faultyLines, checktime);
      
      unknown = collFileList.size() - total;
      csvChecker->saveCheckResults(id, total, foundList.size()/4, (wrongCrcList.size()+wrongSizeList.size())/4,
                                   missList.size()/3, renameList.size()/4, dupList.size()/4, unknown);
      //qDebug() << "KbvCsvCheckThread::run id time" <<id <<checktime; //###########
      //qDebug() << "KbvCsvCheckThread::run      total" <<total; //###########
      //qDebug() << "KbvCsvCheckThread::run    correct" <<foundList.size()/4; //###########
      //qDebug() << "KbvCsvCheckThread::run      wrong" <<(wrongCrcList.size()+wrongSizeList.size())/4; //###########
      //qDebug() << "KbvCsvCheckThread::run    missing" <<missList.size()/3; //###########
      //qDebug() << "KbvCsvCheckThread::run     rename" <<renameList.size()/4; //###########
      //qDebug() << "KbvCsvCheckThread::run duplicates" <<dupList.size()/4; //###########
      //qDebug() << "KbvCsvCheckThread::run    unknown" <<unknown; //###########
    }
}
/*************************************************************************//*!
 * Searches the files in 'csvFile' and copies them to the upload directory.
 * The csv-file and all involved directories must be valid.
 * Uses: csvPath, collPath (source), tradeDir (target)
 */
void    KbvCsvCheckThread::copyToUploadDir()
{
  QMultiMap<QString, QString> collFileList;
  QMultiMap<int, QString>     collCRCList;
  QStringList     entryList;
  QString         csvLineItems, name, path;
  QFile           csvFile, file;
  QFileInfo       fileInfo;
  QTextStream     inCSV;

  int             crc, size, crc1, size1, csvLines, total;

  total = 0;
  csvLines = 0;
  filesWithIssues.clear();

  //parse collRoot and list all files in collFileList
  //collFileList: name, path and collCRCList: crc32, path, name 
  parseDirectories(collPath, collFileList, collCRCList);
  
  //open csv-file
  csvFile.setFileName(csvPath);
  if(!csvFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      //file problem, clear threadlist, no check results
      csvChecker->saveCheckResults(id, 0, 0, 0, 0, 0, 0, 0);
      return; 
    }

  inCSV.setDevice(&csvFile);
  
  //Outer loop: process all lines in csv-file
  while(!inCSV.atEnd() && !abortThread)
    {
      //read line from csv-file and split components
      csvLines++;
      csvLineItems = inCSV.readLine();
      //qDebug() << "KbvCsvCheckThread::copyToUploadDir line"<<csvLineItems; //###########
      //ignores empty or corrupt lines
      if(splitCsvLine(csvLineItems, name, crc, size, path))
        {
          //process all correct lines of collFileList
          total++;
          if(collFileList.contains(name))
            {
              entryList = collFileList.values(name);  //all paths for name
              
              //Inner loop: check all occurances of name:
              for(int i=0; i < entryList.size(); i++)
                {
                  crc1 = globalFunc.crc32FromFile(entryList.at(i) + name);
                  fileInfo.setFile(entryList.at(i) + name);
                  size1 = fileInfo.size();
                  
                  //check crc
                  if((crc1 == crc) && (size1 == size))
                    {
                      //copy to upload directory
                      if(!file.copy(entryList.at(i)+name, tradeDir+name))
                        {
                          //not copied
                          filesWithIssues.append(name + "\n");
                        }
                    }
                  else
                    {
                      //crc or file size wrong
                      filesWithIssues.append(name + "\n");
                    }
                } //end of inner loop
            }
          else
            {
              //file not found
              filesWithIssues.append(name + "\n");
            }
        }
    } //end of loop

  //show issues when thread finishes
  if(!filesWithIssues.isEmpty())
    {
      filesWithIssues.prepend(tr("Some files couldn't be copied:") + "\n\n");
    }

  //clear threadlist only, don't change results for this threadId
  csvChecker->saveCheckResults(id, 0, 0, 0, 0, 0, 0, 0);
}
/*************************************************************************//*!
 * Move the files in 'csvFile' from download directory to the collection
 * paths. For this 'csvFile' must have ECSV structure including valid paths.
 * The csv-file and all involved directories must be valid. Since the csv-file
 * contains relative paths we must prepend the collection root dir.
 * The function does not care about the reasonability of the paths but checks
 * for formal correctness.
 * Uses: collRootDir (collection root dir), path (collection sub dir) and
 * tradeDir (download dir)
 */
void    KbvCsvCheckThread::moveToCollection()
{
  QMap<QString, QVariant> recordData;
  QSqlQuery       query;
  QString         csvLineItems, name, path, str, stmt, stmt1, stmt2, stmt3;
  QDir            dir;
  QFile           csvFile, file;
  QFileInfo       fileInfo;
  QTextStream     inCSV;
  int             crc, size, crc1, size1;
  bool            moved;

  stmt1 = QString("UPDATE OR REPLACE album SET crc32 = :crc32, fileSize = :fileSize "
                  "WHERE filePath LIKE '%1' AND fileName like '%2'");
  stmt2 = QString("INSERT INTO album (filePath,fileName,icon,imageW,imageH,dateChanged,timeChanged,crc32,"
                  "fileSize, keyWords) VALUES "
                  "(:filePath,:fileName,:icon,:imageW,:imageH,:dateChanged,:timeChanged,:crc32,:fileSize,:keyWords)");
  stmt3 = QString("DELETE FROM album WHERE filePath LIKE '%1' AND fileName like '%2'");

  filesWithIssues.clear();

  //open csv-file
  csvFile.setFileName(csvPath);
  if(!csvFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      //file problem, clear threadlist only, don't change results for this threadId
      csvChecker->saveCheckResults(id, 0, 0, 0, 0, 0, 0, 0);
      return; 
    }

  //check db connection
  if(!db.open())
    {
      //database problem, clear threadlist only, don't change results for this threadId
      csvChecker->saveCheckResults(id, 0, 0, 0, 0, 0, 0, 0);
      return;
    }

  inCSV.setDevice(&csvFile);
  query = QSqlQuery(db);
  db.transaction();

  //Outer loop: process all lines in csv-file
  while(!inCSV.atEnd() && !abortThread)
    {
      //read line from csv-file and split components
      csvLineItems = inCSV.readLine();
      //ignores empty or corrupt lines, check all components for correctness
      if(splitCsvLine(csvLineItems, name, crc, size, path))
        {
          //Check for target dir and create it when reqired
          str = collRootDir + path;
          if(str.endsWith("/"))
            {
              str.truncate(str.length()-1);
            }
          dir.setPath(str);
          if(!dir.exists())
            {
              //qDebug() << "KbvCsvCheckThread::moveToCollection make path"<<str; //###########
              dir.mkpath(str);
            }
          path = collRootDir + path;
          //calculate crc32 and size from file in download dir
          crc1 = globalFunc.crc32FromFile(tradeDir + name);
          if(crc1 != 0)
            {
              fileInfo.setFile(tradeDir + name);
              size1 = fileInfo.size();
              if((crc1 == crc) && (size1 == size))
                {
                  //move from download dir to collection path
                  //the copy function doesn't overwrite existing files
                  //qDebug() << "KbvCsvCheckThread::moveToCollection "<<tradeDir + name; //###########
                  //qDebug() << "KbvCsvCheckThread::moveToCollection to  "path; //###########
                  if(file.exists(path + name))
                    {
                      if(file.remove(path + name))
                        {
                          file.setFileName(tradeDir + name);
                          moved = file.copy(path + name);
                          if(moved)
                            {
                              //old file removed, new file copied
                              //adjust crc and filesize in db record
                              stmt = stmt1.arg(path).arg(name);
                              query.prepare(stmt);
                              query.bindValue(":fileSize",  QVariant(size));
                              query.bindValue(":crc32",     QVariant(crc));
                              query.exec();
                            }
                          else
                            {
                              //old file removed, new file not copied
                              //remove orphaned record
                              stmt = stmt3.arg(path).arg(name);
                              query.prepare(stmt);
                              query.exec();
                            }

                        }
                      else
                        {
                          //old file not removed, do not copy new file
                          moved = false;
                        }
                    }
                  else
                    {
                      file.setFileName(tradeDir + name);
                      moved = file.copy(path + name);
                      if(moved)
                        {
                          //add new record
                          globalFunc.createRecordData(path, name, recordData, dbIconSize, dbKeywordType);
                          query.prepare(stmt2);
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
                          query.bindValue(":userDate",    QVariant());
                          query.bindValue(":userComment", QVariant());
                          query.exec();
                        }
                    }
                  if(moved)
                    {
                      //success, remove source file
                      file.remove();
                    }
                  else
                    {
                      filesWithIssues.append(name + "\n");
                      //qDebug() << "KbvCsvCheckThread::moveToCollection not moved "<<name; //###########
                    }
                }
              else
                {
                  //crc32 or size wrong
                  filesWithIssues.append(name + "\n");
                 //qDebug() << "KbvCsvCheckThread::moveToCollection wrong crc,size "<<name; //###########
                }
            }
          else
            {
              //crc32 failed or file not found
              filesWithIssues.append(name + "\n");
              //qDebug() << "KbvCsvCheckThread::moveToCollection failed crc "<<name; //###########
            }
        }
      else
        {
          //empty or corrupt line in csv-file or path corrupt or missing
          //qDebug() << "KbvCsvCheckThread::moveToCollection wrong csv line "<<name; //###########
        }
  } //end of loop
  db.commit();

  //show issues when thread finishes
  if(!filesWithIssues.isEmpty())
    {
      filesWithIssues.prepend(tr("Some files couldn't be moved:") + "\n\n");
    }
  //clear threadlist, don't change results for this threadId
  csvChecker->saveCheckResults(id, 0, 0, 0, 0, 0, 0, 0);
}
/*************************************************************************//*!
 * Create ECSV file from files of selected directory and all sub directories.
 * Basically the path get added (ECSV), file separator is '/', line terminator
 * is '\n' (line feed) and coding is UTF8.
 * If windows style is reqired, file separator is '\', line terminator is
 * '\r\n' (carriage return - line feed) and coding is ISO 8859-15.
 * Uses: collPath (source dir), tradeDir (target dir for ECSV)
 */
void    KbvCsvCheckThread::createCsvFile()
{
  QStringList     collFileList;
  QStringList     entryList;
  QString         csvLine, name, path;
  QFile           csvFile;
  QFileInfo       fileInfo;
  QTextStream     outCSV;
  QTime           duration;
  quint32         crc;
  qint64          size;

  duration.start();
  
  //parse selected dir and sub directories
  //collect name, path in collFileList 
  parseForCSV(collPath, collFileList);
  
  //csv-file: name = name of selected dir + file count
  entryList = collPath.split(QString("/"), QString::SkipEmptyParts);
  name = entryList.last();
  path = tradeDir;
  if(!tradeDir.endsWith(QString("/")))
    {
      path.append(QString("/"));
    }
  path.append(name);
  path.append(QString("_%1.ecsv").arg(collFileList.size()/2));

  csvFile.setFileName(path);
  if(!csvFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
      //file problem, clear threadlist, no check results
      csvChecker->saveCheckResults(id, 0, 0, 0, 0, 0, 0, 0);
      return; 
    }
  csvFile.resize(0);
  outCSV.setDevice(&csvFile);
  
  //collFileList contains pairs of name, path, so step is 2
  for(int i=0; i<collFileList.size(); i+=2)
    {
      name = collFileList.at(i);
      path = collFileList.at(i+1);
      crc = globalFunc.crc32FromFile(path + name);
      fileInfo.setFile(path + name);
      size = fileInfo.size();
      
      //make path relative to source dir
      path.remove(collPath);
      if(!path.isEmpty())
        {
          if(!path.startsWith("/"))
            {
              path.prepend("/");
            }
        }
      
      //qDebug() << "KbvCsvCheckThread::createCsvFile line:"<<i <<name <<path <<flags; //###########
      csvLine.clear();
      csvLine.append(name + ",");
      csvLine.append(QString("%1,").arg(size, 0, 10));
      csvLine.append(QString("%1,").arg(crc, 8, 16, QChar('0')));
      
      if(flags & KbvCheck::ecsvLinux)
        {
          //Unix/Linux: UTF8
          csvLine.append(path + ("\n"));
          outCSV << csvLine;
        }
      else
        {
          //windows style: ISO-8859-15
          path.replace("/", "\\");
          csvLine.append(path + ("\r\n"));
          outCSV.setCodec("ISO 8859-15");
          outCSV << csvLine.toLatin1();
        }
    }
  csvFile.flush();
  csvFile.close();
  //clear threadlist only, don't change results for this threadId
  csvChecker->saveCheckResults(id, 0, 0, 0, 0, 0, 0, 0);
  //qDebug() <<  QString("create CSV: time = %L1s").arg(double(duration.elapsed()/1000.0), 0, 'f', 3);
}

/*************************************************************************//*!
 * Extract duplicate files from 'found' to 'duplicate'.
 * foundlist: name, crc32, size, path - dupList: name, crc32, size, path
 * foundlist can contain multiple entries of same 'name'/'path' pairs
 * Both lists get modified.
 */
void    KbvCsvCheckThread::extractDuplicates(QStringList &found, QStringList &duplicates)
{
  QString   name, path, size, crc;
  int       j, k;
  
  //extract duplicates from foundlist; sets of same files can appear at different positions
  k = 0;
  while(k < found.size())
    {
      name = found.at(k);
      crc = found.at(k+1);
      size =  found.at(k+2);
      path = found.at(k+3);
      
      if(!name.isEmpty())
        {
          //parse foundList for multiple entries of 'name'
          //start each search from index of previously found item
          j = k;
          while(j >= 0)
            {
              j = found.indexOf(name, j+1);   //returns -1 when not found
              if(j >= 0)
                {
                  //when this occurance of 'name' has same path: remove from 'found'
                  //otherwise: add to 'duplicates' when not already contained
                  if(path != found.at(j+3))
                    {
                      //when not contained, add this to 'duplicates'
                      //1. entry is the first occurance
                      if(!inList(duplicates, name, path))
                        {
                          //qDebug() << "KbvCsvCheckThread::extractDuplicates dup"<<k <<name; //###########
                          //qDebug() << "KbvCsvCheckThread::extractDuplicates dup"<<j <<path; //###########
                          //qDebug() << "KbvCsvCheckThread::extractDuplicates dup"<<j <<found.at(j+3); //###########
                          duplicates.append(name);
                          duplicates.append(crc);
                          duplicates.append(size);
                          duplicates.append(path);
                        }
                      //2. entry is the first duplicate found
                      if(!inList(duplicates, name, found.at(j+3)))
                        {
                          //qDebug() << "KbvCsvCheckThread::extractDuplicates dup"<<k <<name; //###########
                          //qDebug() << "KbvCsvCheckThread::extractDuplicates dup"<<j <<path; //###########
                          //qDebug() << "KbvCsvCheckThread::extractDuplicates dup"<<j <<found.at(j+3); //###########
                          duplicates.append(found.at(j));
                          duplicates.append(found.at(j+1));
                          duplicates.append(found.at(j+2));
                          duplicates.append(found.at(j+3));
                        }
                    }
                  else
                    {
                      //qDebug() << "KbvCsvCheckThread::extractDuplicates multi"<<k <<name; //###########
                      //qDebug() << "KbvCsvCheckThread::extractDuplicates multi"<<j <<path; //###########
                      //qDebug() << "KbvCsvCheckThread::extractDuplicates multi"<<j <<found.at(j+3); //###########
                      //name,path already in 'found': set name invalid
                      found.replace(j, "");
                    }
                }
            } //end of inner loop
        }
      k += 4;
    } //end of loop over 'found'

  //purge 'found'
  k = 0;
  while(k < found.size())
    {
      if(found.at(k).isEmpty())
        {
          found.removeAt(k); //empty name
          found.removeAt(k); //crc32
          found.removeAt(k); //size
          found.removeAt(k); //path
        }
      else
        {
          k += 4;
        }
    }
}
/*************************************************************************//*!
 * Rename files. renameList contains all files with wrong name but matching
 * size and crc32. Files which couldn't be renamed remain in the list and
 * appear in the report. All renamed files are added to foundlist.
 * Structure of renameList: correct name, crc, size, wrong path/name
 * Structure of foundlist: name, size, crc32, path
 */
void    KbvCsvCheckThread::renameFiles(QStringList &foundlist, QStringList &renamelist)
{
  QSqlQuery     query;
  QFile         file;
  QString       path, oldname, stmt, stmt1;
  int           n;

  stmt1 = QString("UPDATE OR REPLACE album SET fileName = :fileName WHERE filePath LIKE '%1' AND fileName like '%2'");
  query = QSqlQuery(db);
  filesWithIssues.clear();

  if(flags & (KbvCheck::rename != 0))
    {
      db.transaction();
      for(int k=0; k<renamelist.size(); k+=4)
        {
          n = renamelist.at(k+3).lastIndexOf("/");
          path = renamelist.at(k+3).left(n+1);
          n = renamelist.at(k+3).length() - n - 1;
          oldname =  renamelist.at(k+3).right(n);
          
          //qDebug() << "KbvCsvCheckThread::renameFiles path oldname"<<oldname;//###########
          //qDebug() << "KbvCsvCheckThread::renameFiles path newname"<<path+renamelist.at(k);//###########
          
          file.setFileName(renamelist.at(k+3));
          if(file.rename(path+renamelist.at(k)))
            {
              //adjust name in db record
              stmt = stmt1.arg(path).arg(oldname);
              query.prepare(stmt);
              query.bindValue(":fileName",  QVariant(renamelist.at(k)));
              query.exec();

              //add renamed file to foundlist
              foundlist.append(renamelist.at(k));   //name
              foundlist.append(renamelist.at(k+1)); //crc32
              foundlist.append(renamelist.at(k+2)); //size
              foundlist.append(path);

              //set name invalid in renamelist
              renamelist.replace(k, "");
            }
          else
            {
              filesWithIssues.append(oldname + "\n");
            }
        }
      db.commit();

      //purge renamelist
      n = 0;
      while(n < renamelist.size())
        {
          if(renamelist.at(n).isEmpty())
            {
              renamelist.removeAt(n); //empty name
              renamelist.removeAt(n); //crc32
              renamelist.removeAt(n); //size
              renamelist.removeAt(n); //path
            }
          else
            {
              n += 4;
            }
        }
    }

  //show issues when thread finishes
  if(!filesWithIssues.isEmpty())
    {
      //show issues when thread finishes
      filesWithIssues.prepend(tr("Some files couldn't be renamed:") + "\n\n");
    }
}

/*************************************************************************//*!
 * Returns true, when the pair 'name'/'path' already are in list duplicates.
 * Structure of duplicates: name, crc32, size, path
 */
bool    KbvCsvCheckThread::inList(const QStringList &duplicates, const QString &name, const QString &path)
{
  int   k, index, maxdup;
  
  index = 0;
  maxdup = duplicates.count(name);
  for(k=0; k<maxdup; k++)
    {
      index = duplicates.indexOf(name, index);
      if(index >= 0)
        {
          //name found at index, check path at index+3
          if(path == duplicates.at(index+3))
            {
              return true;                            
            }
        }
      //increase index so we do not stuck at first found
      index++;
    }
  return false;
}
/*************************************************************************//*!
 * Create a file containing all wrong and missing files for trading. The file
 * will have the path and name of the csv file extended by '_miss_wrong_'.
 * missList: name, crc32, size
 * wrongSizeList: name, crc32, size, path
 * wrongCrcList: name, crc32, size, path
 */
void    KbvCsvCheckThread::createWrongMiss(const QStringList &wrongCrc, const QStringList &wrongSize, const QStringList &missing)
{
  QDir            dir;
  QFile           wrongFile;
  QTextStream     outWrong;
  QString         wrongPath, str, linend;
  int             wrongmiss, n;
  
  //create new file name including the number of missing and wrong files
  wrongmiss = wrongCrc.size()/4 + wrongSize.size()/4 + missing.size()/3;
  str = QString("_miss_wrong_%1").arg(wrongmiss);
  wrongPath = csvPath;
  n = wrongPath.lastIndexOf(".");
  if(n<0)
    {
      wrongPath.append(str + ".csv");
    }
  else
    {
      wrongPath.insert(n, str);
    }
  
  //find and remove old 'miss_wrong' file
  n = wrongPath.lastIndexOf("/");
  str = wrongPath.mid(n+1, (wrongPath.lastIndexOf("_")-n));
  
  dir.setPath(wrongPath.left(n+1));
  QStringList nameFilter("*.csv");
  QStringList fileList = dir.entryList(nameFilter, (QDir::Files | QDir::NoSymLinks));
  for(n=0; n<fileList.size(); n++)
    {
      if(fileList.at(n).startsWith(str))
        {
          dir.remove(fileList.at(n));
        }
    }
  
  //create new 'miss_wrong' file
  if(wrongmiss > 0)
    {
      if(flags & KbvCheck::ecsvLinux)
        {
          linend = QString("\n");
          outWrong.setCodec("UTF-8");
        }
      else
        {
          linend = QString("\r\n");
          outWrong.setCodec("ISO 8859-15");

        }
      wrongFile.setFileName(wrongPath);
      if(wrongFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
          wrongFile.resize(0);
          outWrong.setDevice(&wrongFile);
          
          for(int i=0; i<wrongCrc.size(); i+=4)
            {
              outWrong <<wrongCrc.at(i) <<"," <<wrongCrc.at(i+2) <<"," <<wrongCrc.at(i+1) <<",wrong crc" <<linend;
            }
    
          for(int i=0; i<wrongSize.size(); i+=4)
            {
              outWrong <<wrongSize.at(i) <<"," <<wrongSize.at(i+2) <<"," <<wrongSize.at(i+1) <<",wrong size" <<linend;
            }
          
          for(int i=0; i<missing.size(); i+=3)
            {
              outWrong <<missing.at(i) <<"," <<missing.at(i+2) <<"," <<missing.at(i+1) <<",missing" <<linend;
            }
          wrongFile.flush();
          wrongFile.close();
        }
    }
}
/*************************************************************************//*!
 * Collect all result lists into a report file. The report file will have the
 * name of the csv file extended by '_report' in the same dir.
 * Uses: csvPath, flags
 */
void    KbvCsvCheckThread::createReport(const QStringList &found, const QStringList &wrongCrc,
                                        const QStringList &wrongSize, const QStringList &wrongPath,
                                        const QStringList &missing, const QStringList &duplicates, const QStringList &rename,
                                        const QStringList &faultyLines, const QString &checktime)
{
  Q_UNUSED(duplicates)
  QFile           reportFile;
  QTextStream     outReport;
  QString         reportPath, str1, linend;
  int             n;

  n = csvPath.lastIndexOf(".");
  if(n<0)
    {
      reportPath = csvPath.append("_report.txt");
    }
  else
    {
      reportPath = csvPath.left(n);
      reportPath.append("_report.txt");
    }
  //qDebug() << "KbvCsvCheckThread::createReport" <<reportPath; //###########
  reportFile.setFileName(reportPath);

  if(flags & KbvCheck::ecsvLinux)
    {
      linend = QString("\n");
      outReport.setCodec("UTF-8");
    }
  else
    {
      linend = QString("\r\n");
      outReport.setCodec("ISO 8859-15");
    }
  
  if(reportFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      reportFile.resize(0);
      outReport.setDevice(&reportFile);
      
      //Header
      QDateTime dt = QDateTime::currentDateTime();
      str1 = dt.toString("yyyy.MM.dd-hh:mm");
      
      outReport <<QString("Imarca csv checker") <<linend;
      outReport <<QString("Date: %1").arg(str1) <<linend;
      outReport <<QString("Collection: %1").arg(collPath) <<linend;
      outReport <<QString("CSV file: %1").arg(csvPath) <<linend;
      outReport <<checktime <<linend <<linend;
      if(faultyLines.size() > 0)
        {
          outReport <<QString("Corrupt csv lines found:") <<linend;
          outReport <<faultyLines.at(0);
          for(int i=1; i<faultyLines.size(); i++)
            {
              outReport <<"," <<faultyLines.at(i);
            }
          outReport <<linend <<linend;
        }

      //incorrect files
      outReport <<QString("Wrong crc files: %1").arg(wrongCrc.size()/4) <<linend;
      outReport <<QString("--------------------------------------------------------------------------------") <<linend;
      for(int i=0; i<wrongCrc.size(); i+=4)
        {
          outReport <<wrongCrc.at(i) <<"," <<wrongCrc.at(i+2) <<"," <<wrongCrc.at(i+1) <<"," <<wrongCrc.at(i+3) <<linend;
        }
      outReport <<linend;

      outReport <<QString("Wrong size files: %1").arg(wrongSize.size()/4) <<linend;
      outReport <<QString("--------------------------------------------------------------------------------") <<linend;
      for(int i=0; i<wrongSize.size(); i+=4)
        {
          outReport <<wrongSize.at(i) <<"," <<wrongSize.at(i+2) <<"," <<wrongSize.at(i+1) <<"," <<wrongSize.at(i+3) <<linend;
        }
      outReport <<linend;

      if(flags & KbvCheck::includePath)
        {
          outReport <<QString("Wrong path files: %1").arg(wrongPath.size()/2) <<linend;
          outReport <<QString("--------------------------------------------------------------------------------") <<linend;
          for(int i=0; i<wrongPath.size(); i+=2)
            {
              outReport <<wrongPath.at(i) <<"," <<wrongPath.at(i+1) <<linend;
            }
          outReport <<linend;
        }
      
      //missing files
      outReport <<QString("Missing files: %1").arg(missing.size()/3) <<linend;
      outReport <<QString("--------------------------------------------------------------------------------") <<linend;
      for(int i=0; i<missing.size(); i+=3)
        {
          outReport <<missing.at(i) <<"," <<missing.at(i+2) <<"," <<missing.at(i+1) <<linend;
        }
      outReport <<linend;

      //duplicates
      outReport <<QString("Duplicates: %1").arg(duplicates.size()/4) <<linend;
      outReport <<QString("--------------------------------------------------------------------------------") <<linend;
      for(int i=0; i<duplicates.size(); i+=4)
        {
          outReport <<duplicates.at(i) <<"," <<duplicates.at(i+2) <<"," <<duplicates.at(i+1) <<"," <<duplicates.at(i+3) <<linend;
        }
      outReport <<linend;

      outReport <<QString("Rename: %1").arg(rename.size()/4) <<linend;
      outReport <<QString("--------------------------------------------------------------------------------") <<linend;
      for(int i=0; i<rename.size(); i+=4)
        {
          outReport <<rename.at(i+3) <<" --> " <<rename.at(i) <<linend;
        }
      outReport <<linend;
      
      if(flags & KbvCheck::correctFilesInReport)
        {
          //correct files
          outReport <<QString("Correct files: %1").arg(found.size()/4) <<linend;
          outReport <<QString("--------------------------------------------------------------------------------") <<linend;
          for(int i=0; i<found.size(); i+=4)
            {
              outReport <<found.at(i) <<"," <<found.at(i+2) <<"," <<found.at(i+1) <<"," <<found.at(i+3) <<linend;
            }
          outReport <<linend;
        }

      reportFile.flush();
      reportFile.close();
    }
}

/*************************************************************************//*!
 * Parse the path in recursive way to fetch all sub directories. All found
 * files are collected in the stringlists collFileList (name & path) and
 * collCRCList (crc32, path/name).
 * On abort the recursion must return properly step by step. Hence do not
 * alter variable "abort" (this only can happen when the tab gets closed).
 */
void    KbvCsvCheckThread::parseDirectories(QString path, QMultiMap<QString, QString> &collFileList,
                                            QMultiMap<int, QString> &collCRCList)
{
  QDir          actualDir;
  QStringList   dirlist, filelist;
  QString       filepath;
  int           i, k;
  quint32       crc;
  
  actualDir.setPath(path);
  actualDir.setSorting(QDir::Name | QDir::LocaleAware);
  QDir::Filters filter = QDir::AllDirs | QDir::NoDotAndDotDot;
  actualDir.setFilter(filter);

  dirlist = actualDir.entryList();

  actualDir.setFilter(QDir::Files | QDir::NoSymLinks);
  filelist = actualDir.entryList();
  filepath = path + "/";
  for(i=0; i<filelist.size(); i++)
    {
      //qDebug() << "KbvCollectionThread::parseDirectories" <<  filelist.at(i); //###########
      collFileList.insert(filelist.at(i), filepath);
      crc = globalFunc.crc32FromFile(filepath + filelist.at(i));
      collCRCList.insert(crc, filepath + filelist.at(i));
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
      parseDirectories(filepath + dirlist[k], collFileList, collCRCList);
    }
}
/*************************************************************************//*!
 * Parse the path in recursive way to fetch all sub directories. All found
 * files are collected in the stringlists collFileList (name & path) and
 * collCRCList (crc32, path/name).
 * On abort the recursion must return properly step by step. Hence do not
 * alter variable "abort" (this only can happen when the tab gets closed).
 */
void    KbvCsvCheckThread::parseForCSV(QString path, QStringList &collFileList)
{
  QDir          actualDir;
  QStringList   dirlist, filelist;
  QString       filepath;
  int           i, k;
  
  actualDir.setPath(path);
  actualDir.setSorting(QDir::Name | QDir::LocaleAware);
  QDir::Filters filter = QDir::AllDirs | QDir::NoDotAndDotDot;
  actualDir.setFilter(filter);

  dirlist = actualDir.entryList();

  actualDir.setFilter(QDir::Files | QDir::NoSymLinks);
  filelist = actualDir.entryList();
  filepath = path + "/";
  for(i=0; i<filelist.size(); i++)
    {
      //qDebug() << "KbvCollectionThread::parseForCSV" <<  filelist.at(i); //###########
      collFileList.append(filelist.at(i));
      collFileList.append(filepath);
    }

  //recursion: parse sub dirs of this branch
  for(k=0; k<dirlist.length(); k++)
    {
      //on abort the recursion must return properly step by step
      if (abortThread)
        {
          return;
        }
      //qDebug() << "KbvCollectionThread::parseForCSV" << filepath << dirlist[k]; //###########
      parseForCSV(filepath + dirlist[k], collFileList);
    }
}
/*************************************************************************//*!
 * Split 'line' of csv-file into the components 'name', 'size', 'crc' and
 * 'path'. Return true when name is not empty, crc and size could be converted
 * faultless to integer and path is valid.
 * Don't care about comment.
 */
bool    KbvCsvCheckThread::splitCsvLine(QString &line, QString &name, int &crc, int &size, QString &path)
{
  QStringList   csvComponents;
  QString       str;
  bool          ok;
  
  //qDebug() << "KbvCsvCheckThread::splitCsvLine line:" <<line; //###########
  path = "/";
  if(line.isEmpty())
    {
      name.clear();
      return  false;
    }
  
  //remove single or multiple ',' at line end
  while(line.endsWith(","))
    {
      line.remove(line.size()-1, 1);
    }

  //split line and check for at least name, size, crc32
  csvComponents = line.split(",", QString::KeepEmptyParts);
  // < 3 components: name, size only
  if(csvComponents.size() < 3)
    {
      return false;
    }

  // >= 3 components: name, size, crc32, [path, comment]
  name = csvComponents.at(0).trimmed();
  str = csvComponents.at(1).trimmed();
  size = str.toUInt(&ok, 10);
  if(!ok)
    {
      return false;
    }

  str = csvComponents.at(2).trimmed();  //crc32 is padded to 8 characters
  if(str.length() == 8)
    {
      crc = str.toUInt(&ok, 16);
      if(!ok)
        {
          return  false;
        }
    }
  else
    {
      return false;
    }
  
  // > 3 components: name, size, crc, [path], [comment]
  //ignore comment
  if(csvComponents.size() > 3) 
    {
      //Check path and convert to the form: / ... / ... /
      path = csvComponents.at(3).trimmed();
      if(!path.isEmpty())
        {
          path.replace("\\", "/");
          if(!path.startsWith(QString("/")))    //looks like comment
            {
              path = "/"; //ignore comment, set path empty
            }
          if(!path.endsWith(QString("/")))
            {
              path.append(QString("/"));
            }
          return true;
        }
      else
        {
          path = "/";
          return true;
        }
    }
  //3 components in csv line and all ok, path is empty: "/"
  return true;
}
/*****************************************************************************/
