/*****************************************************************************
 * kbv image viewer
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.03.23
 *****************************************************************************/
#ifndef KBVIMAGELABEL_H_
#define KBVIMAGELABEL_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvRubberBand.h"

class KbvImageLabel : public QLabel
{
  Q_OBJECT

public:
  KbvImageLabel(QWidget *parent = 0, Qt::WindowFlags f = 0);
  virtual ~KbvImageLabel();

  void  removeSelectionRect(void);

signals:
  void  selectRectangle(const QRect &rect);
  void  mousePosition(const QPoint &position);

private:
  void  mousePressEvent(QMouseEvent *event);
  void  mouseMoveEvent(QMouseEvent *event);
  void  mouseReleaseEvent(QMouseEvent *event);

  QPoint        clipSelectionRect(QPoint position);

  bool          mDrawRect;
  int           mResizing;
  QPoint        mStartPoint;
  QPoint        mEndPoint;
  KbvRubberBand *mRubberBand;


  enum {NoPull=0, PullLeft, PullRight, PullUp, PullDown};
  int           mMargin = 2;

};

#endif /*KBVIMAGELABEL_H_*/
/****************************************************************************/
