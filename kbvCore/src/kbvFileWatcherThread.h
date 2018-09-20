/*****************************************************************************
 * kvb file watcher thread
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.09.18
 *****************************************************************************/
#ifndef KBVFILEWATCHERTHREAD_H_
#define KBVFILEWATCHERTHREAD_H_
#include <QtCore>
#include <QtGui>
#include "kbvGlobal.h"
#include "kbvConstants.h"

class   KbvFileModel;

class KbvFileWatcherThread : public QThread
{
  Q_OBJECT

public: KbvFileWatcherThread(QObject * parent = nullptr);
        virtual ~KbvFileWatcherThread();

void  startThread(const QString &rootdir, QList<QPair<QString, qint64> > *modelList, QSize iconsize);
void  stopThread(void);

signals:
void  newItem(Kbv::kbvItem*);
void  replaceItem(int, Kbv::kbvItem*);
void  removeEmptyItems();
void  invalidItem(int row);
void  threadFinished();

private:
void  run();
void  readDirContent(QString dirpath, QStringList &list);
int   getFileCountInDir(QString dirpath);

KbvGlobal     globalFunc;

QList<QPair<QString, qint64> > *L1;   //list of files in model
QStringList                     L2;   //list of files in dir
QString       watchedDir;
QSize         iconSize;
bool          abort;
};
#endif /* KBVFILEWATCHERTHREAD_H_ */
/*****************************************************************************/
