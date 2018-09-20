/*****************************************************************************
 * kbvCollectionModel
 * This is the standard model for albums and collections
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2011.01.23
 ****************************************************************************/
#ifndef KBVCOLLECTIONMODEL_H_
#define KBVCOLLECTIONMODEL_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvCollectionThread.h"
#include "kbvCollectionWatchThread.h"
#include "kbvBusyAnimation.h"
#include "kbvGeneral.h"
#include "kbvGlobal.h"
#include "kbvConstants.h"
#include "kbvDBInfo.h"

class KbvCollectionThread;

class KbvCollectionModel : public QAbstractListModel
{
  Q_OBJECT

public:
  KbvCollectionModel(QObject *parent=nullptr, KbvDBInfo *databaseInfo=nullptr);
  virtual  ~KbvCollectionModel();

  KbvCollectionWatchThread  *watchThread;

  void          setViewMode(int mode);
  QVariant      data(const QModelIndex &index, int role) const;
  bool          setData(const QModelIndex &index, const QVariant &value, int role);
  Qt::ItemFlags flags(const QModelIndex &index) const;
  int           columnCount(const QModelIndex &idx) const;
  int           rowCount(const QModelIndex &idx) const;
  QModelIndex   index(int row, int col, const QModelIndex &parent) const;
  QModelIndex   parent(const QModelIndex &idx) const;
  void          clear();
  void          updateDatabase();
  bool          insertItems(int row, int count);
  bool          replaceItem(int row, Kbv::kbvItem *item);
  bool          removeItem(int row);
  int           findItem(int role1, QVariant data1, int role2, QVariant data2);
  void          setThreads(KbvCollectionThread *collthread, KbvCollectionWatchThread *watchthread);
  void          tabIsVisible(bool visible);
  void          dragMoveCleanUp(const QList<QPersistentModelIndex> &idxList);
  bool          multipleRename(QModelIndexList idxList,
                         QString prefix, QString suffix, QString fileExt,
                         int startValue, int  stepValue,
                         int numerals, int combination);
  bool          singleRename(QModelIndexList idxList,
                             QString prefix, QString suffix, QString fileExt, int combination);
  QString       getActualBranch();
  
  
public slots:
  void  refresh();                  // F5 or View|Refresh was activated
  void  readFromDB(const QString &name, const QString &branch);
  void  readinOrUpdateDB(const QString &path);
  void  renameDBBranch(const QString &names);
  void  removeFiles(QMap<QString, QString> list);
  void  removeSelection(const QModelIndexList &indexList);
  void  removeEmptyItems();
  void  appendItem(Kbv::kbvItem *item);
  void  appendItems(Kbv::kbvItemList list);
  void  updateItem(Kbv::kbvItem *item);
  void  invalidateItem(qint64 primaryKey);
  void  endOfReading();
  void  endOfUpdate(const QString &text);
  void  askForUserInput(QString text1, QString text2);
  void  statusText1(const QString &text);
  void  statusText2(const QString &text);
  void  statusText3(const QString &text);

signals:
  void  animationInfoText(QString title, QString text); //kbvCollectionTabs
  void  userInput(int result);                          //kbvCollectionModel
  void  infoText1(QString text);
  void  infoText2(QString text);
  void  infoText3(QString text);
  void  enableSorting(bool sort);

private:
  KbvGeneral            generalFunc;
  KbvGlobal             globalFunc;
  KbvCollectionThread   *collThread;
  QSqlDatabase          db;
  QString               dbLocation, dbconnection, dbName;
  int                   dbType, dbIconSize, dbKeywordType;
  QString               collRootDir, actualBranch;
  QList<Kbv::kbvItem* > itemList;
  bool                  weAreVisible;
  int                   viewMode;
};

#endif /* KBVCOLLECTIONMODEL_H_ */
/****************************************************************************/
