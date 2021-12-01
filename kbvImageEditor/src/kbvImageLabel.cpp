/*****************************************************************************
 * kvb image viewer
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.04.25
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvImageLabel.h"

KbvImageLabel::KbvImageLabel(QWidget *parent, Qt::WindowFlags flags) : QLabel(parent, flags)
{
  this->setCursor(Qt::CrossCursor);
  this->setMouseTracking(true);
  mRubberBand = new KbvRubberBand(QRubberBand::Rectangle, this);

  mDrawRect = false;
  mResizing = NoPull;
}

KbvImageLabel::~KbvImageLabel()
{
  //qDebug() << "KbvImageLabel::~KbvImageLabel"; //###########
}

/*************************************************************************//*!
 * Track mouse position when left button is pressed.
 * On left mouse button press and when no resizing is performed the variable
 * "drawRect" is set, the rubber band is shown with no dimensions and a
 * signal is emitted containing the mouse position.
 * On resizing all work must be done in the mouse move event.
 */
void KbvImageLabel::mousePressEvent(QMouseEvent *event)
  {
    if ((event->buttons() == Qt::LeftButton) && (mResizing == NoPull))
      {
        mStartPoint = event->pos();
        mEndPoint = mStartPoint;
        mDrawRect = true;

        mRubberBand->setGeometry(mStartPoint.x(), mStartPoint.y(), 0, 0);
        mRubberBand->show();
        emit selectRectangle(QRect(mStartPoint.x(), mStartPoint.y(), 0, 0));
        emit mousePosition(event->pos());
        //qDebug() << "KbvImageLabel::mousePressEvent rubberband" <<startPoint <<endPoint; //###########
      }
  }

/*************************************************************************//*!
 * Track mouse movement. The cursor is set to a cross cursor inside the
 * visible label region and to an arrow cursor outside.
 * While left button is pressed the rectangle is drawn between startpoint and
 * cursor position. The rectangle is drawn by Qt from top left to bottom right.
 * When the rectangle starts from a point which is not top left the coordinates
 * must be normalised to achieve this behaviour.
 * When no button is pressed and the mouse position meets the border of the
 * rectangle the signal "resizing" indicates the direction to resize the rect.
 */
void KbvImageLabel::mouseMoveEvent(QMouseEvent *event)
  {
    QRect rect;
    QPoint  sp, ep;
    QSize   size;

    emit mousePosition(event->pos());
    
    //When the left mouse button is pressed and drawRect is true
    //the selection rectangle gets drawn
    if((event->buttons() == Qt::LeftButton) && mDrawRect)
      {
        //Draw selection rectangle, normalise coordinates
        size.setWidth(event->pos().x() - mStartPoint.x());
        size.setHeight(event->pos().y() - mStartPoint.y());
 
        //from left to right and down
        if((size.width()>=0) && (size.height()>=0))
          {
            //qDebug() << "KbvImageLabel::mouseMoveEvent left-right-down"; //###########
            sp = mStartPoint;
            ep = event->pos();
          }
        //from left to right and up
        if((size.width()>=0) && (size.height()<0))
          {
            //qDebug() << "KbvImageLabel::mouseMoveEvent left-right-up"; //###########
            sp.setX(mStartPoint.x());
            sp.setY(event->pos().y());
            ep.setX(event->pos().x());
            ep.setY(mStartPoint.y());
          }
        //from right to left and down
        if((size.width()<0) && (size.height()>=0))
          {
            //qDebug() << "KbvImageLabel::mouseMoveEvent right-left-down"; //###########
            sp.setY(mStartPoint.y());
            sp.setX(event->pos().x());
            ep.setX(mStartPoint.x());
            ep.setY(event->pos().y());
          }
        //from right to left and up
        if((size.width()<0) && (size.height()<0))
          {
            //qDebug() << "KbvImageLabel::mouseMoveEvent right-left-up"; //###########
            sp.setX(event->pos().x());
            sp.setY(event->pos().y());
            ep.setX(mStartPoint.x());
            ep.setY(mStartPoint.y());
          }
 
        mEndPoint = ep;
 
        mRubberBand->setGeometry(QRect(sp, ep));
        rect = mRubberBand->geometry();
        emit selectRectangle(rect);
      }
    
    //When the left mouse button is pressed and drawRect is false the rectangle
    //must be resized due to variable "resizing".
    if((event->buttons() == Qt::LeftButton) && !mDrawRect)
      {
        size.setWidth(mEndPoint.x() - mStartPoint.x());
        size.setHeight(mEndPoint.y() - mStartPoint.y());
          
        switch (mResizing)
          {
          case NoPull:
            {
              break;
            }
          case PullLeft:
            {
              mStartPoint.setX(clipSelectionRect(event->pos()).x());
              size.setWidth(mEndPoint.x() - mStartPoint.x());
              mRubberBand->setGeometry(QRect(mStartPoint, size));
              break;
            }
          case PullUp:
            {
              mStartPoint.setY(clipSelectionRect(event->pos()).y());
              size.setHeight(mEndPoint.y() - mStartPoint.y());
              mRubberBand->setGeometry(QRect(mStartPoint, size));
              break;
            }
          case PullRight:
            {
              mEndPoint.setX(clipSelectionRect(event->pos()).x());
              size.setWidth(mEndPoint.x() - mStartPoint.x());
              mRubberBand->setGeometry(QRect(mStartPoint, size));
              break;
            }
          case PullDown:
            {
              mEndPoint.setY(clipSelectionRect(event->pos()).y());
              size.setHeight(mEndPoint.y() - mStartPoint.y());
              mRubberBand->setGeometry(QRect(mStartPoint, size));
              break;
            }
          }
          rect = mRubberBand->geometry();
          emit selectRectangle(rect);
      }

    //Change cursor to indicate resizing when no mouse butten is pressed,
    //mouse position is inside the margin (2 pixel) around a rectangle edge
    //and set variable "resizing".
    //The mouse position must be between
    // - the upper and lower x coordinate when pulling a vertical edge
    // - the left and right y coordinate when pulling a horizonical edge
    
    if((event->buttons() == Qt::NoButton) && mRubberBand->isVisible())
      {
        sp = mRubberBand->geometry().topLeft();
        ep.setX(sp.x() + mRubberBand->geometry().width());
        ep.setY(sp.y() + mRubberBand->geometry().height());

        //grab left vertical border
        if((event->pos().x() >= sp.x()-mMargin) && (event->pos().x() <= sp.x()+mMargin))
          {
            //qDebug() << "KbvImageLabel::mouseMoveEvent grabbing" <<event->pos(); //###########
            if((event->pos().y() > sp.y()) && (event->pos().y() < ep.y()))
              {
                mResizing = PullLeft;
                this->setCursor(Qt::SplitHCursor);
              }
          }
        //grab right vertical border
        else if((event->pos().x() >= ep.x()-mMargin) && (event->pos().x() <= ep.x()+mMargin))
          {
            //qDebug() << "KbvImageLabel::mouseMoveEvent grabbing" <<event->pos(); //###########
            if((event->pos().y() > sp.y()) && (event->pos().y() < ep.y()))
              {
                mResizing = PullRight;
                this->setCursor(Qt::SplitHCursor);
              }
          }
        //grab top horizontal border
        else if((event->pos().y() >= sp.y()-mMargin) && (event->pos().y() <= sp.y()+mMargin))
          {
            //qDebug() << "KbvImageLabel::mouseMoveEvent grabbing" <<event->pos(); //###########
            if((event->pos().x() > sp.x()) && (event->pos().x() < ep.x()))
              {
                mResizing = PullUp;
                this->setCursor(Qt::SplitVCursor);
              }
          }
        //grab bottom horizontal border
        else if((event->pos().y() >= ep.y()-mMargin) && (event->pos().y() <= ep.y()+mMargin))
          {
            //qDebug() << "KbvImageLabel::mouseMoveEvent grabbing" <<event->pos(); //###########
            if((event->pos().x() > sp.x()) && (event->pos().x() < ep.x()))
              {
                mResizing = PullDown;
                this->setCursor(Qt::SplitVCursor);
              }
          }
        else
          {
            //no grabbing
            //qDebug() << "KbvImageLabel::mouseMoveEvent no grabbing" <<event->pos(); //###########
            mResizing = NoPull;
            this->setCursor(Qt::CrossCursor);
          }
      }
    
    //No selection: show mouse position in statusbar
    if((event->buttons() == Qt::NoButton) && !mRubberBand->isVisible())
      {
        emit selectRectangle(QRect(event->pos(), QSize(0,0)));
        //qDebug() << "KbvImageLabel::mouseMoveEvent no selection" <<event->pos(); //###########
      }
  }

