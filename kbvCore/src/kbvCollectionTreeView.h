/*****************************************************************************
 * KbvCollectionTreeView.h
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01s
 * Created: 2010.10.17
 *****************************************************************************/
#ifndef KBVCOLLECTIONTREEVIEW_H_
#define KBVCOLLECTIONTREEVIEW_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include "kbvGeneral.h"
#include "kbvCollectionTreeModel.h"
#include "kbvDBOptions.h"
#include "kbvDBExport.h"
#include "kbvDBImport.h"
#include "kbvDBImExportProgress.h"
#include "kbvDBImExportThread.h"
#include "kbvDBInfo.h"

class KbvCollectionTreeView : public QTreeView
{
  Q_OBJECT

public:
  KbvCollectionTreeView(QWidget * parent = nullptr);

  virtual   ~KbvCollectionTreeView();

void    setModelAndConnect(QAbstractItemModel *model);

public slots:
void    enableMenu(int index);
void    collapseItem(const QString &title);
void    databaseInfo();
void    databaseCreate();
void    databaseRemove();
void    databaseOpen(const QString database);
void    databaseImport();
void    databaseExport();
void    databaseUnmount(QStringList mountInfo);
void    databaseRead();
void    databaseUpdate();
void    collRename();
void    collInsert();
void    collDelete();
void    setWaitCursor(bool wait);
void    databaseImExportFinished(int flag);
void    databaseImExportsetDB(QString name);

signals:
void    collItemActivated(const QString &name, const QString &branch, int type, const QString &location, const QString &root);
void    collRemovedFromTree(const QString &name);
void    updateDB(const QString &name, const QString &path, int type, const QString &location, const QString &root);
void    keyCtrlAPressed(QKeyEvent *event);

protected:
void    dragEnterEvent(QDragEnterEvent* event);
void    dragLeaveEvent(QDragLeaveEvent* event);
void    dragMoveEvent(QDragMoveEvent* event);
void    dropEvent(QDropEvent* event);

private slots:
void    itemSelected(const QModelIndex &index);
void    itemExpanded(const QModelIndex &index);
void    scrollTreeTimer(void);

private:
bool    databaseDirExists(QString path);
void    createPopUp();
void    keyPressEvent(QKeyEvent *event);
void    contextMenuEvent(QContextMenuEvent *event);
void    mousePressEvent(QMouseEvent *event);
void    mouseMoveEvent(QMouseEvent *event);
void    mouseReleaseEvent(QMouseEvent *event);
void    scrollTree(QPoint mousePosition, bool enable);
bool    createDBInfo(KbvDBInfo *info, const QString name, const QString location, const QString root, int type);

QMenu         *popupCollection;
QAction       *actDBCreate, *actDBRemove, *actDBRead, *actDBUpdate, *actDBInfo;
QAction       *actRename, *actInsert, *actDelete;

KbvCollectionTreeModel  *collTreeModel;
KbvGeneral            generalFunc;
KbvDBOptions          *dbOptionsDialog;
KbvDBExport           *dbExportDialog;
KbvDBImport           *dbImportDialog;
KbvDBImExportProgress *dbImExProgress;
KbvDBImExportThread   *dbImExThread;

QModelIndex     dragItemIndex;
QPoint          dragStartPos;
QString         activatedTitle;
QString         activatedPath;

QRubberBand     *dropIndicator;
QTimer          *scrollTimer;
int             value, margin;
int             xmargin, ymargin, wmargin, hmargin;

QSqlDatabase    treeDB;
KbvDBInfo       *dbInfo;
QString         dbName, dbConnection;
QString         dbLocation, dbVer, dbDescription, dbRootDir;
int             dbType, dbIconSize, dbKeywordType;
};
#endif /* KBVCOLLECTIONTREEVIEW_H_ */
/****************************************************************************/
