/*****************************************************************************
 * kvb main status bar
 * The status bar comprises three status labels for messages and a progress bar.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2012.09.27
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvMainStatus.h"

KbvMainStatus::KbvMainStatus(QWidget *parent) : QStatusBar(parent)
{
  kbvStatusLbl1 = new QLabel("", this, 0);
  kbvStatusLbl1->setMinimumSize(70, 20);
  kbvStatusLbl1->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

  kbvStatusLbl2 = new QLabel("", this, 0);
  kbvStatusLbl2->setMinimumSize(70, 20);
  kbvStatusLbl2->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

  kbvStatusLbl3 = new QLabel("", this, 0);
  kbvStatusLbl3->setMinimumSize(200, 20);
  kbvStatusLbl3->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

  kbvStatusProgress = new QProgressBar(this);
  kbvStatusProgress->setMinimumSize(150, 20);
  kbvStatusProgress->setRange(0, 1000);
  kbvStatusProgress->reset();
  kbvStatusProgress->setTextVisible(false);

  this->insertWidget(0, kbvStatusLbl1, 0);
  this->insertWidget(1, kbvStatusLbl2, 0);
  this->insertWidget(2, kbvStatusLbl3, 1);
  this->insertWidget(3, kbvStatusProgress, 0);
}

KbvMainStatus::~KbvMainStatus()
{
  delete  kbvStatusLbl1;
  delete  kbvStatusLbl2;
  delete  kbvStatusLbl3;
  delete  kbvStatusProgress;
}
/*************************************************************************//*!
 *SLOT: set value of progressbar
 */
void    KbvMainStatus::setProgressValue(int promille)
{
  //qDebug() << "KbvMain::setProgressValue" << promille; //###########
  kbvStatusProgress->setValue(promille);
}
/*************************************************************************//*!
 *SLOT: set the text of status field 1
 */
void    KbvMainStatus::setStatusText1(QString text)
{
  kbvStatusLbl1->setText(text);
}
/*************************************************************************//*!
 *SLOT: set text of status field 2
 */
void    KbvMainStatus::setStatusText2(QString text)
{
  kbvStatusLbl2->setText(text);
}
/*************************************************************************//*!
 *SLOT: set text of status field 3
 */
void    KbvMainStatus::setStatusText3(QString text)
{
  kbvStatusLbl3->setText(text);
}
/****************************************************************************/

