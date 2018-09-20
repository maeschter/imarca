/*****************************************************************************
 * kvb slide view
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.11.29
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
  *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvSlideView.h"
#include "kbvSlideScene.h"

extern  KbvSetvalues    *settings;


KbvSlideView::KbvSlideView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent)
{
  this->setVisible(false);
  QIcon icon;
  icon.addPixmap(QPixmap(QString::fromUtf8(":/kbvmain/res/kbv_crown_20x32.png")), QIcon::Normal, QIcon::Off);
  this->setWindowIcon(icon);
  this->setWindowTitle("Imarca");
  this->setAttribute(Qt::WA_DeleteOnClose, true);

  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  int   prim = desktop.primaryScreen();
  screenSize = desktop.screenGeometry(prim);
  QRectF rectf(screenSize);         //(0,0-w,h)
  //qDebug() << "KbvSlideView::KbvSlideView" <<screenSize; //###########


  //Rectangle for full screen mode due to slide change mode
  //Replace/fade-in: visible rectangle = scene rectangle = desktop size
  //Insert left/right:  scene rectangle = (3*desktop_w, desktop_h)
  // visible rectangle = desktop size in the middle of the scene rectangle
  if (settings->slideFullScreen)
    {
      this->setWindowState(Qt::WindowFullScreen);
      this->setBackgroundBrush(QBrush(settings->slideBackColour, Qt::SolidPattern));
      this->setFrameShape(QFrame::NoFrame);
      this->setVisible(true);
      switch(settings->slideChangeId)
      {
        case KbvConf::slideReplace:
          {
            this->setSceneRect(rectf);
          }
          break;
        case KbvConf::slideFadeIn:
          {
            this->setSceneRect(rectf);
          }
          break;
        case KbvConf::slideInsertLeft:
          {
            rectf.setX(qreal (screenSize.width()));
            rectf.setWidth(qreal (screenSize.width()));
            this->setSceneRect(rectf);        //w,0-2w,h)
          }
          break;
        case KbvConf::slideInsertRight:
          {
            rectf.setX(qreal (screenSize.width()));
            rectf.setWidth(qreal (screenSize.width()));
            this->setSceneRect(rectf);        //w,0-2w,h)
          }
          break;
        default:
          {
            this->setSceneRect(rectf);
          }
          break;
      }
    }

  thiscursor = this->cursor();
  this->setCursor(Qt::BlankCursor);

  connect(scene, SIGNAL(finish()), this, SLOT(finish()));
  connect(scene, SIGNAL(sceneRectChanged(const QRectF)), this, SLOT(updateSceneRect(const QRectF)));
  connect(scene, SIGNAL(itemPathName(const QString)), this, SLOT(showTitle(const QString)));
}

KbvSlideView::~KbvSlideView()
{
  //qDebug() << "KbvSlideView::~KbvSlideView"; //###########
  this->setCursor(thiscursor);
  delete    this->scene();
}

/*************************************************************************//*!
 * Key press event\n
 * Quit application on ESC-key. All other key events are passed to base class
 * which propagates them to the scene.
 */

void    KbvSlideView::keyPressEvent(QKeyEvent *keyEvent)
{
  if (keyEvent->key() == Qt::Key_Escape)
    {
      //qDebug() << "KbvSlideView::keyPressEvent close"; //###########
      this->close();
    }
  else
    {
      QGraphicsView::keyPressEvent(keyEvent);
    }
}

/*************************************************************************//*!
 * SLOT: From scene: no graphics files selected, finish slide show.
 */
void    KbvSlideView::finish(void)
{
  this->close();
}

/*************************************************************************//*!
 * SLOT: normal window size\n
 * The scene notifies the view that the scene's scene rectangle has changed.
 * At start the view gets shown when the first image is properly sized.
 */
void    KbvSlideView::updateSceneRect(const QRectF &rect)
{
  //qDebug() << "KbvSlideView::updateSceneRect" << rect; //###########
  this->resize(int (rect.width()),int (rect.height()));
  this->setVisible(true);
}

/*************************************************************************//*!
 * SLOT: set window title
 */
void    KbvSlideView::showTitle(const QString &title)
{
  this->setWindowTitle(title);
}
/****************************************************************************/
