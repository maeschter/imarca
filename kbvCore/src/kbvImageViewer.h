/*****************************************************************************
 * kbv image viewer
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2017.05.01
 *****************************************************************************/
#ifndef KBVIMAGEVIEWER_H_
#define KBVIMAGEVIEWER_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <kbvExiv.h>            //libKbvExiv.so

class KbvImageViewer : public QLabel
{
  Q_OBJECT

public:
  KbvImageViewer(QWidget *parent = nullptr, Qt::WindowFlags f = 0);
  virtual ~KbvImageViewer();

  void  showImages(const QList<QPair<QString, QString> > filelist);
  
signals:
  void    deleteFiles(QMap<QString, QString>);
  
private slots:
  void    mousePressEvent(QMouseEvent *event);
  void    mouseMoveEvent(QMouseEvent *event);
  void    keyPressEvent(QKeyEvent *event);
  void    closeEvent(QCloseEvent *event);
  void    permitChangeover();

private:  
  void    imageChange(int previousnext);
  void    adjustAndShow(QPair<QString, QString>);
  void    showMetaData(bool show);

  QList<QPair<QString, QString> > *playlist;
  QMap<QString, QString>  deletelist;
  QDesktopWidget  desktop;
  QRect           desktopSize, screenSize, geo_full, geo_normal;
  QPixmap         image;
  QTimer          *timer1;
  QPainter        *painter;
  int             current;
  bool            showText, permitChange;
};

#endif /*KBVIMAGEVIEWER_H_*/
/****************************************************************************/
