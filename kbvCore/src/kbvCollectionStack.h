/*****************************************************************************
 * KbvCollectionStack
 * This is the stack for collection view, model, thread and database
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-06-19 22:24:01 +0200 (Mo, 19. Jun 2017) $
 * $Rev: 1354 $
 * Created: 2012.05.14
 ******************************************************************************/
#ifndef KBVCOLLECTIONSTACK_H_
#define KBVCOLLECTIONSTACK_H_
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "kbvTabWidget.h"
#include "kbvSortFilterModel.h"
#include "kbvCollectionDragDrop.h"
#include "kbvCollectionThread.h"
#include "kbvCollectionWatchThread.h"
#include "kbvCollectionModel.h"
#include "kbvCollectionView.h"
#include "kbvBusyAnimation.h"
#include "kbvDBInfo.h"
#include "kbvRecordInfo.h"
#include "kbvCollectionPopUp.h"
#include "kbvCsvChecker.h"


class KbvCollectionStack : public KbvTabWidget
{
  Q_OBJECT

public:
  KbvCollectionStack(QWidget *parent=nullptr, QString dbPath="", QString dbName="", QString rootDir="",
                    int dbType=0, KbvCollectionDragDrop *dragDropHandler=nullptr);
  virtual
  ~KbvCollectionStack();

  KbvSortFilterModel    *collSortProxy;
  KbvCollectionView     *collView;
  KbvCollectionModel    *collModel;
  KbvCsvChecker         *csvChecker;
  QString               getCollectionRoot();
  bool                  failed;

  KbvDBInfo*            getDatabaseInfo();
signals:
  void    search();
  void    imageMetadata();
  void    imageEdit();
  void    imageBatchEdit();

public slots:
  void    currentTabTitle(QString title); //current tab is "title"
  void    showDBInfo();                   //main menu
  void    showRecordInfo();               //local pop up/main menu
  void    askForUserInput(QString text1, QString text2);
  void    showAnimation(QString title, bool show);

private slots:
  void    warnings(QString text, bool modal);
  void    setAnimationInfo(QString title, QString info);
  void    endOfAnimation(int index);

private:
  void    contextMenuEvent(QContextMenuEvent *event);
  void    createPopup();

  QSqlDatabase              collDB;
  KbvRecordInfo             *recordInfo;
  KbvDBInfo                 *databaseInfo;
  KbvBusyAnimation          *animation;
  KbvCollectionThread       *collThread;
  KbvCollectionWatchThread  *watchThread;
  KbvCollectionPopUp        *popupMenu;
  QStackedWidget            *stackWidget;

  QString                   dbName, collDBConnection;
  QString                   dbLocation, dbVer, dbDescription, collRoot;
  int                       dbType, dbIconSize, dbKeywordType;
  bool                      weAreVisible;
};
#endif /* KBVCOLLECTIONSTACK_H_ */
