/* KbvCollectionTreeModel.h
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2010.10.27
 ****************************************************************************/
#ifndef KBVCOLLECTIONTREEMODEL_H_
#define KBVCOLLECTIONTREEMODEL_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include "kbvGeneral.h"

class KbvCollectionTreeModel : public QStandardItemModel
{
  Q_OBJECT

public:
  KbvCollectionTreeModel(QObject * parent = nullptr);

  virtual   ~KbvCollectionTreeModel();

  QVariant          data(const QModelIndex &index, int role) const;
  bool              setData(const QModelIndex &index, const QVariant &value, int role);
  Qt::ItemFlags     flags(const QModelIndex &index) const;
  void              moveItem(const QModelIndex &source, const QModelIndex &target);
  void              createItem(const QString name, const QString rootdir, int type, bool attention, bool joined);
  void              deleteItem(const QModelIndex &index);
  void              insertItem(const QModelIndex &index, const QString name);
  void              removeDatabase(const QModelIndex &index);
  void              renameDatabase(const QModelIndex &index, const QString oldName, const QString newName);
  bool              childNameExists(const QString name, int type, const QModelIndex &parent);
  QStandardItem*    albumAnchor();
  QStandardItem*    collectionAnchor();
  Qt::DropActions   supportedDropActions() const;

signals:
void        setWaitCursor(bool wait);
void        collRenamedInTree(const QString &name);
void        branchRenamedInTree(const QString &name);

public slots:
bool        expand(const QModelIndex &index);
void        collapse(const QModelIndex &index);
QModelIndex indexFromCollection(const QString &title);
void        setCollTreeState();
void        storeCollTreeState();

private:
KbvGeneral    generalFunc;
QCollator     collator;
bool          existsDatabase(const QString &name);
QString       tooltip(const QModelIndex &index) const;
QStandardItem *albumAnchorItem, *collectionAnchorItem;
};

#endif /* KBVCOLLECTIONTREEMODEL_H_ */
/****************************************************************************/
