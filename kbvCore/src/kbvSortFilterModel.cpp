/*****************************************************************************
 * kbvCollectionModel
 * This is the sort and filter model for albums and collections.
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2013.01.03
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvSortFilterModel.h"

KbvSortFilterModel::KbvSortFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
  sortorder = Qt::AscendingOrder;
  sortrole = Kbv::FileNameRole;
  this->sort(0, sortorder);
  this->setSortRole(sortrole);
  this->setSortCaseSensitivity(Qt::CaseInsensitive);
  
  //natural sorting of file names containing numbers
  collator.setLocale(QLocale::system());
  collator.setNumericMode(true);
  
}

KbvSortFilterModel::~KbvSortFilterModel()
{
  //qDebug() << "KbvSortFilterModel::~KbvSortFilterModel"; //###########
}
/*************************************************************************//*!
 * For sortRole = Kbv::sortFileName the QCollator provides a natural sorting
 * of filenames where numbers appear in correct order.
 * All other sort roles use the compare function of QVariant. For Kbv::NoneRole
 * the method returs false hence nothing get sorted.
 */
bool    KbvSortFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  if(this->sortRole() == Kbv::FileNameRole)
    {
      QVariant leftData = sourceModel()->data(left, Kbv::FileNameRole);
      QVariant rightData = sourceModel()->data(right, Kbv::FileNameRole);
      return collator.compare(leftData.toString(), rightData.toString()) < 0;
    }
  else if(this->sortRole() == Kbv::FileSizeRole)
    {
      QVariant leftData = sourceModel()->data(left, Kbv::FileSizeRole);
      QVariant rightData = sourceModel()->data(right, Kbv::FileSizeRole);
      return leftData < rightData;
    }
  else if(this->sortRole() == Kbv::FileDateRole)
    {
      QVariant leftData = sourceModel()->data(left, Kbv::FileDateRole);
      QVariant rightData = sourceModel()->data(right, Kbv::FileDateRole);
      return leftData < rightData;
    }
  else if(this->sortRole() == Kbv::ImageSizeRole)
    {
      QVariant leftData = sourceModel()->data(left, Kbv::ImageSizeRole);
      QVariant rightData = sourceModel()->data(right, Kbv::ImageSizeRole);
      return leftData < rightData;
    }
  else if(this->sortRole() == Kbv::UserDateRole)
    {
      QVariant leftData = sourceModel()->data(left, Kbv::UserDateRole);
      QVariant rightData = sourceModel()->data(right, Kbv::UserDateRole);
      return leftData < rightData;
    }
  else //Kbv::NoneRole
    {
      return false;
    }
}
/*************************************************************************//*!
 * SLOT: enable sorting of column 0 by sortrole or disable sorting by using
 * the non existent sort role NoneRole.
 */
void    KbvSortFilterModel::enableSorting(bool enable)
{
  if(enable)
    {
      this->setSortRole(sortrole);
      this->sort(0, sortorder);
    }
  else
    {
      this->setSortRole(Kbv::NoneRole);
    }
  //qDebug() << "KbvSortFilterModel::enableSorting" << enable << sortrole; //###########
}
/*************************************************************************//*!
 * SLOT: one of the sort buttons on the information bar has been clicked.
 */
void    KbvSortFilterModel::sortButtonClicked(int button)
{
  if(button == Kbv::up)
    {
      this->sortorder = Qt::AscendingOrder;
    }
  if(button == Kbv::down)
    {
      this->sortorder = Qt::DescendingOrder;
    }
  this->sort(0,sortorder);
  //qDebug() << "KbvSortFilterModel::sortButtonClicked sortorder" <<sortorder; //###########
}
/*************************************************************************//*!
 * SLOT: a sort role on the information bar has been selected.
 */
void    KbvSortFilterModel::sortRoleChanged(int sortRole)
{

  switch(sortRole)
  {
    case Kbv::sortFileName:
      {
        this->setSortRole(Kbv::FileNameRole);
        this->sortrole = Kbv::FileNameRole;
      }
    break;
    case Kbv::sortFileSize:
      {
        this->setSortRole(Kbv::FileSizeRole);
        this->sortrole = Kbv::FileSizeRole;
      }
    break;
    case Kbv::sortFileDate:
      {
        this->setSortRole(Kbv::FileDateRole);
        this->sortrole = Kbv::FileDateRole;
      }
    break;
    case Kbv::sortImageSize:
      {
        this->setSortRole(Kbv::ImageSizeRole);
        this->sortrole = Kbv::ImageSizeRole;
      }
    break;
    case Kbv::sortUserDate:
      {
        this->setSortRole(Kbv::UserDateRole);
        this->sortrole = Kbv::UserDateRole;
      }
    break;
    default:
      {
        this->setSortRole(Kbv::FileNameRole);
        this->sortrole = Kbv::FileNameRole;
      }
    break;
  }
  this->sort(0,sortorder);
  //qDebug() << "KbvSortFilterModel::sortRoleChanged role" <<sortRole; //###########
}
/****************************************************************************/
