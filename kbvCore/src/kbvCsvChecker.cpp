/*****************************************************************************
 * kbvCsvChecker
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2014.11.15
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvSetvalues.h"
#include "kbvGeneral.h"
#include "kbvInformationDialog.h"
#include "kbvCsvCheckThread.h"
#include "kbvCsvChecker.h"

extern  KbvSetvalues          *settings;
extern  KbvInformationDialog  *informationDialog;

KbvCsvChecker::KbvCsvChecker(QWidget *parent, KbvDBInfo *databaseInfo) : QDialog(parent)
{
  dbInfo = databaseInfo;
  dbName = databaseInfo->getName();
  collRootDir = databaseInfo->getRootDir();


  informationDialog = new KbvInformationDialog(this);
  infoCaption = QString(tr("Same process is still running"));
  infoStr = QString(tr("Please wait until all processes have finished."));
  
  ui.setupUi(this);
  ui.csvThreadLbl1->setVisible(false);
  ui.csvThreadLbl2->setVisible(false);
  ui.csvProgressBar->setVisible(false);

  QStringList headerLabels;
  headerLabels.append(QString(tr("CSV file")));
  headerLabels.append(QString(tr("Collection directory")));
  ui.csvFileTable->setHorizontalHeaderLabels(headerLabels);
  ui.csvFileTable->horizontalHeader()->setVisible(true);
  ui.csvFileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui.csvFileTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui.csvFileTable->setTextElideMode(Qt::ElideLeft);
  ui.csvFileTable->setMouseTracking(true);
  
  ui.csvConfCheckPaths->setEnabled(true);         //Check for correct paths
  ui.csvConfRenameByCSV->setEnabled(true);        //Rename files due to CSV file
  ui.csvConfCorrectFileReport->setEnabled(true);  //Add correct files to report
  ui.csvConfLineEndLinux->setEnabled(true);       //Use linux line termination in reports and csv
  ui.csvConfRenameByCSV->setToolTip(QString(tr("Rename files due to the names found in CSV file\n"
                                            "when crc32 and size match.\n"
                                            "Please update the database after this action.")));
  
  ui.csvCollLbl1->setToolTip(QString(tr("Create an ECSV file containing name, size, crc and path for all\n"
                                        "files found in the selected source directory and its sub dirs.\n"
                                        "The path is relative to the source directory.\n"
                                        "UNIX/Linux: directory separator is '/' and file coding is UTF-8.\n"
                                        "Windows: directory separator is '\\' and file coding is ISO-8859-15.")));
  ui.csvCollLbl2->setToolTip(QString(tr("Move files from download dir to the collection paths due to ECSV file.\n"
                                        "When a path not yet exists the path will be created. Existing files\n"
                                        "will be overwitten. Remaining files in download dir indicate problems\n"
                                        "with the file itself or corrupt CSV).")));
  ui.csvCollLbl3->setToolTip(QString(tr("Search files due to the names found in selected CSV file and copy them\n"
                                        "to the upload directory when crc32 and size match.")));
  
  this->setWindowTitle(QString("Imarca checker") + " - " + dbName);
  this->move(100,100);
  
  this->readSettings();
  if(ui.csvFileTable->rowCount() > 0)
    {
      ui.csvFileTable->selectRow(0);
      this->fileItemChanged();            //set check results
    }
  
  QDir  dir;
  homedir = dir.homePath();
  
  connect(ui.csvCloseButton,  SIGNAL(clicked(bool)),    this, SLOT(close(bool)));
  connect(ui.csvCheckButton1, SIGNAL(clicked(bool)),    this, SLOT(checkCollection(bool)));
  connect(ui.csvCheckButton2, SIGNAL(clicked(bool)),    this, SLOT(openReport(bool)));
  connect(ui.csvCheckButton3, SIGNAL(clicked(bool)),    this, SLOT(openCSV(bool)));
  connect(ui.csvFileButton1,  SIGNAL(clicked(bool)),    this, SLOT(addCsvFile(bool)));
  connect(ui.csvFileButton2,  SIGNAL(clicked(bool)),    this, SLOT(removeCsvFile(bool)));
  connect(ui.csvDirButton1,   SIGNAL(clicked(bool)),    this, SLOT(setDownloadDir(bool)));
  connect(ui.csvDirButton2,   SIGNAL(clicked(bool)),    this, SLOT(setUploadDir(bool)));
  connect(ui.csvFileTable,    SIGNAL(itemSelectionChanged()),         this, SLOT(fileItemChanged()));
  connect(ui.csvFileTable,    SIGNAL(itemEntered(QTableWidgetItem*)), this, SLOT(fileTableToolTip(QTableWidgetItem*)));
  
  connect(ui.csvCollButton1,  SIGNAL(clicked(bool)),      this, SLOT(createCsvFile(bool)));
  connect(ui.csvCollButton2,  SIGNAL(clicked(bool)),      this, SLOT(moveToCollection(bool)));
  connect(ui.csvCollButton3,  SIGNAL(clicked(bool)),      this, SLOT(copyToUploadDir(bool)));
  connect(ui.csvTabs,         SIGNAL(currentChanged(int)),this, SLOT(currentTab(int)));
}
KbvCsvChecker::~KbvCsvChecker()
{
  //qDebug() << "KbvCsvChecker::~KbvCsvChecker";//###########
}
/*************************************************************************//*!
 * SLOT: the selected CSV file changed.
 * Load last check result of this file when there are sufficient entries.
 * When multiple files are selected undefined results will be displayed.
 */
