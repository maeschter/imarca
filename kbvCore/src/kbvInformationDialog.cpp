/*****************************************************************************
 * KbvInformationDialog
 * This class provides an information dialog (information, warning, critical).
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2016-03-13 12:00:38 +0100 (So, 13. Mär 2016) $
 * $Rev: 1080 $
 * Created: 2012.07.02
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvInformationDialog.h"

KbvInformationDialog::KbvInformationDialog(QWidget *parent) : QMessageBox(parent)
{
  this->setWindowTitle("Imarca");
  this->setModal(false);
}
KbvInformationDialog::~KbvInformationDialog()
{
  //qDebug() << "KbvInformationDialog::~KbvInformationDialog";//###########
}
/*************************************************************************//*!
 * Set caption, info text and icon and execute the information dialog.
 * iconIndex: 0=information, 1=warning, 2=critical, 3=yes/no question,
 * 4=yes/yestoall/no/cancel descition.
 */
int   KbvInformationDialog::perform(QString caption, QString infoText, int iconIndex)
{
  this->setText(caption);
  this->setInformativeText(infoText);

  switch (iconIndex)
  {
    case 0: // information
      {
        this->setIcon(QMessageBox::Information);
        this->setStandardButtons(QMessageBox::Close);
        this->setDefaultButton(QMessageBox::Close);
        break;
      }
    case 1: // warning
      {
        this->setIcon(QMessageBox::Warning);
        this->setStandardButtons(QMessageBox::Close);
        this->setDefaultButton(QMessageBox::Close);
        break;
      }
    case 2: // critical
      {
        this->setIcon(QMessageBox::Critical);
        this->setStandardButtons(QMessageBox::Close);
        this->setDefaultButton(QMessageBox::Close);
        break;
      }
    case 3: // yes/no decision
      {
        this->setIcon(QMessageBox::Question);
        this->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        this->setDefaultButton(QMessageBox::No);
        break;
      }
    case 4: // yes/yestoall/no/cancel decision
      {
        this->setIcon(QMessageBox::Question);
        this->setStandardButtons(QMessageBox::Yes|QMessageBox::YesToAll|QMessageBox::No|QMessageBox::Cancel);
        this->setDefaultButton(QMessageBox::YesToAll);
        break;
      }
    default:
      {
        this->setIcon(QMessageBox::NoIcon);
        this->setStandardButtons(QMessageBox::Close);
        this->setDefaultButton(QMessageBox::Close);
        break;
      }
  }
  return  QMessageBox::exec();
}
/*************************************************************************//*!
 * Set caption text and icon: 0=information, 1=warning, 2=critical,
 * 3=yes/no question, 4=yes/yestoall/no/cancel descition.
 */
void    KbvInformationDialog::setCaptionAndIcon(QString caption, QString infoText, int iconIndex)
{
  this->setText(caption);
  this->setInformativeText(infoText);

  switch (iconIndex)
  {
    case 0: // information
      {
        this->setIcon(QMessageBox::Information);
        this->setStandardButtons(QMessageBox::Close);
        this->setDefaultButton(QMessageBox::Close);
        break;
      }
    case 1: // warning
      {
        this->setIcon(QMessageBox::Warning);
        this->setStandardButtons(QMessageBox::Close);
        this->setDefaultButton(QMessageBox::Close);
        break;
      }
    case 2: // critical
      {
        this->setIcon(QMessageBox::Critical);
        this->setStandardButtons(QMessageBox::Close);
        this->setDefaultButton(QMessageBox::Close);
        break;
      }
    case 3: // yes/no decision
      {
        this->setIcon(QMessageBox::Question);
        this->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        this->setDefaultButton(QMessageBox::No);
        break;
      }
    case 4: // yes/yestoall/no/cancel decision
      {
        this->setIcon(QMessageBox::Question);
        this->setStandardButtons(QMessageBox::Yes|QMessageBox::YesToAll|QMessageBox::No|QMessageBox::Cancel);
        this->setDefaultButton(QMessageBox::YesToAll);
        break;
      }
    default:
      {
        this->setIcon(QMessageBox::NoIcon);
        this->setStandardButtons(QMessageBox::Close);
        this->setDefaultButton(QMessageBox::Close);
        break;
      }
  }
}
/****************************************************************************/
