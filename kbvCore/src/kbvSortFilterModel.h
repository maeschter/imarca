/*****************************************************************************
 * kbvCollectionSortModel
 * This is the sort and filter model for albums and collections
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2013.01.03
 ****************************************************************************/
#ifndef KBVSORTFILTERMODEL_H_
#define KBVSORTFILTERMODEL_H_
#include <QtCore>

class KbvSortFilterModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
    KbvSortFilterModel(QObject *parent=nullptr);
    virtual
    ~KbvSortFilterModel();

bool    lessThan(const QModelIndex &left, const QModelIndex &right) const;

public slots:
void    sortButtonClicked(int sortButton);
void    sortRoleChanged(int sortRole);
void    enableSorting(bool enable);

private:
QCollator     collator;
Qt::SortOrder sortorder;
int           sortrole;
};
#endif /* KBVSORTFILTERMODEL_H_ */
/****************************************************************************/