void  KbvCsvChecker::fileItemChanged()
{
  int     n, row;
  
  row = ui.csvFileTable->currentRow();
  if(ui.csvFileTable->selectionModel()->selectedRows(0).count() > 1)
    {
      ui.csvDateLbl2->setText("---"); //last check date
      ui.csvNumLbl01->setText("0");   //totalFiles
      ui.csvNumLbl02->setText("0");   //correctFiles
      ui.csvNumLbl03->setText("0");   //incorrectFiles
      ui.csvNumLbl04->setText("0");   //missingFiles
      ui.csvNumLbl05->setText("0");   //renameFiles
      ui.csvNumLbl06->setText("0");   //duplicates
      ui.csvNumLbl07->setText("0");   //unknownFiles
    }
  else
    {
      n = KbvCheck::resultsize * row;
      if(resultlist.size() >= n)
        {
          ui.csvDateLbl2->setText(resultlist.at(n+0)); //last check date
          ui.csvNumLbl01->setText(resultlist.at(n+1)); //totalFiles
          ui.csvNumLbl02->setText(resultlist.at(n+2)); //correctFiles
          ui.csvNumLbl03->setText(resultlist.at(n+3)); //incorrectFiles
          ui.csvNumLbl04->setText(resultlist.at(n+4)); //missingFiles
          ui.csvNumLbl05->setText(resultlist.at(n+5)); //renameFiles
          ui.csvNumLbl06->setText(resultlist.at(n+6)); //duplicates
          ui.csvNumLbl07->setText(resultlist.at(n+7)); //unknownFiles
        }
    }
}
/*************************************************************************//*!
 * SLOT: check collection using selected CSV files.
 * Each csv file will be processed in it's own thread. The threads and check
 * results are identified by the row number (see threadList). As long as at
 * least one thread is running the add/remove buttons will be locked.
 * The threadList contains the row numbers of all started threads and is
 * used in saveCheckResults().
 * Note: each thread created below will delete itself when finished.
 * Produced results appear always in choosen style unix/windows.
 */
