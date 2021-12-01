/*****************************************************************************
 * kbvDirView
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.02.05
 ****************************************************************************/
#ifndef KBVDIRVIEW_H_
#define KBVDIRVIEW_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvDirModel.h"
#include "kbvFileModel.h"

class KbvDirView : public QTreeView
{
  Q_OBJECT

public:
	KbvDirView(QWidget * parent = nullptr);
	virtual ~KbvDirView();

void    setModelAndConnect(QAbstractItemModel *model);
void    setFileModel(KbvFileModel *model);

signals:
void    itemActivated(const QModelIndex &index);
void    clearStatusMessage(void);
void    keyCtrlAPressed(QKeyEvent *event);

public slots:
void    renameDir(void);
void    insertDir(void);
void    removeDir(void);
void    expandDir(const QModelIndex &index);

protected:
void    dragEnterEvent(QDragEnterEvent* event);
void    dragMoveEvent(QDragMoveEvent *event);
void    dragLeaveEvent(QDragLeaveEvent* event);
void    dropEvent(QDropEvent *event);

private slots:
void    itemSelected(const QModelIndex &index);
void    setWaitCursor(bool wait);
void    scrollTreeTimer(void);
void	currentChanged(const QModelIndex & current, const QModelIndex & previous);

private:
void    createPopup();
void    keyPressEvent(QKeyEvent *event);
void    contextMenuEvent(QContextMenuEvent *event);
void    mouseReleaseEvent(QMouseEvent *event);
void    scrollTree(QPoint mousePosition, bool enable);

KbvDirModel   *dirModel;
KbvFileModel  *fileModel;
QMenu         *popupDirView;
QAction	      *actRenameDir, *actInsertDir, *actRemoveDir;
QModelIndex   index;
QCursor	      cursor;
QRubberBand   *dropIndicator;
QTimer	      *scrollTimer;
QString	      displayedDir;
int	      value, margin;
int	      xmargin, ymargin, wmargin, hmargin;

};

#endif /*KBVDIRVIEW_H_*/

/****************************************************************************/
