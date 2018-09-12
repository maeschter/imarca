/*****************************************************************************
 * kvbSearchThread
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 15:27:08 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1470 $
 * Created: 2012.08.24
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#ifndef KBVSEARCHTHREAD_H_
#define KBVSEARCHTHREAD_H_
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "kbvGeneral.h"
#include "kbvGlobal.h"
#include "kbvConstants.h"
#include "kbvDBInfo.h"


class KbvSearchThread : public QThread
{
  Q_OBJECT

public:
  KbvSearchThread(QObject *parent=nullptr);
  virtual ~KbvSearchThread();

void    setDatabase(QSqlDatabase database, KbvDBInfo *databaseInfo);
void    start(QStringList &keywordList, QStringList &pathsList, QList<int> &multilist, int function);
void    run();

signals:
void    newItems(Kbv::kbvItemList);
void    replaceItem(int row, Kbv::kbvItem *item);
void    insertItems(int row, int count);
void    updateItem(Kbv::kbvItem *item);
void    invalidate(qint64 primaryKey);
void    endOfSearching();
void    endOfReplaceing();
void    warning(QString str, bool modal);
void    statusText1(QString text);
void    statusText2(QString text);
void    statusText3(QString text);

private:
void    search();
void    replace();
void    remove();

QSqlDatabase    db;
QString         dbLocation, dbconnection, dbName, dbVer, dbDescription, collRoot;
int             dbType, dbIconSize, dbKeywordType;
bool            searchPath, abortThread, error;

KbvGeneral      generalFunc;
KbvGlobal       globalFunc;
Kbv::kbvItemList newlist;
QStringList     entryList;
QString         errorList;
int             dateFirst, dateLast, task, cat, prec;
};
#endif /*KBVSEARCHTHREAD_H_*/
/****************************************************************************/