void  KbvCsvChecker::checkCollection(bool checked)
{
  Q_UNUSED(checked)
  QModelIndexList   idxList0, idxList1;
  QString           csvFile, checkDir, tradeDir;
  int               count, row, flags;
  
  tradeDir = QString(""); //upload or download directory when required
  
  flags = KbvCheck::checkCollection;
  if(ui.csvConfRenameByCSV->isChecked())
    {
      //rename due to csv-file
      flags = flags | KbvCheck::rename;
    }
  if(ui.csvConfCorrectFileReport->isChecked())
    {
      //include correct files in report
      flags = flags | KbvCheck::correctFilesInReport;
    }
  if(ui.csvConfLineEndLinux->isChecked())
    {
      //create ECSV file
      flags = flags | KbvCheck::ecsvLinux;
    }
  if(ui.csvConfCheckPaths->isChecked())
    {
      //check paths of ecsv-file
      flags = flags | KbvCheck::includePath;
    }

  idxList0 = ui.csvFileTable->selectionModel()->selectedRows(0);  //selected csv-files
  idxList1 = ui.csvFileTable->selectionModel()->selectedRows(1);  //selected check dirs
  count = idxList0.size();
  if(count > 0)
    {
      ui.csvThreadLbl1->setVisible(true);
      ui.csvThreadLbl2->setVisible(true);
      ui.csvFileButton1->setEnabled(false);
      ui.csvFileButton2->setEnabled(false);

      //Get selected row numbers as thread ids, limit number of threads
      //threadList.clear();
      for(int k=0; k<count; k++)
        {
          if(threadList.size() < KbvCheck::threadMaxId)
            {
              row = idxList0.at(k).row();
              csvFile  = idxList0.at(k).data(Qt::DisplayRole).toString();
              checkDir = idxList1.at(k).data(Qt::DisplayRole).toString();
          
              //create thread only when not present
              if(!csvFile.isEmpty() && !checkDir.isEmpty() && !threadList.contains(row))
                {
                  checkThread = new KbvCsvCheckThread(nullptr, dbInfo);
                  threadList.insert(row, checkThread);
                  checkThread->start(row, csvFile, checkDir, tradeDir, this, flags);
                  //qDebug() << "KbvCsvChecker::checkCollection "<<row <<checkThread <<csvFile <<checkDir <<tradeDir;//###########
                }
            }
           else
             {
               informationDialog->perform(QString(tr("Max. number of threads reached.")), QString(), 0);
               break; //leave for-loop
             }
        }
      ui.csvThreadLbl1->setNum(threadList.size());
      ui.csvProgressBar->setVisible(true);
    }
}
/*************************************************************************//*!
 * SLOT: collect check results. Called by a finishing thread.
 * The thread is identified by an id which is identical to the row number
 * of csv file list (user interface).
 * When all threads have finished, the add/remove buttons get enabled.
 * Note: each thread will delete itself when finished.
 */
void  KbvCsvChecker::saveCheckResults(int threadId, int total, int correct, int incorrect,
                                      int missing, int rename, int duplicates, int unknown)
{
  QString   str1, str2;
  int       pos;
  
  //lock so only one thread can write at a time
  mutex.lock();
  QDateTime dt = QDateTime::currentDateTime();
  str1 = dt.toString("yyyy.MM.dd - hh:mm");
  str2 = QString("%L1");

  //generate results for normal thread ids
  if(threadId < KbvCheck::threadMaxId)
    {
      pos = KbvCheck::resultsize * threadId;
      resultlist.replace(pos+0, str1);
      resultlist.replace(pos+1, str2.arg(total));
      resultlist.replace(pos+2, str2.arg(correct));
      resultlist.replace(pos+3, str2.arg(incorrect));
      resultlist.replace(pos+4, str2.arg(missing));
      resultlist.replace(pos+5, str2.arg(rename));
      resultlist.replace(pos+6, str2.arg(duplicates));
      resultlist.replace(pos+7, str2.arg(unknown));
      fileItemChanged();
      //qDebug() << "KbvCsvChecker::saveCheckResults save:" <<threadId;//###########
    }

  threadList.remove(threadId);
  ui.csvThreadLbl1->setNum(threadList.size());

  if(threadList.isEmpty())
    {
      ui.csvFileButton1->setEnabled(true);
      ui.csvFileButton2->setEnabled(true);
      ui.csvThreadLbl1->setVisible(false);
      ui.csvThreadLbl2->setVisible(false);
      ui.csvProgressBar->setVisible(false);
      //qDebug() << "KbvCsvChecker::saveCheckResults end" <<threadId;//###########
      this->update();
    }
  mutex.unlock();
}
/*************************************************************************//*!
 * SLOT: move files from download dir to collection due to ECSV-file.
 * When other threads are running an information window will be shown and
 * no thread get started (since this could produce a thread id which is
 * already in use and we cannot properly cleanup the threadlist).
 * The parent dir is the collection root. All ECSV paths are relative to this
 * collection root and will be created when not alredy existent. Only files
 * with matching crc32 and size get moved.
 * Note: each thread created below will delete itself when finished.
 */
