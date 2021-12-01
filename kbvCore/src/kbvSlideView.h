/*****************************************************************************
 * kvb slide view
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.11.29
 *****************************************************************************/
#ifndef KBVSLIDEVIEW_H_
#define KBVSLIDEVIEW_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>


class KbvSlideView : public QGraphicsView
{
  Q_OBJECT

public:
	KbvSlideView(QGraphicsScene *scene, QWidget *parent=0);
	virtual ~KbvSlideView();

signals:


public slots:
void    updateSceneRect(const QRectF &rect);

private slots:
void    finish(void);
void    showTitle(const QString &title);

private:
void    keyPressEvent(QKeyEvent *keyEvent);

QDesktopWidget  desktop;
QRect           screenSize;
QCursor         thiscursor;

};
#endif /*KBVSLIDEVIEW_H_*/
/****************************************************************************/
