/*****************************************************************************
 * kbvSearchView
 * This is the standard view for albums and collections
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2011.10.04
 *****************************************************************************/
#ifndef KBVSEARCHVIEW_H_
#define KBVSEARCHVIEW_H_

#include <QListView>
#include "kbvGeneral.h"
#include "kbvGlobal.h"
#include "kbvSearchThread.h"
#include "kbvSearchModel.h"
#include "kbvSortFilterModel.h"
#include "kbvDBInfo.h"

class KbvSearchView : public QListView
{
  Q_OBJECT

public:
	KbvSearchView(QWidget * parent=nullptr);
	virtual ~KbvSearchView();

void          setModels(KbvSortFilterModel *sortmodel, KbvSearchModel *model);
void          setDatabaseInfo(KbvDBInfo *dbInfo);
QModelIndexList&  getSelectedItems();
QVariant          getItemData(const QModelIndex &index, int role);

signals:
void    infoText2(QString text);
void    infoText3(QString text);

public slots:
void    changeViewMode(int mode);
void    renameFromMenu();	      // File|Rename or F2
void    updateGridSize();	      // New icon size from database
void    displaySlideShow(void);	      // Picture|Slide show or F8
void    displaySlideShow(QModelIndex idx); // doble click or return key
void    setVisibleFlag(bool visible);

void    keyPressEvent(QKeyEvent *event);
void    mousePressEvent(QMouseEvent *event);
void    mouseMoveEvent(QMouseEvent *event);

private:
void    filesToClipboard(const QModelIndexList &indexList, bool move);

KbvGeneral	    generalFunc;
KbvGlobal           globalFunc;
QClipboard          *clipboard;
KbvSearchModel      *searchModel;
KbvSortFilterModel  *sortModel;

QString		dbName, dbLocation;
int		dbType, dbIconSize;
QPoint		dragStartPos;
bool		weAreVisible; //this is the visible tab
ViewMode	mode;
QModelIndexList selectedItems;
};

#endif /*KBVSEARCHVIEW_H_*/
/****************************************************************************/
