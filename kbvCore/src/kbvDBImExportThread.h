/*****************************************************************************
 * kvb database import export thread
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2017.01.30
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#ifndef KBVDBIMEXPORTTHREAD_H_
#define KBVDBIMEXPORTTHREAD_H_
#include <QObject>
#include <QtCore>
#include <QtSql>

class KbvDBImExportThread : public QThread
{
  Q_OBJECT

public:
KbvDBImExportThread(QObject *parent=nullptr);
virtual ~KbvDBImExportThread();

public slots:
void  start(const QString &name, const QString &location, const QString &rootDir, const QString &targetDir, int task);
bool  stop(void);

signals:
void  files(int num);
void  size(quint64 size);
void  progress(int prog);
void  finished(int flag);
void  setDB(QString name);

private:
void  run();
void  dbExport();
void  dbImport();
void  createReport(const QStringList filelist, int task);

QSqlDatabase    db;
QString         dbName, dbLocation, dbconnection, targetDir, collRoot;
int             task;
bool            abort;

};

#endif // KBVDBIMEXPORTTHREAD_H
/*****************************************************************************/
