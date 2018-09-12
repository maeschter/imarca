/*****************************************************************************
 * kvbCollectionThread
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-02-22 18:20:30 +0100 (Mi, 22. Feb 2017) $
 * $Rev: 1156 $
 * Created: 2011.11.15
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#ifndef KBVCOLLECTIONTHREAD_H_
#define KBVCOLLECTIONTHREAD_H_
#include <QtCore>
#include <QtSql>
#include "kbvGeneral.h"
#include "kbvGlobal.h"
#include "kbvConstants.h"
#include "kbvDBInfo.h"

class KbvCollectionThread : public QThread
{
  Q_OBJECT

public:
  KbvCollectionThread(QObject *parent=nullptr, KbvDBInfo *databaseInfo=nullptr);
  virtual ~KbvCollectionThread();

struct  Results
{
  int    r1;
  int    r2;
  qint64 r3;
  int    r4;
};

void    start(QStringList &strlist, int func);
bool    stop();
void    run();
void    visible();

public slots:
void    getUserInput(int result);

signals:
void    warning(const QString s1, bool m);
void    askForUserInput(const QString text1, const QString text2);
void    newItems(Kbv::kbvItemList list);
void    statusText1(QString text);
void    statusText2(QString text);
void    statusText3(QString text);
void    endOfReading();
void    endOfUpdate(QString text);

private:
void    readOut();
void    readIn();
void    remove();
void    replace();
void    rename();
void    readInCollection(QString path, Results &res);
void    parseDirectories(QString path, QList<QPair<QString, QString> > &files, Results &res);
int     removeOrphaned(const QString path, QString &report);
int     addNew(QString dir, QString &report, Results &res);
int     removeMultiple(const QString path, QString &report);

KbvGeneral      generalFunc;
KbvGlobal       globalFunc;
QSqlDatabase    db;
QString         dbLocation, dbconnection, dbName;
QString         collRootDir;
int             dbType, dbIconSize, dbKeywordType, task;
bool            abortThread, collVisible;
bool            yesall, yes, no, cancel;
QStringList     entryList;
Kbv::kbvItemList newlist;
QString         fileDir, errorList;

QWaitCondition  waitForUserInput;   //sync thread to dialog
QMutex          mutex;

};
#endif /*KBVCOLLECTIONTHREAD_H_*/
/****************************************************************************/
