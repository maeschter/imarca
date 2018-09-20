/* kbvSearchModel
 * This is the standard model for search in albums and collections
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.08.13
 ****************************************************************************/
#ifndef KBVSEARCHMODEL_H_
#define KBVSEARCHMODEL_H_
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "kbvGeneral.h"
#include "kbvGlobal.h"
#include "kbvSearchThread.h"
#include "kbvDBInfo.h"
#include "kbvConstants.h"

class KbvSearchModel : public QAbstractListModel
{
  Q_OBJECT

public:
  KbvSearchModel(QObject *parent=nullptr);
  virtual  ~KbvSearchModel();

  void    setDatabase(QSqlDatabase database, KbvDBInfo *databaseInfo);
  void    startThread(QStringList &keywordList, QStringList &pathsList, QList<int> &multilist, int function);
  void    setViewMode(int mode);
  QVariant       data(const QModelIndex &index, int role) const;
  bool          setData(const QModelIndex &index, const QVariant &value, int role);
  Qt::ItemFlags  flags(const QModelIndex &index) const;
  int           columnCount(const QModelIndex &idx) const;
  int           rowCount(const QModelIndex &idx) const;
  QModelIndex   index(int row, int col, const QModelIndex &parent) const;
  QModelIndex   parent(const QModelIndex &idx) const;
  bool  removeItem(int row);
  int   findItem(int role1, QVariant data1, int role2, QVariant data2);
  void  dbSettings(int typ, int iconsize);
  void  dragMoveCleanUp(const QList<QPersistentModelIndex> &idxList);
  void  ctrlXCleanUp(const QModelIndexList &idxList);
  bool  multipleRename(QModelIndexList idxList,
                       QString prefix, QString suffix, QString fileExt,
                       int startValue, int  stepValue, int numerals, int combination);
  bool  singleRename(QModelIndexList idxList,
                     QString prefix, QString suffix, QString fileExt, int combination);

signals:
  void  infoText1(QString text);
  void  infoText2(QString text);
  void  infoText3(QString text);
  void  enableSorting(bool sort);
  void  warning(QString str, bool modal);

public slots:
  void  clear();
  void  endOfReading();
  void  updateModel(const QString &pathname);
  void  removeFiles(QMap<QString, QString> list);
  void  removeSelection(const QModelIndexList &indexList);
  void  removeEmptyItems();
  bool  replaceItem(int row, Kbv::kbvItem *item);
  bool  insertItems(int row, int count);
  void  appendItems(Kbv::kbvItemList list);
  void  updateItem(Kbv::kbvItem  *item);
  void  invalidateItem(qint64 primaryKey);
  void  askForUserInput(QString text1, QString text2);
  void  statusText1(QString text);
  void  statusText2(QString text);
  void  statusText3(QString text);

signals:
  void  userInput(int result);

private:
  KbvGeneral            generalFunc;
  KbvGlobal             globalFunc;
  QModelIndexList       dragIdxList;
  KbvSearchThread       *searchThread;
  QList<Kbv::kbvItem* > itemList;
  QSqlDatabase          searchDB;
  QString               dbName, dbVer, dbDescription, collRoot;
  int                   dbType, dbIconSize, dbKeywordType;
  int                   viewMode;
};

#endif /* KBVSEARCHMODEL_H_ */
/****************************************************************************/
