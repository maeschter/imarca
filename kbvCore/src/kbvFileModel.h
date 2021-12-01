/*****************************************************************************
 * kvb file model
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2011.01.27
 *****************************************************************************/
#ifndef KBVFILEMODEL_H_
#define KBVFILEMODEL_H_
#include <QtCore>
#include <QtGui>
#include "kbvGeneral.h"
#include "kbvGlobal.h"
#include "kbvConstants.h"
#include "kbvFileModelThread.h"
#include "kbvFileWatcherThread.h"

class KbvFileModel : public QAbstractListModel
{
  Q_OBJECT

public:	        KbvFileModel(QObject *parent = nullptr);
                virtual ~KbvFileModel();

void            setViewMode(int mode);

QVariant        data(const QModelIndex &index, int role) const;
bool            setData(const QModelIndex &index, const QVariant &value, int role);
Qt::ItemFlags   flags(const QModelIndex &index) const;
Qt::DropActions supportedDropActions() const;
Qt::DropActions supportedDragActions() const;
bool            dropData(const QMimeData *mimeData, Qt::DropAction action,
                             const QString targetDir, bool visible, bool paste);
void            dragMoveCleanUp(const QList<QPersistentModelIndex> &idxList);
void            ctrlXCleanUp(const QModelIndexList &idxList);
void            insertEmptyItems(int position, int count);
void            insertItem(int position, Kbv::kbvItem *item);
int             columnCount(const QModelIndex &idx) const;
int             rowCount(const QModelIndex &idx) const;
QModelIndex     index(int row, int col, const QModelIndex &parent) const;
QModelIndex     parent(const QModelIndex &idx) const;
bool            multipleRename(QModelIndexList idxList,
                              QString prefix, QString suffix, QString fileExt,
                              int startValue, int  stepValue, int numerals, int combination);
bool            singleRename(QModelIndexList idxList,
                            QString prefix, QString suffix, QString fileExt, int combination);
void            updateItem(const QString &pathName, const QString &fileName);

QString                 rootDir;
KbvFileModelThread      *fileModelThread;
KbvFileWatcherThread    *fileWatchThread;

signals:
void    scrollViewToTop(void);
void    enableSorting(bool sort);
void    infoText1(QString text);
void    infoText2(QString text);
void    infoText3(QString text);

public slots:
void    startFillThread(const QString &newDir);
void    fillThreadFinished();
void    startFileWatchThread(const QString path);
void    watchThreadFinished();
void    enableDirMonitor(bool enable);
void    refreshModel();
void    renamedDirUpdate(const QString &path);
void    updateOptions(QSize size, int cycle);
void    removeEmptyItems();
void    replaceItem(int position, Kbv::kbvItem *item);
void    removeItem(int position);
void    removeFiles(QMap<QString, QString> list);
void    removeSelection(QModelIndexList idxList);
int     findItem(const QVariant &value, int role);
void    appendItem(Kbv::kbvItem *item);
void    invalidateItem(int row);
void    statusText1(QString text);
void    statusText2(QString text);
void    statusText3(QString text);
void    sortEnable();
void    sortDisable();

private:
KbvGeneral      generalFunc;
KbvGlobal       globalFunc;
//QModelIndexList   dragIdxList;
QList<QPair<QString, qint64> > watchList;
QList<Kbv::kbvItem* > itemList;
QFileSystemWatcher    *dirMonitor;
QSize           iconSize;
QStringList     fileEntryList;
QTimer          *watchTimer;
int             dirWatchCycle, viewMode;
bool            lock, isRunning, restart;
};
#endif /*KBVFILEMODEL_H_*/
/****************************************************************************/
