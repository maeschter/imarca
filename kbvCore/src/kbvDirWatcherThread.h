/*****************************************************************************
 * kvb dir watcher thread
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2011.01.03
 *****************************************************************************/
#ifndef KBVDIRWATCHERTHREAD_H_
#define KBVDIRWATCHERTHREAD_H_
#include <QtCore>
#include <QtGui>
#include "kbvConstants.h"

class KbvDirWatcherThread : public QThread
{
  Q_OBJECT

public: KbvDirWatcherThread(QObject * parent = 0);
        virtual ~KbvDirWatcherThread();

void    startThread(const QString dir, const QModelIndex index, int cycleTime);
void    stopThread(void);
void    readDirContent(QStringList &list, QString &dir);

signals:
void    insertRow(Kbv::kbvDirWatchItem item);
void    removeRow(Kbv::kbvDirWatchItem item);
void    checkExpansionGrip(QModelIndex index);

protected:
void    run();

private slots:
void    startByTimer();
void    startTimer();

private:
Kbv::kbvDirWatchItem  watchedItem;
QModelIndex           watchedIndex;
QTimer                *watchTimer;
QString               watchedDir;
QStringList           L1, L2;
int                   watchCycle;
bool                  abort;

};
#endif /* KBVDIRWATCHERTHREAD_H_ */
/*****************************************************************************/
