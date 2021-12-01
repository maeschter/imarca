/*****************************************************************************
 * kvb slide scene
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.11.29
 *****************************************************************************/
#ifndef KBVSLIDESCENE_H_
#define KBVSLIDESCENE_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>


class KbvSlideScene : public QGraphicsScene
{
  Q_OBJECT

public:
	KbvSlideScene(QWidget *parent = 0);
	virtual ~KbvSlideScene();

void    startShow(QList<QPair<QString, QString> > list);
void    slidePreviousNext(int previousnextcurrent);

signals:
void    finish(void);
void    itemPathName(QString ipn);
void    deleteFiles(QMap<QString, QString>);

public slots:


private slots:
void        slideChangeOver(void);
void        slideAutomatic(void);

private:
void        mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
void        keyPressEvent(QKeyEvent *keyEvent);
QPixmap     adjustSize(QPixmap pixmap);
void        showMetaData(bool show);

QDesktopWidget  desktop;
QRect           desktopSize, screenSize;

QGraphicsPixmapItem *currentSlide, *oldSlide;
QGraphicsTextItem   *textInfo;

QList<QPair<QString, QString> > *playlist;
QMap<QString, QString>		deletelist;

QTimer      *timer1, *timer2;
int         currentIndex, direction;
bool        changeBusy, metaData;
qreal       opaque_step;
qreal       x_step;
int         x_step_count;
};
#endif /*KBVSLIDESCENE_H_*/
/****************************************************************************/