/*************************************************************************//*!
 * Selection or resizing finished. When no rectangle was drawn it is set
 * invisible (no selection) therefore mouse position can be shown.
 * The helper variables for selection and resizing are reset.
 */
void KbvImageLabel::mouseReleaseEvent(QMouseEvent *event)
  {
    Q_UNUSED (event);
    QRect rect;
    
    mDrawRect = false;
    mResizing = NoPull;
    rect = mRubberBand->geometry();
    mStartPoint = mRubberBand->geometry().topLeft();
    mEndPoint.setX(rect.x() + rect.width());
    mEndPoint.setY(rect.y() + rect.height());
    this->setCursor(Qt::CrossCursor);

    //qDebug() << "KbvImageLabel::mouseReleaseEvent rubberband" <<startPoint <<endPoint; //###########
    if(mStartPoint == mEndPoint)
      {
        mRubberBand->setVisible(false);
      }
  }

/*************************************************************************//*!
 * Returns a point which is the mouse position inside the visible label region.
 * When mouse position exeeds the visible region one or both coordinates of
 * the point are limited to the border of visible region and cursor is set to an
 * arrow cursor.
 */
QPoint  KbvImageLabel::clipSelectionRect(QPoint position)
  {
    QPoint      point;
    QRegion     region;
    QRect       edges;

    region = this->visibleRegion();
    edges = region.boundingRect();
    point = position;
    
    if (!region.contains(position))
      {
        point = position;
        if(point.x() < edges.x())
          {
            point.setX(edges.x());
          }
        if(point.y() < edges.y())
          {
            point.setY(edges.y());
          }
        if(point.x() > (edges.x()+edges.width()))
          {
            point.setX(edges.x()+edges.width());
          }
        if(point.y() > (edges.y()+edges.height()))
          {
            point.setY(edges.y()+edges.height());
          }
      }
    return point;
  }

/*************************************************************************//*!
 * Clear startpoint, endpoint and all flags, send an empty selection for
 * clearing of statuslabels and set rubberband invisible.
 */
void  KbvImageLabel::removeSelectionRect()
  {
    mDrawRect = false;
    mResizing = NoPull;
    mStartPoint = QPoint(0, 0);
    mEndPoint = QPoint(0, 0);
    mRubberBand->setVisible(false);
    this->setCursor(Qt::CrossCursor);
    emit (selectRectangle(QRect(mStartPoint, mEndPoint)));

  }

/****************************************************************************/