void  KbvCsvChecker::moveToCollection(bool checked)
{
  Q_UNUSED(checked)
  QString           caption, csvFile, tradeDir;
  int               threadId, flags;

  //only, when no other 'move' thread is running
  if(!threadList.contains(KbvCheck::threadMoveColl))
    {
      caption = QString(tr("Move to collection: select CSV file"));
      csvFile = QFileDialog::getOpenFileName(this, caption, collRootDir, "*.csv *.ecsv;; *");
      if(!csvFile.isEmpty())
        {
          threadId = KbvCheck::threadMoveColl;
          flags = KbvCheck::moveToCollection;
          tradeDir = ui.csvDirLineEdit1->text();  //download directory
          if(!tradeDir.isEmpty())
            {
              //qDebug() << "KbvCsvChecker::moveToCollection"<<threadId <<csvFile <<collRootDir <<tradeDir;//###########
              checkThread = new KbvCsvCheckThread(nullptr, dbInfo);
              checkThread->start(threadId, csvFile, collRootDir, tradeDir, this, flags);
              threadList.insert(threadId, checkThread);
              
              ui.csvProgressBar->setVisible(true);
              ui.csvThreadLbl1->setNum(threadList.size());
              ui.csvThreadLbl1->setVisible(true);
              ui.csvThreadLbl2->setVisible(true);
              ui.csvFileButton1->setEnabled(false);
              ui.csvFileButton2->setEnabled(false);
            }
        }
    }
  else
    {
      informationDialog->perform(infoCaption, infoStr, 1);
    }
}
/*************************************************************************//*!
 * SLOT: search files due to selected csv file and copy them to upload dir.
 * Paths in the csv file are ignored. Files get copied when crc32 and size
 * match.
 * When other threads are running an information window will be shown and
 * no thread get started (since this could produce a thread id which is
 * already in use and we cannot properly cleanup the threadlist).
 * Note: each thread created below will delete itself when finished.
 */
void  KbvCsvChecker::copyToUploadDir(bool checked)
{
  Q_UNUSED(checked)
  QString           caption, csvFile, tradeDir;
  int               threadId, flags;
  
  //only, when no other 'upload' thread is running
  if(!threadList.contains(KbvCheck::threadCopyUpload))
    {
      caption = QString(tr("Copy to upload dir: select CSV file"));
      csvFile = QFileDialog::getOpenFileName(this, caption, collRootDir, "*.csv *.ecsv;; *");
      if(!csvFile.isEmpty())
        {
          if(!collRootDir.isEmpty())
            {
              threadId = KbvCheck::threadCopyUpload;
              flags = KbvCheck::copyToUploadDir;
              tradeDir = ui.csvDirLineEdit2->text();  //upload directory
              if(!tradeDir.isEmpty())
                {
                  //qDebug() << "KbvCsvChecker::copyToUploadDir"<<threadId <<csvFile <<checkDir <<tradeDir;//###########
                  checkThread = new KbvCsvCheckThread(nullptr, dbInfo);
                  checkThread->start(threadId, csvFile, collRootDir, tradeDir, this, flags);
                  threadList.insert(threadId, checkThread);
                  
                  ui.csvProgressBar->setVisible(true);
                  ui.csvThreadLbl1->setNum(threadList.size());
                  ui.csvThreadLbl1->setVisible(true);
                  ui.csvThreadLbl2->setVisible(true);
                  ui.csvFileButton1->setEnabled(false);
                  ui.csvFileButton2->setEnabled(false);
                }
            }
        }
    }
  else
    {
      informationDialog->perform(infoCaption, infoStr, 1);
    }
}
/*************************************************************************//*!
 * SLOT: create a ECSV file for the collection or a branch. The paths in the
 * ECSV file are relative to the selected root dir.
 * When other threads are running an information window will be shown and
 * no thread get started (since this could produce a thread id which is
 * already in use and we cannot properly cleanup the threadlist).
 * Note: each thread created below will delete itself when finished.
 */
