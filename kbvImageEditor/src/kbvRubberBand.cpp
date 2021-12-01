/*****************************************************************************
 * kvb rubberband
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.05.30
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvRubberBand.h"

KbvRubberBand::KbvRubberBand(Shape s, QWidget *p) : QRubberBand(s, p)
{
  QPalette      palette;
  QBrush        brush;

  brush.setColor(Qt::gray);
  
  palette = this->palette();
  palette.setBrush(QPalette::Highlight, brush);
  this->setPalette(palette);
}

KbvRubberBand::~KbvRubberBand()
{
  //qDebug() << "KbvRubberBand::~KbvRubberBand"; //###########
}
/****************************************************************************/
