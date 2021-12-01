/*****************************************************************************
 * kbvCsvChecker
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2014.11.15
 ****************************************************************************/
#ifndef KBVCSVCHECKER_H_
#define KBVCSVCHECKER_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvGeneral.h"
#include "kbvDBInfo.h"
#include "ui_kbvcvscheck.h"
#include "kbvCsvCheckThread.h"

class Ui_kbvCsvCheckClass;
class KbvCsvCheckThread;

class KbvCsvChecker : public QDialog
{
  Q_OBJECT
  
public:
  explicit KbvCsvChecker(QWidget *parent=nullptr, KbvDBInfo *databaseInfo=nullptr);

  virtual
  ~KbvCsvChecker();
  
signals:
  
public slots:
  void    saveCheckResults(int threadId, int total, int correct, int incorrect,
                           int missing, int rename, int duplicates, int unknown);

private slots:
  void    checkCollection(bool checked);
  void    setDownloadDir(bool checked);
  void    setUploadDir(bool checked);
  void    addCsvFile(bool checked);
  void    removeCsvFile(bool checked);
  void    openReport(bool checked);
  void    openCSV(bool checked);
  void    fileItemChanged();
  void    moveToCollection(bool checked);
  void    copyToUploadDir(bool checked);
  void    createCsvFile(bool checked);
  void    fileTableToolTip(QTableWidgetItem *item);
  void    currentTab(int index);
  void    close(bool checked);

private:
  void    readSettings();
  void    writeSettings();
  void    closeEvent(QCloseEvent *event);
  void    resizeEvent(QResizeEvent *event);

  Ui::kbvCsvCheckClass  ui;
  KbvGeneral            generalFunc;
  KbvDBInfo             *dbInfo;
  KbvCsvCheckThread     *checkThread;
  QStringList           resultlist;
  QString               dbName, collRootDir, homedir, infoCaption, infoStr;
  QMap<int, KbvCsvCheckThread*> threadList;
  QMutex                mutex;
};
#endif // KBVCSVCHECKER_H_
/****************************************************************************/
