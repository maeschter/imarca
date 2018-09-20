/*****************************************************************************
 * kvb file view
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.03.02
 *****************************************************************************/
#ifndef KBVFILEVIEW_H_
#define KBVFILEVIEW_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QClipboard>
#include "kbvGeneral.h"
#include "kbvGlobal.h"
#include "kbvFileModel.h"
#include "kbvSortFilterModel.h"

class KbvFileView : public QListView
{
  Q_OBJECT

public:
	KbvFileView(QWidget * parent = nullptr);
	virtual ~KbvFileView();

void              setModels(KbvSortFilterModel *sortmodel, KbvFileModel *model);
QModelIndexList&  getSelectedItems();
QVariant          getItemData(const QModelIndex &index, int role);

signals:
void    infoText2(QString text);
void    infoText3(QString text);
void	fileOpenColl(const QString database);

public slots:
void    changeViewMode(int mode);   //list view <-> icon view
void    updateOptions();            // New values from options dialog available
void    renameFromMenu();           // File|Rename or F2 was activated
void    displaySlideShow(void);     // Image|Slide show or F8 was activated
void    displaySlideShow(QModelIndex idx);     // doble click or return key
void    currentTabIndex(int index); //current tab is "index"

void    keyPressEvent(QKeyEvent *event);
void    mousePressEvent(QMouseEvent *event);
void    mouseMoveEvent(QMouseEvent *event);
void    dragMoveEvent(QDragMoveEvent *event);
void    dropEvent(QDropEvent *event);
void    dragEnterEvent(QDragEnterEvent* event);

private slots:
void	openDatabase();	  //wird z. Zt. nicht verwendet

private:
void    filesToClipboard(const QModelIndexList &indexList, bool move);

KbvGeneral	    generalFunc;
KbvGlobal           globalFunc;
KbvFileModel        *fileModel;
KbvSortFilterModel  *sortModel;
QClipboard          *clipboard;

QSize               iconSize;       //Option
QPoint              dragStartPos;
bool                weAreVisible; //this is the visible tab
bool                dragToExtern;
ViewMode            mode;
QModelIndexList     selectedItems;
};

#endif /*KBVFILEVIEW_H_*/
/****************************************************************************/
