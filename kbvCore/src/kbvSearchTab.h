/*****************************************************************************
 * KbvSearchTab
 * This is the widget for search tab
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2013.01.24
 ****************************************************************************/
#ifndef KBVSEARCHTAB_H_
#define KBVSEARCHTAB_H_
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "kbvTabWidget.h"
#include "kbvSearchThread.h"
#include "kbvSortFilterModel.h"
#include "kbvSearchModel.h"
#include "kbvSearchView.h"
#include "kbvDBSearchDialog.h"
#include "kbvCollectionPopUp.h"
#include "kbvDBInfo.h"
#include "kbvRecordInfo.h"

class KbvSearchTab : public KbvTabWidget
{
  Q_OBJECT

public:
  KbvSearchTab(QWidget *parent = nullptr);
  virtual
  ~KbvSearchTab();

  KbvSearchView     *searchView;
  KbvSearchModel    *searchModel;
  KbvDBInfo         *databaseInfo;
  KbvRecordInfo     *recordInfo;

signals:
  void	imageEdit();
  void	imageBatchEdit();
  void	imageMetadata();

public slots:
  void    currentTabIndex(int index);
  bool    search(KbvDBInfo* dbInfo);
  void    searchClear();
  void    showDBInfo();                   //main menu
  void    showRecordInfo();               //local pop up/main menu

private slots:
  void    warnings(QString text, bool modal);
  void    contextMenuEvent(QContextMenuEvent *event);

private:
  bool    setDBConnection(const QString name, const QString dbPathName);

QString             dbconnection, dbName, dbLocation, dbVer, dbDescription, collRoot;
int                 dbType, dbIconSize, dbKeywordType;

QSqlDatabase        searchDB;
KbvSortFilterModel  *sortModel;
KbvDBSearchDialog   *searchDialog;
KbvCollectionPopUp  *popupMenu;
bool                weAreVisible;
};
#endif /* KBVSEARCHTAB_ */
/****************************************************************************/
