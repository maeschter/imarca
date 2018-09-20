/*****************************************************************************
 * kbvCollectionView
 * This is the standard view for albums and collections
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2011.10.04
 *****************************************************************************/
#ifndef KBVCOLLECTIONVIEW_H_
#define KBVCOLLECTIONVIEW_H_
#include <QtCore>
#include <QtGui>
#include "kbvGlobal.h"
#include "kbvSortFilterModel.h"
#include "kbvCollectionModel.h"
#include "kbvCollectionDragDrop.h"
#include "kbvDBInfo.h"

class KbvCollectionView : public QListView
{
  Q_OBJECT

public:
	KbvCollectionView(QWidget * parent=nullptr, KbvDBInfo *databaseInfo=nullptr);
	virtual ~KbvCollectionView();

void        setModels(KbvSortFilterModel *sortmodel, KbvCollectionModel *model);
void        setDragDropHandler(KbvCollectionDragDrop *dragDropHandler);
QString     actualBranch(QString title);
QModelIndexList&  getSelectedItems();
QVariant          getItemData(const QModelIndex &index, int role);

signals:
void    infoText2(QString text);
void    infoText3(QString text);

public slots:
void    changeViewMode(int mode);
void    tabIsVisible(bool visible);
void    updateOptions();		    // New values from options dialog available
void    renameFromMenu();		    // File|Rename or F2 was activated
void    displaySlideShow(void);		    // Picture|Slide show or F8 was activated
void    displaySlideShow(QModelIndex idx);  // doble click or return key
void    setActualBranch(QString title, QString branch);
void    ctrlVCleanUp(const QStringList &mimeStrings, const QStringList &pathList);

void    keyPressEvent(QKeyEvent *event);
void    mousePressEvent(QMouseEvent *event);
void    mouseMoveEvent(QMouseEvent *event);
void    dragMoveEvent(QDragMoveEvent *event);
void    dropEvent(QDropEvent *event);
void    dragEnterEvent(QDragEnterEvent* event);

private:
void    filesToClipboard(const QModelIndexList &indexList, bool move);

KbvGlobal             globalFunc;
KbvSortFilterModel    *sortModel;
KbvCollectionModel    *collModel;
KbvCollectionDragDrop *dndHandler;
KbvDBInfo	      *dbInfo;
QClipboard            *clipboard;
QPoint		      dragStartPos;
QString		      dbName, branch;
int		      dbType, dbIconSize;
bool		      weAreVisible; //this is the visible tab
QModelIndexList	      selectedItems;
};

#endif /*KBVCOLLECTIONVIEW_H_*/
/****************************************************************************/
