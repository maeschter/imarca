/*****************************************************************************
 * kvbCsvCheckThread
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2014.11.21
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#ifndef KBVCSVCHECKTHREAD_H_
#define KBVCSVCHECKTHREAD_H_
#include <QtCore>
#include <QtSql>
#include "kbvGlobal.h"
#include "kbvCsvChecker.h"
#include "kbvDBInfo.h"

class KbvCsvChecker;

class KbvCsvCheckThread : public QThread
{
  Q_OBJECT
  
public:
  explicit KbvCsvCheckThread(QObject *parent=nullptr, KbvDBInfo *databaseInfo=nullptr);
  virtual  ~KbvCsvCheckThread();
  
  void    start(int id, QString csvFilePath, QString collectionPath, QString exchangeDir, KbvCsvChecker *checker, int flags);
  bool    stop();
  void    run();

private slots:
  void    showFilesWithIssues();

private:
  void    checkDirectories();
  void    copyToUploadDir();
  void    moveToCollection();
  void    createCsvFile();
  void    parseDirectories(QString path, QMultiMap<QString, QString> &fileList, QMultiMap<int, QString> &crcList);
  void    parseForCSV(QString path, QStringList &fileList);
  bool    splitCsvLine(QString &line, QString &name, int &crc, int &size, QString &path);
  void    createReport(const QStringList &found, const QStringList &wrongCrc, const QStringList &wrongSize, const QStringList &wrongPath,
                       const QStringList &missing, const QStringList &duplicates, const QStringList &rename,
                       const QStringList &faultyLines, const QString &checktime);
  void    createWrongMiss(const QStringList &wrongCrc, const QStringList &wrongSize, const QStringList &missing);
  void    renameFiles(QStringList &found, QStringList &rename);
  void    extractDuplicates(QStringList &found, QStringList &duplicates);
  bool    inList(const QStringList &duplicates, const QString &name, const QString &path);
  
  KbvGlobal       globalFunc;
  KbvCsvChecker   *csvChecker;
  QSqlDatabase    db;
  QString         dbName, dbLocation, dbconnection, collRootDir;
  int             dbType, dbIconSize, dbKeywordType;
  QString         csvPath, collPath, tradeDir, filesWithIssues;
  int             id, flags;
  bool            abortThread;
};

#endif // KBVCSVCHECKTHREAD_H_
/****************************************************************************/