void  KbvCsvChecker::createCsvFile(bool checked)
{
  Q_UNUSED(checked)
  QString           caption, checkDir, targetDir;
  int               threadId, flags;

  //only, when no other 'create CSV' thread is running
  if(!threadList.contains(KbvCheck::threadCreateCSV))
    {
      caption = QString(tr("Create CSV file: select source directory"));
      checkDir = QFileDialog::getExistingDirectory(this, caption, collRootDir,
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
      if(!checkDir.isEmpty())
         {
          caption = QString(tr("Create CSV file: select target dir for CSV file"));
          targetDir = QFileDialog::getExistingDirectory(this, caption, collRootDir,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
          if(!targetDir.isEmpty())
            {
              flags = KbvCheck::createCsvFile;
              if(ui.csvConfLineEndLinux->isChecked())
                {
                  flags = flags | KbvCheck::ecsvLinux;
                }
              threadId = KbvCheck::threadCreateCSV;

              //qDebug() << "KbvCsvChecker::createCsvFile"<<threadId <<checkDir <<targetDir;//###########
              checkThread = new KbvCsvCheckThread(nullptr, dbInfo);
              checkThread->start(threadId, "", checkDir, targetDir, this, flags);
              threadList.insert(threadId, checkThread);
              
              ui.csvProgressBar->setVisible(true);
              ui.csvThreadLbl1->setNum(threadList.size());
              ui.csvThreadLbl1->setVisible(true);
              ui.csvThreadLbl2->setVisible(true);
              ui.csvFileButton1->setEnabled(false);
              ui.csvFileButton2->setEnabled(false);
            }
        }
    }
  else
    {
      informationDialog->perform(infoCaption, infoStr, 1);
    }
}

/*************************************************************************//*!
 * SLOT: set the directory where collection files have been downloaded.
 */
void  KbvCsvChecker::setDownloadDir(bool checked)
{
  Q_UNUSED(checked)
  QString     dlDir, caption;
  
  caption = QString(tr("Select download directory"));
  dlDir = QFileDialog::getExistingDirectory(this, caption, homedir,
                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  
  if(!dlDir.isEmpty())
    {
      if(!dlDir.endsWith("/"))
        {
          dlDir.append("/");
        }
      ui.csvDirLineEdit1->setText(dlDir);
      this->writeSettings();
    }
}
/*************************************************************************//*!
 * SLOT: set the directory for uploading collection files.
 */
void  KbvCsvChecker::setUploadDir(bool checked)
{
  Q_UNUSED(checked)
  QString     ulDir, caption;

  caption = QString(tr("Select upload directory"));
  ulDir = QFileDialog::getExistingDirectory(this, caption, homedir,
                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  
  if(!ulDir.isEmpty())
    {
      if(!ulDir.endsWith("/"))
        {
          ulDir.append("/");
        }
      ui.csvDirLineEdit2->setText(ulDir);
      this->writeSettings();
    }
}
/*************************************************************************//*!
 * SLOT: append a row and set a csv file at column 0 then set a directory
 * at column 1 of the table widget.
 */
void  KbvCsvChecker::addCsvFile(bool checked)
{
  Q_UNUSED(checked)
  QTableWidgetItem  *item;
  QString           file, caption;
  int               row;

  caption = QString(tr("Select CSV file"));
  file = QFileDialog::getOpenFileName(this, caption, collRootDir, "*.csv *.ecsv ;;*");
  if(!file.isEmpty())
    {
      //append row and set csv file to column 0
      row = ui.csvFileTable->rowCount();
      ui.csvFileTable->insertRow(row);
      item = new QTableWidgetItem(file);
      ui.csvFileTable->setItem(row, 0, item);
      
      caption = QString(tr("Select collection directory"));
      file = QFileDialog::getExistingDirectory(this, caption, collRootDir,
                                               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
      if(!file.isEmpty())
        {
          item = new QTableWidgetItem(file);
          ui.csvFileTable->setItem(row, 1, item);
        }

      //append empty check results for this row
      for(int i=0; i<KbvCheck::resultsize; i++)
        {
          resultlist.append("0");
        }
      this->writeSettings();
    }
}
/*************************************************************************//*!
 * SLOT: remove selected row in the table widget (csv-file & collection dir).
 */
void  KbvCsvChecker::removeCsvFile(bool checked)
{
  Q_UNUSED(checked)
  QModelIndexList   idxList;
  int               count, row;
  
  idxList = ui.csvFileTable->selectionModel()->selectedRows(0);
  count = idxList.size();
  //qDebug() << "KbvCsvChecker::removeCsvFile count"<<count;//###########
  if(count > 0)
    {
      //Remove from last to first to keep the index list valid
      for(int k=count-1; k>=0; k--)
        {
          row = idxList.at(k).row();

          //remove row and related result values
          //block signals otherwise the app crashes on removing the last row!
          ui.csvFileTable->blockSignals(true);
          ui.csvFileTable->removeRow(row);
          ui.csvFileTable->blockSignals(false);
          for(int i=0; i<KbvCheck::resultsize; i++)
            {
              resultlist.removeAt(KbvCheck::resultsize*row); //all items move to this position!!
            } 
        }
    }
  this->writeSettings();  
}
/*************************************************************************//*!
 * SLOT: open check reports of selected csv files.
 */
void  KbvCsvChecker::openReport(bool checked)
{
  Q_UNUSED(checked)
  KbvGeneral        gerneralFunc;
  QFileInfo         info;
  QModelIndexList   idxList;
  QStringList       paramsList;
  QString           path;
  int               n;
  
  idxList = ui.csvFileTable->selectionModel()->selectedRows(0);
  for(int k=0; k<idxList.size(); k++)
    {
      path = idxList.at(k).data(Qt::DisplayRole).toString();
      n = path.lastIndexOf(".");
      if(n<0)
        {
          path = path.append("_report.txt");
        }
      else
        {
          path = path.left(n);
          path = path.append("_report.txt");
        }
      info.setFile(path);
      //qDebug() << "KbvCsvChecker::openReport file"<<path;//###########
      
      if(info.exists())
        {
          paramsList.append(path);
          generalFunc.openApplication("text/plain", paramsList);
        }
    }
}
/*************************************************************************//*!
 * SLOT: open source CSV files of selected csv files.
 */
void  KbvCsvChecker::openCSV(bool checked)
{
  Q_UNUSED(checked)
  QFileInfo         info;
  QModelIndexList   idxList;
  QStringList       paramsList;
  QString           path;
  int               n;
  
  idxList = ui.csvFileTable->selectionModel()->selectedRows(0);
  for(int k=0; k<idxList.size(); k++)
    {
      path = idxList.at(k).data(Qt::DisplayRole).toString();
      n = path.lastIndexOf(".");
      if(n<0)
        {
          path = path.append(".csv");
        }
      info.setFile(path);
      //qDebug() << "KbvCsvChecker::openCSV file"<<path;//###########
      
      if(info.exists())
        {
          paramsList.append(path);
          generalFunc.openApplication("text/plain", paramsList);
        }
    }
}
/*************************************************************************//*!
 * SLOT: close button pressed. Triggers closeEvent()
 */
void  KbvCsvChecker::close(bool checked)
{
  Q_UNUSED(checked)
  QDialog::close();
}

/*************************************************************************//*!
 * Close window: stop working threads, save settings and quit checker.
 * Note: The view gets frozen hence we cannot display any progress.
 */
void  KbvCsvChecker::closeEvent(QCloseEvent *event)
{
  KbvCsvCheckThread*  thread;
  QCursor             cursor;

  //qDebug() << "KbvCsvChecker::closeEvent";//###########
  //Close running threads. Thread deletes itself with delay
  if(threadList.size() > 0)
    {
      cursor = this->cursor();
      this->setCursor(Qt::WaitCursor);
      
      QMap<int, KbvCsvCheckThread*>::const_iterator iter = threadList.constBegin();
      while (iter != threadList.constEnd())
        {
          thread = iter.value();
          thread->stop();
          iter++;
        }
      this->setCursor(cursor);
    }
  threadList.clear();
  ui.csvThreadLbl1->setNum(0);
  ui.csvFileButton1->setEnabled(true);
  ui.csvFileButton2->setEnabled(true);
  ui.csvThreadLbl1->setVisible(false);
  ui.csvThreadLbl2->setVisible(false);
  ui.csvProgressBar->setVisible(false);
  this->writeSettings();
  event->accept();
}
/*************************************************************************//*!
 * When the tab "check" (=0) is visible we adjust the two columns of the table
 * widget to the same size.
 */
void  KbvCsvChecker::currentTab(int index)
{
  QSize size;

  if(index == 0)
    {
      size = ui.csvFileTable->size();
      ui.csvFileTable->setColumnWidth(0, size.width()/2);
    }
}
/*************************************************************************//*!
 * When the dialog receives a resize event we adjust the two columns of the
 * table widget to the same size.
 */
void  KbvCsvChecker::resizeEvent(QResizeEvent *event)
{
  QSize size;

  size = ui.csvFileTable->size();
  ui.csvFileTable->setColumnWidth(0, size.width()/2);

  //pass to base class
  QDialog::resizeEvent(event);
}

/*************************************************************************//*!
 * Save check parameters and results. The section starts with group [dbname].
 * Immediately below the mark both the download and upload dirs are written.
 * The following array 'csvCheck' contains all data for each csv-file.
 */
void  KbvCsvChecker::writeSettings()
{
  QString           text;
  int               count;
  
  settings->beginGroup("CsvChecker");
  //clear existing entries of child group
  settings->remove(dbName);

  settings->beginGroup(dbName);
  settings->setValue("optDownloadDir", QVariant(ui.csvDirLineEdit1->text()));
  settings->setValue("optUploadDir",   QVariant(ui.csvDirLineEdit2->text()));

  settings->setValue("optCheckPath",   QVariant(ui.csvConfCheckPaths->isChecked()));
  settings->setValue("optRename",      QVariant(ui.csvConfRenameByCSV->isChecked()));
  settings->setValue("optCorrReport",  QVariant(ui.csvConfCorrectFileReport->isChecked()));
  settings->setValue("optLineEnd",     QVariant(ui.csvConfLineEndLinux->isChecked()));

  //Save the list of csv files and their check results as array
  settings->beginWriteArray("csvCheck");
  count = ui.csvFileTable->rowCount();
  for(int k=0; k<count; k++)
    {
      settings->setArrayIndex(k);
      
      text = ui.csvFileTable->item(k, 0)->text();
      settings->setValue("csvFile",        QVariant(text));
      text = ui.csvFileTable->item(k, 1)->text();
      settings->setValue("checkPath",      QVariant(text));

      settings->setValue("lastDate",       QVariant(resultlist.at(k*KbvCheck::resultsize + 0)));
      settings->setValue("totalFiles",     QVariant(resultlist.at(k*KbvCheck::resultsize + 1)));
      settings->setValue("correctFiles",   QVariant(resultlist.at(k*KbvCheck::resultsize + 2)));
      settings->setValue("incorrectFiles", QVariant(resultlist.at(k*KbvCheck::resultsize + 3)));
      settings->setValue("missingFiles",   QVariant(resultlist.at(k*KbvCheck::resultsize + 4)));
      settings->setValue("renameFiles",    QVariant(resultlist.at(k*KbvCheck::resultsize + 5)));
      settings->setValue("duplicates",     QVariant(resultlist.at(k*KbvCheck::resultsize + 6)));
      settings->setValue("unknownFiles",   QVariant(resultlist.at(k*KbvCheck::resultsize + 7)));
    }
  settings->endArray();
    
  settings->endGroup();
  settings->endGroup(); //group csvChecker
  settings->sync();
}
/*************************************************************************//*!
 * Read check parameters and results. The section starts with group [dbname].
 * Immediately below the mark both the download and upload dirs are read.
 * The following array 'csvCheck' contains all data for each csv-file.
 */
void  KbvCsvChecker::readSettings()
{
  QTableWidgetItem  *item;
  QString           text;
  int               count;
  //qDebug() << "KbvCsvChecker::readSettings";//###########
  
  settings->beginGroup("CsvChecker");
  settings->beginGroup(dbName);
  ui.csvDirLineEdit1->setText(settings->value("optDownloadDir", QVariant("")).toString());
  ui.csvDirLineEdit2->setText(settings->value("optUploadDir",   QVariant("")).toString());

  ui.csvConfCheckPaths->setChecked(settings->value("optCheckPath",  QVariant("")).toBool());
  ui.csvConfRenameByCSV->setChecked(settings->value("optRename",     QVariant("")).toBool());
  ui.csvConfCorrectFileReport->setChecked(settings->value("optCorrReport", QVariant("")).toBool());
  ui.csvConfLineEndLinux->setChecked(settings->value("optLineEnd", QVariant("")).toBool());

  //Get csv-files and check results for this database
  resultlist.clear();
  count = settings->beginReadArray("csvCheck");
  for(int k=0; k<count; k++)
    {
      //qDebug() << "KbvCsvChecker::readSettings array part"<<k;//###########
      settings->setArrayIndex(k);
      
      ui.csvFileTable->insertRow(k);
      text = settings->value("csvFile", QVariant("")).toString();
      item = new QTableWidgetItem(text);
      ui.csvFileTable->setItem(k, 0, item);
      text = settings->value("checkPath", QVariant("")).toString();
      item = new QTableWidgetItem(text);
      ui.csvFileTable->setItem(k, 1, item);
      
      resultlist.append(settings->value("lastDate", QVariant("0")).toString());
      resultlist.append(settings->value("totalFiles", QVariant("0")).toString());
      resultlist.append(settings->value("correctFiles", QVariant("0")).toString());
      resultlist.append(settings->value("incorrectFiles", QVariant("0")).toString());
      resultlist.append(settings->value("missingFiles", QVariant("0")).toString());
      resultlist.append(settings->value("renameFiles", QVariant("0")).toString());
      resultlist.append(settings->value("duplicates", QVariant("0")).toString());
      resultlist.append(settings->value("unknownFiles", QVariant("0")).toString());
    }
  settings->endArray();
  settings->endGroup();
  settings->endGroup(); //group csvChecker
}
/*************************************************************************//*!
 * SLOT: The tooltip displays the full content of the cell under the mouse
 * cursor.
 */
void  KbvCsvChecker::fileTableToolTip(QTableWidgetItem *item)
{
  QString   tooltip;
  
  tooltip = item->data(Qt::DisplayRole).toString();
  if(item->column() == 0)
    {
      tooltip.append(QString(tr("\nCSV files must be coded in UTF8!")));
    }
  ui.csvFileTable->setToolTip(tooltip);
}

/****************************************************************************/
