/*****************************************************************************
 * kbvTabBar
 * This is the tab bar for kbv collection tabs
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2011.10.12
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvTabBar.h"

KbvTabBar::KbvTabBar(QWidget *parent) : QTabBar(parent)
{
  this->setShape(QTabBar::RoundedNorth);
}

KbvTabBar::~KbvTabBar()
{
  //qDebug() << "KbvTabBar::~KbvTabBr"; //###########
}
/*************************************************************************//*!
 * Catch double click and close tab as this is neither the file tab nor the
 * search tab.
 */
void    KbvTabBar::mouseDoubleClickEvent(QMouseEvent *event)
{
  int       index;

  index = this->tabAt(event->pos());
  //qDebug() << "KbvTabBar::mouseDoubleClickEvent on" << index; //###########
  if (event->button() == Qt::LeftButton)
    {
      if(!(index == KbvConf::fileViewTabIndex || index == KbvConf::searchViewTabIndex))
        {
          //closing the tab is done via signal in collectionTabs where the
          //the tab widget is deleted and database connection closed.
          emit  closeTab(index);
        }
    }
}

/*************************************************************************//*!
 * Catch key press event to send a signal (see mouse press event).
 * Delegate event to base class to perform further key events of base class.
 */

void    KbvTabBar::keyPressEvent(QKeyEvent *event)
{
  int k;

  k = event->key();
  if((k == Qt::Key_Left) || (k == Qt::Key_Right))
    {
      //qDebug() << "KbvTabBar::keyPressEvent <-- or -->"; //###########
      emit keyOrMouseSelTab(this->currentIndex());
    }
  QTabBar::keyPressEvent(event);
}
/****************************************************************************/
