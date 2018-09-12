/*****************************************************************************
 * kbvDirModel
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-01-11 12:17:25 +0100 (Do, 11. Jan 2018) $
 * $Rev: 1379 $
 * Created: 2009.02.08
 ****************************************************************************/
#ifndef KBVDIRMODEL_H_
#define KBVDIRMODEL_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvGeneral.h"

class KbvDirModel : public QStandardItemModel
{
  Q_OBJECT

public:
	KbvDirModel(QObject * parent = nullptr);
	virtual ~KbvDirModel();

QVariant        data(const QModelIndex &index, int role) const;
Qt::ItemFlags   flags(const QModelIndex &index) const;
Qt::DropActions supportedDropActions() const;
QStandardItem*  userRoot();
QStandardItem*  fileSystemRoot();

signals:
void    expandTree(const QModelIndex &index);
void    expandView(const QModelIndex &index);
void    collapseView(const QModelIndex &index);
void    setCurrent(const QModelIndex &index);
void    dirActivated(const QString &path);
void    expandedDirRenamed(const QString &path);
void    setWaitCursor(bool wait);
void    dirTreeStateIsSet(const bool finished);
void    fileWatcherSnooze(const bool snooze);

public slots:
bool    expandDir(const QModelIndex &index);
void    collapseDir(const QModelIndex &index);
void    activatedDir(const QModelIndex &index);
void    insertDir(const QModelIndex &index, const QString &name);
void    removeDir(const QModelIndex &index);
void    renameDir(const QModelIndex &index, const QString &name);
void    checkExpansionGrip(const QModelIndex &parent);
void    setTreeState(QStringList paths);
void    storeTreeState();
void    mountDevice(QStringList mountInfo);
void    unmountDevice(QStringList mountInfo);

private slots:
void    dirWatcher();

private:
QStringList	parseBranch(const QStandardItem *parent);
QStandardItem*	findItemForPath(const QString &selectedPath, bool &itemValid);
bool		hasSubDirs(const QString &path);

KbvGeneral      generalFunc;
QStandardItem	*rootItem, *userItem, *fileSystemItem;
QStandardItem	*dirItem, *dummyItem;
QDir		*actualDir;
QCollator	collator;
QString		displayedPath;
QTimer		*watchTimer;
QPersistentModelIndex	  watchedIndex;
};

#endif /*KBVDIRMODEL_H_*/
/****************************************************************************/
