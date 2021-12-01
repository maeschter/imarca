/*****************************************************************************
 * kvb collection watcher thread
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2016.05.27
 *****************************************************************************/
#ifndef KBVCOLLECTIONWATCHTHREAD_H_
#define KBVCOLLECTIONWATCHTHREAD_H_
#include <QObject>
#include <QtCore>
#include <QtSql>
#include "kbvGlobal.h"
#include "kbvConstants.h"
#include "kbvDBInfo.h"

class KbvCollectionWatchThread : public QThread
{
  Q_OBJECT

public:
explicit KbvCollectionWatchThread(QObject *parent=nullptr, KbvDBInfo *databaseInfo=nullptr, int interval=5000);
virtual ~KbvCollectionWatchThread();

void  start(const QString &branch);
bool  stop(void);
void  visible(bool visible);

signals:
void  invalidate(qint64 primaryKey);
void  newItem(Kbv::kbvItem *item);
void  updateItem(Kbv::kbvItem *item);
void  finished();

private slots:
void  startByTimer();
void  startTimer();

private:
void  run();
void  readFileNamesForBranch();
void  readDatasetsForBranch();

KbvGlobal       globalFunc;
QStringList             L1;     //file names in branch
QMap<QString, quint32>  L2;     //file names and crc values in db
QMap<QString, qint64>   L3;     //file names and prim keys in db
Kbv::kbvItem            *item;  //item for collModel
QMap<QString, QVariant> recordData;
QMap<QString, qint64>::const_iterator itpk;
QVariant                userDate, userComment;

QTimer          *watchTimer;
int             interval;       //timer interval
QString         path;           //watched branch
bool            abort, isVisible;
QSqlDatabase    db;
QString         dbLocation, dbconnection, dbName;
QString         collRootDir, stmt1, stmt2, stmt3, stmt4, stmt5;
int             dbType, dbIconSize, dbKeywordType, task;

};

#endif // KBVCOLLECTIONWATCHTHREAD_H
/*****************************************************************************/
